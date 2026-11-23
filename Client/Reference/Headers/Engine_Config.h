#pragma once

/*===========================================================================
    빌드/성능 튜닝 스위치를 한 곳에 모아둔 헤더.
    여기 값만 바꾸면 되도록 유지할 것. (Engine 을 고쳤으면 UpdateLib.bat 실행)
===========================================================================*/


/*---------------------------------------------------------------------------
    assimp(FBX 로더) 사용 여부

    1 : .bin 모델 파일이 없으면 FBX 를 assimp 로 읽어서 .bin 을 구워낸다.
        (개발용. 모델을 바꿨거나 .bin 이 아직 없을 때 한 번만 필요하다)
    0 : assimp 코드가 통째로 컴파일에서 빠진다. .bin 만 읽는다.
        (배포용 / 노트북용. assimp DLL 도 필요 없어진다)

    ※ 0 으로 바꾸기 전에 반드시 1 로 한 번 실행해서 .bin 을 만들어 둘 것.
    ※ 이 값을 바꾸면 Engine 과 MainApp 을 둘 다 리빌드해야 한다.
      (헤더의 멤버 구성이 달라지므로 한쪽만 빌드하면 크래시난다)
---------------------------------------------------------------------------*/
#define USE_ASSIMP_BAKE     0


/*---------------------------------------------------------------------------
    지형(Terrain)

    TERRAIN_MAP_SIZE    : 높이맵 한 변의 샘플 수. 월드 크기와 1:1 (4096 x 4096)
    TERRAIN_VERTEX_STEP : 몇 샘플마다 정점을 하나 놓을지.
                          4 → 정점 1024x1024, 정점버퍼 약 50MB  (노트북 권장)
                          2 → 정점 2048x2048, 정점버퍼 약 200MB (데스크탑)
                          1 → 예전과 동일(4096x4096, 740MB). 쓰지 말 것.
    TERRAIN_CHUNK_CELLS : 청크 한 변의 셀 수. 컬링/LOD 단위.
                          STEP=4, CELLS=64 → 청크 하나가 월드 256칸, 총 16x16=256개
    TERRAIN_LOD_COUNT   : LOD 단계 수. 단계마다 인덱스 간격이 2배가 된다(1,2,4,8,16)
    TERRAIN_SKIRT_DEPTH : LOD 경계의 틈(크랙)을 가리는 치마의 깊이(월드 단위)

    ※ 치마는 '이웃 청크와 LOD 가 다른 변'에만 그린다. 모든 변에 그리면 경계마다
      벽이 서서 바닥에 격자 무늬 선이 보인다(실제로 그랬다).
    ※ 이 맵의 높낮이는 17.4 ~ 68.5, 즉 전체 기복이 51칸뿐이다. 실측한 LOD 단계별
      최대 크랙은 7.3 / 12.7 / 16.2 / 21.1 / 23.3 칸이라 24 면 충분하고도 남는다.
---------------------------------------------------------------------------*/
#define TERRAIN_MAP_SIZE        4096
#define TERRAIN_VERTEX_STEP     4
#define TERRAIN_CHUNK_CELLS     64
#define TERRAIN_LOD_COUNT       5
#define TERRAIN_SKIRT_DEPTH     24.f


/*---------------------------------------------------------------------------
    그림자 맵 해상도

    그림자는 탱크 주변 600 x 600 범위만 찍는다(ShadowMap.cpp 의 ortho 300).
    8192 는 텍셀당 0.07칸이라 과했고, 깊이버퍼만 256MB 를 먹었다.
    4096 이면 텍셀당 0.15칸이고 깊이버퍼는 64MB, 픽셀 처리량은 1/4 이다.

    ※ 2048 까지 내렸다가 되돌렸다. 텍셀이 0.29칸이 되니 지형이 자기 자신에게
      그림자를 지면서(shadow acne) 탱크 주변 바닥이 거무튀튀해지고 줄무늬가 생겼다.
      "가까운 데만 바닥에 선이 보인다" 는 게 정확히 이거였다 - 그림자맵이 덮는
      범위가 딱 탱크 주변이라서. 더 내리려면 PSO.cpp 의 setForShadow() 에 있는
      DepthBias(10000) / SlopeScaledDepthBias(1.0) 도 같이 올려야 한다.
---------------------------------------------------------------------------*/
#define SHADOWMAP_SIZE          8192

/*---------------------------------------------------------------------------
    디스크립터 힙 구간 (2주차 Bloom 준비)

    SRV 힙 (shader-visible, 하나만 바인딩 가능하므로 구간을 나눠 쓴다)
      [0 ~ SRV_ENGINE_RESERVED-1]   엔진 전용: 그림자맵, HDR, Bloom ...
      [SRV_ENGINE_RESERVED ~ ]      TextureMgr 텍스처. 텍스처 인덱스 0 = 분홍 기본 텍스처
      힙 칸 = SRV_ENGINE_RESERVED + 텍스처 인덱스
      slot 5 테이블은 SRV_ENGINE_RESERVED 칸부터 바인딩 → gTextureMaps[i] = 텍스처 인덱스 i
      MAX_TEXTURE_COUNT 는 Common.hlsl 의 gTextureMaps[500] 배열 크기와 같아야 한다

    RTV 힙 (셰이더가 안 읽으므로 SetDescriptorHeaps 와 무관)
      [0 ~ SwapChainBufferCount-1]  백버퍼
      [그 뒤 RTV_ENGINE_RESERVED 칸] 엔진 오프스크린 RT: HDR 씬, Bloom 핑퐁 ...
---------------------------------------------------------------------------*/
#define SRV_ENGINE_RESERVED     16
#define MAX_TEXTURE_COUNT       500
#define SRV_HEAP_SIZE           (SRV_ENGINE_RESERVED + MAX_TEXTURE_COUNT)

// 엔진 전용 SRV 칸 번호 (0 ~ SRV_ENGINE_RESERVED-1)
#define SRV_SLOT_SHADOWMAP      0
#define SRV_SLOT_HDR            1

#define RTV_ENGINE_RESERVED     4

// 엔진 전용 RTV 칸 번호 (0 ~ RTV_ENGINE_RESERVED-1, 백버퍼 뒤 위치)
#define RTV_SLOT_HDR            0

// HDR 씬 렌더타겟 포맷. 1.0을 넘는 밝기를 그대로 담아 Bloom bright pass가 골라낼 수 있게 한다
#define HDR_FORMAT              DXGI_FORMAT_R16G16B16A16_FLOAT
