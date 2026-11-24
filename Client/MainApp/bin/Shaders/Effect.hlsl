//=============================================================================
// Sky.fx by Frank Luna (C) 2011 All Rights Reserved.
//=============================================================================

// Include common HLSL code.
#include "Common.hlsl"

struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC : TEXCOORD;
    float3 TangentU : TANGENT;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float2 TexC : TEXCOORD;
};
 
VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    vout.PosH = mul(mul(mul(float4(vin.PosL, 1.0f), gWorld), gView), gProj);
    float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
    vout.TexC = texC.xy;
    
    return vout;
}

// sRGB texture -> linear (same as Shaders1/Common.hlsl). Effects blend into the linear HDR RT
float3 SRGBToLinear(float3 c)
{
    return (c <= 0.04045f) ? c / 12.92f : pow(max((c + 0.055f) / 1.055f, 0.0f), 2.4f);
}

float4 PS(VertexOut pin) : SV_Target
{
    MaterialData matData = gMaterialData[gMaterialIndex];
    
    uint diffuseMapIndex = matData.DiffuseMapIndex;
    
    float4 color = gTextureMaps[diffuseMapIndex].Sample(gsamAnisotropicClamp, pin.TexC); /*float4(1.f, 1.f, 1.f, 1.f);*/
    
    if (color.a <= 0.01f)
        discard;

    color.rgb = SRGBToLinear(color.rgb);
    
    return color;
	
}

