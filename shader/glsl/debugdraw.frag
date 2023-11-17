// https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Shader_modules#page_Compiling-the-shaders

#version 450

layout(binding=1)uniform sampler3D tex3DSampler;

layout(location=0)in vec3 fragColor;
layout(location=1)in vec2 fragTexCoord;

layout(location=0)out vec4 outColor;

void main(){
    // outColor = vec4(fragColor, 1.0); // 将顶点颜色作为颜色进行插值输出
    // outColor = vec4(fragTexCoord,0.0, 1.0); // 将纹理坐标作为颜色进行插值输出
    // outColor = texture(texSampler, fragTexCoord); // 从纹理中获取颜色
    // outColor=texture(texSampler,fragTexCoord*2.);// 从纹理中获取颜色,并放大两倍
    // outColor=vec4(fragColor*texture(texSampler,fragTexCoord).rgb,1.); // 和顶点颜色进行混合
    outColor=texture(tex3DSampler,vec3(fragTexCoord.x,fragTexCoord.y,27.0/41.0));
    
}