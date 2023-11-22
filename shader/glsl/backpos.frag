#version 450

layout(location=0)in vec3 inColor;
layout(location=1)in vec3 inBackPos;

layout(location=0)out vec4 outColor;
layout(location=1)out vec4 outBackPos;
void main(){
    
    outColor= vec4(inColor,1.);
    outBackPos= vec4(inBackPos.xyz,1.);
}