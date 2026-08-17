#include "Client_pch.h"
#include "Ping.h"
#include "GameInstance.h"
#include "Network_Manager.h"
#include "Terrain.h"

CPing::CPing() : CRenderObject()
{
}

CPing::CPing(CPing& rhs) : CRenderObject(rhs)
{
	m_MatIndex = rhs.m_MatIndex;
}

HRESULT CPing::Initialize_Prototype()
{
	__super::Initialize_Prototype();

	MaterialData mat{};

	mat.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("PingTex", CTexture::Create(L"../bin/Models/FinalUI/Ping.dds"));

	m_MatIndex = CGameInstance::Get_Instance()->Add_Material("PingMat", mat);

	return S_OK;
}

HRESULT CPing::Initialize(void* pArg)
{
	m_RG = CRenderer::RG_BLEND;

	__super::Initialize(pArg);

	m_CBBindingCom = (CBBinding*)m_GameInstance->Get_Component("CBBindingCom", nullptr);
	m_Terrain = (CTerrain*)m_GameInstance->GetGameObject("Terrain", 0);
	m_CameraFree = (CTransform*)m_GameInstance->Get_Object_Component("Camera", 0, "TransformCom");
	m_DroneCamera = (CTransform*)m_GameInstance->Get_Object_Component("Camera", 1, "TransformCom");

	Set_Position();

	m_CBBindingCom->Set_MaterialIndex(m_MatIndex);

	m_VIBuffer = (CVIBuffer_Quad*)m_GameInstance->Get_Component("VIBuffer_QuadCom");

	m_TexCoordTransformCom->Identity();

	return S_OK;
}

void CPing::Tick(float fTimeDelta)
{

	m_TimeDelta += fTimeDelta;

}

void CPing::LateTick(float fTimeDelta)
{
	m_TransformCom->Identity();
	
	_vector Look;
	
	if (Network_Manager::GetInstance()->MyControlTarget == CONTROL_DRONE)
		Look= m_DroneCamera->Get_State(CTransform::STATE_POSITION) - LayContactPos;
	else
		Look = m_CameraFree->Get_State(CTransform::STATE_POSITION) - LayContactPos;

	m_TransformCom->Adjust_Axis(XMVector4Normalize(-Look));
	m_TransformCom->Set_Scale(XMVectorGetX(0.08f * XMVector3Length(Look)));
	m_TransformCom->Set_State(CTransform::STATE_POSITION, LayContactPos);

	m_CBBindingCom->Set_World_TexCoord_And_Update(m_TransformCom, m_TexCoordTransformCom);

	__super::LateTick(fTimeDelta);


	if (m_TimeDelta > 5.f)
		isDead = true;

}

void CPing::Render()
{
	m_RendererCom->ChangePSO("PingPSO");

	m_CBBindingCom->Set_On_Shader();

	m_VIBuffer->Render();

	m_RendererCom->ChangePSO("EffectPSO");
}

void CPing::Set_Position()
{
	_vector Pos, Look;
	Pos = m_DroneCamera->Get_State(CTransform::STATE_POSITION);
	Look = m_DroneCamera->Get_State(CTransform::STATE_LOOK);
	

	bool isUnder = false;
	LayContactPos = Pos;

	for (int i = 0; i < 1000; ++i)
	{
		LayContactPos += Look;
		float y = m_Terrain->Get_Terrain_Heights(XMVectorGetX(LayContactPos), XMVectorGetZ(LayContactPos));
		if (y > XMVectorGetY(LayContactPos))
		{
			isUnder = true;

			/*  XMVectorSetY 는 새 벡터를 '돌려주는' 함수다. 예전에는 반환값을 버려서
				지형 높이로 맞추는 이 줄이 아무 일도 하지 않았고, 핑이 지면에 붙지 않고
				레이가 지나간 자리(지면보다 약간 아래)에 찍히고 있었다.               */
			LayContactPos = XMVectorSetY(LayContactPos, y);
			break;
		}
	}

	if (isUnder == false)
		isDead = true;

	m_TransformCom->Set_State(CTransform::STATE_POSITION, LayContactPos);
}

_vector CPing::Get_Pos()
{
	return LayContactPos;
}

/*  서버가 준 좌표로 덮어쓴다. 짝(드론 조작자)이 찍은 핑을 받았을 때만 부른다.

	★ isDead 를 반드시 되돌려야 한다.
	   이 객체는 AddObject 로 만들어지고, 그 안의 Initialize 가 인자 없는
	   Set_Position() 을 부른다. 그건 '내 드론 카메라' 에서 지형으로 레이를 쏘는
	   함수인데, 받는 쪽(조종수)의 드론 카메라는 엉뚱한 데를 보고 있으므로 대개 빗나간다.
	   그러면 isDead 가 서서, 바로 뒤에 좌표를 제대로 넣어줘도 다음 프레임에 사라졌다.
	   지금 시점에는 서버가 준 지형 위 좌표가 확정돼 있으니 되살린다.

	   표시 시간(5초)도 여기서 다시 잰다 - 생성 시점이 아니라 좌표가 정해진 시점부터다.  */
void CPing::Set_Position(float x, float y, float z)
{
	LayContactPos = XMVectorSet(x, y, z, 1.f);

	isDead      = false;
	m_TimeDelta = 0.f;
}

void CPing::Free()
{
	Safe_Release(m_VIBuffer);

	__super::Free();
}

CPing* CPing::Create()
{
	CPing* pInstance = new CPing;
	pInstance->Initialize_Prototype();
	return pInstance;
}

CPing* CPing::Clone(void* pArg)
{
	CPing* pInstance = new CPing(*this);
	pInstance->Initialize(pArg);
	return pInstance;
}
