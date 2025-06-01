#include "..\Public\Model.h"
#include "Bone.h"
#include "Mesh.h"
#include "Texture.h"
#include "Animation.h"

CModel::CModel(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList)
	: CComponent(pDevice, pCommandList)
{
}

CModel::CModel(const CModel & rhs)
	: CComponent(rhs)
	, m_iNumMeshes(rhs.m_iNumMeshes)
	//, m_Meshes(rhs.m_Meshes)
	, m_eModelType(rhs.m_eModelType)
	, m_PivotMatrix(rhs.m_PivotMatrix)
{
	for (auto& pPrototypeBone : rhs.m_Bones)
	{
		m_Bones.push_back(pPrototypeBone->Clone());
	}

	for (auto& pMesh : rhs.m_Meshes)
	{
		CMesh* pClonedMesh = dynamic_cast<CMesh*>(pMesh->Clone(nullptr));
		pClonedMesh->Set_Bone(m_Bones);
		m_Meshes.push_back(pClonedMesh);
	}
}

HRESULT CModel::Initialize_Prototype(TYPE eModelType, const string& strModelFilePath, _fmatrix PivotMatrix)
{
	m_eModelType = eModelType;

	/* aiProcess_PreTransformVertices : */
	/* 이 모델을 구성하는 메시들이 혹여 지정된 상태대로 그려져야한다면.(메시의 이름과 같은 이름을 가진 뼈가 존재한다면) */
	/* 그 지정된 상태행렬응ㄹ 이 메시의 정점에 미리 곱해서 로드한다. */
	/* aiProcess_PreTransformVertices옵션을 넣어서 로드하게되면 이미 정점에 뼈의 상태가 적용되어있는것이다. */
	/* 추후에 뼈를 움직이게되면 정점에 들어가있는 뼈 상태를 빼서 넣는다라는 복잡한과정. */
	/* 그래서 해당 옵션을 주고 로드하게되면 어심프자체에서 애니메이션에 관련된 정보를 삭제해버린다. */
	/* 이 옵션을 통해 로드하는 경우는 반드시 애니메이션이 없는 경우에만 처리를 해야되겠다. */
	_uint		iFlag = aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast;

	if (TYPE_NONANIM == eModelType)
		iFlag |= aiProcess_PreTransformVertices;

	/* m_pAIScene안에 .fbx파일에 담겨있던 정보들이 담긴다.  */	
	m_pAIScene = m_Importer.ReadFile(strModelFilePath, iFlag);
	if (nullptr == m_pAIScene)
		return E_FAIL;

	XMStoreFloat4x4(&m_PivotMatrix, PivotMatrix);

	/* m_pAIScene안에 들어가있는 정보들을 우리가 사용하기 좋은 형태로 생성, 저장해주는 작업을 수행하면 되겠다. */
	/* mRootNode : 모든 뼈의 가장 시작이되는 뼈다. */	
	/* 내 모델의 모든 뼈를 구분없이 다 로드한다. */
	/* WHY? 뼈들의 상태정보를 가진 뼈를 만들어내는 과정. */
	/* 뼈의 상태정보 = 내뼈의 상태 * 부모뼈의 상태 */
	/* 내뼈의 정보만 필요한게아니라 이 뼈의 부모가 대체 누구였는가?! */
	/* 부모자식간의 뼈의 관계 + 뼈의 상태를 로드하는 작업이 Ready_Bones함수의 역활이다. */
	/* aiNode */
	/*if (FAILED(Ready_Bones(m_pAIScene->mRootNode, -1)))
		return E_FAIL;*/
	Load_Tank_Bones();
	//Todo : 뼈행렬 어떻게 할 지 처리

	/* 모델 = 메시 + 메시 + ...*/
	/* 각각의 메시 = VB, IB */
	/* 결국, 메시의 정점과 인덱스들을 로드하는 작업. VB, IB를 생성한다. */
	if (FAILED(Ready_Meshes()))
		return E_FAIL;

	
	return S_OK;
}

HRESULT CModel::Initialize(void * pArg)
{
	return S_OK;
}

void CModel::Set_MatOffsets(_uint MatOffset)
{
	for (auto& Mesh : m_Meshes)
		Mesh->Set_MaterialIndexOffset(MatOffset);
}

void CModel::Set_MatIndex(_uint MeshIndex, _uint MatIndex)
{
	m_Meshes[MeshIndex]->Set_MaterialIndex(MatIndex);
}

void CModel::Invalidate_Bones()
{
	for (auto& pBone : m_Bones)
	{
		pBone->Invalidate_CombinedTransformationMatrix(m_Bones);
	}
}

void CModel::Update()
{
	for (auto& mesh : m_Meshes) {
		mesh->Update();
	}

	/*for (int i = 0; i < m_Meshes.size() - 1; ++i)
	{
		m_Meshes[i]->Update();
	}*/
}

