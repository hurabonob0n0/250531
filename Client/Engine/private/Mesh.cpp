#include "Mesh.h"
#include "Bone.h"
#include "Shader.h"
#include "GameInstance.h"

int CMesh::g_MeshIndex = 0;

CMesh::CMesh(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pContext)
	: CVIBuffer(pDevice, pContext)
{
}

CMesh::CMesh(CMesh & rhs)
	: CVIBuffer(rhs)
{
	strcpy_s(m_szName, rhs.m_szName); 
	m_iMaterialIndex = rhs.m_iMaterialIndex;
}

HRESULT CMesh::Initialize_Prototype()
{
	char szFileName[MAX_PATH] = "";
	sprintf_s(szFileName, "../bin/Models/Tank/TankMeshInfo_%d.bin", g_MeshIndex++);

	// 2. 바이너리 읽기 모드로 파일 열기
	std::ifstream fin(szFileName, std::ios::binary);
	if (!fin.is_open())
	{
		// 파일을 열지 못했을 경우, 디버깅을 위해 어떤 파일을 못 열었는지 메시지 박스로 표시
		char szErrorMsg[MAX_PATH + 50];
		sprintf_s(szErrorMsg, "Failed to open Mesh file for reading:\n%s", szFileName);
		MessageBoxA(nullptr, szErrorMsg, "Error", MB_OK);
		return E_FAIL;
	}

	// 3. 메시 기본 정보(헤더)를 파일에서 읽어와 멤버 변수에 저장
	fin.read(reinterpret_cast<char*>(&m_iMaterialIndex), sizeof(_uint));
	fin.read(m_szName, sizeof(char) * MAX_PATH);
	fin.read(reinterpret_cast<char*>(&m_VertexNum), sizeof(_uint));
	fin.read(reinterpret_cast<char*>(&m_IndexNum), sizeof(_uint));
	fin.read(reinterpret_cast<char*>(&m_IndexFormat), sizeof(DXGI_FORMAT));
	// 저장 시 D3D11_PRIMITIVE_TOPOLOGY로 저장했으므로 동일한 타입으로 읽어야 합니다.
	fin.read(reinterpret_cast<char*>(&m_PrimitiveType), sizeof(D3D11_PRIMITIVE_TOPOLOGY));
	fin.read(reinterpret_cast<char*>(&m_VertexBufferByteSize), sizeof(_uint));
	fin.read(reinterpret_cast<char*>(&m_IndexBufferByteSize), sizeof(_uint));

	m_VertexByteStride = sizeof(VTXMESH);

	// 4. 정점 데이터를 담을 메모리를 동적 할당하고 파일에서 데이터 읽기
	VTXMESH* pVertices = new VTXMESH[m_VertexNum];
	fin.read(reinterpret_cast<char*>(pVertices), m_VertexBufferByteSize);

	// 5. 인덱스 데이터를 담을 메모리를 동적 할당하고 파일에서 데이터 읽기
	_ulong* pIndices = new _ulong[m_IndexNum];
	fin.read(reinterpret_cast<char*>(pIndices), m_IndexBufferByteSize);

	fin.close();

	// 6. 읽어온 데이터로 GPU 버퍼 생성
	__super::Create_Buffer(&m_VertexBufferGPU, &m_VertexBufferUploader, pVertices, m_VertexBufferByteSize);
	__super::Create_Buffer(&m_IndexBufferGPU, &m_IndexBufferUploader, pIndices, m_IndexBufferByteSize);

	// 7. 사용이 끝난 동적 할당 메모리 해제
	delete[] pVertices;
	pVertices = nullptr;
	delete[] pIndices;
	pIndices = nullptr;

	return S_OK;
}

HRESULT CMesh::Initialize(void * pArg)
{
	m_CBBinding = (CBBinding*)CGameInstance::Get_Instance()->Get_Component("CBBindingCom", nullptr);
	return S_OK;
}

void CMesh::Set_Bone(const vector<class CBone*> Bones)
{
	for (auto& pBone : Bones) {
		if (strcmp(m_szName, pBone->Get_BoneName()) == 0) {
			m_Bone = pBone;
			break;
		}
	}
	Safe_AddRef(m_Bone);
}

void CMesh::Set_Team(int team)
{
	m_CBBinding->Set_Pad1(team);
}

_float4x4 CMesh::Get_CombinedMatrix()
{
	return m_Bone->Get_CombinedMatrix();
}

