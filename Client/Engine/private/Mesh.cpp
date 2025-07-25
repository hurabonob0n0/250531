#include "Mesh.h"
#include "Bone.h"
#include "Shader.h"
#include "GameInstance.h"

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

HRESULT CMesh::Initialize_Prototype(CModel::TYPE eModelType, const aiMesh * pAIMesh, const vector< class CBone*>& Bones, _fmatrix PivotMatrix)
{
	m_iMaterialIndex = pAIMesh->mMaterialIndex;
	strcpy_s(m_szName, pAIMesh->mName.data);

	//std::ofstream fout("../bin/MeshInfo.txt", std::ios::app); // append 모드
	//if (fout.is_open())
	//{
	//	fout << g_MeshNum++ << ". " << "MeshName: " << m_szName << ", MaterialIndex: " << m_iMaterialIndex << std::endl;
	//	fout.close();
	//}

	m_VertexNum = pAIMesh->mNumVertices;
	m_IndexNum = pAIMesh->mNumFaces * 3;
	m_IndexFormat = DXGI_FORMAT_R32_UINT;
	m_PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_IndexBufferByteSize = m_IndexNum * sizeof(std::uint32_t);

	//47872 * 3

#pragma region VERTEX_BUFFER

	HRESULT		hr = eModelType == CModel::TYPE_NONANIM ? Ready_NonAnim_Mesh(pAIMesh, PivotMatrix) : Ready_Anim_Mesh(pAIMesh, Bones, PivotMatrix);
	if (FAILED(hr))
		return E_FAIL;
	
#pragma endregion


#pragma region INDEX_BUFFER
	_ulong* pIndices = new _ulong[m_IndexNum];
	ZeroMemory(pIndices, m_IndexBufferByteSize);

	_uint		iNumIndices = 0;

	for (size_t i = 0; i < pAIMesh->mNumFaces; ++i)
	{
		aiFace	AIFace = pAIMesh->mFaces[i];

		pIndices[iNumIndices++] = AIFace.mIndices[0];
		pIndices[iNumIndices++] = AIFace.mIndices[1];
		pIndices[iNumIndices++] = AIFace.mIndices[2];
	}

	__super::Create_Buffer(&m_IndexBufferGPU, &m_IndexBufferUploader, pIndices, m_IndexBufferByteSize);

	delete[] pIndices;
	pIndices = nullptr;
#pragma endregion

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

void CMesh::Update()
{
	m_CBBinding->Set_CBIndex();
	_float4x4 mat = m_Bone->Get_CombinedMatrix();
	m_CBBinding->Set_WorldMatrix(mat);
	m_CBBinding->Set_TexCoordMatrix(XMMatrixIdentity());

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

	/* 메시의 뼈의 갯수(mNumBones). (전체뼈의 갯수( x), 이 메시에게 영향ㅇ르 주는 뼈의 갯수(o) */
	/* vBlendIndices : 이 메시에게 영향ㅇ르 주는 뼈기준의 인덱스들 */
	/* vBlendWeights : */
	
	/*for (auto& pBone : Bones) {
		if (strcmp(m_szName, pBone->Get_BoneName()) == 0) {
			m_Bone = pBone;
			break;
		}
	}
	
	Safe_AddRef(m_Bone);*/

	__super::Create_Buffer(&m_VertexBufferGPU, &m_VertexBufferUploader, pVertices, m_VertexBufferByteSize);

	/*std::ofstream fout("../bin/Models/Tank/TankVertecies", std::ios::binary);
	if (fout.is_open())
	{
		fout.write(reinterpret_cast<const char*>(pVertices), m_VertexBufferByteSize);
		fout.close();
	}
	else
	{
		MessageBoxA(nullptr, "Failed to open TankVertecies file for writing.", "Error", MB_OK);
	}*/

	delete[] pVertices;
	pVertices = nullptr;

	return S_OK;
}

CMesh* CMesh::Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pContext, CModel::TYPE eModelType, const aiMesh* pAIMesh, const vector<class CBone*>& Bones, _fmatrix PivotMatrix)
{
	CMesh* pInstance = new CMesh(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eModelType, pAIMesh, Bones, PivotMatrix)))
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
