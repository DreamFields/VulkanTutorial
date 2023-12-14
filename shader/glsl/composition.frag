
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
layout(binding=4)uniform sampler1D lutTexSampler;

layout(location=0)in vec3 inColor;
layout(location=1)in vec2 inTexCoord;
layout(location=2)in vec3 inFrontPos;

layout(location=0)out vec4 outColor;

vec4 get3DTextureColor(vec3 pos){
    vec4 sampleColor=texture(tex3DSampler,pos);
    float intensity=sampleColor.r*255.+sampleColor.g*255.*255.-abs(dicomUbo.minVal);
    intensity=(intensity-dicomUbo.windowCenter)/dicomUbo.windowWidth+.5;
    intensity=clamp(intensity,0.,1.);
    if(intensity==0.)return vec4(0.);
    // 通过采样器，从lutTexSampler中加载数据，相当于传递函数的实现
    vec3 color=texture(lutTexSampler,intensity).rgb;
    return vec4(color,.5);
}

vec4 rayplusMethod(float rayLength,vec3 dir,vec3 currentPos){
    // 设置一个颜色的累积器
    vec4 accumulatedColor=vec4(0.);
    
    // 设置一个 Alpha 的累积器
    float accumulatedAlpha=0.;
    
    // 射线传播了多长的距离
    float accumulatedLength=0.;
    
    vec3 currentPosition=currentPos;
    
    float alphaSample=0.;
    
    float stepLength=rayLength/float(dicomUbo.steps);
    
    for(int i=0;i<dicomUbo.steps;i++)
    {
        vec4 sampleColor=get3DTextureColor(currentPosition);
        if(sampleColor.r!=0.){
            // 按步长缩放alpha使最终的颜色不变。
            alphaSample=25.6*stepLength*(1.-accumulatedAlpha)*dicomUbo.tau;
            
            // 执行合成
            accumulatedColor+=sampleColor*alphaSample;
            
            // 存储到目前为止积累的alpha。
            accumulatedAlpha+=alphaSample;
        }
        // 推进射线
        currentPosition+=dir*stepLength;
        accumulatedLength+=stepLength;
        // 如果遍历的长度大于射线长度，或者累计的alpha达到1.0，那么退出。
        if(accumulatedLength>=rayLength||accumulatedAlpha>=1.)
        break;
    }
    
    return accumulatedColor;
}

vec4 absorptionMethod(float stepLength,float rayLength,vec3 dir,vec3 currentPos){
    // Initialize Transparency and Radiance sampleColor
    vec3 E=vec3(0.);
    // T
    float T=1.;
    bool isAccurate=true;
    // Evaluate form 0 to D
    for(float s=0.;s<rayLength;){
        // Get the current step or the remaining interval
        float h=min(stepLength,rayLength-s);
        // Get the current position
        vec3 pos=currentPos+dir*(s+h*.5);
        // Get the sampleColor from the 3D texture
        vec4 sampleColor=get3DTextureColor(pos);
        
        if(!isAccurate){
            // ---------Iteration A: tau*h 很小时结果较好---------------
            // Accumulate the sampleColor
            E=E+T*sampleColor.rgb*dicomUbo.tau*dicomUbo.glow;
            // Accumulate the transparency  // !即 T = T * exp(-tau * deltaS)
            T+=T*exp(-dicomUbo.tau*h);
            // 或者使用泰勒展开式 T=T*(1.-dicomUbo.tau*h);
            // T=T*(1.-dicomUbo.tau*h);
        }
        else{
            // ---------Iteration B: tau*h 很大时精确---------------
            // Accumulate the sampleColor
            float F=exp(-dicomUbo.tau*h);
            E=E+T*sampleColor.rgb*(1.-F)*dicomUbo.glow;
            // Accumulate the transparency  // !即 T = T * exp(-tau * deltaS)
            T=T*F;
        }
        
        if((1.-T)>.99)break;
        
        // Go to the next interval
        s=s+h;
    }
    
    return vec4(E.rgb,1.);
}

vec4 opaqueMethod(float stepLength,float rayLength,vec3 dir,vec3 currentPos){
    // Initialize Transparency and Radiance sampleColor
    vec4 dst=vec4(0.);
    // accmulate alpha
    float accmulateAlpha=0.;
    // Evaluate form 0 to D
    for(float s=0.;s<rayLength;){
        // Get the current step or the remaining interval
        float h=min(stepLength,rayLength-s);
        // Get the current position
        vec3 pos=currentPos+dir*(s+h*.5);
        // Get the sampleColor from the 3D texture
        vec4 sampleColor=get3DTextureColor(pos);
        
        if(sampleColor.r!=0.){
            dst=dst+sampleColor*(1.-accmulateAlpha)*dicomUbo.glow;
            accmulateAlpha=accmulateAlpha+dicomUbo.tau*(1.-accmulateAlpha);
        }
        if(accmulateAlpha>.99)break;
        
        // Go to the next interval
        s=s+h;
    }
    return dst;
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
    // 控制stepLength
    float stepLength=dicomUbo.stepLength==0.?rayLength/float(dicomUbo.steps):dicomUbo.stepLength;
    
    vec4 color=rayplusMethod(rayLength,dir,currentPos);
    // vec4 color=absorptionMethod(stepLength,rayLength,dir,currentPos);
    // vec4 color=opaqueMethod(stepLength,rayLength,dir,currentPos);
    
    outColor=color;
}