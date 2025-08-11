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

    vout.PosH = mul(mul(float4(vin.PosL, 1.0f), gWorld), gTexTransform);
    //float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
    vout.TexC = vin.TexC;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    MaterialData matData = gMaterialData[gMaterialIndex];
    
    uint diffuseMapIndex = matData.DiffuseMapIndex;
    
    float4 color = gTextureMaps[diffuseMapIndex].Sample(gsamAnisotropicClamp, pin.TexC) /*+ float4(0.1f, 0.1f, 0.1f, 0.f)*/; /*float4(1.f, 1.f, 1.f, 1.f);*/
    
    
   if (gObjPad0 == 2) // 점령지
    {
        float p = saturate(gObjPad2 * 0.01f); // 0~1

        if (gObjPad1 == 1) // 블루: 왼->오
        {
            if (pin.TexC.x > p)
                discard;
        }
        else if (gObjPad1 == 2) // 레드: 오->왼
        {
            // 오른쪽부터 p만큼 채움 => x < 1-p 는 버림
            if (pin.TexC.x < (1.0f - p))
                discard;
        }
    }
    else if (gObjPad0 == 1) // HPBar (왼->오)
    {
        float p = saturate(gObjPad2 * 0.01f);
        if (pin.TexC.x > p)
            discard;
    }
    
    return color;
	
}

