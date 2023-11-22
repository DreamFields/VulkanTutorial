
#version 450

layout(binding=1)uniform sampler3D tex3DSampler;
// input_attachment_index对应于subpasses[1].pInputAttachments的索引
// 也就是说，这里的input_attachment_index=0，对应于subpasses[1].pInputAttachments[0]
// 这里的binding=2，对应于 compositionDescriptorWrite[2].dstBinding = 2
layout(input_attachment_index=0,binding=2) uniform subpassInput inputBackpos;

layout(location=0)in vec3 inColor;
layout(location=1)in vec2 inTexCoord;

layout(location=0)out vec4 outColor;

void main(){
    // outColor = vec4(inColor,1.0);
    // outColor=texture(tex3DSampler,vec3(inTexCoord.x,inTexCoord.y,27.0/41.0));
    // 通过subpassLoad函数，从subpassInput中加载数据
    vec3 backPos = subpassLoad(inputBackpos).xyz;
    outColor = vec4(backPos,1.0);
}