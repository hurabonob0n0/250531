#pragma once
#include "Engine_Defines.h"
#include "Engine_Config.h"

BEGIN(Engine)

/*===========================================================================
    CMeshModel(나무/드론/제트기)이 쓰는 모델 바이너리 형식.

    <파일 헤더>
        char  szMagic[4]    "JMSH"
        uint  iVersion
        uint  iNumMeshes
    <메시마다>
        uint      iVertexNum
        uint      iIndexNum
        VTXMESH   Vertices[iVertexNum]
        _ulong    Indices[iIndexNum]

    피벗행렬과 탄젠트 유무(type)는 구울 때 이미 반영되어 있다.
    그래서 읽는 쪽은 아무 계산 없이 GPU 로 그대로 올리기만 하면 된다.

    ※ 탱크(CModel/CMesh)는 예전부터 쓰던 TankMeshInfo_%d.bin 형식을 그대로 쓴다.
       (메시마다 파일 1개. CMesh::Initialize_Prototype 참고)
===========================================================================*/

#define MESHBINARY_MAGIC    "JMSH"
#define MESHBINARY_VERSION  1

typedef struct tagMeshBinaryHeader
{
    char    szMagic[4];
    _uint   iVersion;
    _uint   iNumMeshes;
} MESHBINARY_HEADER;

END
