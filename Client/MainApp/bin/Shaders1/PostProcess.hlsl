//=============================================================================
// PostProcess.hlsl
// 풀스크린 패스 공용. 루트시그니처는 PostProcessRS
//   slot 0 : t0  입력 텍스처 (HDR RT 등)
//   slot 1 : b0  루트 상수 4개 (gPostParams, 패스마다 의미가 다름)
//   정적 샘플러 s0~s6 은 Common.hlsl 과 같은 순서. 여기서는 필요한 것만 선언
// Common.hlsl 은 include 하지 않는다 (t0 = gCubeMap 등 DefaultRS 기준 레지스터가 겹친다)
//
// CShader 가 진입점 이름을 "VS"/"PS" 로 고정하므로, PS 는 #define 으로 골라 컴파일한다
//   (없음)        : HDR → 백버퍼 복사   (CopyToBackBufferPS)
//   BRIGHT_PASS   : Bloom bright pass   (BrightPassPS)
//   BLUR          : 분리형 가우시안 1방향 (BlurPS)
//   COMPOSITE     : HDR + Bloom 합성 + 톤매핑 → 백버퍼 (CompositePS)
//=============================================================================

Texture2D gInput  : register(t0);   // slot 0
Texture2D gInput2 : register(t1);   // slot 2 (합성 패스의 Bloom)

// 루트 상수 (PostProcessRS slot 1, 32비트 4개)
//   bright pass : x = 휘도 임계값
//   blur        : xy = 한 텍셀만큼의 uv 이동 방향 (가로 = (1/width, 0), 세로 = (0, 1/height))
//   composite   : x = Bloom 강도, y = 노출, z = 톤매핑 모드 (0 없음/saturate, 1 Reinhard, 2 ACES)
cbuffer cbPostProcess : register(b0)
{
    float4 gPostParams;
};

SamplerState gsamPointClamp  : register(s1);
SamplerState gsamLinearClamp : register(s3);

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float2 TexC : TEXCOORD;
};

// 풀스크린 삼각형: 정점 버퍼 없이 DrawInstanced(3, 1, 0, 0)
//   id 0 → uv (0,0) → NDC (-1, 1)  왼쪽 위
//   id 1 → uv (2,0) → NDC ( 3, 1)  화면 오른쪽 밖
//   id 2 → uv (0,2) → NDC (-1,-3)  화면 아래 밖
// 화면을 덮는 큰 삼각형 하나. 사각형(삼각형 2개)과 달리 대각선 경계에서 픽셀이 두 번 처리되지 않는다
// uv 의 v 는 아래로 증가, NDC 의 y 는 위로 증가 → y = 1 - 2v (그림자 T 행렬의 y -0.5 와 같은 이유)
VertexOut VS(uint vid : SV_VertexID)
{
    VertexOut vout;
    vout.TexC = float2((vid << 1) & 2, vid & 2);
    vout.PosH = float4(vout.TexC.x * 2.0f - 1.0f, 1.0f - vout.TexC.y * 2.0f, 0.0f, 1.0f);
    return vout;
}

#if defined(BRIGHT_PASS)

// Bloom bright pass: 휘도가 임계값을 넘는 부분만 남긴다. 출력 RT 는 절반 해상도
// 절반 해상도 픽셀 중심의 uv 는 원본 텍셀 2x2 의 공유 꼭짓점에 떨어진다
//   → 선형 샘플링 한 번이 2x2 평균(박스 필터)이 되어 다운샘플과 추출을 한 패스로 처리
float4 PS(VertexOut pin) : SV_Target
{
    float3 hdr = gInput.Sample(gsamLinearClamp, pin.TexC).rgb;

    // 사람 눈 기준 휘도 (Rec.709 가중치: 초록에 가장 민감)
    float luminance = dot(hdr, float3(0.2126f, 0.7152f, 0.0722f));

    // 임계값을 넘은 만큼만 비율로 남긴다. 색상(채도)은 유지하고 밝기만 줄인다
    // (색 채널마다 max(c - t, 0) 을 하면 색이 바래거나 틀어진다)
    float threshold = gPostParams.x;
    float contribution = max(luminance - threshold, 0.0f) / max(luminance, 1e-4f);

    return float4(hdr * contribution, 1.0f);
}

#elif defined(BLUR)

