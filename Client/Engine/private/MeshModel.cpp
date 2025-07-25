#include "..\Public\MeshModel.h"
#include "VIBuffer_Mesh.h"

CMeshModel::CMeshModel(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList)
	: CComponent(pDevice, pCommandList)
{
}

CMeshModel::CMeshModel(const CMeshModel& rhs)
	: CComponent(rhs)
	, m_iNumMeshes(rhs.m_iNumMeshes)
{
	for (auto& pMesh : rhs.m_Meshes)
	{
		CVIBuffer_Mesh* pClonedMesh = dynamic_cast<CVIBuffer_Mesh*>(pMesh->Clone(nullptr));
		m_Meshes.push_back(pClonedMesh);
	}
}

HRESULT CMeshModel::Initialize_Prototype(const string& strModelFilePath)
{
	//_uint		iFlag = aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast | aiProcess_PreTransformVertices;
	//_uint		iFlag = aiProcessPreset_TargetRealtime_Fast | aiProcess_PreTransformVertices;
	_uint		iFlag = aiProcess_PreTransformVertices;

	/* m_pAIScene안에 .fbx파일에 담겨있던 정보들이 담긴다.  */
	m_pAIScene = m_Importer.ReadFile(strModelFilePath, iFlag);
	if (nullptr == m_pAIScene)
		return E_FAIL;

	if (FAILED(Ready_Meshes()))
		return E_FAIL;

	return S_OK;
}

HRESULT CMeshModel::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CMeshModel::Render(_uint iMeshIndex)
{
	/* iMeshIndex에 해당하는 메시에 영향을 주는 뼈들을 모아서 셰이더로 전달한다. */
	m_Meshes[iMeshIndex]->Render();

	return S_OK;
}

HRESULT CMeshModel::Ready_Meshes()
{
	/* 현재 모델을 구성하는 메시의 갯수. */
	m_iNumMeshes = m_pAIScene->mNumMeshes;

	m_Meshes.reserve(m_iNumMeshes);

	for (size_t i = 0; i < m_iNumMeshes; i++)
	{
		/* VB, IB를 만든다. */
		CVIBuffer_Mesh* pMesh = CVIBuffer_Mesh::Create(m_Device, m_CommandList, m_pAIScene->mMeshes[i]);
		if (nullptr == pMesh)
			return E_FAIL;

		m_Meshes.push_back(pMesh);
	}

	return S_OK;
}

CMeshModel* CMeshModel::Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pContext, const string& strModelFilePath)
{
	CMeshModel* pInstance = new CMeshModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(strModelFilePath)))
	{
		MSG_BOX("Failed to Created : CMeshModel");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CMeshModel::Clone(void* pArg)
{
	CMeshModel* pInstance = new CMeshModel(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMeshModel");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeshModel::Free()
{
	__super::Free();

	for (auto& pMesh : m_Meshes)
		Safe_Release(pMesh);

	m_Importer.FreeScene();

}
