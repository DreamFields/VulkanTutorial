
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
    vec3 voxelSize;
    vec3 voxelResolution;
    vec3 boxSize;
    float windowCenter;
    float windowWidth;
    float minVal;
    float alphaCorrection;// 作为透明度的矫正系数
    float stepLength;
    float glow;
    int steps;
}dicomUbo;
layout(binding=4)uniform sampler1D lutTexSampler;
layout(binding=5)uniform sampler3D extCoeffSampler;

layout(location=0)in vec3 inColor;
layout(location=1)in vec2 inTexCoord;
layout(location=2)in vec3 inFrontPos;

layout(location=0)out vec4 outColor;

#define CONSIDER_BORDERS
//#define USE_EARLY_TERMINATION
//#define ALWAYS_SPLIT_CONES
//#define USE_FALLOFF_FUNCTION

#ifdef USE_EARLY_TERMINATION
const float max_ext=log(1./.05);
#endif

vec4 get3DTextureColor(vec3 worldPos){
    // 将世界坐标转换为纹理坐标,并归一化后再采样
    vec3 texPos=worldPos/dicomUbo.boxSize;
    // vec4 sampleColor=texture(tex3DSampler,texPos);
    vec4 sampleColor=textureLod(tex3DSampler,texPos,2.);
    float intensity=sampleColor.r*255.+sampleColor.g*255.*255.-abs(dicomUbo.minVal);
    intensity=(intensity-dicomUbo.windowCenter)/dicomUbo.windowWidth+.5;
    intensity=clamp(intensity,0.,1.);
    if(intensity==0.)return vec4(0.);
    // 通过采样器，从lutTexSampler中加载数据，相当于传递函数的实现
    vec3 color=texture(lutTexSampler,intensity).rgb;
    // 将1.0-intensity作为alpha值，即遮光量或者说消光系数，浓度越大，遮光量越大，alpha越小
    return vec4(color,1.-intensity);
}

vec4 getExtCoeff(vec3 worldPos){
    // 将世界坐标转换为纹理坐标,并归一化后再采样
    vec3 texPos=worldPos/dicomUbo.boxSize;
    // vec4 sampleColor=texture(extCoeffSampler,texPos);
    vec4 sampleColor=textureLod(extCoeffSampler,texPos,2.);
    // if(sampleColor.r==0.)return vec4(0.);
    // return sampleColor;
    
    float intensity=sampleColor.r*255.+sampleColor.g*255.*255.-abs(dicomUbo.minVal);
    intensity=(intensity-dicomUbo.windowCenter)/dicomUbo.windowWidth+.5;
    intensity=clamp(intensity,0.,1.);
    if(intensity==0.)return vec4(0.);
    // 通过采样器，从lutTexSampler中加载数据，相当于传递函数的实现
    vec3 color=texture(lutTexSampler,intensity).rgb;
    // 将1.0-intensity作为alpha值，即遮光量或者说消光系数，浓度越大，遮光量越大，alpha越小
    return vec4(color,1.-intensity);
}

// todo 目前距离场可视化暂时采用直接体绘制的方法，需改进
vec4 getDistanceField(vec3 worldPos){
    // 将世界坐标转换为纹理坐标,并归一化后再采样
    vec3 texPos=worldPos/dicomUbo.boxSize;
    
    vec4 volumeColor=get3DTextureColor(worldPos);
    
    if(volumeColor.r==0.){
        return vec4(texture(extCoeffSampler,texPos).a);
    }
    return vec4(0.,0.,0.,0.);
}

// Evaluating sections that are approximated with only one sample
/* float Cone1RayOcclusion(vec3 pos_from_zero,vec3 coneDir,vec3 cameraUp,vec3 cameraRight)
{
    float track_distance=OccInitialStep;
    occ_rays[0]=0.;
    last_amptau[0]=0.;
    
    // For each section... do...
    int ith_step=0;
    while(ith_step<OccConeIntegrationSamples[0])
    {
        // vec4 [ distance_to_next_integration | radius_cone | coef_rescale ]
        vec4 section_info=GetOcclusionSectionInfo(ith_step);
        
        float interval_distance=section_info.r;// interval间隔的步长
        float mipmap_level=section_info.g;// mipmap level
        float d_integral=section_info.b;// 系数
        float gaussian_amp=section_info.a;// 当前项的高斯积分结果 \frac{\left(p_r\sigma\sqrt{2\pi}\right)^2}{A_c} ，未乘以 \tau_s
        
        vec3 pos=pos_from_zero+coneDir*track_distance;
        
        float Tau_s=GetGaussianExtinction(pos,mipmap_level);
        
        float amptau=Tau_s*gaussian_amp;
        
        #ifdef USE_FALLOFF_FUNCTION
        occ_rays[0]+=(last_amptau[0]+amptau)*d_integral*falloffunction(track_distance);
        #else
        occ_rays[0]+=(last_amptau[0]+amptau)*d_integral*OccUIWeight;
        #endif
        
        last_amptau[0]=amptau;
        
        #ifdef USE_EARLY_TERMINATION
        // if the occlusion cone reaches a certain amount of remaining light, return
        if(occ_rays[0]>max_ext)return exp(-occ_rays[0]);
        #endif
        
        // update tracked distance
        track_distance+=section_info.r;
        
        // Next section
        ith_step=ith_step+1;
    }
    
    #ifdef ALWAYS_SPLIT_CONES
    return Cone3RayOcclusion(pos_from_zero,track_distance,coneDir,cameraUp,cameraRight);
    #else
    // If we have more integration steps to resolve, then...
    if(OccConeIntegrationSamples[1]+OccConeIntegrationSamples[2]>0)
    return Cone3RayOcclusion(pos_from_zero,track_distance,coneDir,cameraUp,cameraRight);
    #endif
    
    // Return the how much non shadowed is the sample color
    return exp(-occ_rays[0]);
} */