// 분리형 가우시안 블러 (한 방향). 가로 패스 → 세로 패스 두 번이면 2D 가우시안과 같다
//   2D 9x9 = 81 샘플 → 가로 9 + 세로 9 = 18 샘플 (가우시안은 x, y 로 분리되는 함수라 가능)
//
// 9탭 커널을 선형 샘플링으로 5번에 읽는다
//   9탭 가중치 = 이항계수 12번째 줄 가운데 (924, 792, 495, 220, 66) / 4070 (바깥 작은 값은 버리고 합이 1이 되게)
//             = 0.2270, 0.1946, 0.1216, 0.0541, 0.0162
//   이웃한 두 텍셀 사이를 가중치 비율대로 찍으면 하드웨어 선형 보간이 두 텍셀 가중합을 한 번에 해 준다
//   탭 1,2 → 오프셋 (1*0.1946 + 2*0.1216) / 0.3162 = 1.3846, 가중치 0.1946 + 0.1216 = 0.3162
//   탭 3,4 → 오프셋 (3*0.0541 + 4*0.0162) / 0.0703 = 3.2308, 가중치 0.0541 + 0.0162 = 0.0703
static const float kOffsets[3] = { 0.0f, 1.3846153846f, 3.2307692308f };
static const float kWeights[3] = { 0.2270270270f, 0.3162162162f, 0.0702702703f };

float4 PS(VertexOut pin) : SV_Target
{
    float2 texelStep = gPostParams.xy;

    float3 color = gInput.Sample(gsamLinearClamp, pin.TexC).rgb * kWeights[0];
    [unroll]
    for (int i = 1; i < 3; ++i)
    {
        float2 offset = texelStep * kOffsets[i];
        color += gInput.Sample(gsamLinearClamp, pin.TexC + offset).rgb * kWeights[i];
        color += gInput.Sample(gsamLinearClamp, pin.TexC - offset).rgb * kWeights[i];
    }
    return float4(color, 1.0f);
}

#elif defined(COMPOSITE)

// Reinhard: x / (1 + x). 어떤 큰 값도 1 미만으로 눌러 준다. 단순하지만 전체가 물빠진 듯 어두워지기 쉽다
float3 TonemapReinhard(float3 x)
{
    return x / (1.0f + x);
}

// ACES Filmic 근사 (Krzysztof Narkowicz 2015). 어두운 쪽은 살짝 누르고 중간은 대비를 올리고 밝은 쪽은 부드럽게 1로
// 영화용 ACES RRT+ODT 곡선을 다항식 비율로 맞춘 것. 입력에 0.6 정도를 곱해 쓰는 경우가 많지만 여기서는 노출로 조절
float3 TonemapACES(float3 x)
{
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

// 합성: 씬 HDR 에 Bloom(절반 해상도를 선형 샘플로 늘림)을 더한 뒤 노출 → 톤매핑 → 백버퍼(0~1)
// 톤매핑 전까지는 1.0 을 넘는 값을 그대로 유지해야 한다 (그래서 HDR RT, Bloom RT 모두 FLOAT 포맷)
float4 PS(VertexOut pin) : SV_Target
{
    float bloomIntensity = gPostParams.x;
    float exposure       = gPostParams.y;
    int   tonemapMode    = (int)gPostParams.z;

    float3 scene = gInput.Sample(gsamLinearClamp, pin.TexC).rgb;
    float3 bloom = gInput2.Sample(gsamLinearClamp, pin.TexC).rgb;

    float3 hdr = (scene + bloom * bloomIntensity) * exposure;

    float3 ldr;
    if (tonemapMode == 1)
        ldr = TonemapReinhard(hdr);
    else if (tonemapMode == 2)
        ldr = TonemapACES(hdr);
    else
        ldr = saturate(hdr);    // 톤매핑 없음: 1 넘는 값은 잘려서 하얗게 뭉친다

    return float4(ldr, 1.0f);
}

#else

// HDR → 백버퍼 복사 (임시). 백버퍼는 R8G8B8A8_UNORM 이라 1.0 을 넘는 값은 어차피 잘린다.
// 톤매핑 전 단계이므로 예전처럼 saturate 로 자르기만 해서 화면이 이전과 같아야 정상
// 입력과 출력 해상도가 같으면 선형 샘플링도 텍셀 중심을 정확히 집어 포인트와 결과가 같다.
// 선형으로 둔 이유: B 키 디버그 보기에서 절반 해상도 Bloom RT 를 화면 크기로 늘려 볼 때 덜 깍두기처럼 보이게
float4 PS(VertexOut pin) : SV_Target
{
    float3 hdr = gInput.Sample(gsamLinearClamp, pin.TexC).rgb;
    return float4(saturate(hdr), 1.0f);
}

#endif
