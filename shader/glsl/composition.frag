
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

layout(location=0)in vec3 inColor;
layout(location=1)in vec2 inTexCoord;
layout(location=2)in vec3 inFrontPos;

layout(location=0)out vec4 outColor;

vec4 get3DTextureColor(vec3 pos){
    vec4 color=texture(tex3DSampler,pos);
    // vec3 color = pow(color_srgb.rgb,vec3(1./2.2));
    float intensity=color.r*255. + color.g*255.*255. - 1111.;
    // if(color.b==0.){
        //         intensity=-intensity;
    // }
    intensity=(intensity-45.)/95.+.5;
    intensity=clamp(intensity,0.,1.);
    return vec4(intensity,intensity,intensity,1.);
    
    // int value=61;
    // float test=(value-45.)/95.+.5;
    // return vec4(intensity,pow(intensity,2.2),pow((pow(color_srgb.b,2.2)+0.1),1./2.2),1.);
    // return vec4(intensity,test,color.b,1.);
}

void main(){
    // outColor = vec4(inColor,1.0);
    
    // 通过subpassLoad函数，从subpassInput中加载数据
    vec3 backPos=subpassLoad(inputBackpos).xyz;
    // outColor=vec4(backPos,1.);
    
    vec3 dir=normalize(backPos-inFrontPos);
    float rayLength=length(backPos-inFrontPos);
    float steps=800.;
    float stepLength=rayLength/steps;
    vec3 currentPos=inFrontPos;
    float sampleAlpha=.5;
    
    // 从前向后遍历采样点
    vec4 accumulateColor=vec4(0.,0.,0.,0.);
    float accumulateAlpha=0.;
    float accumulateLength=0.;
    
    for(int i=0;i<steps;i++){
        vec4 sampleColor=get3DTextureColor(currentPos);
        // vec4 sampleColor=texture(tex3DSampler,currentPos);
        if(sampleColor.r!=0.){
            accumulateColor+=sampleColor*(1.-accumulateAlpha);
            accumulateAlpha+=sampleAlpha*(1.-accumulateAlpha);
        }
        accumulateLength+=stepLength;
        currentPos+=dir*stepLength;
        if(accumulateAlpha>.99||accumulateLength>=rayLength){
            break;
        }
    }
    
    outColor=accumulateColor;
    // outColor=texture(tex3DSampler,vec3(inTexCoord.y,27.0/41.0,inTexCoord.x));
    // outColor = get3DTextureColor(vec3(inTexCoord.y,27.0/41.0,inTexCoord.x));
    // outColor=get3DTextureColor(vec3(inTexCoord.x,inTexCoord.y,27./41.));
    // outColor = texture(tex3DSampler,vec3(inTexCoord.x,inTexCoord.y,27./41.));
    // outColor = texture(tex3DSampler,vec3(inTexCoord.y,27.0/41.0,inTexCoord.x));
}