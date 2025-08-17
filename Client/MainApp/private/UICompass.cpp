#include "Client_pch.h"
#include "UICompass.h"
#include "GameInstance.h"
#include "Network_Manager.h"

CUICompass::CUICompass() : CUIObject()
{
}

CUICompass::CUICompass(CUICompass& rhs) : CUIObject(rhs)
{

}

HRESULT CUICompass::Initialize_Prototype()
{
	__super::Initialize_Prototype();

	return S_OK;
}

HRESULT CUICompass::Initialize(void* pArg)
{
	m_RG = CRenderer::RG_UI;

	__super::Initialize(pArg);

	m_CameraFree = (CTransform*)m_GameInstance->Get_Object_Component("Camera", 0, "TransformCom");
	m_DroneCamera = (CTransform*)m_GameInstance->Get_Object_Component("Camera", 1, "TransformCom");

	MaterialData mat{};

	mat.DiffuseMapIndex = m_GameInstance->Add_Texture("CompassUI", CTexture::Create(L"../bin/Models/FinalUI/ComapssBar.dds"));

	m_CBBinding->Set_MaterialIndex(m_GameInstance->Add_Material("CompassUI", mat));

	m_CBBinding->Set_Pad0(3);

	m_VIBuffer = (CVIBuffer_Quad*)m_GameInstance->Get_Component("VIBuffer_QuadCom");

	__super::Set_Scale(1.f, 0.05f * 1.777f);

	__super::Set_Pos(960.f, 1040.f);

	return S_OK;
}

void CUICompass::Tick(float fTimeDelta)
{
	__super::Tick(fTimeDelta);
}

void CUICompass::LateTick(float fTimeDelta)
{
	_vector Look;
	if (Network_Manager::GetInstance()->MyControlTarget == CONTROL_DRONE)
		Look = m_DroneCamera->Get_State(CTransform::STATE_LOOK);
	else
		Look = m_CameraFree->Get_State(CTransform::STATE_LOOK);

	float yaw = atan2(XMVectorGetX(Look), XMVectorGetZ(Look));
	float yRotation = XMConvertToDegrees(yaw);
	if (yRotation < 0.0f)
		yRotation += 360.0f;
	__super::Set_TexPos(yRotation / 360.f - 0.5f, 0.f);
	__super::Set_TexScale(1.f, 1.f);

	__super::LateTick(fTimeDelta);
}

void CUICompass::Render()
{
	__super::Render();

	m_VIBuffer->Render();
}

void CUICompass::Free()
{
	Safe_Release(m_VIBuffer);

	__super::Free();
}

CUICompass* CUICompass::Create()
{
	CUICompass* pInstance = new CUICompass;
	pInstance->Initialize_Prototype();
	return pInstance;
}

CUICompass* CUICompass::Clone(void* pArg)
{
	CUICompass* pInstance = new CUICompass(*this);
	pInstance->Initialize(pArg);
	return pInstance;
}
