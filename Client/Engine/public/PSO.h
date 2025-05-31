#pragma once
#include "Base.h"

BEGIN(Engine)

class CRootSignature;
class CShader;
class CGraphic_Device;

class CPSO : public CBase
{
public:
    enum INPUTLAYOUT_TYPE { IT_POS, IT_POS_TEX, IT_POS_NOR, IT_POS_NOR_TEX, IT_CUBE_TEX, IT_MESH, IT_ANIM_MESH, IT_END };

public:
    CPSO();
    virtual ~CPSO() = default;

public:
    ID3D12PipelineState* Get() const { return m_PSO; }

public:
    CPSO* SetInputLayout(INPUTLAYOUT_TYPE eLayout);
    CPSO* SetVS(CShader* VS);
    CPSO* SetPS(CShader* PS);
    CPSO* SetRS(ID3D12RootSignature* RS);
    CPSO* SetForSkyBox();
    CPSO* Create_PSO();

private:
    ID3D12PipelineState* m_PSO = nullptr;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC m_PsoDesc{};

public:
    static CPSO* Create();
    void Free() override;
};

END