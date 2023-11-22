#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outBackPos;

void main(){
    gl_Position=ubo.proj * ubo.view * ubo.model * vec4(inPosition,1.);
    outColor = inColor;
    // outBackPos = vec3(ubo.model * vec4(inPosition,1.));
    outBackPos = inColor; // 将背面的颜色传递进去进行测试
}