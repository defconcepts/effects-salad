_templates.resize(4);
params.resize(_templates.size());
memset(&params[0], 0, sizeof(params[0]) * params.size());
int i = 0;
params[i].Thickness = 3;
params[i].TopRadius =  8.0f / 13.0f;
params[i].TetSize = 0.1f;
params[i].NumSides = 5;
params[i].CanExplode = _explode;
params[i].Dest = &_templates[i];
params[i].Windows.Columns = 2;
params[i].Windows.Rows = 10;
params[i].Windows.Size = glm::vec2(0.5);
params[i].InsetDepth = 0.1;
++i;
params[i].Thickness = 3.0f;
params[i].TopRadius =  1.0f;
params[i].TetSize = 0.1f;
params[i].NumSides = 4;
params[i].CanExplode = _explode;
params[i].Windows.Columns = 3;
params[i].Windows.Rows = 10;
params[i].Windows.Size = glm::vec2(0.5);
params[i].Dest = &_templates[i];
params[i].InsetDepth = 0.1;
++i;
params[i].Thickness = 3.0f;
params[i].TetSize = 0.1f;
params[i].TopRadius =  1.2f;
params[i].NumSides = 3;
params[i].CanExplode = _explode;
params[i].Windows.Columns = 3;
params[i].Windows.Rows = 10;
params[i].Windows.Size = glm::vec2(0.5);
params[i].Dest = &_templates[i];
params[i].InsetDepth = 0.1;
++i;
params[i].Thickness = 3.0f;
params[i].TetSize = 0.1f;
params[i].TopRadius = 1;
params[i].NumSides = _explode ? 32 : 64;
params[i].Windows.Columns = 0;
params[i].Windows.Rows = 0;
params[i].CanExplode = _explode;
params[i].Dest = &_templates[i];
