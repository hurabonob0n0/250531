#include "PSO.h"
#include "Shader.h"
#include "GameInstance.h"

CPSO::CPSO()
{
    ZeroMemory(&m_PsoDesc, sizeof(m_PsoDesc));

    m_PsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    m_PsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    m_PsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    m_PsoDesc.SampleMask = UINT_MAX;
    m_PsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    m_PsoDesc.NumRenderTargets = 1;
    m_PsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    m_PsoDesc.SampleDesc.Count = 1;
    m_PsoDesc.SampleDesc.Quality = 0;
    m_PsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    //Todo : 나중에 디퍼드 셰이딩 or Blending 해야 하면 설정해줘야 함.

    //Todo : 진짜 나중에 구조까지 깔끔하게 맞추고 싶으면 SetVS는 내가 만든 클래스를 주고 SetRS는 ID3D클래스를 사용하는데 맞춰주기
    //-----------아래는 참고-----------------------
    /*1. CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT)
    FillMode = D3D12_FILL_MODE_SOLID;
    CullMode = D3D12_CULL_MODE_BACK;
    FrontCounterClockwise = FALSE;
    DepthClipEnable = TRUE;*/

    /*CD3DX12_BLEND_DESC(D3D12_DEFAULT)
    AlphaToCoverageEnable = FALSE;
    IndependentBlendEnable = FALSE;
    RenderTarget[0].BlendEnable = FALSE;
    RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;*/

    /*CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
    DepthEnable = TRUE;
    DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    StencilEnable = FALSE;*/
}

CPSO* CPSO::SetInputLayout(INPUTLAYOUT_TYPE eLayout)
{
    switch (eLayout)
    {
    case IT_POS:
        m_PsoDesc.InputLayout = { VTXPOS::VertexElements, VTXPOS::iNumElements };
        break;
    case IT_POS_NOR:
        m_PsoDesc.InputLayout = { VTXPOSNOR::VertexElements, VTXPOSNOR::iNumElements };
        break;
    case IT_POS_TEX:
        m_PsoDesc.InputLayout = { VTXPOSTEX::VertexElements, VTXPOSTEX::iNumElements };
        break;
    case IT_POS_NOR_TEX:
        m_PsoDesc.InputLayout = { VTXNORTEX::VertexElements, VTXNORTEX::iNumElements };
        break;
    case IT_CUBE_TEX:
        m_PsoDesc.InputLayout = { VTXCUBETEX::VertexElements, VTXCUBETEX::iNumElements };
        break;
    case IT_MESH:
        m_PsoDesc.InputLayout = { VTXMESH::VertexElements, VTXMESH::iNumElements };
        break;
    case IT_ANIM_MESH:
        m_PsoDesc.InputLayout = { VTXANIMMESH::VertexElements, VTXANIMMESH::iNumElements };
        break;
    default:
        m_PsoDesc.InputLayout = { nullptr, 0 };
        MSG_BOX("INVALID INPUTLAYOUT");
        break;
    }
    return this;
}

CPSO* CPSO::SetVS(CShader* VS)
{
    m_PsoDesc.VS = { reinterpret_cast<BYTE*>(VS->Get()), VS->Get_Buffer_Size() };
    return this;
}

CPSO* CPSO::SetPS(CShader* PS)
{
    m_PsoDesc.PS = { reinterpret_cast<BYTE*>(PS->Get()), PS->Get_Buffer_Size() };
    return this;
}

CPSO* CPSO::SetRS(ID3D12RootSignature* RS)
{
    m_PsoDesc.pRootSignature = RS;
    return this;
}

CPSO* CPSO::SetForSkyBox() {

    CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);

    // 깊이 테스트는 하되...
    depthStencilDesc.DepthEnable = TRUE;

    // 깊이 **쓰기만 끔**
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

    // 비교 함수는 그대로 LESS_EQUAL
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    // (스텐실은 안 쓴다고 가정)
    depthStencilDesc.StencilEnable = FALSE;

    m_PsoDesc.DepthStencilState = depthStencilDesc;

    m_PsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

    return this;
}

CPSO* CPSO::Create_PSO() {
    GETDEVICE->CreateGraphicsPipelineState(&m_PsoDesc, IID_PPV_ARGS(&m_PSO));
    return this;
}

CPSO* CPSO::Create()
{
    CPSO* pInstance = new CPSO;
    return pInstance;
}

void CPSO::Free()
{
    Safe_Release(m_PSO);
}