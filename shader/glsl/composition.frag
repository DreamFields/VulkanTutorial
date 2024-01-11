
#version 450

// 设置精度
// precision highp float;
// precision highp int;
precision highp sampler3D;
layout(binding=0)uniform UniformBufferObject{
    mat4 model;
    mat4 view;
    mat4 proj;
}ubo;
layout(binding=1)uniform sampler3D tex3DSampler;
// input_attachment_index对应于subpasses[1].pInputAttachments的索引
// 也就是说，这里的input_attachment_index=0，对应于subpasses[1].pInputAttachments[0]
// 这里的binding=2，对应于 compositionDescriptorWrite[2].dstBinding = 2
layout(input_attachment_index=0,binding=2)uniform subpassInput inputBackpos;
layout(binding=3)uniform DicomUniformBufferObject{
    vec3 voxelSize;
    vec3 voxelResolution;
    vec3 boxSize;
    vec3 realSize;
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
layout(binding=6)uniform sampler1D TexOccConeSectionsInfo;
layout(std140,binding=7)uniform OcclusionUniformBufferObject{
    // float OccInitialStep;
    // float OccRay7AdjWeight;
    vec4 OccConeRayAxes[10];
    // int OccConeIntegrationSamples[3];
}occlusionUbo;

layout(location=0)in vec3 inColor;
layout(location=1)in vec2 inTexCoord;
layout(location=2)in vec3 inFrontPos;
layout(location=3)in vec3 fragCameraUp;
layout(location=4)in vec3 fragCameraRight;

layout(location=0)out vec4 outColor;

#define CONSIDER_BORDERS
//#define USE_EARLY_TERMINATION
//#define ALWAYS_SPLIT_CONES
//#define USE_FALLOFF_FUNCTION

#ifdef USE_EARLY_TERMINATION
const float max_ext=log(1./.05);
#endif

// volume的真实最大边
float maxVolumeRealEdge=dicomUbo.realSize.r/dicomUbo.boxSize.r;

vec3 worldPos2VolumePos(vec3 worldPos){
    // 将世界坐标转换为归一化的纹理坐标
    vec3 texPos=worldPos/dicomUbo.boxSize;
    return texPos*dicomUbo.realSize;
}

vec4 get3DTextureColor(vec3 worldPos){
    // 将世界坐标转换为纹理坐标,并归一化后再采样
    vec3 texPos=worldPos/dicomUbo.boxSize;
    vec4 sampleColor=texture(tex3DSampler,texPos);
    // vec4 sampleColor=textureLod(tex3DSampler,texPos,2.);
    float intensity=sampleColor.r*255.+sampleColor.g*255.*255.-abs(dicomUbo.minVal);
    intensity=(intensity-dicomUbo.windowCenter)/dicomUbo.windowWidth+.5;
    intensity=clamp(intensity,0.,1.);
    if(intensity==0.)return vec4(0.);
    // 通过采样器，从lutTexSampler中加载数据，相当于传递函数的实现
    vec3 color=texture(lutTexSampler,intensity).rgb;
    // 将1.0-intensity作为alpha值，即遮光量或者说消光系数，浓度越大，遮光量越大，alpha越小
    // return vec4(color,1.-intensity);
    return vec4(color,intensity);
}

vec4 getExtCoeff(vec3 worldPos){
    // 将世界坐标转换为纹理坐标,并归一化后再采样
    vec3 texPos=worldPos/dicomUbo.boxSize;
    vec4 sampleColor=texture(extCoeffSampler,texPos);
    // vec4 sampleColor=textureLod(extCoeffSampler,texPos,4.);
    // return sampleColor;
    
    // 当extCoeffSampler存的是intensity时使用下面的代码
    float intensity=sampleColor.r;
    intensity=clamp(intensity,0.,1.);
    if(intensity==0.)return vec4(0.);
    vec3 color=texture(lutTexSampler,intensity).rgb;
    return vec4(color,intensity);
    
    // 当extCoeffSampler存的是高低8位时使用下面的代码
    /*     float intensity=sampleColor.r*255.+sampleColor.g*255.*255.-abs(dicomUbo.minVal);
    intensity=(intensity-dicomUbo.windowCenter)/dicomUbo.windowWidth+.5;
    intensity=clamp(intensity,0.,1.);
    if(intensity==0.)return vec4(0.);
    // 通过采样器，从lutTexSampler中加载数据，相当于传递函数的实现
    vec3 color=texture(lutTexSampler,intensity).rgb;
    // 将1.0-intensity作为alpha值，即遮光量或者说消光系数，浓度越大，遮光量越大，alpha越小
    // return vec4(color,1.-intensity);
    return vec4(color,intensity); */
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

///////////////////////////////////////////////////////////
// Occlusion
float occ_rays[7];
float last_amptau[7];
float OccInitialStep=3.;
float OccRay7AdjWeight=.972955;
// vec4 OccConeRayAxes[10];
int OccConeIntegrationSamples[3]=int[](1,4,0);// !暂时修改为1,4,0

vec4 GetOcclusionSectionInfo(int id)
{
    return texelFetch(TexOccConeSectionsInfo,id,0).rgba;
}

float GetGaussianExtinction(vec3 volume_pos,float mipmaplevel){
    // 当extCoeffSampler存的是高低8位时使用下面的代码
    vec3 tex_pos=volume_pos/dicomUbo.realSize;
    vec4 sampleColor=textureLod(extCoeffSampler,tex_pos,mipmaplevel);
    float intensity=sampleColor.r*255.+sampleColor.g*255.*255.-abs(dicomUbo.minVal);
    intensity=(intensity-dicomUbo.windowCenter)/dicomUbo.windowWidth+.5;
    intensity=clamp(intensity,0.,1.);
    // return 1.-intensity;
    return intensity;
    
    // 当extCoeffSampler存的是intensity时使用下面的代码
    /*     vec3 tex_pos=volume_pos/dicomUbo.realSize;
    vec4 sampleColor=textureLod(extCoeffSampler,tex_pos,mipmaplevel);
    if(sampleColor.r==0.)return 0.;
    return sampleColor.r; */
}

float Cone7RayOcclusion(vec3 volume_pos_from_zero,float track_distance,vec3 coneDir,vec3 cameraUp,vec3 cameraRight)
{
    // transform 3 to 7
    occ_rays[6]=occ_rays[5]=occ_rays[2];
    occ_rays[4]=occ_rays[3]=occ_rays[1];
    float avg=(occ_rays[2]+occ_rays[1]+occ_rays[0])/3.;
    occ_rays[2]=occ_rays[1]=occ_rays[0];
    occ_rays[0]=avg;
    
    last_amptau[6]=last_amptau[5]=last_amptau[2];
    last_amptau[4]=last_amptau[3]=last_amptau[1];
    float avgt=(last_amptau[2]+last_amptau[1]+last_amptau[0])/3.;
    last_amptau[2]=last_amptau[1]=last_amptau[0];
    last_amptau[0]=avgt;
    
    vec3 vk[7]={coneDir*occlusionUbo.OccConeRayAxes[3].z+cameraUp*occlusionUbo.OccConeRayAxes[3].y+cameraRight*occlusionUbo.OccConeRayAxes[3].x,
        coneDir*occlusionUbo.OccConeRayAxes[4].z+cameraUp*occlusionUbo.OccConeRayAxes[4].y+cameraRight*occlusionUbo.OccConeRayAxes[4].x,
        coneDir*occlusionUbo.OccConeRayAxes[5].z+cameraUp*occlusionUbo.OccConeRayAxes[5].y+cameraRight*occlusionUbo.OccConeRayAxes[5].x,
        coneDir*occlusionUbo.OccConeRayAxes[6].z+cameraUp*occlusionUbo.OccConeRayAxes[6].y+cameraRight*occlusionUbo.OccConeRayAxes[6].x,
        coneDir*occlusionUbo.OccConeRayAxes[7].z+cameraUp*occlusionUbo.OccConeRayAxes[7].y+cameraRight*occlusionUbo.OccConeRayAxes[7].x,
        coneDir*occlusionUbo.OccConeRayAxes[8].z+cameraUp*occlusionUbo.OccConeRayAxes[8].y+cameraRight*occlusionUbo.OccConeRayAxes[8].x,
    coneDir*occlusionUbo.OccConeRayAxes[9].z+cameraUp*occlusionUbo.OccConeRayAxes[9].y+cameraRight*occlusionUbo.OccConeRayAxes[9].x};
    
    int step0=OccConeIntegrationSamples[0]+OccConeIntegrationSamples[1];
    // For each section... do...
    int ith_step=0;
    while(ith_step<OccConeIntegrationSamples[2])
    {
        // vec4 [ distance_to_next_integration | radius_cone | trapezoidalinterval | amplitude ]
        vec4 section_info=GetOcclusionSectionInfo(step0+ith_step);
        
        float interval_distance=section_info.r;
        float mipmap_level=section_info.g;
        float d_integral=section_info.b;
        float gaussian_amp=section_info.a;
        
        bool oc_term=true;
        // update occlusion cone
        for(int i=0;i<7;i++)
        {
            vec3 cur_volume_pos=volume_pos_from_zero+vk[i]*track_distance;
            
            float Tau_s=GetGaussianExtinction(cur_volume_pos,mipmap_level);
            
            float amptau=gaussian_amp*Tau_s;
            
            // #ifdef USE_FALLOFF_FUNCTION
            // occ_rays[i]+=(last_amptau[i]+amptau)*d_integral*falloffunction(track_distance);
            // #else
            occ_rays[i]+=(last_amptau[i]+amptau)*d_integral/* *OccUIWeight */;
            // #endif
            last_amptau[i]=amptau;
        }
        
        #ifdef USE_EARLY_TERMINATION
        // if the occlusion cone reaches a certain amount of remaining light, return
        if(occ_rays[0]>max_ext&&occ_rays[1]>max_ext&&occ_rays[2]>max_ext
        &&occ_rays[3]>max_ext&&occ_rays[4]>max_ext&&occ_rays[5]>max_ext&&occ_rays[6]>max_ext)
        return(exp(-occ_rays[0])+(exp(-occ_rays[1])+exp(-occ_rays[2])+exp(-occ_rays[3])+
        exp(-occ_rays[4])+exp(-occ_rays[5])+exp(-occ_rays[6]))*OccRay7AdjWeight)/(1.+OccRay7AdjWeight*6.);
        #endif
        
        // update tracked distance
        track_distance+=section_info.r;
        
        // Next section
        ith_step=ith_step+1;
    }
    
    return(exp(-occ_rays[0])+(exp(-occ_rays[1])+exp(-occ_rays[2])+exp(-occ_rays[3])+
    exp(-occ_rays[4])+exp(-occ_rays[5])+exp(-occ_rays[6]))*OccRay7AdjWeight)/(1.+OccRay7AdjWeight*6.);
}

float Cone3RayOcclusion(vec3 volume_pos_from_zero,float track_distance,vec3 coneDir,vec3 cameraUp,vec3 cameraRight)
{
    // transform 1 to 3
    occ_rays[2]=occ_rays[0];
    occ_rays[1]=occ_rays[0];
    occ_rays[0]=occ_rays[0];
    
    last_amptau[2]=last_amptau[0];
    last_amptau[1]=last_amptau[0];
    last_amptau[0]=last_amptau[0];
    
    vec3 vk[3]={coneDir*occlusionUbo.OccConeRayAxes[0].z+cameraUp*occlusionUbo.OccConeRayAxes[0].y+cameraRight*occlusionUbo.OccConeRayAxes[0].x,
        coneDir*occlusionUbo.OccConeRayAxes[1].z+cameraUp*occlusionUbo.OccConeRayAxes[1].y+cameraRight*occlusionUbo.OccConeRayAxes[1].x,
    coneDir*occlusionUbo.OccConeRayAxes[2].z+cameraUp*occlusionUbo.OccConeRayAxes[2].y+cameraRight*occlusionUbo.OccConeRayAxes[2].x};
    
    int step0=OccConeIntegrationSamples[0];
    // For each section... do...
    int ith_step=0;
    while(ith_step<OccConeIntegrationSamples[1])
    {
        // vec4 [ distance_to_next_integration | radius_cone | interval | amplitude ]
        vec4 section_info=GetOcclusionSectionInfo(step0+ith_step);
        
        float interval_distance=section_info.r;
        float mipmap_level=section_info.g;
        float d_integral=section_info.b;
        float gaussian_amp=section_info.a;
        
        // update occlusion cone
        for(int i=0;i<3;i++)
        {
            vec3 cur_volume_pos=volume_pos_from_zero+vk[i]*track_distance;
            
            float Tau_s=GetGaussianExtinction(cur_volume_pos,mipmap_level);
            
            float amptau=Tau_s*gaussian_amp;
            
            // #ifdef USE_FALLOFF_FUNCTION
            // occ_rays[i]+=(last_amptau[i]+amptau)*d_integral*falloffunction(track_distance);
            // #else
            occ_rays[i]+=(last_amptau[i]+amptau)*d_integral/* *OccUIWeight */;
            // #endif
            
            last_amptau[i]=amptau;
        }
        
        #ifdef USE_EARLY_TERMINATION
        // if the occlusion cone reaches a certain amount of remaining light, return
        if(occ_rays[0]>max_ext&&occ_rays[1]>max_ext&&occ_rays[2]>max_ext)
        return(exp(-occ_rays[0])+exp(-occ_rays[1])+exp(-occ_rays[2]))/3.;
        #endif
        
        // update tracked distance
        track_distance+=section_info.r;
        
        // Next section
        ith_step=ith_step+1;
    }
    
    #ifdef ALWAYS_SPLIT_CONES
    return Cone7RayOcclusion(volume_pos_from_zero,track_distance,coneDir,cameraUp,cameraRight);
    #else
    // If we have more integration steps to resolve, then...
    if(OccConeIntegrationSamples[2]>0)
    return Cone7RayOcclusion(volume_pos_from_zero,track_distance,coneDir,cameraUp,cameraRight);
    #endif
    
    return(exp(-occ_rays[0])+exp(-occ_rays[1])+exp(-occ_rays[2]))/3.;
}

// Evaluating sections that are approximated with only one sample
float Cone1RayOcclusion(vec3 volume_pos_from_zero,vec3 coneDir,vec3 cameraUp,vec3 cameraRight)
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
        
        vec3 cur_volume_pos=volume_pos_from_zero+coneDir*track_distance;
        
        float Tau_s=GetGaussianExtinction(cur_volume_pos,mipmap_level);
        
        float amptau=Tau_s*gaussian_amp;
        
        // #ifdef USE_FALLOFF_FUNCTION
        // occ_rays[0]+=(last_amptau[0]+amptau)*d_integral*falloffunction(track_distance);
        // #else
        occ_rays[0]+=(last_amptau[0]+amptau)*d_integral/* *OccUIWeight */;
        // #endif
        
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
    return Cone3RayOcclusion(volume_pos_from_zero,track_distance,coneDir,cameraUp,cameraRight);
    #else
    // If we have more integration steps to resolve, then...
    if(OccConeIntegrationSamples[1]+OccConeIntegrationSamples[2]>0)
    return Cone3RayOcclusion(volume_pos_from_zero,track_distance,coneDir,cameraUp,cameraRight);
    #endif
    
    // Return the how much non shadowed is the sample color
    return exp(-occ_rays[0]);
}

vec4 ShadeSample(vec3 worldPos,vec3 dir,vec3 v_up,vec3 v_right){
    // 将世界坐标转换为纹理坐标,并归一化后再采样
    vec3 texPos=worldPos/dicomUbo.boxSize;
    
    vec4 L=get3DTextureColor(worldPos);
    if(L.r==0.)return vec4(0.);
    float ka=0.,kd=0.,ks=0.;
    
    // Directional Ambient Occlusion
    float IOcclusion=.5;
    int ApplyOcclusion=1;
    if(ApplyOcclusion==1)
    {
        ka=.5f;
        IOcclusion=Cone1RayOcclusion(worldPos2VolumePos(worldPos),-dir,v_up,v_right);
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
    
    if(rayLength<stepLength){
        return vec4(0.);
    }
    // Evaluate form 0 to D
    for(float s=0.;s<rayLength;){
        sampleCnt+=1.;
        // Get the current step or the remaining interval
        float h=min(stepLength,rayLength-s);
        // Get the current position
        vec3 pos=currentPos+dir*(s+h*.5);
        // Get the sampleColor from the 3D texture
        // vec4 sampleColor=get3DTextureColor(pos);
        // vec4 sampleColor=getExtCoeff(pos);
        // vec4 sampleColor=getDistanceField(pos);
        
        vec4 sampleColor=ShadeSample(pos,dir,normalize(fragCameraUp),normalize(fragCameraRight));
        
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
    
    return E;
    // return vec4(E.rgb,sampleCnt*stepLength/rayLength);
}

// 投射测试代码
vec4 casingTest(float stepLength,float rayLength,vec3 dir,vec3 currentPos){
    // Initialize Transparency and Radiance sampleColor
    vec4 E=vec4(0.);
    // T
    float T=1.;
    bool isInside=false;
    int afterFrontEmptyCnt=0;// 第一次遇到非空体素(进入体积前表面)后，到光线结束时，经过的空体素的行进次数
    int validCnt=0;// 非空体素的行进次数
    int beforeFrontEmptyCnt=0;// 第一次遇到非空体素(进入体积前表面)前的行进次数
    int allEmptyCnt=0;// 所有经过的空体素的行进次数
    
    // Evaluate form 0 to D
    for(float s=0.;s<rayLength;){
        // Get the current step or the remaining interval
        float h=min(stepLength,rayLength-s);
        // Get the current position
        vec3 pos=currentPos+dir*(s+h*.5);
        // 利用最终的渲染结果来计算空体素的长度
        // vec4 sampleColor=ShadeSample(pos,dir,normalize(fragCameraUp),normalize(fragCameraRight));
        vec4 sampleColor=get3DTextureColor(pos);
        
        // Go to the next interval
        s=s+h;
        
        if(sampleColor.r!=0.){
            isInside=true;
        }
        
        // 计算体素内部采样次数
        if(isInside&&sampleColor.r==0.){
            afterFrontEmptyCnt+=1;
        }
        
        if(!isInside)beforeFrontEmptyCnt+=1;
        
        if(sampleColor.r==0.){
            allEmptyCnt+=1;
            continue;
        }
        else validCnt+=1;
        
        // ---------Iteration B: tau*h 很大时精确---------------
        // Accumulate the sampleColor
        float F=exp(-dicomUbo.alphaCorrection*sampleColor.a*h);
        E=E+T*sampleColor*(1.-F)*dicomUbo.glow;
        // Accumulate the transparency  // !即 T = T * exp(-tau * deltaS)
        T=T*F;
        
        // if((1.-T)>.99)break; // 提前终止步进，已禁用
    }
    
    // return E;
    // 计算光线各种情况下行进的距离占总长的比例
    vec4 res=vec4(0.);
    res.r=clamp(float(beforeFrontEmptyCnt)*stepLength/rayLength,0.,1.);
    res.g=clamp(float(afterFrontEmptyCnt)*stepLength/rayLength,0.,1.);
    res.b=clamp(float(validCnt)*stepLength/rayLength,0.,1.);
    res.a=clamp(float(allEmptyCnt)*stepLength/rayLength,0.,1.);
    return res;
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
    vec4 forwardCastRes=casingTest(stepLength,rayLength,dir,currentPos);// 正向投射
    vec4 backwardCastRes=casingTest(stepLength,rayLength,-dir,backPos);// 反向投射
    // |  beforFrontEmpty    |  insideEmpty+valid  |  afterBackEmpty    |
    // |  forwardCastRes.r   |  forwardCastRes.g  + forwardCastRes.b    |
    // |  backwardCastRes.g + backwardCastRes.b    |  backwardCastRes.r |
    float beforFrontEmpty=forwardCastRes.r;
    float insideEmpty=forwardCastRes.g-backwardCastRes.r;
    float afterBackEmpty=backwardCastRes.r;
    float allEmpty=forwardCastRes.a;// == backwardCastRes.a
    // outColor=vec4((beforFrontEmpty+insideEmpty+afterBackEmpty)/allEmpty); // 测试是否正确
    
    /*
    空体素的比率，越接近1，说明空体素越多。
    head的用例ww=250，wc=250左右时明显。
    mouse的用例ww=600，wc=250左右时明显。
    */
    // outColor=vec4(insideEmpty,insideEmpty,insideEmpty,1.);
    
    // vec2 texCoord=vec2(inTexCoord.x,inTexCoord.y);
    // vec3 testPos=vec3(texCoord.y,27./41.,texCoord.x);// todo 由于立方体尺寸改变，这里需要重新计算纹理坐标
    // outColor=get3DTextureColor(testPos);
    // outColor=getExtCoeff(testPos);
}