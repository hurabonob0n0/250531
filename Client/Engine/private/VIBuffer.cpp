#include "VIBuffer.h"


CVIBuffer::CVIBuffer(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList) : CComponent(pDevice, pCommandList)
{
}

CVIBuffer::CVIBuffer(CVIBuffer& rhs) : CComponent(rhs)
{
	m_VertexByteStride = rhs.m_VertexByteStride;
	m_VertexNum = rhs.m_VertexNum;
	m_VertexBufferByteSize = rhs.m_VertexBufferByteSize;

	m_IndexFormat = rhs.m_IndexFormat;
	m_IndexNum = rhs.m_IndexNum;
	m_IndexBufferByteSize = rhs.m_IndexBufferByteSize;

	m_PrimitiveType = rhs.m_PrimitiveType;

	m_VertexBufferGPU = rhs.m_VertexBufferGPU;
	m_IndexBufferGPU = rhs.m_IndexBufferGPU;

#ifndef _DEBUG
	m_VertexBufferGPUaddress = rhs.m_VertexBufferGPU->GetGPUVirtualAddress();
	m_IndexBufferGPUaddress = rhs.m_IndexBufferGPU->GetGPUVirtualAddress();
#endif

	Safe_AddRef(m_VertexBufferGPU);
	Safe_AddRef(m_IndexBufferGPU);

	Safe_AddRef(m_CommandList);
	Safe_AddRef(m_Device);
}

void CVIBuffer::Create_Buffer(ID3D12Resource** GPU, ID3D12Resource** Uploader, const void* initData, UINT64 byteSize)
{
	// 디폴트 버퍼 (GPU 전용) 생성
	if (FAILED(m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(GPU))))
		MSG_BOX("Failed to Create : VIBuffer");

	// 업로드 버퍼 (CPU 접근 가능) 생성
	if (FAILED(m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(Uploader))))
		MSG_BOX("Failed to Create : VIBufferUploader");

	// 복사할 데이터 설명
	void* mappedData = nullptr;
	CD3DX12_RANGE readRange(0, 0); // 읽기 안 할 거니까 0~0
	if (FAILED((*Uploader)->Map(0, &readRange, &mappedData)))
		MSG_BOX("Failed to Map Uploader");
	memcpy(mappedData, initData, byteSize);
	(*Uploader)->Unmap(0, nullptr);

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		*GPU,
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_COPY_DEST));

	// 업로드 버퍼 → 디폴트 버퍼 복사
	m_CommandList->CopyBufferRegion(*GPU, 0, *Uploader, 0, byteSize);

	// 디폴트 버퍼 상태 GENERIC_READ로 전이
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		*GPU,
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_GENERIC_READ));

	// 주의: 아직 CommandList가 실행되기 전이기 때문에 uploadBuffer는 해제하면 안된다!

}

HRESULT CVIBuffer::Render()
{
	m_CommandList->IASetVertexBuffers(0, 1, &VertexBufferView());
	m_CommandList->IASetIndexBuffer(&IndexBufferView());
	m_CommandList->IASetPrimitiveTopology(m_PrimitiveType);

	m_CommandList->DrawIndexedInstanced(m_IndexNum, 1, 0, 0, 0);

	return S_OK;
}

D3D12_VERTEX_BUFFER_VIEW CVIBuffer::VertexBufferView() const
{
	D3D12_VERTEX_BUFFER_VIEW vbv;
#ifndef _DEBUG
	vbv.BufferLocation = m_VertexBufferGPUaddress;//m_VertexBufferGPU->GetGPUVirtualAddress();
#else
	vbv.BufferLocation = m_VertexBufferGPU->GetGPUVirtualAddress();
#endif
	vbv.StrideInBytes = m_VertexByteStride;
	vbv.SizeInBytes = m_VertexBufferByteSize;

	return vbv;
}

D3D12_INDEX_BUFFER_VIEW CVIBuffer::IndexBufferView() const
{
	D3D12_INDEX_BUFFER_VIEW ibv;
#ifndef _DEBUG
	ibv.BufferLocation = m_IndexBufferGPUaddress;//m_IndexBufferGPU->GetGPUVirtualAddress();
#else
	ibv.BufferLocation = m_IndexBufferGPU->GetGPUVirtualAddress();
#endif
	ibv.Format = m_IndexFormat;
	ibv.SizeInBytes = m_IndexBufferByteSize;

	return ibv;
}

D3D12_PRIMITIVE_TOPOLOGY CVIBuffer::PrimitiveType() const
{
	return m_PrimitiveType;
}

void CVIBuffer::DisposeUploaders()
{
	Safe_Release(m_VertexBufferUploader);
	Safe_Release(m_IndexBufferUploader);
}

void CVIBuffer::Free()
{
	DisposeUploaders();

	Safe_Release(m_VertexBufferGPU);
	Safe_Release(m_IndexBufferGPU);

	__super::Free();
}

