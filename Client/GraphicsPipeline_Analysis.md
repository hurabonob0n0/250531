# 그래픽스 파이프라인 분석 & 개선 로드맵

> 대상: `Client/Engine` (자체 DX12 엔진 DLL) + `Client/MainApp` (게임)
> 작성 기준: 현재 `main` 브랜치 코드 정독 결과

---

## 0. 한 줄 요약

**Frank Luna DX12 교과서 렌더러 + 지형 LOD/궤도 변형 커스텀** 상태.
기반은 탄탄하나 화면이 밋밋한 이유는 명확함 → **LDR + AA 없음 + 포스트프로세싱 없음**.
DX12 셋업에 **죽은 코드 / 잘못된 위치의 설정 / 헷갈리는 디스크립터 힙 레이아웃**이 섞여 있음(동작은 함).

---

## 1. 전체 아키텍처 (레이어)

```
MainApp (게임 로직)
  └─ CGameInstance  ── 엔진 파사드(싱글톤). 아래 매니저들을 소유/중계
       ├─ CGraphic_Dev        : Device / SwapChain / CmdQueue / RTV·DSV 힙 / Present
       ├─ CFrameResourceMgr   : ★ 실제 프레임 동기화(Fence) + 프레임별 CmdAllocator + 업로드 CB
       ├─ CRenderer           : 렌더 그룹 수집 → 패스 순서대로 Draw
       ├─ CObjectMgr          : GameObject 레이어 관리(문자열 키 + 인덱스)
       ├─ CTextureMgr         : ★ 단일 SRV 힙(500) = 바인드리스 텍스처 배열
       ├─ CMaterialMgr        : StructuredBuffer<MaterialData> 업로드
       ├─ CPSOMgr / CRootSignatureMgr : 이름→PSO / 이름→RootSignature 맵
       └─ CShadowMap          : 방향광 섀도우맵(4096) + 자체 SRV/DSV
```

핵심: **CGameInstance가 모든 매니저를 들고 있는 God-object 파사드**. 렌더 상태(디스크립터 힙, 섀도우맵 등)를 `Renderer`가 `m_GameInstance->...`로 직접 꺼내 씀.

---

## 2. 프레임 흐름 (한 프레임에 실제로 일어나는 순서)

`MainApp::Run()` (`MainApp.cpp:435`) 루프 안:

```
Update(Timer)
  └─ GameInstance::Update
       ├─ FrameResourceMgr::BeginFrame()      // ★ CurrentFrameIndex++ → 그 프레임 Fence 대기
       └─ ObjectMgr::Update                    // CPU: 트랜스폼/CB 데이터 갱신
PhysX::Update
Late_Update(Timer)
  └─ GameInstance::Late_Update
       ├─ ShadowMap::Late_Update               // 광원 뷰/프로젝션 갱신
       ├─ ObjectMgr::LateUpdate                // 렌더 그룹에 등록(AddtoRenderObjects)
       └─ MaterialMgr::Update_Mats             // 머티리얼 StructuredBuffer 업로드
Draw()
  └─ GameInstance::Draw
       ├─ Renderer::Render()                   // ↓ 3장 참조: 여기서 CmdList Reset+기록+Present
       ├─ FrameResourceMgr::SignalAndAdvance() // ★ Signal(fence) + 이 프레임 fence값 저장
       └─ Renderer::ResetRenderObjects()
```

> **주의점**: `BeginFrame()`(펜스 대기)은 `Update`에서, `Reset_CommandList_and_Allocator`는 `Renderer::Render()` 맨 앞(섀도우 패스)에서 호출됨. 즉 커맨드 기록 리셋과 펜스 대기 지점이 떨어져 있음. 동작은 하지만 흐름이 한눈에 안 들어옴.

---

## 3. ★ 디스크립터 힙 / 루트 시그니처 / 바인딩 모델 (핵심)

### 3-1. 디스크립터 힙 구성

