// https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Shader_modules#page_Compiling-the-shaders

#version 450
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location=0)out vec3 fragColor;

void main(){
    gl_Position=vec4(inPosition,0.,1.);
    fragColor=inColor;
}