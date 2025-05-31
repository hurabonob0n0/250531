#pragma once
#include "Component.h"

BEGIN(Engine)

class CFrameResourceMgr;
class CTransform;

class ENGINE_DLL CBBinding : public CComponent
{
private:
	CBBinding(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList,class CFrameResourceMgr* pFrameResource);
	CBBinding(CBBinding& rhs);
	virtual ~CBBinding() = default;

public:
	void Set_CBIndex();
	void Set_WorldMatrix(CTransform* pTransformCom);
	void Set_WorldMatrix(_matrix worldmat);
	void Set_WorldMatrix(_float4x4 worldMat);
	void Set_TexCoordMatrix(CTransform* pTexCoordTransformCom);
	void Set_TexCoordMatrix(_matrix TexCoordmat);
	void Set_MaterialIndex(_uint MatIndex);

private:
	UINT CalcConstantBufferByteSize(UINT byteSize)
	{
		return (byteSize + 255) & ~255;
	}

public:
	void Set_World_TexCoord_And_Update(CTransform* pTransformCom, CTransform* pTexCoordTransformCom);
	void Update_CBView();

public:
	void Set_On_Shader();

private:
	_uint m_ObjCBIndex = 0;
	ObjectConstants m_ObjConstants{};

private:
	CFrameResourceMgr* m_FrameResourceMgr = nullptr;

public:
	static CBBinding* Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList, class CFrameResourceMgr* pFrameResource);
	CComponent* Clone(void* pArg) override;
	virtual void Free();
};

END