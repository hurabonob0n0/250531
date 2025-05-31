#pragma once
#include "Base.h"
#include "Shader.h"

BEGIN(Engine)

class CShader_Mgr : public CBase
{
	DECLARE_SINGLETON(CShader_Mgr)

public:
	CShader_Mgr() = default;
	virtual ~CShader_Mgr() = default;

public:
	// Shader 추가
	HRESULT AddShader(const string& shaderName, CShader::SHADER_TYPE ST, const wstring& filepath, const D3D_SHADER_MACRO* defines);

	// Shader 정보 얻기
	CShader* GetShaderObj(const string& shaderName) const;

	// Shader 버퍼 정보
	BYTE* GetShader(const string& shaderName) const;
	SIZE_T GetBufferSize(const string& shaderName) const;

private:
	unordered_map<string, CShader*> m_Shaders;

public:
	virtual void Free() override;
};

END