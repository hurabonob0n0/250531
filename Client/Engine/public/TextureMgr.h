//#pragma once
//#include "Base.h"
//#include "Texture.h"
//
//BEGIN(Engine)
//
//class ENGINE_DLL CTextureMgr : public CBase
//{
//public:
//	enum TEXTURETYPE{TT_TEXTURE2D,TT_TEXTURECUBE,TT_SHADOWMAP, TT_END};
//
//	DECLARE_SINGLETON(CTextureMgr)
//private:
//	CTextureMgr() = default;
//	virtual~CTextureMgr() = default;
//
//public:
//	int Add_Texture(string texname, CTexture* texInstance, TEXTURETYPE TT = TT_TEXTURE2D);
//	void Add_ShadowMap(ID3D12Resource** Resource);
//	ID3D12Resource* Get_Texture(string texname) { return m_TexMap[texname]->Get_Texture(); }
//	void Make_DescriptorHeap();
//	void Set_DescriptorHeap();
//
//	void Resize_TexMap() { m_TexMap.reserve(500); }
//
//public:
//	ID3D12DescriptorHeap* Get_DescriptorHeap() { return srvDescriptorHeap; }
//	CD3DX12_CPU_DESCRIPTOR_HANDLE Get_CPUSRVDescriptorHeap() { return hDescriptor; }
//
//private:
//	ID3D12DescriptorHeap* srvDescriptorHeap;
//	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor;
//
//private:
//	unordered_map<string, CTexture*> m_TexMap;
//
//public:
//	void Free();
//};
//
//END

#pragma once
#include "Base.h"
#include "Texture.h"

BEGIN(Engine)

class ENGINE_DLL CTextureMgr : public CBase
{
public:
	enum TEXTURETYPE { TT_TEXTURE2D, TT_TEXTURECUBE, TT_SHADOWMAP, TT_END };

	DECLARE_SINGLETON(CTextureMgr)
private:
	CTextureMgr() = default;
	virtual~CTextureMgr() = default;

public:
	_uint Add_Texture(string texname, CTexture* texInstance, TEXTURETYPE TT = TT_TEXTURE2D);
	void Add_ShadowMap(ID3D12Resource** Resource);
	ID3D12Resource* Get_Texture(string texname);
	void Make_DescriptorHeap();
	void Set_DescriptorHeap();

	void Resize_TexMap();

public:
	ID3D12DescriptorHeap* Get_DescriptorHeap() { return srvDescriptorHeap; }
	CD3DX12_CPU_DESCRIPTOR_HANDLE Get_CPUSRVDescriptorHeap() { return hDescriptor; }

private:
	ID3D12DescriptorHeap* srvDescriptorHeap;
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor;

private:
	// 이름과 텍스처 포인터를 한 쌍으로 묶어 vector에서 관리
	vector<pair<string, CTexture*>> m_Textures;

public:
	void Free();
};

END