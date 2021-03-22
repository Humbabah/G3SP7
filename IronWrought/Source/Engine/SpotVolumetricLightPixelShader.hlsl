#include "SpotLightShaderStructs.hlsli"
#include "MathHelpers.hlsli"

#define NUM_SAMPLES 128
#define NUM_SAMPLES_RCP 0.0078125

// RAYMARCHING
#define TAU 0.0001
#define PHI 100000.0

#define PI_RCP 0.31830988618379067153776752674503

sampler defaultSampler : register(s0);
sampler shadowSampler : register(s1);
Texture2D normalTextureGBuffer : register(t2);
Texture2D depthTexture : register(t21);
//Texture2D shadowDepthTexture : register(t22);

float4 PixelShader_WorldPosition(float2 uv)
{
    // Depth sampling
    float z = depthTexture.Sample(defaultSampler, uv).r;
    float x = uv.x * 2.0f - 1;
    float y = (1 - uv.y) * 2.0f - 1;
    float4 projectedPos = float4(x, y, z, 1.0f);
    float4 viewSpacePos = mul(spotLightToCameraFromProjection, projectedPos);
    viewSpacePos /= viewSpacePos.w;
    float4 worldPos = mul(spotLightToWorldFromCamera, viewSpacePos);

    worldPos.a = 1.0f;
    return worldPos;
}

float3 GBuffer_Normal(float2 uv)
{
    float3 normal = normalTextureGBuffer.Sample(defaultSampler, uv).rgb;
    return normal;
}

//float3 SampleShadowPos(float3 projectionPos)
//{
//    float2 uvCoords = projectionPos.xy;
//    uvCoords *= float2(0.5f, -0.5f);
//    uvCoords += float2(0.5f, 0.5f);

//    float nonLinearDepth = shadowDepthTexture.Sample(shadowSampler, uvCoords).r;
//    float oob = 1.0f;
//    if (projectionPos.x > 1.0f || projectionPos.x < -1.0f || projectionPos.y > 1.0f || projectionPos.y < -1.0f)
//    {
//        oob = 0.0f;
//    }

//    float a = nonLinearDepth * oob;
//    float b = projectionPos.z;
//    b = invLerp(-0.5f, 0.5f, b) * oob;

//    b *= oob;

//    //return 1.0f;
    
//    if (b - a < 0.001f)
//    {
//        return 1.0f;
//    }
//    else
//    {
//        return 0.0f;
//    }
//}

//float3 ShadowFactor(float3 viewPos)
//{
//    float4 projectionPos = mul(toDirectionalLightProjection, viewPos);
//    return SampleShadowPos(projectionPos.xyz);
//}

void ExecuteRaymarching(inout float3 rayPositionLightVS, float3 invViewDirLightVS, inout float3 rayPositionWorld, float3 invViewDirWorld, float stepSize, float l, inout float3 VLI)
{
    rayPositionLightVS.xyz += stepSize * invViewDirLightVS.xyz;

    // March in world space in parallel
    rayPositionWorld += stepSize * invViewDirWorld;
    //..
    
    float3 shadowTerm = /*ShadowFactor(rayPositionLightVS.xyz).xxx*/1.0f;
    
    // Distance to the current position on the ray in light view-space
    float d = length(rayPositionLightVS.xyz);
    float dRcp = rcp(d); // reciprocal
    
    // Calculate the final light contribution for the sample on the ray...
    float3 intens = TAU * (shadowTerm * (PHI * 0.25 * PI_RCP) * dRcp * dRcp) * exp(-d * TAU) * exp(-l * TAU) * stepSize;
    
    // World space attenuation
    float3 toLight = spotLightPositionAndRange.xyz - rayPositionWorld.xyz;
    float lightDistance = length(toLight);
    toLight = normalize(toLight);
    float linearAttenuation = lightDistance / spotLightPositionAndRange.w;
    linearAttenuation = 1.0f - linearAttenuation;
    linearAttenuation = saturate(linearAttenuation);
    float physicalAttenuation = saturate(1.0f / (lightDistance * lightDistance));
    float angleAttenuation = pow(max(dot(spotLightDirectionAndAngleExponent.xyz, -toLight), 0.0f), spotLightDirectionAndAngleExponent.w);
    //float attenuation = angleAttenuation * linearAttenuation * physicalAttenuation;
    //float attenuation = physicalAttenuation;
    //float attenuation = linearAttenuation;
    float attenuation = angleAttenuation;
    //float attenuation = 1.0f;
    
    // ... and add it to the total contribution of the ray
    VLI += intens * attenuation;
}

// !RAYMARCHING

SpotLightPixelOutput main(SpotLightGeometryToPixel input)
{
    SpotLightPixelOutput output;
    
    float raymarchDistanceLimit = 99999.0f;
    
    // ...
    float2 screenUV = (input.myUV.xy / input.myUV.z) * 0.5f + 0.5f;
    float3 worldPosition = PixelShader_WorldPosition(screenUV).rgb;
    float3 camPos = spotLightCameraPosition.xyz;
    
    // For marching in world space in parallel
    float3 rayPositionWorld = worldPosition;
    float3 invViewDirWorld = normalize(camPos - worldPosition);
    // ..
    
    float3 lightPos = spotLightTransform._41_42_43;
    worldPosition -= lightPos.xyz;
    float3 positionLightVS = mul(spotLightToView, worldPosition);
   
    camPos -= lightPos.xyz;
    float3 cameraPositionLightVS = mul(spotLightToView, camPos);
    
    // Reduce noisyness by truncating the starting position
    //float raymarchDistance = trunc(clamp(length(cameraPositionLightVS.xyz - positionLightVS.xyz), 0.0f, raymarchDistanceLimit));
    float4 invViewDirLightVS = float4(normalize(cameraPositionLightVS.xyz - positionLightVS.xyz), 0.0f);
    float raymarchDistance = clamp(length(cameraPositionLightVS.xyz - positionLightVS.xyz), 0.0f, raymarchDistanceLimit);
    
    // Calculate the size of each step
    float stepSize = raymarchDistance * NUM_SAMPLES_RCP;
    float3 rayPositionLightVS = positionLightVS.xyz;
    
    // The total light contribution accumulated along the ray
    float3 VLI = 0.0f;
    
    // Start ray marching
    [loop]
    for (float l = raymarchDistance; l > stepSize; l -= stepSize)
    {
        ExecuteRaymarching(rayPositionLightVS, invViewDirLightVS.xyz, rayPositionWorld, invViewDirWorld, stepSize, l, VLI);
    }
    
    output.myColor.rgb = spotLightColorAndIntensity.rgb * VLI;
    output.myColor.a = 1.0f;
    return output;
}