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
//=============================================================================

Texture2D gInput : register(t0);

// 루트 상수 (PostProcessRS slot 1, 32비트 4개)
//   bright pass : x = 휘도 임계값
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
