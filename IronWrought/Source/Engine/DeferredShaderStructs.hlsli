struct VertexInput
{
    unsigned int myIndex : SV_VertexID;
};

struct VertexModelInput
{
    float4 myPosition   : POSITION;
    float4 myNormal     : NORMAL;
    float4 myTangent    : TANGENT;
    float4 myBiTangent   : BITANGENT;
    float2 myUV         : UV;
    float4 myBoneID     : BONEID;
    float4 myBoneWeight : BONEWEIGHT;
};

struct VertexToPixel
{
    float4 myPosition   : SV_POSITION;
    float2 myUV         : UV;
};
struct VertexModelToPixel
{
    float4 myPosition       : SV_POSITION;
    float4 myWorldPosition  : WORLDPOSITION;
    float4 myNormal         : NORMAL;
    float4 myTangent        : TANGENT;
    float4 myBinormal       : BINORMAL;
    float2 myUV             : UV;
};

struct PixelOutput
{
    float4 myColor : SV_TARGET;
};

cbuffer FrameBuffer : register(b0)
{
    float4x4 toCamera;
    float4x4 toProjection;
    float4 cameraPosition;
}

cbuffer ObjectBuffer : register(b1)
{
    float4x4 toWorld;
}

cbuffer LightBuffer : register(b2)
{
    float4 toDirectionalLight;
    float4 directionalLightColor;
}

cbuffer PointLightBuffer : register(b3)
{
    float4 myColorAndIntensity;
    float4 myPositionAndRange;
}


TextureCube environmentTexture : register(t0);

Texture2D worldPositionTexture : register(t1);
Texture2D albedoTextureGBuffer : register(t2);
Texture2D normalTextureGBuffer : register(t3);
Texture2D vertexNormalTexture : register(t4);
Texture2D metalnessTexture : register(t5);
Texture2D roughnessTexture : register(t6);
Texture2D ambientOcclusionTexture : register(t7);
Texture2D emissiveTexture : register(t8);

Texture2D albedoTexture : register(t9);
Texture2D materialTexture : register(t10);
Texture2D normalTexture : register(t11);

sampler defaultSampler : register(s0);

/* // Backup
TextureCube environmentTexture : register(t0);
Texture2D worldPositionTexture : register(t1);
Texture2D albedoTexture : register(t2);
Texture2D normalTexture : register(t3);
Texture2D vertexNormalTexture : register(t4);
Texture2D metalnessTexture : register(t5);
Texture2D roughnessTexture : register(t6);
Texture2D ambientOcclusionTexture : register(t7);
Texture2D emissiveTexture : register(t8);
sampler defaultSampler : register(s0);
*/