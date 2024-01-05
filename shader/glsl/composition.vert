#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location=0)out vec3 outColor;
layout(location=1)out vec2 outTexCoord;
layout(location=2)out vec3 outFrontPos;
layout(location=3)out vec3 fragCameraUp;
layout(location=4)out vec3 fragCameraRight;

void main(){
    gl_Position=ubo.proj * ubo.view * ubo.model * vec4(inPosition,1.);
    outColor=inColor;
    outTexCoord=inTexCoord;
    outFrontPos=vec3(ubo.model * vec4(inPosition,1.));
    // 计算相机的上方向和右方向
    fragCameraUp = vec3(ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]);
    fragCameraRight = vec3(ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]);
}