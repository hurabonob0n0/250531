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
	_uint Add_Texture(const string& texname, const wchar_t* Filename, TEXTURETYPE TT = TT_TEXTURE2D);	// loads the file only if texname is not registered yet
	_uint Find_Texture(const string& texname) const;	// texture index, 0 (= pink default texture) if not found
	void Add_ShadowMap(ID3D12Resource** Resource);
	void Add_DefaultTexture();	// registers the pink texture as texture index 0. Call once right after Make_DescriptorHeap
	ID3D12Resource* Get_Texture(string texname);
	void Make_DescriptorHeap();
	void Set_DescriptorHeap();

	void Resize_TexMap();

public:
	ID3D12DescriptorHeap* Get_DescriptorHeap() { return srvDescriptorHeap; }
	CD3DX12_CPU_DESCRIPTOR_HANDLE Get_CPUSRVDescriptorHeap() { return hDescriptor; }

	// Heap layout: see Engine_Config.h (SRV_ENGINE_RESERVED)
	// Engine-only region [0, SRV_ENGINE_RESERVED): shadow map, HDR, Bloom ...
	CD3DX12_CPU_DESCRIPTOR_HANDLE Get_EngineSrvCpuHandle(_uint slot) const;
	CD3DX12_GPU_DESCRIPTOR_HANDLE Get_EngineSrvGpuHandle(_uint slot) const;
	// Texture region: start of gTextureMaps (root slot 5), and one texture (e.g. sky cube map, root slot 3)
	CD3DX12_GPU_DESCRIPTOR_HANDLE Get_TextureTableGpuHandle() const;
	CD3DX12_GPU_DESCRIPTOR_HANDLE Get_TextureGpuHandle(_uint texIndex) const;

private:
	_int Find_Index(const string& texname) const;	// -1 if not found
	ID3D12DescriptorHeap* srvDescriptorHeap;
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor;

private:
	// 이름과 텍스처 포인터를 한 쌍으로 묶어 vector에서 관리
	vector<pair<string, CTexture*>> m_Textures;

public:
	void Free();
};

END