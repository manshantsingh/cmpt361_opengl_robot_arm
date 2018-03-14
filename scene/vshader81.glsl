#version 150 

in  vec4 vPosition;
in  vec4 vNormal;

out vec3 normalInterp;
out vec3 vertPos;

out vec3 ambientColor;
out vec3 diffuseColor;
out vec3 specColor;

// uniform vec4 ChosenColor;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat4 NormalMatrix;

uniform vec3 AmbientColor;
uniform vec3 DiffuseColor;
uniform vec3 SpecColor;

void main() 
{
    gl_Position = Projection * ModelView * vPosition;

    vec4 vertPos4 = ModelView * vPosition;
    vertPos = vec3(vertPos4) / vertPos4.w;
    normalInterp = vec3(NormalMatrix * vNormal);

    ambientColor = AmbientColor;
    diffuseColor = DiffuseColor;
    specColor = SpecColor;
} 
