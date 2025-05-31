#pragma once
#include "Base.h"

BEGIN(Engine)

class CShader : public CBase
{
public:
	enum SHADER_TYPE{ST_VS,ST_PS,ST_END};

public:
	CShader(SHADER_TYPE ST, const wstring& filepath, const D3D_SHADER_MACRO* defines);
	virtual ~CShader() = default;

public:
	BYTE*	Get() const { return (BYTE*)m_Shader->GetBufferPointer(); }
	SIZE_T	Get_Buffer_Size() const { return m_Shader->GetBufferSize(); }

private:
	ID3DBlob* m_Shader = {};

public:
	static CShader* Create(SHADER_TYPE ST,const wstring& filepath, const D3D_SHADER_MACRO* defines);
	void Free() override;
};

END