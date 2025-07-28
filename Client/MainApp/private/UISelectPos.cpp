#include "Client_pch.h"
#include "UISelectPos.h"
#include "GameInstance.h"

CUISelectPos::CUISelectPos() : CUIObject()
{
}

CUISelectPos::CUISelectPos(CUISelectPos& rhs) : CUIObject(rhs)
{

}

HRESULT CUISelectPos::Initialize_Prototype()
{
	__super::Initialize_Prototype();

	return S_OK;
}

HRESULT CUISelectPos::Initialize(void* pArg)
{
	m_RG = CRenderer::RG_UI;

	__super::Initialize(pArg);

	MaterialData mat{};

	mat.DiffuseMapIndex = m_GameInstance->Add_Texture("UISelectPos", CTexture::Create(L"../bin/Models/UI/6.dds"));

	m_CBBinding->Set_MaterialIndex(m_GameInstance->Add_Material("UISelectPos", mat));

	m_VIBuffer = (CVIBuffer_Quad*)m_GameInstance->Get_Component("VIBuffer_QuadCom");

	m_TransformCom->Set_Scale(CTransform::STATE_UP, 1.77f);

	m_TransformCom->Set_Scale(1.5f);

	//m_TransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(0.f, -0.09f, 0.f, 1.f));

	//m_CBBinding->Set_Pad0(1);

   // m_isFPS = false;

	return S_OK;
}

void CUISelectPos::Tick(float fTimeDelta)
{
	__super::Tick(fTimeDelta);
}

void CUISelectPos::LateTick(float fTimeDelta)
{
	__super::LateTick(fTimeDelta);
}

void CUISelectPos::Render()
{
	if (render)
	{
		__super::Render();
		m_VIBuffer->Render();
	}
}

void CUISelectPos::Free()
{
	Safe_Release(m_VIBuffer);

	__super::Free();
}

CUISelectPos* CUISelectPos::Create()
{
	CUISelectPos* pInstance = new CUISelectPos;
	pInstance->Initialize_Prototype();
	return pInstance;
}

CUISelectPos* CUISelectPos::Clone(void* pArg)
{
	CUISelectPos* pInstance = new CUISelectPos(*this);
	pInstance->Initialize(pArg);
	return pInstance;
}