| 힙 | 타입 | 개수 | Shader Visible | 위치 |
|----|------|------|----------------|------|
| RTV 힙 | RTV | 2 (백버퍼) | - | `Graphic_Dev` |
| DSV 힙 | DSV | 2 | - | `Graphic_Dev` |
| **SRV 힙** | CBV_SRV_UAV | **500** | ✅ | `TextureMgr::Make_DescriptorHeap` |
| 섀도우 DSV | DSV | 1 | - | `ShadowMap` (자체) |

**"한꺼번에 올려놓고 쓰는" 설계는 실제로 구현돼 있음** → 셰이더 가시 SRV 힙 1장(500 슬롯)에 전부 올려두고, 셰이더에서 인덱스로 동적 접근(`gTextureMaps[diffuseMapIndex]`). 바인드리스 의도 = 실현됨.

### 3-2. SRV 힙 슬롯 레이아웃 (중요 — 헷갈리는 부분)

```
slot 0 : ShadowMap SRV      ← TextureMgr::Add_ShadowMap()가 '힙 시작(offset 0)'에 씀
slot 1 : 텍스처 #1           ← Add_Texture()는 newIndex(1-based)에 씀
slot 2 : 텍스처 #2
 ...
slot N : 텍스처 #N (스카이 큐브맵 포함 — 큐브맵도 이 힙에 같이 들어감)
```

- 텍스처 인덱스가 **1-based**라, 머티리얼의 `DiffuseMapIndex`도 1부터 시작.
- **슬롯 0을 섀도우맵과 텍스처 배열이 공유**함: 아래 table 5가 힙 시작(=slot 0)부터 500개를 잡으므로 `gTextureMaps[0]` == 섀도우맵. 텍스처가 1-based라 충돌은 안 나지만 **레이아웃이 직관적이지 않음**.

### 3-3. 루트 시그니처 "DefaultRS" (`RootSignature.cpp:84`)

| 파라미터 | 종류 | 레지스터 | 내용 | 바인딩 위치 |
|---------|------|---------|------|------------|
| 0 | Root CBV | b0 | ObjectCB (per-object) | `CBBinding.cpp:73` |
| 1 | Root CBV | b1 | PassCB (per-frame) | `FrameResource.cpp:23` |
| 2 | Root SRV | t0,space1 | `StructuredBuffer<MaterialData>` | `FrameResource.cpp:28`, `MaterialMgr.cpp:120` |
| 3 | **Table** SRV | t0,space0 (1개) | `gCubeMap` (스카이) | **Camera_Free/Drone.cpp** (offset=스카이 머티리얼 인덱스) |
| 4 | **Table** SRV | t1,space0 (1개) | `gShadowMap` | `Renderer.cpp:114` (= `m_ShadowMap->mhGpuSrv`) |
| 5 | **Table** SRV | t2,space0 (**500개**) | `gTextureMaps[500]` (바인드리스) | `Renderer.cpp:89,115` (= 힙 시작) |
| 6 | Root SRV | t1,space1 | `StructuredBuffer<InstanceData>` | `FrameResource.cpp:33` |

- **Table 3 (큐브맵)은 카메라 오브젝트가 렌더할 때만 바인딩**됨(스카이 그릴 때). 같은 SRV 힙 안에서 스카이 머티리얼 인덱스만큼 offset 해서 잡음.
- **Table 5는 힙 시작부터 500개**를 통째로 잡음 → 여기서 셰이더가 `gTextureMaps[머티리얼인덱스]`로 직접 인덱싱. **이게 바인드리스 핵심.**
- 정적 샘플러 6종(point/linear/aniso × wrap/clamp) + 섀도우 비교 샘플러 상시 등록 → 좋음.

### 3-4. 상수/구조화 버퍼

