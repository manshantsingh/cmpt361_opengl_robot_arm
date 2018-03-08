#version 150 

in  vec4 vPosition;
// in  vec4 vColor;
out vec4 color;

uniform vec4 ChosenColor;
uniform mat4 ModelView;
uniform mat4 Projection;

void main() 
{
    color = ChosenColor;
    gl_Position = Projection * ModelView * vPosition;
} 
