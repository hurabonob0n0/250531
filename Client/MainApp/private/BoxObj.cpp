#include "Client_pch.h"
#include "BoxObj.h"
#include "GameInstance.h"

CBoxObj::CBoxObj() : CRenderObject()
{
}

CBoxObj::CBoxObj(CBoxObj& rhs) : CRenderObject(rhs)
{
}

HRESULT CBoxObj::Initialize_Prototype()
{
    __super::Initialize_Prototype();

    return S_OK;
}

HRESULT CBoxObj::Initialize(void* pArg)
{
    m_RG = CRenderer::RG_BLEND;

    __super::Initialize(pArg);

   /* MaterialData MD{};

    XMStoreFloat4x4( &MD.MatTransform, XMMatrixIdentity());
    MD.DiffuseMapIndex = m_GameInstance->Add_Texture("BrickDiffuse", CTexture::Create(L"../bin/Models/Basic/bricks.dds"));
    MD.NormalMapIndex = m_GameInstance->Add_Texture("BrickNormal", CTexture::Create(L"../bin/Models/Basic/bricks_nmap.dds"));

    Set_MatIndex(m_GameInstance->Add_Material("BrickMat", MD));*/

    m_VIBuffer = (CVIBuffer_Geos*)m_GameInstance->Get_Component("VIBuffer_GeosCom");



    return S_OK;
}

void CBoxObj::Tick(float fTimeDelta)
{
    __super::Tick(fTimeDelta);
}

void CBoxObj::LateTick(float fTimeDelta)
{
    __super::LateTick(fTimeDelta);
}

void CBoxObj::Render()
{
    __super::Render();

    m_VIBuffer->Render();
}

void CBoxObj::Free()
{
    Safe_Release(m_VIBuffer);

    __super::Free();
}

CBoxObj* CBoxObj::Create()
{
    CBoxObj* pInstance = new CBoxObj;
    pInstance->Initialize_Prototype();
    return pInstance;
}

CRenderObject* CBoxObj::Clone(void* pArg)
{
    CBoxObj* pInstance = new CBoxObj(*this);
    pInstance->Initialize(pArg);
    return pInstance;
}
