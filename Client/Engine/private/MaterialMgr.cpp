#include "MaterialMgr.h"
#include "GameInstance.h"

IMPLEMENT_SINGLETON(CMaterialMgr)

int CMaterialMgr::Add_Material(string matName, MaterialData matInstance)
{
	int index = 0;
	for (auto pair : m_MatMap)
	{
		if (pair.first == matName)
			return index;
		++index;
	}
	m_MatMap[matName] = matInstance;
	return index;
}

_uint CMaterialMgr::Get_Mat_Index(string matName)
{
	_uint index = 0;
	for (auto& iter = m_MatMap.begin(); iter != m_MatMap.end(); ++iter) {
		if (iter->first == matName)
			return index;
		++index;
	}
	return index;
}

void CMaterialMgr::Set_Materials()
{
	auto matBuffer = CGameInstance::Get_Instance()->Get_Current_FrameResource()->m_MaterialCB->Resource();
	GETCOMMANDLIST->SetGraphicsRootShaderResourceView(2, matBuffer->GetGPUVirtualAddress());
	//Todo: 커맨드리스트 가지고 있도록 하기.
}

void CMaterialMgr::Update_Mats()
{
	auto currMaterialBuffer = CGameInstance::Get_Instance()->Get_Current_FrameResource()->m_MaterialCB;
	_uint index = 0;
	for (auto& e : m_MatMap)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		MaterialData mat = e.second;

		XMMATRIX matTransform = XMLoadFloat4x4(&mat.MatTransform);

		MaterialData matData;
		matData.DiffuseAlbedo = mat.DiffuseAlbedo;
		matData.FresnelR0 = mat.FresnelR0;
		matData.Roughness = mat.Roughness;
		XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
		matData.DiffuseMapIndex = mat.DiffuseMapIndex;
		matData.NormalMapIndex = mat.NormalMapIndex;

		currMaterialBuffer->CopyData(index, matData);
		++index;
		//Todo:나중에 프레임리소스별로 한 번씩 갱신하면된다.
	}
}

void CMaterialMgr::Resize()
{
	m_MatMap.reserve(1000);
}

void CMaterialMgr::Free()
{
}

