#include "Client_pch.h"
#include "SmokeEffect.h"
#include "GameInstance.h"
#include "Network_Manager.h"

CSmokeEffect::CSmokeEffect() : CRenderObject()
{
}

CSmokeEffect::CSmokeEffect(CSmokeEffect& rhs) : CRenderObject(rhs)
{
	MD = rhs.MD;
}

HRESULT CSmokeEffect::Initialize_Prototype()
{
	__super::Initialize_Prototype();

	MD.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("SMokeEffect_D", CTexture::Create(L"../bin/Models/Effect/SmokeEffect.dds"));
	MD.NormalMapIndex = 0;//m_GameInstance->Add_Texture("Effect_N", CTexture::Create(L"../bin/Models/Effect/Explode_8x8.dds"));
	XMStoreFloat4x4(&MD.MatTransform, XMMatrixIdentity());

	return S_OK;
}

HRESULT CSmokeEffect::Initialize(void* pArg)
{
	m_RG = CRenderer::RG_BLEND;

	__super::Initialize(pArg);

	m_CBBindingCom = (CBBinding*)m_GameInstance->Get_Component("CBBindingCom", nullptr);

	m_CBBindingCom->Set_MaterialIndex(m_GameInstance->Add_Material("EffectMat", MD));

	m_VIBuffer = (CVIBuffer_Quad*)m_GameInstance->Get_Component("VIBuffer_QuadCom");

	m_CameraTransform = (CTransform*)m_GameInstance->Get_Object_Component("Camera", 0, "TransformCom");

	m_DroneCamera = (CTransform*)m_GameInstance->Get_Object_Component("Camera", 1, "TransformCom");

	Safe_AddRef(m_CameraTransform);

	Safe_AddRef(m_DroneCamera);

	return S_OK;
}

void CSmokeEffect::Tick(float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	_vector vecreversepos;
	if (Network_Manager::GetInstance()->MyControlTarget == CONTROL_DRONE)
		vecreversepos = m_TransformCom->Get_State(CTransform::STATE_POSITION) - m_DroneCamera->Get_State(CTransform::STATE_POSITION);
	else
		vecreversepos = m_TransformCom->Get_State(CTransform::STATE_POSITION) - m_CameraTransform->Get_State(CTransform::STATE_POSITION);

	_vector finalpos = m_TransformCom->Get_State(CTransform::STATE_POSITION) + vecreversepos;

	m_TransformCom->Look_At(finalpos);

	m_TransformCom->Set_Scale(4.f);
}

void CSmokeEffect::LateTick(float fTimeDelta)
{
	if (m_AccumulatedTime < 3.f)
	{
		__super::LateTick(fTimeDelta);

		m_AccumulatedTime += fTimeDelta;

		// 총 프레임 수
		const int totalFrames = 40;
		const int framesPerRow = 8;

		// 애니메이션 속도 (초당 프레임 수)
		float frameRate = 24.f;
		int currentFrame = static_cast<int>(m_AccumulatedTime * frameRate);
		//if (currentFrame >= totalFrames) currentFrame = totalFrames - 1; // 마지막 프레임 고정

		int row = currentFrame / framesPerRow;
		int col = currentFrame % framesPerRow;

		// UV 변환행렬 구성
		XMMATRIX scale = XMMatrixScaling(1.0f / 8.0f, 1.0f / 5.0f, 1.f);
		XMMATRIX trans = XMMatrixTranslation((float)col / 8.0f, (float)row / 5.0f, 0.f);

		// 최종 텍스처 변환 행렬
		XMMATRIX texTransform = scale * trans;

		m_TexCoordTransformCom->Set_WorldMatrix(texTransform);

		//XMStoreFloat4x4(&m_TexCoordTransformCom, texTransform);

		m_CBBindingCom->Set_World_TexCoord_And_Update(m_TransformCom, m_TexCoordTransformCom);

		if (currentFrame > totalFrames)
			isDead = true;
	}
	//if (m_AccumulatedTime >= 3)
	//    return;

	//__super::LateTick(fTimeDelta);

	//m_AccumulatedTime += fTimeDelta*2.f;

	//// 총 프레임 수
	//const int totalFrames = 64;
	//const int framesPerRow = 8;

	//// 애니메이션 속도 (초당 프레임 수)
	//float frameRate = 24.f;
	//int currentFrame = static_cast<int>(m_AccumulatedTime * frameRate);
	//if (currentFrame >= totalFrames) currentFrame = totalFrames - 1; // 마지막 프레임 고정

	//int row = currentFrame / framesPerRow;
	//int col = currentFrame % framesPerRow;

	//// UV 변환행렬 구성
	//XMMATRIX scale = XMMatrixScaling(1.0f / 8.0f, 1.0f / 8.0f, 1.f);
	//XMMATRIX trans = XMMatrixTranslation((float)col / 8.0f, (float)row / 8.0f, 0.f);

	//// 최종 텍스처 변환 행렬
	//XMMATRIX texTransform = scale * trans;

	//m_TexCoordTransformCom->Set_WorldMatrix(texTransform);

	////XMStoreFloat4x4(&m_TexCoordTransformCom, texTransform);

	//m_CBBindingCom->Set_World_TexCoord_And_Update(m_TransformCom, m_TexCoordTransformCom);


}

void CSmokeEffect::Render()
{
	if (m_AccumulatedTime >= 3)
		return;
	m_CBBindingCom->Set_On_Shader();

	m_VIBuffer->Render();
}

void CSmokeEffect::Free()
{
	Safe_Release(m_VIBuffer);
	Safe_Release(m_CBBindingCom);
	Safe_Release(m_CameraTransform);
	Safe_Release(m_DroneCamera);

	__super::Free();
}

CSmokeEffect* CSmokeEffect::Create()
{
	CSmokeEffect* pInstance = new CSmokeEffect;
	pInstance->Initialize_Prototype();
	return pInstance;
}

CRenderObject* CSmokeEffect::Clone(void* pArg)
{
	CSmokeEffect* pInstance = new CSmokeEffect(*this);
	pInstance->Initialize(pArg);
	return pInstance;
}
