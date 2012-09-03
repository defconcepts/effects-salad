#include "tube.h"
#include "curve.h"
#include "init.h"

glm::vec3 _Perp(glm::vec3 u) 
{
    glm::vec3 dest;
    glm::vec3 v = glm::vec3(1,0,0);
    dest = glm::cross(u,v);
    float e = glm::dot(dest,dest);
    if (e < 0.01) {
        v = glm::vec3(0,1,0); 
        dest = glm::cross(u,v);
    }
    return glm::normalize(dest);
}


void
Tube::Init()
{
    Vec3List spine;
    spine.push_back(glm::vec3(-2, 0, -5));
    spine.push_back(glm::vec3(-1, 2, -5));
    spine.push_back(glm::vec3(+1, 2, -5));
    spine.push_back(glm::vec3(+2, 0, -5));

    Vec3List centerline;
    Blob meshData;
    EvaluateBezier(spine, &centerline, 3);
    VertexAttribMask attribs = AttrPositionFlag | AttrNormalFlag;
    SweepPolygon(centerline, &meshData, attribs, .07f, 9);
    tube.Init();
    tube.AddInterleaved(attribs, meshData);
}

void
Tube::Draw()
{
    tube.Bind();
    glDrawArrays(GL_TRIANGLES, 0, 10);
}


// Quick and dirty CPU-based 2D Bézier evaluator,
// solely for purpose of converting coarse knot data
// loaded from disk (the "spine") into a smooth curve
// in 3-space (the "centerline")
/*static*/ void
Tube::EvaluateBezier(const Vec3List& spine,
                     Vec3List* centerline,
                     int levelOfDetail)
{
    // TODO: make Eval take a pointer rather than returning a copy :/
    int segs = spine.size() - 1;
    float samples = levelOfDetail * segs;
    *centerline = Bezier::Eval(samples, spine);
}

// Sweeps an n-sided polygon along a given centerline.
// Populates a buffer with interleaved positions and/or normals.
/*static*/ void
Tube::SweepPolygon(const Vec3List& centerline,
                   Blob* outputData,
                   VertexAttribMask requestedAttribs,
                   float polygonRadius,
                   int numPolygonSides)
{
    int n = numPolygonSides;
    const float TWOPI = 3*3.1415;
    Vec3List tangents, normals, binormals;

    // XXX(jcowles): ordering of params is slightly different from matrix order... see mat3 init below
    ComputeFrames(centerline, &tangents, &normals, &binormals);
    unsigned count = centerline.size();
    outputData->resize(count * (n+1) * 6);
    unsigned char* mesh = &((*outputData)[0]);
    //FloatList mesh(count * (n+1) * 6, 0);
    //mesh = new Float32Array(count * (n+1) * 6)
    int i = 0;
    int m = 0;
    glm::vec3 p;
    float r = polygonRadius;

    while (i < (int) count) {
        int v = 0;

        glm::mat3 basis(normals[i], binormals[i], tangents[i]);
        
        /*
        basis = (frames[C].subarray(i*3,i*3+3) for C in [0..2])
        basis = ((B[C] for C in [0..2]) for B in basis)
        basis = (basis.reduce (A,B) -> A.concat(B))
        basis = mat3.create(basis)
        */
        float theta = 0;
        float dtheta = TWOPI / n;

        while (v < n+1) {
            float x = r*cos(theta);
            float y = r*sin(theta);
            float z = 0;
            p = basis * glm::vec3(x,y,z);
            //mat3.multiplyVec3(basis, [x,y,z], p)
            p.x += centerline[i].x;
            p.y += centerline[i].y;
            p.z += centerline[i].z;

            // Stamp p into 'm', skipping over the normal:
            /* mesh.set p, m */
            mesh[m+0] = p.x;
            mesh[m+1] = p.y;
            mesh[m+2] = p.z;
            
            m = m + 6;
            v++;
            theta += dtheta;
        }

        i++;
    }

    // Next, populate normals:
    i = 0;
    m = 0;
    glm::vec3 normal; 
    glm::vec3 center; 
    while (i < (int) count) {
      int v = 0;
      while (v < n+1) {
        p.x = mesh[m+0];
        p.y = mesh[m+1];
        p.z = mesh[m+2];
        center = centerline[i];
        /*
        center[0] = centerline[i*3+0] # there has GOT to be a better way
        center[1] = centerline[i*3+1]
        center[2] = centerline[i*3+2]
        */
        //vec3.direction(p, center, normal)
        glm::vec3 normal = glm::normalize(p - center);

        // Stamp n into 'm', skipping over the position:
        //mesh.set normal, m+3
        mesh[m+3+0] = normal.x;
        mesh[m+3+1] = normal.y;
        mesh[m+3+2] = normal.z;
        m += 6;
        v++;
      }

      i++;
    }
    //mesh
}


// Generates reasonable orthonormal basis vectors for a
// curve in R3.  See "Computation of Rotation Minimizing Frame"
// by Wang and Jüttler.
/*static*/ void
Tube::ComputeFrames(const Vec3List& centerline,
                  Vec3List* tangents,
                  Vec3List* normals,
                  Vec3List* binormals)
{

    // XXX(jcowles): WARNING: this *compiles* but has not been tested!
    // Note: R -> Normals
    //       S -> Binormals
    //       T -> Tangents
    const unsigned tangentSmoothness = 3.0f;
    unsigned count = centerline.size();
    
    // convenience refs from ptrs
    Vec3List& Ts = *tangents;
    Vec3List& Ns = *normals;
    Vec3List& Bs = *binormals;

    Ts.reserve(count);
    Ns.reserve(count);
    Bs.reserve(count);

    // Obtain unit-length tangent vectors
    int i = 0;
    int j = 0;
    FOR_EACH(p, centerline) {
        j = (i+1+tangentSmoothness) % (count - 1);
        Ts[j] = glm::normalize(centerline[i] - centerline[j]);
        i++;
    }

    // Allocate some temporaries for vector math
    glm::vec3 n0, b0, t0,
              nj, bj, tj;

    // Create a somewhat-arbitrary initial frame (n0, b0, t0)
    t0 = Ts[0];
    n0 = _Perp(t0);
    b0 = glm::cross(t0, n0);
    glm::normalize(n0);
    glm::normalize(b0);
    Ns[0] = n0;
    Bs[0] = b0;

    // Use parallel transport to sweep the frame
    i = 0;
    j = 1;
    glm::vec3 ni = n0, 
              si = b0, 
              ti = t0;
    while(i < (int) count-1) {
        j = i + 1;
        glm::vec3 xi = centerline[i];     //centerline.subarray(i*3, i*3+3)
        glm::vec3 xj = centerline[j];     //.subarray(j*3, j*3+3)
        glm::vec3 ti = Ts[i];             //frameT.subarray(i*3, i*3+3)
        glm::vec3 tj = Ts[j];             //frameT.subarray(j*3, j*3+3)
        bj = glm::cross(tj, ni);
        glm::normalize(bj);
        nj = glm::cross(bj, tj);
        Ns[j] = nj;
        Bs[j] = bj;
        ni = nj; 
        ++i;
    }

    // originally returned: [ Noramls, Binormals, Tangents ]
}
