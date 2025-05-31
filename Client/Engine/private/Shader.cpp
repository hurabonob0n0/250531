#include "Shader.h"

CShader::CShader(SHADER_TYPE ST, const wstring& filepath, const D3D_SHADER_MACRO* defines)
{
	switch (ST)
	{
	case Engine::CShader::ST_VS:
		m_Shader = d3dUtil::CompileShader(filepath, defines, "VS", "vs_5_1");
		break;
	case Engine::CShader::ST_PS:
		m_Shader = d3dUtil::CompileShader(filepath, defines, "PS", "ps_5_1");
		break;
	default:
		break;
	}
}

CShader* CShader::Create(SHADER_TYPE ST, const wstring& filepath, const D3D_SHADER_MACRO* defines)
{
	CShader* pInstance = new CShader(ST, filepath, defines);
	return pInstance;
}

void CShader::Free()
{
	Safe_Release(m_Shader);
}
