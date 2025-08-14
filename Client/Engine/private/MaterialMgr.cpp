//#include "MaterialMgr.h"
//#include "GameInstance.h"
//
//IMPLEMENT_SINGLETON(CMaterialMgr)
//
//int CMaterialMgr::Add_Material(string matName, MaterialData matInstance)
//{
//	int index = 0;
//	for (auto pair : m_MatMap)
//	{
//		if (pair.first == matName)
//			return index;
//		++index;
//	}
//	m_MatMap[matName] = matInstance;
//	return index;
//}
//
//_uint CMaterialMgr::Get_Mat_Index(string matName)
//{
//	_uint index = 0;
//	for (auto& iter = m_MatMap.begin(); iter != m_MatMap.end(); ++iter) {
//		if (iter->first == matName)
//			return index;
//		++index;
//	}
//	return index;
//}
//
//_uint CMaterialMgr::Get_Mat_Size()
//{
//	_uint index = 0;
//	for (auto& iter = m_MatMap.begin(); iter != m_MatMap.end(); ++iter) {
//		++index;
//	}
//	return index;
//}
//
//void CMaterialMgr::Set_Materials()
//{
//	auto matBuffer = CGameInstance::Get_Instance()->Get_Current_FrameResource()->m_MaterialCB->Resource();
//	GETCOMMANDLIST->SetGraphicsRootShaderResourceView(2, matBuffer->GetGPUVirtualAddress());
//	//Todo: 커맨드리스트 가지고 있도록 하기.
//}
//
//void CMaterialMgr::Update_Mats()
//{
//	auto currMaterialBuffer = CGameInstance::Get_Instance()->Get_Current_FrameResource()->m_MaterialCB;
//	_uint index = 0;
//	for (auto& e : m_MatMap)
//	{
//		// Only update the cbuffer data if the constants have changed.  If the cbuffer
//		// data changes, it needs to be updated for each FrameResource.
//		MaterialData mat = e.second;
//
//		XMMATRIX matTransform = XMLoadFloat4x4(&mat.MatTransform);
//
//		MaterialData matData;
//		matData.DiffuseAlbedo = mat.DiffuseAlbedo;
//		matData.FresnelR0 = mat.FresnelR0;
//		matData.Roughness = mat.Roughness;
//		XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
//		matData.DiffuseMapIndex = mat.DiffuseMapIndex;
//		matData.NormalMapIndex = mat.NormalMapIndex;
//
//		currMaterialBuffer->CopyData(index, matData);
//		++index;
//		//Todo:나중에 프레임리소스별로 한 번씩 갱신하면된다.
//	}
//}
//
//void CMaterialMgr::Resize()
//{
//	m_MatMap.reserve(1000);
//}
//
//void CMaterialMgr::Free()
//{
//}
//


#include "MaterialMgr.h"
#include "GameInstance.h"

IMPLEMENT_SINGLETON(CMaterialMgr)

_uint CMaterialMgr::Add_Material(string matName, MaterialData matInstance)
{
	// vector를 순회하여 이름이 같은 재질이 있는지 확인
	for (_uint i = 0; i < m_Materials.size(); ++i)
	{
		if (m_Materials[i].first == matName)
			return i; // 이미 존재하면 해당 인덱스 반환
	}

	// 존재하지 않으면 vector 끝에 새로 추가
	m_Materials.push_back({ matName, matInstance });
	return m_Materials.size() - 1; // 새 인덱스 반환
}

_uint CMaterialMgr::Get_Mat_Index(string matName)
{
	for (_uint i = 0; i < m_Materials.size(); ++i)
	{
		if (m_Materials[i].first == matName)
			return i;
	}
	return m_Materials.size(); // 찾지 못하면 size 반환
}

_uint CMaterialMgr::Get_Mat_Size()
{
	return m_Materials.size();
}

void CMaterialMgr::Set_Materials()
{
	auto matBuffer = CGameInstance::Get_Instance()->Get_Current_FrameResource()->m_MaterialCB->Resource();
	GETCOMMANDLIST->SetGraphicsRootShaderResourceView(2, matBuffer->GetGPUVirtualAddress());
}

void CMaterialMgr::Update_Mats()
{
	auto currMaterialBuffer = CGameInstance::Get_Instance()->Get_Current_FrameResource()->m_MaterialCB;

	for (_uint i = 0; i < m_Materials.size(); ++i)
	{
		// 데이터 접근 시 .second를 사용
		MaterialData& mat = m_Materials[i].second;

		XMMATRIX matTransform = XMLoadFloat4x4(&mat.MatTransform);

		MaterialData matData;
		matData.DiffuseAlbedo = mat.DiffuseAlbedo;
		matData.FresnelR0 = mat.FresnelR0;
		matData.Roughness = mat.Roughness;
		XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
		matData.DiffuseMapIndex = mat.DiffuseMapIndex;
		matData.NormalMapIndex = mat.NormalMapIndex;

		currMaterialBuffer->CopyData(i, matData);
	}
}

void CMaterialMgr::Resize()
{
	m_Materials.reserve(1000);
}

void CMaterialMgr::Free()
{
	m_Materials.clear();
}