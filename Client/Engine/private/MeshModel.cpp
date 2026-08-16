#include "..\Public\MeshModel.h"
#include "VIBuffer_Mesh.h"
#include "ModelBaker.h"

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

string CMeshModel::Make_BinPath(const string& strModelFilePath)
{
	const size_t iDot = strModelFilePath.find_last_of('.');
	const size_t iSlash = strModelFilePath.find_last_of("/\\");

	if (string::npos == iDot || (string::npos != iSlash && iDot < iSlash))
		return strModelFilePath + ".bin";

	return strModelFilePath.substr(0, iDot) + ".bin";
}

HRESULT CMeshModel::Initialize_Prototype(const string& strModelFilePath, _fmatrix PivotMatrix, _uint type)
{
	const string strBinPath = Make_BinPath(strModelFilePath);

	/* .bin 이 없으면(= 모델을 새로 넣었거나 처음 돌리는 것이면) FBX 에서 한 번 구워둔다.
	   그 다음부터는 이 분기로 들어오지 않는다. */
	if (!CModelBaker::Is_File_Exist(strBinPath))
	{
		if (!CModelBaker::Bake_MeshModel(strModelFilePath, strBinPath, PivotMatrix, type))
			return E_FAIL;
	}

	return Load_Binary(strBinPath);
}

HRESULT CMeshModel::Load_Binary(const string& strBinPath)
{
	std::ifstream fin(strBinPath, std::ios::binary);
	if (!fin.is_open())
	{
		MessageBoxA(nullptr, ("Failed to open model binary:\n" + strBinPath).c_str(), "Error", MB_OK);
		return E_FAIL;
	}

	MESHBINARY_HEADER Header{};
	fin.read(reinterpret_cast<char*>(&Header), sizeof(MESHBINARY_HEADER));

	if (0 != memcmp(Header.szMagic, MESHBINARY_MAGIC, sizeof(Header.szMagic)) ||
		MESHBINARY_VERSION != Header.iVersion)
	{
		MessageBoxA(nullptr, ("Model binary is not ours or is an old version:\n" + strBinPath +
			"\nDelete it and run once with USE_ASSIMP_BAKE = 1.").c_str(), "Error", MB_OK);
		return E_FAIL;
	}

	m_iNumMeshes = Header.iNumMeshes;
	m_Meshes.reserve(m_iNumMeshes);

	std::vector<VTXMESH> Vertices;
	std::vector<_ulong>  Indices;

	for (_uint i = 0; i < m_iNumMeshes; ++i)
	{
		_uint iVertexNum = 0, iIndexNum = 0;
		fin.read(reinterpret_cast<char*>(&iVertexNum), sizeof(_uint));
		fin.read(reinterpret_cast<char*>(&iIndexNum), sizeof(_uint));

		Vertices.resize(iVertexNum);
		Indices.resize(iIndexNum);

		fin.read(reinterpret_cast<char*>(Vertices.data()), std::streamsize(sizeof(VTXMESH) * iVertexNum));
		fin.read(reinterpret_cast<char*>(Indices.data()), std::streamsize(sizeof(_ulong) * iIndexNum));

		CVIBuffer_Mesh* pMesh = CVIBuffer_Mesh::Create(m_Device, m_CommandList,
			Vertices.data(), iVertexNum, Indices.data(), iIndexNum);
		if (nullptr == pMesh)
			return E_FAIL;

		m_Meshes.push_back(pMesh);
	}

	return S_OK;
}

HRESULT CMeshModel::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CMeshModel::Render(_uint iMeshIndex)
{
	/* iMeshIndex에 해당하는 메시만 그린다. */
	m_Meshes[iMeshIndex]->Render();

	return S_OK;
}

CMeshModel* CMeshModel::Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pContext, const string& strModelFilePath, _fmatrix PivotMatrix, _uint type)
{
	CMeshModel* pInstance = new CMeshModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(strModelFilePath, PivotMatrix, type)))
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

	m_Meshes.clear();
}
