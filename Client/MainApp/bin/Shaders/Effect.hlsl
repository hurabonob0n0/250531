
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

float4 PS(VertexOut pin) : SV_Target
{
    MaterialData matData = gMaterialData[gMaterialIndex];
    
    uint diffuseMapIndex = matData.DiffuseMapIndex;
    
    float4 color = /*float4(1.f, 1.f, 1.f, 1.f);*/ gTextureMaps[diffuseMapIndex].Sample(gsamLinearWrap, pin.TexC);
    
    if (color.a <= 0.1f)
        discard;
    
    return color;
	
}
