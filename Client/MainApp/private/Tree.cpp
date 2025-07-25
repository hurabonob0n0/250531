#include "Client_pch.h"
#include "Tree.h"
#include "GameInstance.h"

CTree::CTree() : CRenderObject()
{
}

CTree::CTree(CTree& rhs) : CRenderObject(rhs)
{
}

HRESULT CTree::Initialize_Prototype()
{
    __super::Initialize_Prototype();

    return S_OK;
}

HRESULT CTree::Initialize(void* pArg)
{
    m_RG = CRenderer::RG_NONBLEND;

    __super::Initialize(pArg);
    
    m_VIBuffer = (CMeshModel*)m_GameInstance->Get_Component("MeshModelCom");

    m_CBBindingCom = (CBBinding*)m_GameInstance->Get_Component("CBBindingCom", nullptr);
    m_CBBindingCom2 = (CBBinding*)m_GameInstance->Get_Component("CBBindingCom", nullptr);

    MaterialData MD{};

    XMStoreFloat4x4(&MD.MatTransform, XMMatrixIdentity());
    MD.DiffuseMapIndex = m_GameInstance->Add_Texture("BarkD", CTexture::Create(L"../bin/Models/Tree/Dead_Tree_02_Bark_Base_Color.dds"));
    MD.NormalMapIndex = m_GameInstance->Add_Texture("BarkN", CTexture::Create(L"../bin/Models/Tree/Dead_Tree_02_Bark_Normal.dds"));
    m_CBBindingCom->Set_MaterialIndex(m_GameInstance->Add_Material("BarkMat", MD));
    
    MD.DiffuseMapIndex = m_GameInstance->Add_Texture("BranchD", CTexture::Create(L"../bin/Models/Tree/Dead_Tree_02_Branch_Base_Color.dds"));
    MD.NormalMapIndex = m_GameInstance->Add_Texture("BranchN", CTexture::Create(L"../bin/Models/Tree/Dead_Tree_02_Branch_Normal.dds"));
    m_CBBindingCom2->Set_MaterialIndex(m_GameInstance->Add_Material("BranchMat", MD));
    m_CBBindingCom2->Set_Pad0(2);

    m_TransformCom->Rotation(m_TransformCom->Get_State(CTransform::STATE_RIGHT), XMConvertToRadians(-90.f));
    m_TransformCom->Set_Scale(0.01f);
    m_TransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(0.f, 40.f, 20.f,1.f));
    //m_TransformCom->Set_Scale(300.f);

    m_TexCoordTransformCom->Set_Scale(CTransform::STATE_RIGHT, 0.5f);
    m_TexCoordTransformCom->Set_Scale(CTransform::STATE_UP, 0.5f);

    return S_OK;
}

void CTree::Tick(float fTimeDelta)
{
    __super::Tick(fTimeDelta);

    //m_TransformCom->Go_Straight(fTimeDelta * 10.f);

    if (m_GameInstance->Key_Down(VK_RIGHT))
        ++m_Test;

    if (m_GameInstance->Key_Down(VK_LEFT))
        --m_Test;
}

void CTree::LateTick(float fTimeDelta)
{
    __super::LateTick(fTimeDelta);

    m_CBBindingCom->Set_World_TexCoord_And_Update(m_TransformCom, m_TexCoordTransformCom);
    m_CBBindingCom2->Set_World_TexCoord_And_Update(m_TransformCom, m_TexCoordTransformCom);
}

void CTree::Render()
{
    m_CBBindingCom->Set_On_Shader();

    m_VIBuffer->Render(m_Test);

    m_CBBindingCom2->Set_On_Shader();

    m_VIBuffer->Render(m_Test+2);
}

void CTree::Free()
{
    Safe_Release(m_VIBuffer);
    Safe_Release(m_CBBindingCom);
    Safe_Release(m_CBBindingCom2);

    __super::Free();
}

CTree* CTree::Create()
{
    CTree* pInstance = new CTree;
    pInstance->Initialize_Prototype();
    return pInstance;
}

CRenderObject* CTree::Clone(void* pArg)
{
    CTree* pInstance = new CTree(*this);
    pInstance->Initialize(pArg);
    return pInstance;
}
