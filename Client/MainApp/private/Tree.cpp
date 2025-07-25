#include "Client_pch.h"
#include "Tree.h"
#include "Terrain.h"
#include "GameInstance.h"

CTree::CTree() : CRenderObject()
{
}

CTree::CTree(CTree& rhs) : CRenderObject(rhs)
{
}

HRESULT CTree::Initialize_Prototype()
{
	__super::Initialize_Prototype();

	return S_OK;
}

HRESULT CTree::Initialize(void* pArg)
{
	m_RG = CRenderer::RG_NONBLEND;

	__super::Initialize(pArg);

	/*m_VIBuffer1 = (CMeshModel*)m_GameInstance->Get_Component("TreeModel1");
	m_VIBuffer2 = (CMeshModel*)m_GameInstance->Get_Component("TreeModel2");
	m_VIBuffer3 = (CMeshModel*)m_GameInstance->Get_Component("TreeModel3");
	m_VIBuffer4 = (CMeshModel*)m_GameInstance->Get_Component("TreeModel4");
	m_VIBuffer5 = (CMeshModel*)m_GameInstance->Get_Component("TreeModel5");
	m_VIBuffer6 = (CMeshModel*)m_GameInstance->Get_Component("TreeModel6");*/

	m_VIBuffers.push_back((CMeshModel*)m_GameInstance->Get_Component("TreeModel1"));
	m_VIBuffers.push_back((CMeshModel*)m_GameInstance->Get_Component("TreeModel2"));
	m_VIBuffers.push_back((CMeshModel*)m_GameInstance->Get_Component("TreeModel3"));
	m_VIBuffers.push_back((CMeshModel*)m_GameInstance->Get_Component("TreeModel4"));
	m_VIBuffers.push_back((CMeshModel*)m_GameInstance->Get_Component("TreeModel5"));
	m_VIBuffers.push_back((CMeshModel*)m_GameInstance->Get_Component("TreeModel6"));

	//m_TerrainCom = (CVIBuffer_Terrain*)m_GameInstance->Get_Component("TerrainCom",this);
	m_Terrain = (CTerrain*)m_GameInstance->GetGameObject("Terrain", 0);
	
	//for (int i = 0; i < 150; ++i)
	//{
	//	m_TreeInfos.emplace_back(TreeInfo{ (_uint)m_GameInstance->Get_RandomI(0,5),_float2(m_GameInstance->Get_RandomF(-600.f,600.f),m_GameInstance->Get_RandomF(-600.f,600.f)) });
	//	m_TreeInfos[i].fScale= m_GameInstance->Get_RandomF(0.5f, 1.f);
	//	m_TreeInfos[i].Set_Y(m_Terrain->Get_Terrain_Heights(m_TreeInfos[i].Position.x, m_TreeInfos[i].Position.z) /*- m_TreeInfos[i].fScale * 2.f*/);
	//	m_TreeInfos[i].fAngle = m_GameInstance->Get_RandomI(0, 360);
	//}

	//for (int i = 0; i < 150; ++i)
	//{
	//	m_TreeInfos[i].m_CBBindingCom = (CBBinding*)CGameInstance::Get_Instance()->Get_Component("CBBindingCom", nullptr);

	//	m_TreeInfos[i].m_CBBindingCom->Set_TexCoordMatrix(m_TexCoordTransformCom);

	//	MaterialData MD{};

	//	XMStoreFloat4x4(&MD.MatTransform, XMMatrixIdentity());
	//	MD.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("BarkD", CTexture::Create(L"../bin/Models/Tree/Dead_Tree_02_Bark_Base_Color.dds"));
	//	MD.NormalMapIndex = CGameInstance::Get_Instance()->Add_Texture("BarkN", CTexture::Create(L"../bin/Models/Tree/Dead_Tree_02_Bark_Normal.dds"));
	//	m_TreeInfos[i].m_CBBindingCom->Set_MaterialIndex(CGameInstance::Get_Instance()->Add_Material("BarkMat", MD));
	//}

	Load_TreeInfos();

	return S_OK;
}

void CTree::Tick(float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	//m_TransformCom->Rotation(m_TransformCom->Get_State(CTransform::STATE_UP), 31.4f * fTimeDelta);

	/*if (m_GameInstance->Key_Down(VK_RIGHT))
		++m_Test;

	if (m_GameInstance->Key_Down(VK_LEFT))
		--m_Test;*/

	if (m_TreeInfos.size() == 0)
		return;

	/*if (m_GameInstance->Key_Down(VK_RETURN))
		Save_TreeInfos();*/

	/*for (int i = 0; i < 50; ++i)
	{
		Set_Col_Tree(i, _float2(-1, -1));
	}*/

	//Set_Col_Tree(0,_float2(-1,-1));
	//Set_Col_Tree(1, _float2(-1, -1));
	//Set_Col_Tree(2, _float2(-1, -1));
}