vec4 ShadeSample(vec3 worldPos,vec3 dir,vec3 v_up,vec3 v_right){
    // 将世界坐标转换为纹理坐标,并归一化后再采样
    vec3 texPos=worldPos/dicomUbo.boxSize;
    
    vec4 L=get3DTextureColor(worldPos);
    float ka=0.,kd=0.,ks=0.;
    
    // Directional Ambient Occlusion
    float IOcclusion=.5;
    int ApplyOcclusion=1;
    if(ApplyOcclusion==1)
    {
        ka=.5f;
        // IOcclusion=Cone1RayOcclusion(worldPos,-dir,v_up,v_right);
    }
    
    // Shadows
    float IShadow=0.;
    int ApplyShadow=0;
    if(ApplyShadow==1)
    {
        // kd=Kdiffuse;
        // ks=Kspecular;
        // IShadow=ShadowEvaluationKernel(tx_pos);
    }
    
    L.rgb=(1./(ka+kd))*(L.rgb*IOcclusion*ka+L.rgb*IShadow*kd);
    return L;
}

vec4 absorptionMethod(float stepLength,float rayLength,vec3 dir,vec3 currentPos){
    // Initialize Transparency and Radiance sampleColor
    vec4 E=vec4(0.);
    // T
    float T=1.;
    bool isAccurate=true;
    float sampleCnt=0.;
    // Evaluate form 0 to D
    for(float s=0.;s<rayLength;){
        sampleCnt+=1.;
        // Get the current step or the remaining interval
        float h=min(stepLength,rayLength-s);
        // Get the current position
        vec3 pos=currentPos+dir*(s+h*.5);
        // Get the sampleColor from the 3D texture
        // vec4 sampleColor=get3DTextureColor(pos);
        vec4 sampleColor=getExtCoeff(pos);
        // vec4 sampleColor=getDistanceField(pos);
        
        // vec4 sampleColor=ShadeSample(pos,dir,vec3(0.,1.,0.),vec3(1.,0.,0.));
        
        // Go to the next interval
        s=s+h;
        
        if(sampleColor.r==0.){
            // vec3 texPos=pos/dicomUbo.boxSize;
            // float minForwardDis=texture(extCoeffSampler,texPos).a*length(dicomUbo.boxSize);
            // if(minForwardDis/4.>h){
                //     s=s+minForwardDis/5.-h;
                //     continue;
            // }
            continue;
        }
        
        if(!isAccurate){
            // ---------Iteration A: tau*h 很小时结果较好---------------
            // Accumulate the sampleColor
            E=E+T*sampleColor*dicomUbo.alphaCorrection*dicomUbo.glow*sampleColor.a;
            // Accumulate the transparency  // !即 T = T * exp(-tau * deltaS)
            T+=T*exp(-dicomUbo.alphaCorrection*sampleColor.a*h);
            // 或者使用泰勒展开式 T=T*(1.-dicomUbo.alphaCorrection*h);
            // T=T*(1.-dicomUbo.alphaCorrection*h);
        }
        else{
            // ---------Iteration B: tau*h 很大时精确---------------
            // Accumulate the sampleColor
            float F=exp(-dicomUbo.alphaCorrection*sampleColor.a*h);
            E=E+T*sampleColor*(1.-F)*dicomUbo.glow;
            // Accumulate the transparency  // !即 T = T * exp(-tau * deltaS)
            T=T*F;
        }
        
        if((1.-T)>.99)break;
    }
    
    // return E;
    return vec4(E.rgb,sampleCnt*stepLength/rayLength);
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
    
    vec4 color=absorptionMethod(stepLength,rayLength,dir,currentPos);
    
    outColor=color;
    // vec3 testPos=vec3(inTexCoord.y,27./41.,inTexCoord.x);// todo 由于立方体尺寸改变，这里需要重新计算纹理坐标
    // outColor=get3DTextureColor(testPos);
    // outColor=getExtCoeff(testPos);
    // outColor=getDistanceField(testPos);
    // outColor=vec4(color.a);// test 测试步进的次数
}