#include "Client_pch.h"
#include "Terrain.h"
#include "Client_Defines.h"
#include "GameInstance.h"

CTerrain::CTerrain() : CRenderObject()
{
}

CTerrain::CTerrain(CTerrain& rhs) : CRenderObject(rhs)
{
}

HRESULT CTerrain::Initialize_Prototype()
{
    __super::Initialize_Prototype();
    return S_OK;
}

float CTerrain::Get_Terrain_Heights(float x, float z)
{
    return m_VIBufferCom->Get_Terrain_Heights(x, z);
}

HRESULT CTerrain::Initialize(void* pArg)
{
    __super::Initialize(pArg);

    m_RG = CRenderer::RG_NONBLEND;

    m_CBBindingCom = (CBBinding*)m_GameInstance->Get_Component("CBBindingCom", nullptr);    

    m_VIBufferCom = (CVIBuffer_Terrain*)m_GameInstance->Get_Component("TerrainCom",this);

    MaterialData mat{};
    mat.DiffuseMapIndex = m_GameInstance->Add_Texture("Terrain_D", CTexture::Create(L"../bin/Models/TerrainDDS/Diffuse.dds"));
    mat.NormalMapIndex = m_GameInstance->Add_Texture("Terrain_N", CTexture::Create(L"../bin/Models/TerrainDDS/Normal.dds"));
    mat.Roughness = 0.9f;
    mat.FresnelR0 = XMFLOAT3{ 0.f,0.f,0.f };
    m_CBBindingCom->Set_MaterialIndex( m_GameInstance->Add_Material("TerrainMat", mat));
    m_CBBindingCom->Set_Pad0(1);

    return S_OK;
}

void CTerrain::Tick(float fTimeDelta)
{
    

    __super::Tick(fTimeDelta);
}

void CTerrain::LateTick(float fTimeDelta)
{
    __super::LateTick(fTimeDelta);

    m_CBBindingCom->Set_World_TexCoord_And_Update(m_TransformCom, m_TexCoordTransformCom);
}

void CTerrain::Render()
{
    m_CBBindingCom->Set_On_Shader();

    m_VIBufferCom->Render();
}

CTerrain* CTerrain::Create()
{
    CTerrain* pInstance = new CTerrain;
    pInstance->Initialize_Prototype();
    return pInstance;
}

CRenderObject* CTerrain::Clone(void* pArg)
{
    CTerrain* pInstance = new CTerrain(*this);
    pInstance->Initialize(pArg);
    return pInstance;
}

void CTerrain::Free()
{
    Safe_Release(m_VIBufferCom);
    Safe_Release(m_CBBindingCom);
    __super::Free();
}
