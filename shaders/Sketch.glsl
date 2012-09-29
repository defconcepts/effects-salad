-- Facets.VS

layout(location = 0) in vec4 Position;

uniform mat4 Projection;
uniform mat4 Modelview;

uniform vec3 Translate;
uniform vec3 Scale = vec3(1);

out vec3 vPosition;

void main()
{
    vPosition = Position.xyz * Scale + Translate;
    gl_Position = Projection * Modelview * vec4(vPosition, 1);
}

-- Facets.GS

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat3 NormalMatrix;
uniform mat4 Projection;
uniform mat4 Modelview;

in vec3 vPosition[3];

out vec4 gColor;
out vec3 gFacetNormal;
out vec3 gPosition;

void main()
{
    gColor = vec4(1, 1, 1, 1);

    vec3 A = vPosition[2].xyz - vPosition[0].xyz;
    vec3 B = vPosition[1].xyz - vPosition[0].xyz;
    gFacetNormal = NormalMatrix * normalize(cross(A, B));

    gPosition = vPosition[0];
    gl_Position = Projection * Modelview * vec4(vPosition[0], 1);
    EmitVertex();

    gPosition = vPosition[1];
    gl_Position = Projection * Modelview * vec4(vPosition[1], 1);
    EmitVertex();

    gPosition = vPosition[2];
    gl_Position = Projection * Modelview * vec4(vPosition[2], 1);
    EmitVertex();
    EndPrimitive();
}

-- Facets.FS

in vec3 gPosition;
in vec4 gColor;
in vec3 gFacetNormal;
out vec4 FragColor;

uniform vec3 LightPosition = vec3(0, 0, 1);
uniform vec3 AmbientMaterial = vec3(0.1, 0.1, 0.1);

float randhash(uint seed, float b)
{
    const float InverseMaxInt = 1.0 / 4294967295.0;
    uint i=(seed^12345391u)*2654435769u;
    i^=(i<<6u)^(i>>26u);
    i*=2654435769u;
    i+=(i<<5u)^(i>>12u);
    return float(b * i) * InverseMaxInt;
}

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    if (gColor.a == 0) {
        discard;
    }

    // Screen door transparency
    if (rand(gPosition.xy) < 0.5) {
        discard;
    }

    vec3 N = normalize(gFacetNormal);
    vec3 L = LightPosition;
    float df = abs(dot(N, L));
    vec3 color = AmbientMaterial + df * gColor.rgb;

    // Craptastic AO:
    float d = clamp((gPosition.y+5)/10, 0, 1);
    color *= d;

    FragColor = vec4(color, gColor.a);
}
