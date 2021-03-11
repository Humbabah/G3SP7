#include "DeferredSamplingFunctions.hlsli"

struct GBufferOutput
{
    float3 myAlbedo             : SV_TARGET0;
    float3 myNormal             : SV_TARGET1;
    float3 myVertexNormal       : SV_TARGET2;
    float4 myMetalRoughAOEm     : SV_TARGET3;
};


GBufferOutput main(VertexModelToPixel input)
{
    VertexToPixel vertToPixel;
    vertToPixel.myPosition  = input.myPosition;
    vertToPixel.myUV        = input.myUV;
    
    float3 albedo = PixelShader_Albedo(vertToPixel.myUV).rgb;
    
    float4 tintMap = PixelShader_TintMap(vertToPixel.myUV).rgba;
    //float3 color1 = float3(1, 0, 0);//* tintMap.r;// tintmask red
    //float3 color2 = float3(0, 1, 0);//* tintMap.g;// tintmask green
    //float3 color3 = float3(0, 0, 1);//* tintMap.b;// tintmask blue
    //float3 color4 = float3(1, 1, 1); //* tintMap.a;// tintmask alpha
    float blendFactor = 0.65f;
    
    float emissive = PixelShader_Emissive(vertToPixel.myUV);
    albedo = lerp(albedo, myTint1.rgb, tintMap.r * blendFactor * (1 - emissive));
    albedo = lerp(albedo, myTint2.rgb, tintMap.g * blendFactor * (1 - emissive));
    albedo = lerp(albedo, myTint3.rgb, tintMap.b * blendFactor * (1 - emissive));
    albedo = lerp(albedo, myTint4.rgb, tintMap.a * blendFactor * (1 - emissive));

    //Reminder: Nico s�ger att gamma korrigering kommer ske fast att det �r on�digt, kan vara en grej!
    
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