void CMesh::Update()
{
	m_CBBinding->Set_CBIndex();
	_float4x4 mat = m_Bone->Get_CombinedMatrix();
	m_CBBinding->Set_WorldMatrix(mat);
	m_CBBinding->Set_TexCoordMatrix(XMMatrixIdentity());
	m_CBBinding->Set_TrackSag(m_TrackSagA, m_TrackSagB, m_TrackParam);

	m_CBBinding->Update_CBView();
}

HRESULT CMesh::Render()
{
	//Bind_CBView_On_Shader();
	m_CBBinding->Set_On_Shader();
	__super::Render();
	return S_OK;
}

HRESULT CMesh::Bind_CBView_On_Shader()
{
	m_CBBinding->Set_On_Shader();
	return S_OK;
}

void CMesh::Set_MaterialIndexOffset(_uint MatOffset)
{
	m_iMaterialIndex += MatOffset;
	m_CBBinding->Set_MaterialIndex(m_iMaterialIndex);
}

void CMesh::Set_MaterialIndex(_uint Matindex)
{
	m_CBBinding->Set_MaterialIndex(Matindex);
}

#if USE_ASSIMP_BAKE
HRESULT CMesh::Ready_NonAnim_Mesh(const aiMesh * pAIMesh, _fmatrix PivotMatrix)
{
	m_VertexByteStride = sizeof(VTXMESH);
	m_VertexBufferByteSize = m_VertexByteStride * m_VertexNum;

	VTXMESH* pVertices = new VTXMESH[m_VertexNum];
	ZeroMemory(pVertices, sizeof(VTXMESH) * m_VertexNum);

	for (size_t i = 0; i < m_VertexNum; i++)
	{
		memcpy(&pVertices[i].vPosition, &pAIMesh->mVertices[i], sizeof(_float3));
		XMStoreFloat3(&pVertices[i].vPosition, XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition), PivotMatrix));

		memcpy(&pVertices[i].vNormal, &pAIMesh->mNormals[i], sizeof(_float3));
		XMStoreFloat3(&pVertices[i].vNormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vNormal), PivotMatrix));

		memcpy(&pVertices[i].vTexcoord, &pAIMesh->mTextureCoords[0][i], sizeof(_float2));
		memcpy(&pVertices[i].vTangent, &pAIMesh->mTangents[i], sizeof(_float3));
	}

	__super::Create_Buffer(&m_VertexBufferGPU, &m_VertexBufferUploader, pVertices, m_VertexBufferByteSize);

	delete[] pVertices;
	pVertices = nullptr;

	return S_OK;
}

HRESULT CMesh::Ready_Anim_Mesh(const aiMesh * pAIMesh, const vector<class CBone*>& Bones, _fmatrix PivotMatrix)
{
	m_VertexByteStride = sizeof(VTXMESH);
	m_VertexBufferByteSize = m_VertexByteStride * m_VertexNum;


	VTXMESH* pVertices = new VTXMESH[m_VertexNum];
	ZeroMemory(pVertices, m_VertexBufferByteSize);

	for (size_t i = 0; i < m_VertexNum; i++)
	{
		memcpy(&pVertices[i].vPosition, &pAIMesh->mVertices[i], sizeof(_float3));
		XMStoreFloat3(&pVertices[i].vPosition, XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition), PivotMatrix));

		memcpy(&pVertices[i].vNormal, &pAIMesh->mNormals[i], sizeof(_float3));
		XMStoreFloat3(&pVertices[i].vNormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vNormal), PivotMatrix));

		memcpy(&pVertices[i].vTexcoord, &pAIMesh->mTextureCoords[0][i], sizeof(_float2));
		memcpy(&pVertices[i].vTangent, &pAIMesh->mTangents[i], sizeof(_float3));
	}

	__super::Create_Buffer(&m_VertexBufferGPU, &m_VertexBufferUploader, pVertices, m_VertexBufferByteSize);

	delete[] pVertices;
	pVertices = nullptr;

	return S_OK;
}
#endif

CMesh* CMesh::Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pContext)
{
	CMesh* pInstance = new CMesh(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMesh");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CMesh::Clone(void * pArg)
{
	CMesh* pInstance = new CMesh(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMesh");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMesh::Free()
{
	Safe_Release(m_Bone);
	Safe_Release(m_CBBinding);
	__super::Free();

}
