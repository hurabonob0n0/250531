#pragma once
#include "Base.h"

BEGIN(Engine)

/*===========================================================================
    FBX -> .bin 굽기 (개발용)

    게임 실행 중에 FBX 를 파싱할 이유가 없다. assimp 는 40MB짜리 M1A2.FBX 하나를
    읽는 데만 몇 초가 걸리고, DLL 도 18MB(디버그)나 따라다닌다.
    그래서 최초 1회만 FBX 를 읽어 정점/인덱스를 그대로 .bin 으로 굽고,
    그 다음부터는 .bin 만 읽는다.

    Engine_Config.h 의 USE_ASSIMP_BAKE 가 0 이면 이 클래스의 함수들은
    아무것도 하지 않고 false 만 돌려준다(= assimp 코드가 통째로 빠진다).
===========================================================================*/
class ENGINE_DLL CModelBaker
{
public:
    /* 탱크(CModel) 전용. 메시 하나당 <strOutDirectory>/TankMeshInfo_%d.bin 을 만든다.
       CMesh::Initialize_Prototype 이 읽는 형식과 바이트 단위로 같아야 한다. */
    static bool Bake_Tank_Meshes(const string& strFBXPath, const string& strOutDirectory, _fmatrix PivotMatrix);

    /* 나무/드론/제트기(CMeshModel) 용. 모델 하나가 .bin 하나가 된다. */
    static bool Bake_MeshModel(const string& strFBXPath, const string& strBinPath, _fmatrix PivotMatrix, _uint iType);

public:
    static bool Is_File_Exist(const string& strPath);
};

END
