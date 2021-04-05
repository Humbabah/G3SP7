#include "DeferredSamplingFunctions.hlsli"

struct GBufferOutput
{
    float3 myAlbedo             : SV_TARGET0;
    float3 myNormal             : SV_TARGET1;
    float3 myVertexNormal       : SV_TARGET2;
    float4 myMetalRoughAOEm     : SV_TARGET3;
};

float4 SampleTintTextureAlbedo(int index, float2 uv)
{
    float tilingModifier = 4.0f;
   
    float4 tintTexture;
    tintTexture = tintTextures[index].Sample(defaultSampler, uv * tilingModifier).rgba;
    
    return tintTexture;
}

GBufferOutput main(VertexModelToPixel input)
{
    VertexToPixel vertToPixel;
    vertToPixel.myPosition  = input.myPosition;
    vertToPixel.myUV        = input.myUV;
    
    float3 albedo = PixelShader_Albedo(vertToPixel.myUV).rgb;
    
    float4 tintMap = PixelShader_TintMap(vertToPixel.myUV).rgba;
    float emissive = PixelShader_Emissive(vertToPixel.myUV);
    if (emissive < 0.99f)
    {
        float blendFactor = 0.65f * (1 - emissive);
        float tint1BlendFactor = tintMap.r * blendFactor;
        float tint2BlendFactor = tintMap.g * blendFactor;
        float tint3BlendFactor = tintMap.b * blendFactor;
        float tint4BlendFactor = tintMap.a * blendFactor;
    
        albedo = lerp(albedo, SampleTintTextureAlbedo(0, vertToPixel.myUV).rgb, tint1BlendFactor);
        albedo = lerp(albedo, SampleTintTextureAlbedo(1, vertToPixel.myUV).rgb, tint2BlendFactor);
        albedo = lerp(albedo, SampleTintTextureAlbedo(2, vertToPixel.myUV).rgb, tint3BlendFactor);
        albedo = lerp(albedo, SampleTintTextureAlbedo(3, vertToPixel.myUV).rgb, tint4BlendFactor);
        
        albedo = lerp(albedo, myTint1.rgb, tint1BlendFactor * myTint1.a);
        albedo = lerp(albedo, myTint2.rgb, tint2BlendFactor * myTint2.a);
        albedo = lerp(albedo, myTint3.rgb, tint3BlendFactor * myTint3.a);
        albedo = lerp(albedo, myTint4.rgb, tint4BlendFactor * myTint4.a);
    }
    else
    {
        albedo = myEmissive.rgb * myEmissive.a;
    }
    
    float3 normal = PixelShader_Normal(vertToPixel.myUV).xyz; 
    if (myNumberOfDetailNormals > 0)
    {
        float detailNormalStrength = PixelShader_DetailNormalStrength(vertToPixel.myUV);
        float strengthMultiplier = DetailStrengthDistanceMultiplier(cameraPosition.xyz, input.myWorldPosition.xyz);
        float3 detailNormal;

        // Blend based on detail normal strength
        // X3512 Sampler array index must be literal expression => DETAILNORMAL_#
        // Sampled detail normal strength value: 
        //      0.2f - 0.24f    == DETAILNORMAL_1
        //      0.26f - 0.49f   == DETAILNORMAL_2
        //      0.51f - 0.74f   == DETAILNORMAL_3
        //      0.76f - 1.0f    == DETAILNORMAL_4
        // Note! This if-chain exists in 3 shaders: PBRPixelShader, GBufferPixelShader and DeferredRenderPassGBufferPixelShader
        // Make this better please
        if (detailNormalStrength > DETAILNORMAL_4_STR_RANGE_MIN)
        {
            detailNormal = PixelShader_DetailNormal(vertToPixel.myUV, DETAILNORMAL_4).xyz;
            detailNormalStrength = (detailNormalStrength - DETAILNORMAL_4_STR_RANGE_MIN + 0.01f) / DETAILNORMAL_STR_RANGE_DIFF;
        }
        else if (detailNormalStrength > DETAILNORMAL_3_STR_RANGE_MIN)
        {
            detailNormal = PixelShader_DetailNormal(vertToPixel.myUV, DETAILNORMAL_3).xyz;
            detailNormalStrength = (detailNormalStrength - DETAILNORMAL_3_STR_RANGE_MIN + 0.01f) / DETAILNORMAL_STR_RANGE_DIFF;
        }
        else if (detailNormalStrength > DETAILNORMAL_2_STR_RANGE_MIN)
        {
            detailNormal = PixelShader_DetailNormal(vertToPixel.myUV, DETAILNORMAL_2).xyz;
            detailNormalStrength = (detailNormalStrength - DETAILNORMAL_2_STR_RANGE_MIN + 0.01f) / DETAILNORMAL_STR_RANGE_DIFF;
        }
        else
        {
            detailNormal = PixelShader_DetailNormal(vertToPixel.myUV, DETAILNORMAL_1).xyz;
            detailNormalStrength = (detailNormalStrength - DETAILNORMAL_1_STR_RANGE_MIN + 0.01f) / DETAILNORMAL_STR_RANGE_DIFF;
        }
        
        detailNormal = SetDetailNormalStrength(detailNormal, detailNormalStrength, strengthMultiplier);
        normal = normal * 0.5 + 0.5;
        detailNormal = detailNormal * 0.5 + 0.5;
        normal = BlendRNM(normal, detailNormal);
        
        //---
        
        // Blend all 4 ontop of eachother
        //for (int i = 0; i < myNumberOfDetailNormals; ++i)
        //{
        //    detailNormal = PixelShader_DetailNormal(vertToPixel, i).myColor.xyz;
        //    detailNormal = SetDetailNormalStrength(detailNormal, detailNormalStrength, strengthMultiplier);
        //    normal = normal * 0.5 + 0.5;
        //    detailNormal = detailNormal * 0.5 + 0.5;
        //    normal = BlendRNM(normal, detailNormal);
        //}
    } // End of if
    
    float3x3 tangentSpaceMatrix = float3x3(normalize(input.myTangent.xyz), normalize(input.myBinormal.xyz), normalize(input.myNormal.xyz));
    normal = mul(normal.xyz, tangentSpaceMatrix);
    normal = normalize(normal);
    
    float ambientOcclusion      = PixelShader_AmbientOcclusion(vertToPixel.myUV);
    float metalness             = PixelShader_Metalness(vertToPixel.myUV);
    float perceptualRoughness   = PixelShader_PerceptualRoughness(vertToPixel.myUV);
    
    GBufferOutput output;    
    output.myAlbedo         = albedo.xyz;
    output.myNormal         = normal.xyz;
    output.myVertexNormal   = input.myNormal.xyz;
    output.myMetalRoughAOEm = float4(metalness, perceptualRoughness, ambientOcclusion, emissive);
    
    return output;
}