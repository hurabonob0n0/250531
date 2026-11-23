//=============================================================================
// PostProcess.hlsl
// 풀스크린 패스 공용. 루트시그니처는 PostProcessRS
//   slot 0 : t0  입력 텍스처 (HDR RT 등)
//   slot 1 : b0  루트 상수 4개 (아직 안 씀. Bloom 임계값, 블러 방향, 노출 등)
//   정적 샘플러 s0~s6 은 Common.hlsl 과 같은 순서. 여기서는 필요한 것만 선언
// Common.hlsl 은 include 하지 않는다 (t0 = gCubeMap 등 DefaultRS 기준 레지스터가 겹친다)
//=============================================================================

Texture2D gInput : register(t0);

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

// HDR → 백버퍼 복사 (임시). 백버퍼는 R8G8B8A8_UNORM 이라 1.0 을 넘는 값은 어차피 잘린다.
// 톤매핑 전 단계이므로 예전처럼 saturate 로 자르기만 해서 화면이 이전과 같아야 정상
// HDR RT 와 백버퍼 해상도가 같으므로 포인트 샘플링이면 텍셀 하나 = 픽셀 하나
float4 PS(VertexOut pin) : SV_Target
{
    float3 hdr = gInput.Sample(gsamPointClamp, pin.TexC).rgb;
    return float4(saturate(hdr), 1.0f);
}
