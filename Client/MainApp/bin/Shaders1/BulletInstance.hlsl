//***************************************************************************************
// Default.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

// Include common HLSL code.
#include "Common.hlsl"

struct VertexIn
{
	float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
	float2 TexC    : TEXCOORD;
	float3 TangentU : TANGENT;
};

struct VertexOut
{
	float4  PosH    : SV_POSITION;
    float3  PosL    : POSITION;
    float   age     : AGE;
};

VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID)
{
	VertexOut vout = (VertexOut)0.0f;

	// Fetch the material data.
    InstanceData data = gInstanceData[instanceID];
	
    // Transform to world space.
    float4 posW = mul(float4(vin.PosL, 1.0f), data.WorldMat);
    vout.PosL = vin.PosL;

    // Transform to homogeneous clip space.
    vout.PosH = mul(posW, gViewProj);
    
    vout.age = data.ObjPad2;
	
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	// Fetch the material data.
	// 1. 기본 총알 색상 정의
    float4 finalColor = float4(1.0f, 0.2f, 0.1f, 1.0f); // 예: 밝은 주황색

    //// 2. 원기둥 효과를 위한 알파 계산
    //// 로컬 좌표의 중심(Z축)으로부터의 거리 계산 (X, Y 평면)
    //float distFromCenter = length(pin.PosL.xy);

    //// 거리가 0.5(정육면체 가장자리)에 가까워질수록 알파 값이 0에 가까워짐
    //// smoothstep을 사용하면 경계가 더 부드러워짐
    //// 0.3까지는 불투명, 0.5에서 완전 투명
    //float cylinderAlpha = 1.0 - distFromCenter / 0.5f;

    //// 3. 시간에 따른 페이드아웃 알파 계산
    float maxAge = 1.f; // 궤적이 사라지기 시작하는 최대 시간 (2초)
    float timeAlpha = 1.0 - saturate(pin.age / maxAge);

    // 4. 최종 알파 값 계산 (두 알파 값을 곱함)
    finalColor.a = /*cylinderAlpha; //*/timeAlpha;

    return finalColor;

}