/* 사전에 뼈의 상태들을 셰이더로 던진다. */
/* Bind_BoneMatrices()함수의 호출이 Render() 함수보다 먼저 호출되어있어야한다. */
/* 지금 그리는 정점들이 뼈의 상탤르 따라가야하기땜누에 .*/

/* 정점들을 그린다. */
HRESULT CModel::Render(_uint iMeshIndex)
{
	/* iMeshIndex에 해당하는 메시에 영향을 주는 뼈들을 모아서 셰이더로 전달한다. */
	m_Meshes[iMeshIndex]->Render();

	/*for (auto& Mesh : m_Meshes)
		Mesh->Render();*/

	return S_OK;
}

void CModel::Make_Root_Combined_Matrix(_fmatrix WorldMat)
{
	m_Bones[0]->Set_CombinedMatrix( m_Bones[0]->Get_BoneMatrix() * WorldMat);
}

void CModel::Multiply_Mesh_Combined_Matrix(_uint iMeshIndex, _fmatrix WorldMat)
{
	/* 메시의 상태행렬에 뼈의 상태행렬을 곱한다. */
	/* 메시의 상태행렬 = 뼈의 상태행렬 * 메시의 상태행렬 */
	m_Meshes[iMeshIndex]->m_Bone->Mul_CombinedTransformationMatrix(WorldMat);
}

void CModel::Set_Transform_Matrix(_uint iMeshIndex, _fmatrix WorldMat)
{
	m_Bones[iMeshIndex]->Set_TransformationMatrix(WorldMat);
}

void CModel::Save_For_Tank_Bones()
{
	struct BoneData {
		char        m_szName[MAX_PATH];
		_float4x4   m_TransformationMatrix;
		int         m_iParentBoneIndex;
	};

	BoneData bones[3];

	strcpy_s(bones[0].m_szName, "RootNode");
	XMStoreFloat4x4(&bones[0].m_TransformationMatrix, XMMatrixIdentity());
	bones[0].m_iParentBoneIndex = -1;

	strcpy_s(bones[1].m_szName, "PotabNode");
	XMStoreFloat4x4(&bones[1].m_TransformationMatrix, XMMatrixIdentity());
	bones[1].m_iParentBoneIndex = 0;

	strcpy_s(bones[2].m_szName, "PosinNode");
	XMStoreFloat4x4(&bones[2].m_TransformationMatrix, XMMatrixIdentity());
	bones[2].m_iParentBoneIndex = 1;

	std::ofstream fout("../bin/Models/Tank/TankBones", std::ios::binary);
	if (fout.is_open())
	{
		uint32_t iNumBones = 58;
		fout.write(reinterpret_cast<const char*>(&iNumBones), sizeof(uint32_t));

		for (int i = 0; i < 3; ++i)
		{
			fout.write(reinterpret_cast<const char*>(bones[i].m_szName), MAX_PATH);
			fout.write(reinterpret_cast<const char*>(&bones[i].m_TransformationMatrix), sizeof(_float4x4));
			fout.write(reinterpret_cast<const char*>(&bones[i].m_iParentBoneIndex), sizeof(int));
		}

		for (int i = 0; i < 55; ++i) {
			if (i == 0 || i == 1 || i == 2 || i == 3 || i == 4 || i == 6 || i == 7 || i == 8 || i == 9 || i == 10 || i == 11 || i == 12 || i == 13 || i == 14 || i == 15 || i == 16 || i == 17 || i == 18 ||
				i == 19 || i == 20 || i == 21 || i == 22 || i == 23 || i == 38 || i == 39)
			{
				m_Meshes[i]->m_Bone->m_iParentBoneIndex = 0;
				fout.write(reinterpret_cast<const char*>(m_Meshes[i]->m_Bone->m_szName), MAX_PATH);
				fout.write(reinterpret_cast<const char*>(&m_Meshes[i]->m_Bone->m_CombindTransformationMatrix), sizeof(_float4x4));
				fout.write(reinterpret_cast<const char*>(&m_Meshes[i]->m_Bone->m_iParentBoneIndex), sizeof(int));
			}
			else if (i == 5 || i == 25 || i == 27  || i == 31 || i == 40 || i == 41 || i == 43 || i == 45 || i == 47 || i == 49 || i == 52 || i == 53 || i == 54)
			{
				m_Meshes[i]->m_Bone->m_iParentBoneIndex = 1;
				fout.write(reinterpret_cast<const char*>(m_Meshes[i]->m_Bone->m_szName), MAX_PATH);
				fout.write(reinterpret_cast<const char*>(&m_Meshes[i]->m_Bone->m_CombindTransformationMatrix), sizeof(_float4x4));
				fout.write(reinterpret_cast<const char*>(&m_Meshes[i]->m_Bone->m_iParentBoneIndex), sizeof(int));
			}
			else if (i == 29 || i == 51 || i == 50 )
			{
				m_Meshes[i]->m_Bone->m_iParentBoneIndex = 2;
				fout.write(reinterpret_cast<const char*>(m_Meshes[i]->m_Bone->m_szName), MAX_PATH);
				fout.write(reinterpret_cast<const char*>(&m_Meshes[i]->m_Bone->m_CombindTransformationMatrix), sizeof(_float4x4));
				fout.write(reinterpret_cast<const char*>(&m_Meshes[i]->m_Bone->m_iParentBoneIndex), sizeof(int));
			}
			else if (i == 24 || i == 26 || i == 28 || i == 30 || i == 32 || i == 34 || i == 36 || i == 46 || i == 37 || i == 35 || i == 33 || i == 44 || i == 48 || i == 42)
			{
				m_Meshes[i]->m_Bone->m_iParentBoneIndex = -1;
				fout.write(reinterpret_cast<const char*>(m_Meshes[i]->m_Bone->m_szName), MAX_PATH);
				fout.write(reinterpret_cast<const char*>(&m_Meshes[i]->m_Bone->m_CombindTransformationMatrix), sizeof(_float4x4));
				fout.write(reinterpret_cast<const char*>(&m_Meshes[i]->m_Bone->m_iParentBoneIndex), sizeof(int));
			}

				
		}

		fout.close();
	}

	
}

