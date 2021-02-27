struct PointLightVertexInput
{
    float4 myPosition : POSITION;
    //float myRange: RANGE;
};

struct PointLightVertexToGeometry
{
    float4 myPosition : POSITION;
    //float myRange : RANGE;
};

struct PointLightGeometryToPixel
{
    float4 myPosition : SV_POSITION;
    float3 myUV : UV;
};

struct PointLightPixelOutput
{
    float4 myColor : SV_TARGET;
};

cbuffer PointLightFrameBuffer : register(b0)
{
    float4x4 pointLightToCamera;
    float4x4 pointLightToWorldFromCamera;
    float4x4 pointLightToProjection;
    float4x4 pointLightToCameraFromProjection;
    float4 pointLightCameraPosition;
}

cbuffer PointLightObjectBuffer : register(b1)
{
    float4x4 pointLightToWorld;
    unsigned int pointLightMyNumberOfDetailNormals;
    unsigned int pointLightMyNumberOfTextureSets;
}

cbuffer PointLightValueBuffer : register(b3)
{
    float4 pointLightColorAndIntensity;
    float4 pointLightPositionAndRange;
}