- `ObjectCB`(b0): per-object 월드/텍스처 변환 + 궤도 변형 파라미터(gTrackSag) — `Common.hlsl`
- `PassCB`(b1): 뷰/프로젝션/역행렬/섀도우 변환/카메라 위치/조명 16개
- `MaterialData`(t0,space1): 알베도/FresnelR0/러프니스/**DiffuseMapIndex/NormalMapIndex** → 텍스처 배열 인덱스
- `InstanceData`(t1,space1): 인스턴싱용 월드행렬 + 머티리얼 인덱스

---

## 4. 렌더 패스 구조 (`Renderer::Render`, `Renderer.cpp:82`)

```
[Shadow Pass]  PSO=ShadowPSO
   ShadowMap DSV로 전환 → depth-only 렌더(Render_NonBlend, 광원 시점)
[Normal Pass]  백버퍼+DSV
   SkyPSO        → Render_Priority   (스카이/카메라)
   PosNorPSO     → Render_NonLight
   DefaultPSO    → Render_NonBlend   (불투명 + 라이팅 + 섀도우)
   EffectPSO     → Render_Blend      (알파 이펙트)
   BulletPathPSO → Render_BulletPath (탄도)
   UIPSO         → Render_UI
   Present()
```

- 렌더 그룹: `RG_PRIORITY, RG_NONLIGHT, RG_NONBLEND, RG_BLEND, RG_UI, RG_BULLETPATH`
- `PassNum` 카운터로 섀도우/노멀 두 패스에서 NonBlend를 재사용(섀도우 때는 Release 안 함).
- **포스트프로세싱 스테이지 없음** → Present로 바로 나감.

---

## 5. 라이팅 / 셰이더 모델

- **Blinn-Phong** (LDR). `LightingUtil.hlsl`에 "LDR이라 스펙 스케일 다운" 명시.
- 방향광 3개(`NUM_DIR_LIGHTS 3`), **점광/스팟광 정의는 있으나 0개로 꺼둠**.
- 섀도우: 방향광 1장, PCF 3×3 (`CalcShadowFactor`).
- 노멀맵 O, 큐브맵 반사 O(조악 — 러프니스 기반 흐림 없음).
- 커스텀: **전차 궤도 변형**(VS에서 로드휠 서스펜션 따라 궤도 정점 이동, `Common.hlsl TrackSag`) — 잘 만듦.
- 지형: **LOD + 프러스텀 컬링 + 스커트**(`VIBuffer_Terrain.cpp`) — 잘 만듦.

---

## 6. ★ DX12 설정 점검 결과 (발견된 이슈)

| # | 이슈 | 위치 | 심각도 | 설명/조치 |
|---|------|------|--------|-----------|
| 1 | **디버그 레이어를 종료 시점에 켬** | `Graphic_Dev.cpp:415~425` (`Free`) | 높음 | `EnableDebugLayer()`가 `Free()`에 있음 → 사실상 검증 0. **`D3D12CreateDevice` 전**으로 옮겨야 함. "설정이 잘 안 돼 있던" 대표 케이스. |
| 2 | **Graphic_Dev의 Fence/FlushCommandQueue 통째 주석** | `Graphic_Dev.cpp:21, 241~264` | 중(혼란) | 실제 동기화는 `FrameResourceMgr`가 함(정상). 죽은 코드/멤버 제거해 혼란 제거. |
| 3 | **MSAA 미사용** | `Graphic_Dev.h:60` `m4xMsaaState=false` | 중 | 품질레벨 조회만 하고 안 씀. AA 전무 → 계단현상 노출. |
| 4 | **백버퍼 LDR** | `Graphic_Dev.h:87` `R8G8B8A8_UNORM` | 중 | HDR 불가 → 블룸/톤매핑 하려면 오프스크린 FP16 필요. |
| 5 | **OnResize 실질 미호출** | `MainApp.cpp:528` WM_SIZE | 낮 | 렌더 해상도 1920×1080 고정이라 티는 안 남. 창 리사이즈시 깊이버퍼/뷰포트 정합성 주의. |
| 6 | **SRV 힙 슬롯 0 공유** | `TextureMgr.cpp:120,158` | 낮(설계) | 섀도우맵(slot0) + 텍스처배열(0부터 500)이 겹침. 1-based라 충돌은 없지만 레이아웃 재정리 권장. |
| 7 | **루트 시그니처 주석 불일치** | `RootSignature.cpp:101` | 낮 | `[10] (t1~t10)` 낡은 주석, 실제는 500개 t2. |
| 8 | **깊이버퍼 typeless 준비만** | `Graphic_Dev.cpp:186~191` | 정보 | SSAO용 SRV 대비 주석 있으나 SSAO 미구현. |
| 9 | **Present(0,0) = VSync off** | `Graphic_Dev.cpp:393` | 정보 | 티어링 가능. 옵션화 여지. |

> **정리하면**: 렌더링은 정상 동작하나, ① 디버그 레이어 위치, ② 죽은 Fence 코드는 반드시 정리 권장. 나머지는 아래 로드맵과 함께 개선.

---

## 7. 개선 / 추가 로드맵 (그래픽 프로그래머 작업)

시각적 임팩트 대비 노력 순.

### Tier 1 — 가성비 최고 (기반)
1. **포스트프로세싱 프레임워크** ★최우선
   씬을 오프스크린 RT에 렌더 → 풀스크린 삼각형 PS 후처리 → 백버퍼 복사.
   위치: `Renderer::Render()` Present 직전(`Renderer.cpp:136`)에 포스트 스테이지 삽입.
   이게 있어야 아래 블룸/톤매핑/비네트/컬러그레이딩이 전부 열림.
2. **HDR + 톤매핑 + 블룸**
   씬 RT를 `R16G16B16A16_FLOAT`로 → 브라이트패스 → 가우시안 블러 → ACES 톤매핑 → LDR.
   포격/머즐플래시/폭발이 빛나서 즉각적인 비주얼 상승.
3. **안티앨리어싱 (FXAA 또는 MSAA)**
   포스트 프레임워크만 있으면 FXAA 한 패스가 제일 쌈. 계단현상 제거.

### Tier 2 — 전차 게임에 딱
4. **SSAO** — Luna 책 그대로 이식 가능(깊이버퍼 typeless 준비도 돼 있음). 전차 접지감.
5. **캐스케이드 섀도우맵(CSM)** — 넓은 야외 전장에 섀도우 1장은 부족. 2~3단으로.
   위치: `ShadowMap`을 배열로 확장, `Renderer` 섀도우 패스 루프화.
6. **PBR(메탈릭-러프니스) + IBL** — 이미 Roughness/FresnelR0 있음. 스카이 큐브맵을 IBL로. 포트폴리오 강점.

### Tier 3 — 전투 이펙트
7. **GPU 파티클 시스템** ★ — 현재 연기는 `AddObject("SmokeEffect")` 3개씩 CPU 생성(`ClientPacketHandler.cpp:371~377`). 빌보드/컴퓨트 파티클로 교체 → 머즐스모크/폭발/궤도 먼지/파편.
8. **동적 라이트(머즐 플래시/폭발 섬광)** — 셰이더 점광 경로 이미 있음(꺼져 있을 뿐). 켜서 활용.
9. **데칼(탄흔/그을음/궤도 자국)** — `S_WEAPON_HIT` 위치 이미 수신. 투영 데칼.
10. **피격 화면 효과(데미지 비네트/조준경 DoF)** — `Handle_S_DAMAGED_TANK`에 셰이크 이미 있음 → 포스트 효과 추가.

### Tier 4 — 최적화 / 엔지니어링 깊이
11. **오브젝트 단위 프러스텀 컬링** — 현재 지형만 컬링. 나무/전차 등 개별 컬링 없음.
12. **나무/식생 GPU 인스턴싱** — `InstanceData` 구조화 버퍼 이미 존재.
13. **깊이 프리패스** — 오버드로우 감소.

### 추천 순서
`1 포스트 프레임워크 → 2 HDR/블룸 → 3 FXAA → 7 GPU파티클 + 8 동적라이트 → (여유) 4 SSAO / 5 CSM → 6 PBR`

이 순서로 하면 "교과서 렌더러" → "직접 구축한 HDR 포스트프로세싱 + GPU 파티클 + PBR" 로 **포트폴리오 서사 완성**.
