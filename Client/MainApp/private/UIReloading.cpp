#include "Client_pch.h"
#include "UIReloading.h"
#include "GameInstance.h"

CUIReloading::CUIReloading() : CUIObject()
{
}

CUIReloading::CUIReloading(CUIReloading& rhs) : CUIObject(rhs)
{

}

HRESULT CUIReloading::Initialize_Prototype()
{
	__super::Initialize_Prototype();

	return S_OK;
}

HRESULT CUIReloading::Initialize(void* pArg)
{
	m_RG = CRenderer::RG_UI;

	__super::Initialize(pArg);

	MaterialData mat{};

	mat.DiffuseMapIndex = m_GameInstance->Add_Texture("UIReloading", CTexture::Create(L"../bin/Models/UI/2.dds"));

	m_CBBinding->Set_MaterialIndex(m_GameInstance->Add_Material("UIReloading", mat));

	m_VIBuffer = (CVIBuffer_Quad*)m_GameInstance->Get_Component("VIBuffer_QuadCom");

	m_TransformCom->Set_Scale(CTransform::STATE_UP, 1.77f);

	m_TransformCom->Set_Scale(2.f);

	m_TransformCom->Set_Scale(0.1f);

	m_TransformCom->Set_Scale(CTransform::STATE_LOOK, 0.f);

	//m_TransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(0.f, -0.09f, 0.f, 1.f));

	//m_CBBinding->Set_Pad0(1);

   // m_isFPS = false;

	return S_OK;
}

void CUIReloading::Tick(float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	if (reloading)
	{
		deltatime += fTimeDelta;
		if (deltatime >= cooltime)
		{
			reloading = false;
			deltatime = 0.f;
		}
	}
}

void CUIReloading::LateTick(float fTimeDelta)
{
	__super::LateTick(fTimeDelta);
}

void CUIReloading::Render()
{
	if (reloading)
	{
		__super::Render();
		m_VIBuffer->Render();
	}
}

void CUIReloading::Free()
{
	Safe_Release(m_VIBuffer);

	__super::Free();
}

CUIReloading* CUIReloading::Create()
{
	CUIReloading* pInstance = new CUIReloading;
	pInstance->Initialize_Prototype();
	return pInstance;
}

CUIReloading* CUIReloading::Clone(void* pArg)
{
	CUIReloading* pInstance = new CUIReloading(*this);
	pInstance->Initialize(pArg);
	return pInstance;
}