void CTree::LateTick(float fTimeDelta)
{
	__super::LateTick(fTimeDelta);

	if (m_TreeInfos.size() == 0)
		return;

	for (auto& Ti : m_TreeInfos)
	{
		if (Ti.isColl == true)
			Ti.ColTime += fTimeDelta*0.5f;
	}


//	m_TreeInfos.erase(m_TreeInfos.begin() + i);

	m_TreeInfos.erase(
		std::remove_if(m_TreeInfos.begin(), m_TreeInfos.end(),
			[](const TreeInfo& info) {
				return info.ColTime >= 3.f;
			}),
		m_TreeInfos.end());
	
}

void CTree::Render()
{

	if (m_TreeInfos.size() == 0)
		return;

	for (int i = 0; i < m_TreeInfos.size(); ++i)
	{
		TreeInfo TI = m_TreeInfos[i];

		TI.m_CBBindingCom->Set_CBIndex();

		Make_TransformMatrix(TI);

		TI.m_CBBindingCom->Set_WorldMatrix(m_TransformCom);
		
		TI.m_CBBindingCom->Update_CBView();

		TI.m_CBBindingCom->Set_On_Shader();

		if (TI.TreeType == 0 || TI.TreeType == 1)
		{
			m_VIBuffers[TI.TreeType]->Render(1);
			m_VIBuffers[TI.TreeType]->Render(3);
		}
		else
			m_VIBuffers[TI.TreeType]->Render(1);
	}

}

void CTree::Set_Col_Tree(int index, _float2 ColPos)
{
	m_TreeInfos[index].isColl = true;
	m_TreeInfos[index].ColPos = ColPos;
}

void CTree::Save_TreeInfos()
{
	struct SaveInfo {
		int TT;
		_float3 Pos;
		float Scale;
		float Angle;
	};
	SaveInfo SI{};

	std::ofstream fout("../bin/Models/Tree/TreeInfos", std::ios::binary);

	for (auto& TI : m_TreeInfos)
	{
		SI.TT = TI.TreeType;
		SI.Pos = _float3(TI.Position);
		SI.Scale = TI.fScale;
		SI.Angle = TI.fAngle;
		fout.write((const char*)&SI, sizeof(SaveInfo));
	}
}

void CTree::Load_TreeInfos()
{
	struct SaveInfo {
		int TT;
		_float3 Pos;
		float Scale;
		float Angle;
	};
	SaveInfo SI{};

	std::ifstream fin("../bin/Models/Tree/TreeInfos", std::ios::binary);

	m_TreeInfos.reserve(150);

	TreeInfo ti;

	for (int i = 0; i<150; ++i)
	{
		fin.read((char*)&SI, sizeof(SaveInfo));
		ti.TreeType = SI.TT;
		ti.Position.x = SI.Pos.x;
		ti.Position.z = SI.Pos.z;
		ti.Position.y = SI.Pos.y;
		ti.fAngle = SI.Angle;
		ti.fScale = SI.Scale;

		ti.m_CBBindingCom = (CBBinding*)CGameInstance::Get_Instance()->Get_Component("CBBindingCom", nullptr);

		ti.m_CBBindingCom->Set_TexCoordMatrix(m_TexCoordTransformCom);

		MaterialData MD{};

		XMStoreFloat4x4(&MD.MatTransform, XMMatrixIdentity());
		MD.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("BarkD", CTexture::Create(L"../bin/Models/Tree/Dead_Tree_02_Bark_Base_Color.dds"));
		MD.NormalMapIndex = CGameInstance::Get_Instance()->Add_Texture("BarkN", CTexture::Create(L"../bin/Models/Tree/Dead_Tree_02_Bark_Normal.dds"));
		ti.m_CBBindingCom->Set_MaterialIndex(CGameInstance::Get_Instance()->Add_Material("BarkMat", MD));
		
		m_TreeInfos.push_back(ti);
	}
}

void CTree::Free()
{
	for (auto& VIBuffer : m_VIBuffers)
		Safe_Release(VIBuffer);

	__super::Free();
}

CTree* CTree::Create()
{
	CTree* pInstance = new CTree;
	pInstance->Initialize_Prototype();
	return pInstance;
}

CRenderObject* CTree::Clone(void* pArg)
{
	CTree* pInstance = new CTree(*this);
	pInstance->Initialize(pArg);
	return pInstance;
}

CTree::TreeInfo::TreeInfo()
{
}

CTree::TreeInfo::TreeInfo(_uint type, _float2 pos)
{
	TreeType = type; Position = _float3(pos.x,0.f,pos.y); 
}
