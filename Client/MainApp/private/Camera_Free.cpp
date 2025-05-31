#include "Client_pch.h"
#include "Camera_Free.h"
#include "GameInstance.h"

CCamera_Free::CCamera_Free() : CCamera()
{
}

CCamera_Free::CCamera_Free(CCamera_Free& rhs) : CCamera(rhs)
{
}

HRESULT CCamera_Free::Initialize_Prototype()
{
	__super::Initialize_Prototype();

	return S_OK;
}

HRESULT CCamera_Free::Initialize(void* pArg)
{
	__super::Initialize(pArg);

	Set_RenderGroup(CRenderer::RG_PRIORITY);
	
	m_CBBindingCom = (CBBinding*)m_GameInstance->Get_Component("CBBindingCom", nullptr);

	m_CBBindingCom->Set_MaterialIndex(m_GameInstance->Add_Texture("SkyBox", CTexture::Create(L"../bin/Models/SkyBox/desertcube1024.dds"), CTextureMgr::TT_TEXTURECUBE));

	CVIBuffer_Geos::BASIC_SUBMESHES BS = CVIBuffer_Geos::BS_SPHERE;

	m_VIBuffer = (CVIBuffer_Geos*)m_GameInstance->Get_Component("VIBuffer_GeosCom", &BS);



	return S_OK;
}

void CCamera_Free::Tick(float fTimeDelta)
{
	__super::Tick(fTimeDelta);
}

void CCamera_Free::LateTick(float fTimeDelta)
{
	m_RendererCom->AddtoRenderObjects(m_RG, this);

	XMMATRIX world = XMMatrixScaling(5000.f,5000.f,5000.f);
	_matrix textransform = m_TexCoordTransformCom->Get_WorldMatrix();
	

	m_CBBindingCom->Set_CBIndex();
	m_CBBindingCom->Set_WorldMatrix(world);
	m_CBBindingCom->Set_TexCoordMatrix(textransform);
	m_CBBindingCom->Update_CBView();

	__super::LateTick(fTimeDelta);
}

void CCamera_Free::Render()
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE skyTexDescriptor(m_GameInstance->Get_SRVDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
	skyTexDescriptor.Offset(0, m_GameInstance->Get_CBVUAVSRVHeapSize());
	GETCOMMANDLIST->SetGraphicsRootDescriptorTable(3, skyTexDescriptor);
	
	m_CBBindingCom->Set_On_Shader();

	m_VIBuffer->Render();
}

void CCamera_Free::Free()
{
	Safe_Release(m_VIBuffer);
	__super::Free();
}

CCamera_Free* CCamera_Free::Create()
{
	CCamera_Free* pInstance = new CCamera_Free;
	pInstance->Initialize_Prototype();
	return pInstance;
}

CCamera_Free* CCamera_Free::Clone(void* pArg)
{
	CCamera_Free* pInstance = new CCamera_Free(*this);
	pInstance->Initialize(pArg);
	return pInstance;
}
