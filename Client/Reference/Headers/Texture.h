#pragma once
#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CTexture : public CBase
{
public:
	CTexture();
	CTexture(CTexture& rhs) = delete;
	virtual~CTexture() = default;

public:
	void Initialize_Prototype(const wchar_t* Filename);

public:
	ID3D12Resource* Get_Texture() { return m_Resource; }

private:
	ID3D12Resource* m_Resource = nullptr;
	ID3D12Resource* m_UploadHeap = nullptr;

public:
	static CTexture* Create(const wchar_t* Filename);
	void Free();
};

END