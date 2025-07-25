#pragma once
#include "Client_Defines.h"
#include "RenderObject.h"

BEGIN(Engine)
class CMeshModel;
class CBBinding;
END

BEGIN(Client)

class CTree : public CRenderObject
{
	struct TreeInfo {
		TreeInfo();
		TreeInfo(_uint type, _float2 pos);
		_uint TreeType;
		_float3 Position;
		float fAngle;
		float fScale;
		bool isColl = false;
		float ColTime = 0.f;
		_float2 ColPos;

		CBBinding* m_CBBindingCom;

		void Set_Y(float y) { Position.y = y; }
	};

public:
	CTree();
	CTree(CTree& rhs);
	virtual ~CTree() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

public:
	void Set_Col_Tree(int index, _float2 ColPos );

private:
	void Make_TransformMatrix(TreeInfo TI) {
		m_TransformCom->Identity();
		m_TransformCom->Set_Scale(0.01f * TI.fScale);

		m_TransformCom->Turn(m_TransformCom->Get_State(CTransform::STATE_UP), XMConvertToRadians(TI.fAngle));

		_vector fallAxis = XMVector3Cross( XMVector4Normalize(XMVectorSet(-1.f, 0.f, -1.f, 0.f)), XMVector4Normalize(m_TransformCom->Get_State(CTransform::STATE_UP)));

		m_TransformCom->Turn(fallAxis, XMConvertToRadians(TI.ColTime) * 30.f);
		
		m_TransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(TI.Position.x, TI.Position.y, TI.Position.z, 1.f));
	}

private:
	void Save_TreeInfos();
	void Load_TreeInfos();

private:
	vector< CMeshModel*> m_VIBuffers;

	class CTerrain* m_Terrain;

	vector<TreeInfo> m_TreeInfos;

	_uint		m_Test = 0;

public:
	void Free() override;
	static CTree* Create();
	CRenderObject* Clone(void* pArg);

};

END