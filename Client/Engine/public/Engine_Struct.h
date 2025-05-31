#pragma once
#include "Engine_Macro.h"
#include <d3d12.h>
#include <DirectXMath.h>
#include <assimp/material.h>

#define MaxLights 16

using namespace DirectX;

static const int gNumFrameResources = 3;

namespace Engine
{

	struct ENGINE_DLL ObjectConstants
	{
		XMFLOAT4X4 World = XMFLOAT4X4{};
		XMFLOAT4X4 TexTransform = XMFLOAT4X4{};
		UINT     MaterialIndex = 0;
		UINT     ObjPad0 = 0;
		UINT     ObjPad1 = 0;
		UINT     ObjPad2 = 0;
	};

	struct ENGINE_DLL Light
	{
		DirectX::XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f };
		float FalloffStart = 1.0f;                          // point/spot light only
		DirectX::XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f };// directional/spot light only
		float FalloffEnd = 10.0f;                           // point/spot light only
		DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };  // point/spot light only
		float SpotPower = 64.0f;                            // spot light only
	};

	struct ENGINE_DLL PassConstants
	{
		DirectX::XMFLOAT4X4 View = XMFLOAT4X4{};
		DirectX::XMFLOAT4X4 InvView = XMFLOAT4X4{};
		DirectX::XMFLOAT4X4 Proj = XMFLOAT4X4{};
		DirectX::XMFLOAT4X4 InvProj = XMFLOAT4X4{};
		DirectX::XMFLOAT4X4 ViewProj = XMFLOAT4X4{};
		DirectX::XMFLOAT4X4 InvViewProj = XMFLOAT4X4{};
		DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
		float cbPerObjectPad1 = 0.0f;
		DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
		DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
		float NearZ = 0.0f;
		float FarZ = 0.0f;
		float TotalTime = 0.0f;
		float DeltaTime = 0.0f;

		DirectX::XMFLOAT4 AmbientLight = { 1.0f, 1.0f, 1.0f, 1.0f };

		// Indices [0, NUM_DIR_LIGHTS) are directional lights;
		// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
		// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
		// are spot lights for a maximum of MaxLights per object.
		Light Lights[MaxLights];
	};


	struct ENGINE_DLL MaterialConstants
	{
		// 이 메터리얼에 해당되는 상수 버퍼의 인덱스입니다.
		int MatCBIndex;

		// 디퓨즈 텍스쳐에 해당하는 SRV 힙의 인덱스입니다.
		int DiffuseSrvHeapIndex = -1;

		// 노말 텍스쳐에 해당하는 SRV 힙의 인덱스입니다.
		int NormalSrvHeapIndex = -1;

		// 메터리얼이 변경되었다는 것을 나타내는 더티 플래그입니다.
		// 그리고 변경됬을 경우 상수 버퍼를 업데이트 해야합니다.
		// 메터리얼 상수 버퍼는 매 프레임마다 존재하기 때문에 모든 프레임 리소스를
		// 업데이트 해야합니다. 그러므로 메터리얼이 변경됬을 때 NumFramesDirty = gNumFrameResources로
		// 설정해서 모든 프레임 리소스가 업데이트 되도록 해야합니다.
		int NumFramesDirty = gNumFrameResources;

		// 셰이딩에 사용되는 메터리얼 상수 버퍼 데이터입니다.
		DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
		float Roughness = 0.25f;
		DirectX::XMFLOAT4X4 MatTransform = XMFLOAT4X4{};
	};

	struct ENGINE_DLL MaterialData
	{
		DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
		float Roughness = 0.5f;

		// Used in texture mapping.
		DirectX::XMFLOAT4X4 MatTransform = XMFLOAT4X4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);

		UINT DiffuseMapIndex = 0;
		UINT NormalMapIndex = 0;
		UINT MaterialPad1;
		UINT MaterialPad2;
	};

}

namespace Engine {
	typedef struct tagModelMaterial
	{
		class CTexture* pMaterials[AI_TEXTURE_TYPE_MAX];
	}MODEL_MATERIAL;

	typedef struct tagKeyFrame
	{
		float			fTime;
		XMFLOAT4		vScale;
		XMFLOAT4		vRotation;
		XMFLOAT4		vTranslation;
	}KEYFRAME;
}

namespace Engine
{
	typedef struct ENGINE_DLL tagVertex_Position
	{
		XMFLOAT3		vPosition;

		static const unsigned int iNumElements = 1;
		static const D3D12_INPUT_ELEMENT_DESC	VertexElements[iNumElements];

	}VTXPOS;

	typedef struct ENGINE_DLL tagVertex_Position_Normal
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vNormal;

		static const unsigned int iNumElements = 2;
		static const D3D12_INPUT_ELEMENT_DESC	VertexElements[iNumElements];

	}VTXPOSNOR;

	typedef struct ENGINE_DLL tagVertex_Position_Texcoord
	{
		XMFLOAT3		vPosition;
		XMFLOAT2		vTexcoord;

		static const unsigned int iNumElements = 2;
		static const D3D12_INPUT_ELEMENT_DESC	VertexElements[iNumElements];

	}VTXPOSTEX;

	typedef struct ENGINE_DLL tagVertex_Position_Normal_Texcoord
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vNormal;
		XMFLOAT2		vTexcoord;

		static const unsigned int iNumElements = 3;
		static const D3D12_INPUT_ELEMENT_DESC	VertexElements[iNumElements];

	}VTXNORTEX;

	typedef struct ENGINE_DLL tagVertex_Position_Cube_Texcoord
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vTexcoord;

		static const unsigned int iNumElements = 2;
		static const D3D12_INPUT_ELEMENT_DESC	VertexElements[iNumElements];

	}VTXCUBETEX;

	typedef struct ENGINE_DLL tagVertex_Mesh
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vNormal;
		XMFLOAT2		vTexcoord;
		XMFLOAT3		vTangent;

		static const unsigned int iNumElements = 4;
		static const D3D12_INPUT_ELEMENT_DESC	VertexElements[iNumElements];

	}VTXMESH;

	typedef struct ENGINE_DLL tagVertex_Anim_Mesh
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vNormal;
		XMFLOAT2		vTexcoord;
		XMFLOAT3		vTangent;

		XMUINT4			vBlendIndices;
		XMFLOAT4		vBlendWeights;

		static const unsigned int iNumElements = 6;
		static const D3D12_INPUT_ELEMENT_DESC	VertexElements[iNumElements];

	}VTXANIMMESH;

}