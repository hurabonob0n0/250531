#include "Client_pch.h"
#include "UIDamaged.h"
#include "GameInstance.h"
#include "Network_Manager.h"

CUIDamaged::CUIDamaged() : CUIObject()
{
}

CUIDamaged::CUIDamaged(CUIDamaged& rhs) : CUIObject(rhs)
{
    
}

HRESULT CUIDamaged::Initialize_Prototype()
{
    __super::Initialize_Prototype();

    return S_OK;
}

HRESULT CUIDamaged::Initialize(void* pArg)
{
    m_RG = CRenderer::RG_UI;

    __super::Initialize(pArg);

    MaterialData mat{};

    mat.DiffuseMapIndex = m_GameInstance->Add_Texture("UIDamaged", CTexture::Create(L"../bin/Models/UI/8.dds"));

    m_CBBinding->Set_MaterialIndex(m_GameInstance->Add_Material("UIDamaged", mat));

    m_VIBuffer = (CVIBuffer_Quad*)m_GameInstance->Get_Component("VIBuffer_QuadCom");

    m_TransformCom->Set_Scale(0.1f);
    m_TransformCom->Set_Scale(CTransform::STATE_UP, 1.777f);

    return S_OK;
}

void CUIDamaged::Tick(float fTimeDelta)
{
    __super::Tick(fTimeDelta);

   /* if(m_GameInstance->Key_Down(VK_RETURN))
        isHit = true;*/

    if (isHit)
    {
        deltatime += fTimeDelta;
        if (deltatime >= cooltime)
        {
            isHit = false;
            deltatime = 0.f;
        }
    }
}

void CUIDamaged::LateTick(float fTimeDelta)
{
    __super::LateTick(fTimeDelta);
}

void CUIDamaged::Render()
{
    if (isHit)
    {
        __super::Render();
        m_VIBuffer->Render();
    }
}

void CUIDamaged::reset_deltatime()
{
    deltatime = 0.f;
}

void CUIDamaged::Free()
{
    Safe_Release(m_VIBuffer);

    __super::Free();
}

CUIDamaged* CUIDamaged::Create()
{
    CUIDamaged* pInstance = new CUIDamaged;
    pInstance->Initialize_Prototype();
    return pInstance;
}

CUIDamaged* CUIDamaged::Clone(void* pArg)
{
    CUIDamaged* pInstance = new CUIDamaged(*this);
    pInstance->Initialize(pArg);
    return pInstance;
}
