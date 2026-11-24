//=============================================================================
// DeferredLighting.hlsl
// Deferred 라이팅. 루트시그니처는 DeferredLightingRS
//   slot 0 : t0~t4 space2  G-Buffer 4장 + 깊이   (힙 엔진 칸 4~8)
//   slot 1 : b1            PassCB (조명, 카메라, 역 ViewProj, 그림자 행렬)
//   slot 2 : t0 space1     머티리얼 버퍼 (안 씀. Set_CurrentFramePBMats 자리 맞춤)
//   slot 3 : t0            큐브맵 (반사)
//   slot 4 : t1            그림자맵
//   slot 5 : b2            gDebugMode (0 조명, 1 노멀, 2 그림자, 3 알베도, 4 광택, 5 발광, 6 깊이)
//   slot 6 : b3            gPointLightCount  (Common.hlsl)
//   slot 7 : t2 space1     gPointLights      (Common.hlsl, Forward 와 같은 버퍼)
//
// 한 파일에 패스 두 개 (CShader 가 진입점을 VS/PS 로 고정하므로 #define 으로 고른다)
//   (없음)              : 방향광 라이팅 패스. 풀스크린 삼각형 1번 (+ 경로 1 이면 점광원 전체 루프)
//   POINT_LIGHT_VOLUME  : 점광원 영역 패스. 점광원 하나 = 인스턴스 하나 = 화면 사각형. 가산 블렌딩
//
// Common.hlsl 을 그대로 include 해서 Forward 와 똑같은 조명 함수(ComputeLighting, ComputePointLight,
// CalcShadowFactor, SchlickFresnelRoughness)를 쓴다. 다른 점은 위치와 노멀을 G-Buffer 에서 얻는다는 것뿐
//=============================================================================

#include "Common.hlsl"

// space2: Common.hlsl 의 t0(큐브맵), t1(그림자맵), t2~(gTextureMaps) 와 겹치지 않게
Texture2D gGBufferAlbedo   : register(t0, space2);
Texture2D gGBufferNormal   : register(t1, space2);
Texture2D gGBufferMaterial : register(t2, space2);
Texture2D gGBufferEmissive : register(t3, space2);
Texture2D gDepthMap        : register(t4, space2);

// 화면 uv + 깊이 → 월드 위치
// 기하 패스: 월드 → (x ViewProj) → 클립 → (÷ w) → NDC (x, y, 깊이)
// 여기서는 거꾸로: NDC → (x 역 ViewProj) → 동차 좌표 → (÷ w) → 월드
// uv 는 아래로 증가, NDC y 는 위로 증가 → y = 1 - 2v
float3 ReconstructWorldPos(float2 uv, float depth)
{
    float4 ndc = float4(uv.x * 2.0f - 1.0f, 1.0f - uv.y * 2.0f, depth, 1.0f);
    float4 posW = mul(ndc, gInvViewProj);
    return posW.xyz / posW.w;
}

#if defined(POINT_LIGHT_VOLUME)
//=============================================================================
// 점광원 영역 패스
// 점광원의 영향 범위는 반경 FalloffEnd 인 구. 그 구가 화면에서 덮는 사각형만 그린다
//   → 화면 구석의 작은 폭발 조명은 픽셀 수백 개만 계산하고 끝난다 (풀스크린 루프는 모든 픽셀이 모든 조명을 돈다)
// 사각형 안이라도 깊이 방향으로 구 밖(훨씬 앞이나 뒤)일 수 있어서 PS 에서 거리 검사를 한 번 더 한다
//   사각형 = 후보를 줄이는 1차 필터, 거리 검사 = 최종 판정
// (구 메시를 그리는 방식보다 단순하다: 카메라가 구 안에 들어가는 경우나 깊이 버퍼 읽기 전용 바인딩을 따로 신경 쓸 필요가 없다.
//  대신 구 바깥 모서리 픽셀까지 PS 가 돈다. 화면을 타일로 나눠 Compute 로 조명 목록을 만드는 Tiled 방식은 9주차 Compute 와 함께)
//=============================================================================

struct LightVolumeOut
{
    float4 PosH       : SV_POSITION;
    nointerpolation uint LightIndex : LIGHTINDEX;
};

