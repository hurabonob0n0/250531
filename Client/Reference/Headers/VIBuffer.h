#pragma once
#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer abstract : public CComponent
{
protected:
	CVIBuffer(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	CVIBuffer(CVIBuffer& rhs);
	virtual ~CVIBuffer() = default;

public:
	void Create_Buffer(ID3D12Resource** GPU, ID3D12Resource** Uploader, const void* initData, UINT64 byteSize);

public:
	virtual HRESULT Render();

public:
	D3D12_VERTEX_BUFFER_VIEW	VertexBufferView()const;
	D3D12_INDEX_BUFFER_VIEW		IndexBufferView()const;
	D3D12_PRIMITIVE_TOPOLOGY	PrimitiveType()const;

protected:
	void DisposeUploaders();

protected:
	ID3D12Resource* m_VertexBufferGPU;
	ID3D12Resource* m_IndexBufferGPU;

#ifndef _DEBUG
	D3D12_GPU_VIRTUAL_ADDRESS	m_VertexBufferGPUaddress;
	D3D12_GPU_VIRTUAL_ADDRESS	m_IndexBufferGPUaddress;

#endif // !_DEBUG


protected:
	ID3D12Resource* m_VertexBufferUploader = nullptr;
	ID3D12Resource* m_IndexBufferUploader = nullptr;

protected:
	UINT						m_VertexByteStride = 0;
	UINT32						m_VertexNum = 0;
	UINT						m_VertexBufferByteSize = 0;

protected:
	DXGI_FORMAT					m_IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT32						m_IndexNum = 0;
	UINT						m_IndexBufferByteSize = 0;

protected:
	D3D12_PRIMITIVE_TOPOLOGY	m_PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

public:
	virtual HRESULT Initialize_Prototype() { return S_OK; }
	virtual HRESULT Initialize(void* pArg) { return S_OK; };

public:
	CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END