void CModel::Load_Tank_Bones()
{
	std::ifstream fin("../bin/Models/Tank/TankBones", std::ios::binary);
	if (!fin.is_open())
	{
		MessageBoxA(nullptr, "Failed to open TankBones file for reading.", "Error", MB_OK);
		return;
	}

	uint32_t iNumBones = 0;
	fin.read(reinterpret_cast<char*>(&iNumBones), sizeof(uint32_t));

	m_Bones.clear();
	m_Bones.reserve(iNumBones);

	for (uint32_t i = 0; i < iNumBones; ++i)
	{
		char szName[MAX_PATH] = "";
		_float4x4 transformation = {};
		int iParentIndex = -1;

		fin.read(reinterpret_cast<char*>(szName), MAX_PATH);
		fin.read(reinterpret_cast<char*>(&transformation), sizeof(_float4x4));
		fin.read(reinterpret_cast<char*>(&iParentIndex), sizeof(int));


		CBone* pBone = CBone::Create(szName, transformation,iParentIndex);  // CBone 생성자 직접 사용
		/*strcpy_s(pBone->m_szName, szName);
		memcpy(&pBone->m_TransformationMatrix, &transformation, sizeof(_float4x4));
		XMStoreFloat4x4(&pBone->m_CombindTransformationMatrix, XMMatrixIdentity());
		pBone->m_iParentBoneIndex = iParentIndex;*/

		m_Bones.push_back(pBone);
	}

	fin.close();
}



HRESULT CModel::Ready_Meshes()
{
	/* 현재 모델을 구성하는 메시의 갯수. */
	m_iNumMeshes = m_pAIScene->mNumMeshes;

	m_Meshes.reserve(m_iNumMeshes);

	for (size_t i = 0; i < m_iNumMeshes; i++)
	{
		/* VB, IB를 만든다. */
		CMesh*			pMesh = CMesh::Create(m_Device, m_CommandList, m_eModelType, m_pAIScene->mMeshes[i], m_Bones, XMLoadFloat4x4(&m_PivotMatrix));
		if (nullptr == pMesh)
			return E_FAIL;

		m_Meshes.push_back(pMesh);
	}

	return S_OK;
}

HRESULT CModel::Ready_Bones(aiNode* pAINode, _int iParentBoneIndex)
{
	CBone*		pBone = CBone::Create(pAINode, iParentBoneIndex);
	if (nullptr == pBone)
		return E_FAIL;
			
	m_Bones.push_back(pBone);

	_int iParentIndex = m_Bones.size() - 1;

	for (size_t i = 0; i < pAINode->mNumChildren; i++)
	{
		Ready_Bones(pAINode->mChildren[i], iParentIndex);
	}
	return S_OK;
}

void CModel::Set_Matrix_to_Bone(_uint iBoneIndex, _matrix Mat)
{
	m_Bones[iBoneIndex]->Set_TransformationMatrix(Mat);
}

CModel * CModel::Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pContext, TYPE eModelType, const string & strModelFilePath, _fmatrix PivotMatrix)
{
	CModel* pInstance = new CModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eModelType, strModelFilePath, PivotMatrix)))
	{
		MSG_BOX("Failed to Created : CModel");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CModel::Clone(void * pArg)
{
	CModel* pInstance = new CModel(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CModel");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CModel::Free()
{
	__super::Free();

	for (auto& pBone : m_Bones)
		Safe_Release(pBone);

	m_Bones.clear();

	for (auto& pMesh : m_Meshes)
		Safe_Release(pMesh);

	m_Importer.FreeScene();

}