LightVolumeOut VS(uint vid : SV_VertexID, uint iid : SV_InstanceID)
{
    LightVolumeOut vout;
    vout.LightIndex = iid;

    Light L = gPointLights[iid];
    float  radius = L.FalloffEnd;
    float3 centerV = mul(float4(L.Position, 1.0f), gView).xyz;    // 뷰 공간 (카메라 원점, +z 가 앞)

    float2 minNdc = float2(-1.0f, -1.0f);
    float2 maxNdc = float2( 1.0f,  1.0f);

    if (centerV.z + radius < 0.1f)
    {
        // 구 전체가 카메라 뒤 → 넓이 0 인 사각형 (래스터라이저가 버린다)
        minNdc = maxNdc = float2(2.0f, 2.0f);
    }
    else if (centerV.z - radius > 0.1f)
    {
        // 구 전체가 카메라 앞: 구를 감싸는 뷰 공간 상자의 꼭짓점 8개를 투영해 화면 사각형을 구한다 (보수적, 실제 원보다 조금 크다)
        // 카메라가 구 안이거나 구가 카메라 평면에 걸치면(else) w 가 0 이하인 꼭짓점이 생겨 투영이 뒤집히므로 그냥 화면 전체
        minNdc = float2( 1e9f,  1e9f);
        maxNdc = float2(-1e9f, -1e9f);
        [unroll]
        for (int k = 0; k < 8; ++k)
        {
            float3 corner = centerV + radius * float3((k & 1) ? 1.0f : -1.0f,
                                                      (k & 2) ? 1.0f : -1.0f,
                                                      (k & 4) ? 1.0f : -1.0f);
            float4 clip = mul(float4(corner, 1.0f), gProj);
            float2 ndc = clip.xy / clip.w;
            minNdc = min(minNdc, ndc);
            maxNdc = max(maxNdc, ndc);
        }
        minNdc = clamp(minNdc, -1.0f, 1.0f);
        maxNdc = clamp(maxNdc, -1.0f, 1.0f);
    }

    // 사각형 = 삼각형 2개 (정점 6개, 버퍼 없이 SV_VertexID 로)
    const float2 kCorners[6] = { float2(0, 0), float2(1, 0), float2(0, 1),
                                 float2(0, 1), float2(1, 0), float2(1, 1) };
    float2 ndcPos = lerp(minNdc, maxNdc, kCorners[vid]);
    vout.PosH = float4(ndcPos, 0.0f, 1.0f);
    return vout;
}

float4 PS(LightVolumeOut pin) : SV_Target
{
    int3 texel = int3(pin.PosH.xy, 0);
    float depth = gDepthMap.Load(texel).r;
    if (depth >= 1.0f)
        discard;                                // 하늘

    // 위치를 먼저 복원해 반경 밖이면 나머지 G-Buffer 는 읽지도 않는다
    float2 uv = pin.PosH.xy * gInvRenderTargetSize;
    float3 posW = ReconstructWorldPos(uv, depth);

    Light L = gPointLights[pin.LightIndex];
    float3 toLight = L.Position - posW;
    if (dot(toLight, toLight) > L.FalloffEnd * L.FalloffEnd)
        discard;

    float4 materialTex = gGBufferMaterial.Load(texel);
    if (materialTex.a > 0.75f)
        discard;                                // 조명 없는 픽셀 (경계선)

    float4 albedoTex = gGBufferAlbedo.Load(texel);
    float3 normalW   = gGBufferNormal.Load(texel).xyz;

    Material mat = { float4(albedoTex.rgb, 1.0f), materialTex.rgb, albedoTex.a };
    float3 toEyeW = normalize(gEyePosW - posW);

    return float4(ComputePointLight(L, mat, posW, normalW, toEyeW), 0.0f);
}

#else
//=============================================================================
// 방향광 라이팅 패스 (풀스크린 삼각형 1번)
//=============================================================================

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

    // 점광원 (렌더 경로 1 에서만 개수 > 0): 픽셀마다 모든 점광원을 돈다. 반경 밖이면 ComputePointLight 가 바로 0
    // 비용 = 화면 픽셀 수 x 점광원 수. 물체 수와는 상관없다
    litColor.rgb += AccumulatePointLights(mat, posW, normalW, toEyeW);

    if (!noShadow)
    {
        float3 r = reflect(-toEyeW, normalW);
        float4 reflectionColor = gCubeMap.Sample(gsamLinearWrap, r);
        reflectionColor.rgb = SRGBToLinear(reflectionColor.rgb);   // 큐브맵도 sRGB 텍스처
        float3 fresnelFactor = SchlickFresnelRoughness(fresnelR0, normalW, r, shininess);   // 거친 표면은 스치는 각도에서도 거울이 되지 않게
        litColor.rgb += shininess * fresnelFactor * reflectionColor.rgb;
    }

    litColor.rgb += emissive;
    return float4(litColor.rgb, 1.0f);
}

#endif
