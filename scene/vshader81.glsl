#version 150 

in  vec4 vPosition;
// in  vec4 vColor;
in  vec4 vNormal;

out vec4 color;
out vec3 normalInterp;
out vec3 vertPos;

// uniform vec4 ChosenColor;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat4 NormalMatrix;
uniform vec4 TotalColor;

void main() 
{
    color = TotalColor;
    gl_Position = Projection * ModelView * vPosition;

    vec4 vertPos4 = ModelView * vPosition;
    vertPos = vec3(vertPos4) / vertPos4.w;
    normalInterp = vec3(NormalMatrix * vNormal);
} 
