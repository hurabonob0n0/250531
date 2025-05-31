#include "Shader_Mgr.h"

IMPLEMENT_SINGLETON(CShader_Mgr)

HRESULT CShader_Mgr::AddShader(const string& shaderName, CShader::SHADER_TYPE ST, const wstring& filepath, const D3D_SHADER_MACRO* defines)
{
	if (m_Shaders.find(shaderName) != m_Shaders.end())
		return E_FAIL; // 이미 있음

	CShader* shader = CShader::Create(ST,filepath,defines);

	if (!shader)
	{
		Safe_Release(shader);
		return E_FAIL;
	}

	m_Shaders[shaderName] = shader;
	return S_OK;
}

CShader* CShader_Mgr::GetShaderObj(const string& shaderName) const
{
	auto iter = m_Shaders.find(shaderName);
	if (iter != m_Shaders.end())
		return iter->second;
	return nullptr;
}

BYTE* CShader_Mgr::GetShader(const string& shaderName) const
{
	auto iter = m_Shaders.find(shaderName);
	if (iter != m_Shaders.end())
		return iter->second->Get();
	return nullptr;
}

SIZE_T CShader_Mgr::GetBufferSize(const string& shaderName) const
{
	auto iter = m_Shaders.find(shaderName);
	if (iter != m_Shaders.end())
		return iter->second->Get_Buffer_Size();
	return 0;
}

void CShader_Mgr::Free()
{
	for (auto& pair : m_Shaders)
	{
		Safe_Release(pair.second); // 혹은 delete pair.second;
	}
	m_Shaders.clear();
}
