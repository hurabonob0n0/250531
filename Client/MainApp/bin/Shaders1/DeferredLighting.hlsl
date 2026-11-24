//=============================================================================
// DeferredLighting.hlsl
// Deferred 라이팅 패스 (풀스크린 삼각형 한 번). 루트시그니처는 DeferredLightingRS
//   slot 0 : t0~t4 space2  G-Buffer 4장 + 깊이   (힙 엔진 칸 4~8)
//   slot 1 : b1            PassCB (조명, 카메라, 역 ViewProj, 그림자 행렬)
//   slot 2 : t0 space1     머티리얼 버퍼 (안 씀. Set_CurrentFramePBMats 자리 맞춤)
//   slot 3 : t0            큐브맵 (반사)
//   slot 4 : t1            그림자맵
//   slot 5 : b2            gDebugMode (0 조명, 1 노멀, 2 그림자, 3 알베도, 4 광택, 5 발광, 6 깊이)
//
// Common.hlsl 을 그대로 include 해서 Forward 와 똑같은 조명 함수(ComputeLighting, CalcShadowFactor,
// SchlickFresnel)를 쓴다. 다른 점은 위치와 노멀을 정점 셰이더가 아니라 G-Buffer 에서 얻는다는 것뿐
// → 화면이 Forward 와 같으면 G-Buffer 기록과 위치 복원이 맞다는 뜻
//=============================================================================

#include "Common.hlsl"

// space2: Common.hlsl 의 t0(큐브맵), t1(그림자맵), t2~(gTextureMaps) 와 겹치지 않게
Texture2D gGBufferAlbedo   : register(t0, space2);
Texture2D gGBufferNormal   : register(t1, space2);
Texture2D gGBufferMaterial : register(t2, space2);
Texture2D gGBufferEmissive : register(t3, space2);
Texture2D gDepthMap        : register(t4, space2);

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float2 TexC : TEXCOORD;
};

// PostProcess.hlsl 과 같은 풀스크린 삼각형
VertexOut VS(uint vid : SV_VertexID)
{
    VertexOut vout;
    vout.TexC = float2((vid << 1) & 2, vid & 2);
    vout.PosH = float4(vout.TexC.x * 2.0f - 1.0f, 1.0f - vout.TexC.y * 2.0f, 0.0f, 1.0f);
    return vout;
}

// 화면 uv + 깊이 → 월드 위치
// 기하 패스: 월드 → (x ViewProj) → 클립 → (÷ w) → NDC (x, y, 깊이)
// 여기서는 거꾸로: NDC → (x 역 ViewProj) → 동차 좌표 → (÷ w) → 월드
// uv 는 아래로 증가, NDC y 는 위로 증가 → y = 1 - 2v (풀스크린 VS 의 역변환)
float3 ReconstructWorldPos(float2 uv, float depth)
{
    float4 ndc = float4(uv.x * 2.0f - 1.0f, 1.0f - uv.y * 2.0f, depth, 1.0f);
    float4 posW = mul(ndc, gInvViewProj);
    return posW.xyz / posW.w;
}

float4 PS(VertexOut pin) : SV_Target
{
    // SV_POSITION 의 xy 는 픽셀 중심 좌표 (0.5, 1.5, ...). Load 는 정수 좌표로 필터링 없이 정확히 그 텍셀을 읽는다
    int3 texel = int3(pin.PosH.xy, 0);
    float depth = gDepthMap.Load(texel).r;

    if (gDebugMode == 6)
    {
        // 깊이 시각화: 0~1 깊이는 원근 때문에 거의 다 1 근처라 그대로 보면 하얗기만 하다 → 뷰 공간 거리로 바꿔서 보여 준다
        float4 posV = mul(float4(pin.TexC.x * 2.0f - 1.0f, 1.0f - pin.TexC.y * 2.0f, depth, 1.0f), gInvProj);
        float viewZ = posV.z / posV.w;
        return float4(saturate(viewZ / 1000.0f).xxx, 1.0f);
    }

    // 깊이 1 = 아무 물체도 안 그려진 곳(하늘). 이 뒤에 스카이가 LESS_EQUAL 로 채운다
    // G-Buffer 를 Clear 하지 않으므로 이 픽셀들의 G-Buffer 값은 쓰레기 → 읽기 전에 빠진다
    if (depth >= 1.0f)
        return float4(0.0f, 0.0f, 0.0f, 1.0f);

    float4 albedoTex   = gGBufferAlbedo.Load(texel);
    float3 normalW     = gGBufferNormal.Load(texel).xyz;
    float4 materialTex = gGBufferMaterial.Load(texel);
    float3 emissive    = gGBufferEmissive.Load(texel).rgb;

    float4 diffuseAlbedo = float4(albedoTex.rgb, 1.0f);
    float  shininess     = albedoTex.a;
    float3 fresnelR0     = materialTex.rgb;
    float  flag          = materialTex.a;

    float3 posW = ReconstructWorldPos(pin.TexC, depth);

    // G-Buffer 시각화
    if (gDebugMode == 1) return float4(normalW * 0.5f + 0.5f, 1.0f);
    if (gDebugMode == 3) return float4(diffuseAlbedo.rgb, 1.0f);
    if (gDebugMode == 4) return float4(shininess.xxx, 1.0f);
    if (gDebugMode == 5) return float4(abs(emissive) * 5.0f, 1.0f);     // 0.05~0.1 짜리 틴트가 보이게 확대

    // 조명 없는 픽셀 (경계선): 발광 색만
    if (flag > 0.75f)
        return float4(emissive, 1.0f);

    bool noShadow = (flag > 0.25f);     // PosNor: 그림자, 반사 없음

    // ---- 여기부터 Default.hlsl Forward PS 와 같은 계산 ----
    float3 toEyeW = normalize(gEyePosW - posW);
    float4 ambient = gAmbientLight * diffuseAlbedo;

    // 그림자: Forward 는 VS 에서 ShadowPosH 를 만들어 보간했지만, 여기서는 복원한 월드 위치에 바로 곱한다
    float3 shadowFactor = float3(1.0f, 1.0f, 1.0f);
    if (!noShadow)
        shadowFactor[0] = CalcShadowFactor(mul(float4(posW, 1.0f), gShadowTransform));

    if (gDebugMode == 2)
        return float4(shadowFactor[0].xxx, 1.0f);

    Material mat = { diffuseAlbedo, fresnelR0, shininess };
    float4 directLight = ComputeLighting(gLights, mat, posW, normalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;

    if (!noShadow)
    {
        float3 r = reflect(-toEyeW, normalW);
        float4 reflectionColor = gCubeMap.Sample(gsamLinearWrap, r);
        float3 fresnelFactor = SchlickFresnel(fresnelR0, normalW, r);
        litColor.rgb += shininess * fresnelFactor * reflectionColor.rgb;
    }

    litColor.rgb += emissive;
    return float4(litColor.rgb, 1.0f);
}
