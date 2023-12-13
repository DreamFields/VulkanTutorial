
#version 450

// 设置精度
precision highp float;
precision highp int;
precision highp sampler3D;

layout(binding=1)uniform sampler3D tex3DSampler;
// input_attachment_index对应于subpasses[1].pInputAttachments的索引
// 也就是说，这里的input_attachment_index=0，对应于subpasses[1].pInputAttachments[0]
// 这里的binding=2，对应于 compositionDescriptorWrite[2].dstBinding = 2
layout(input_attachment_index=0,binding=2)uniform subpassInput inputBackpos;
layout(binding=3)uniform DicomUniformBufferObject{
    float windowCenter;
    float windowWidth;
    float minVal;
    float tau;
    int steps;
    float stepLength;
    float glow;
}dicomUbo;

layout(location=0)in vec3 inColor;
layout(location=1)in vec2 inTexCoord;
layout(location=2)in vec3 inFrontPos;

layout(location=0)out vec4 outColor;

vec4 get3DTextureColor(vec3 pos){
    vec4 sampleColor=texture(tex3DSampler,pos);
    float intensity=sampleColor.r*255.+sampleColor.g*255.*255.-abs(dicomUbo.minVal);
    intensity=(intensity-dicomUbo.windowCenter)/dicomUbo.windowWidth+.5;
    intensity=clamp(intensity,0.,1.);
    return vec4(intensity,0.,0.,1.);
}

void main(){
    // 通过subpassLoad函数，从subpassInput中加载数据
    vec3 backPos=subpassLoad(inputBackpos).xyz;
    // Ray dir
    vec3 dir=normalize(backPos-inFrontPos);
    // Ray length
    float rayLength=length(backPos-inFrontPos);
    // Current position
    vec3 currentPos=inFrontPos;
    // Initialize Transparency and Radiance sampleColor
    vec4 dst=vec4(0.);
    // T
    float T=1.;
    // accmulate alpha
    float accmulateAlpha=0.;
    // control render equation
    bool isUseTransparency=false;

    float stepLength = dicomUbo.stepLength == 0. ? rayLength / float(dicomUbo.steps) : dicomUbo.stepLength;
    
    // Evaluate form 0 to D
    for(float s=0.;s<rayLength;){
        // Get the current step or the remaining interval
        float h=min(stepLength,rayLength-s);
        // Get the current position
        vec3 pos=currentPos+dir*(s+h*.5);
        // Get the sampleColor from the 3D texture
        vec4 sampleColor=get3DTextureColor(pos);

        // 如果使用透明度渲染
        if(isUseTransparency){
            // Accumulate the sampleColor
            dst=dst+T*sampleColor*(1.-exp(-dicomUbo.tau*h))*dicomUbo.glow;
            // Accumulate the transparency  // !即 T = T * exp(-tau * deltaS)
            T=T*exp(-dicomUbo.tau*h);
            
            if((1.-T)>.99)break;
        }else{
            if(sampleColor.r!=0.){
                dst=dst+sampleColor*(1.-accmulateAlpha)*dicomUbo.glow;
                accmulateAlpha=accmulateAlpha+dicomUbo.tau*(1.-accmulateAlpha);
            }
            if(accmulateAlpha>.99)break;
        }
        
        // Go to the next interval
        s=s+h;
    }
    outColor=dst;
}