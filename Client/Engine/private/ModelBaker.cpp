#include "ModelBaker.h"
#include "MeshBinary.h"

bool CModelBaker::Is_File_Exist(const string& strPath)
{
    std::ifstream fin(strPath, std::ios::binary);
    return fin.is_open();
}

#if USE_ASSIMP_BAKE

namespace
{
    /* aiMesh 하나를 우리 정점 형식으로 바꾼다.
       예전 CMesh::Ready_Anim_Mesh / CVIBuffer_Mesh::Ready_Mesh 가 하던 것과 똑같이 해야
       구워둔 .bin 이 예전 화면과 같은 결과를 낸다.
       iType 1 은 탄젠트가 없는 모델(드론/제트기)이다. */
    void Convert_Vertices(const aiMesh* pAIMesh, _fmatrix PivotMatrix, _uint iType, std::vector<VTXMESH>& Vertices)
    {
        Vertices.assign(pAIMesh->mNumVertices, VTXMESH{});

        for (_uint i = 0; i < pAIMesh->mNumVertices; ++i)
        {
            memcpy(&Vertices[i].vPosition, &pAIMesh->mVertices[i], sizeof(_float3));
            XMStoreFloat3(&Vertices[i].vPosition, XMVector3TransformCoord(XMLoadFloat3(&Vertices[i].vPosition), PivotMatrix));

            if (nullptr != pAIMesh->mNormals)
            {
                memcpy(&Vertices[i].vNormal, &pAIMesh->mNormals[i], sizeof(_float3));
                XMStoreFloat3(&Vertices[i].vNormal, XMVector3TransformNormal(XMLoadFloat3(&Vertices[i].vNormal), PivotMatrix));
            }

            if (nullptr != pAIMesh->mTextureCoords[0])
                memcpy(&Vertices[i].vTexcoord, &pAIMesh->mTextureCoords[0][i], sizeof(_float2));

            if (0 == iType && nullptr != pAIMesh->mTangents)
                memcpy(&Vertices[i].vTangent, &pAIMesh->mTangents[i], sizeof(_float3));
        }
    }

    void Convert_Indices(const aiMesh* pAIMesh, std::vector<_ulong>& Indices)
    {
        Indices.clear();
        Indices.reserve(size_t(pAIMesh->mNumFaces) * 3);

        for (_uint i = 0; i < pAIMesh->mNumFaces; ++i)
        {
            const aiFace& AIFace = pAIMesh->mFaces[i];
            Indices.push_back(AIFace.mIndices[0]);
            Indices.push_back(AIFace.mIndices[1]);
            Indices.push_back(AIFace.mIndices[2]);
        }
    }
}

bool CModelBaker::Bake_Tank_Meshes(const string& strFBXPath, const string& strOutDirectory, _fmatrix PivotMatrix)
{
    /* 예전 CModel::Initialize_Prototype(TYPE_ANIM) 이 쓰던 플래그 그대로. 하나라도 다르면
       메시 순서나 정점이 달라져서 TankBones(뼈 순서 고정)와 어긋난다. */
    Assimp::Importer Importer;
    const aiScene* pAIScene = Importer.ReadFile(strFBXPath, aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast);
    if (nullptr == pAIScene)
    {
        MessageBoxA(nullptr, ("Bake failed - cannot read FBX:\n" + strFBXPath).c_str(), "ModelBaker", MB_OK);
        return false;
    }

    for (_uint i = 0; i < pAIScene->mNumMeshes; ++i)
    {
        const aiMesh* pAIMesh = pAIScene->mMeshes[i];

        std::vector<VTXMESH> Vertices;
        std::vector<_ulong>  Indices;
        Convert_Vertices(pAIMesh, PivotMatrix, 0, Vertices);
        Convert_Indices(pAIMesh, Indices);

        char szFileName[MAX_PATH] = "";
        sprintf_s(szFileName, "%sTankMeshInfo_%d.bin", strOutDirectory.c_str(), i);

        std::ofstream fout(szFileName, std::ios::binary);
        if (!fout.is_open())
        {
            MessageBoxA(nullptr, ("Bake failed - cannot write:\n" + string(szFileName)).c_str(), "ModelBaker", MB_OK);
            return false;
        }

        char szName[MAX_PATH] = "";
        strcpy_s(szName, pAIMesh->mName.data);

        const _uint         iMaterialIndex = pAIMesh->mMaterialIndex;
        const _uint         iVertexNum = _uint(Vertices.size());
        const _uint         iIndexNum = _uint(Indices.size());
        const DXGI_FORMAT   IndexFormat = DXGI_FORMAT_R32_UINT;
        const D3D11_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        const _uint         iVertexByteSize = iVertexNum * sizeof(VTXMESH);
        const _uint         iIndexByteSize = iIndexNum * sizeof(_ulong);

        fout.write(reinterpret_cast<const char*>(&iMaterialIndex), sizeof(_uint));
        fout.write(szName, sizeof(char) * MAX_PATH);
        fout.write(reinterpret_cast<const char*>(&iVertexNum), sizeof(_uint));
        fout.write(reinterpret_cast<const char*>(&iIndexNum), sizeof(_uint));
        fout.write(reinterpret_cast<const char*>(&IndexFormat), sizeof(DXGI_FORMAT));
        fout.write(reinterpret_cast<const char*>(&PrimitiveType), sizeof(D3D11_PRIMITIVE_TOPOLOGY));
        fout.write(reinterpret_cast<const char*>(&iVertexByteSize), sizeof(_uint));
        fout.write(reinterpret_cast<const char*>(&iIndexByteSize), sizeof(_uint));
        fout.write(reinterpret_cast<const char*>(Vertices.data()), iVertexByteSize);
        fout.write(reinterpret_cast<const char*>(Indices.data()), iIndexByteSize);
    }

    return true;
}

bool CModelBaker::Bake_MeshModel(const string& strFBXPath, const string& strBinPath, _fmatrix PivotMatrix, _uint iType)
{
    /* 예전 CMeshModel::Initialize_Prototype 이 쓰던 플래그 그대로. */
    Assimp::Importer Importer;
    const aiScene* pAIScene = Importer.ReadFile(strFBXPath, aiProcess_PreTransformVertices);
    if (nullptr == pAIScene)
    {
        MessageBoxA(nullptr, ("Bake failed - cannot read FBX:\n" + strFBXPath).c_str(), "ModelBaker", MB_OK);
        return false;
    }

    std::ofstream fout(strBinPath, std::ios::binary);
    if (!fout.is_open())
    {
        MessageBoxA(nullptr, ("Bake failed - cannot write:\n" + strBinPath).c_str(), "ModelBaker", MB_OK);
        return false;
    }

    MESHBINARY_HEADER Header{};
    memcpy(Header.szMagic, MESHBINARY_MAGIC, sizeof(Header.szMagic));
    Header.iVersion = MESHBINARY_VERSION;
    Header.iNumMeshes = pAIScene->mNumMeshes;

    fout.write(reinterpret_cast<const char*>(&Header), sizeof(MESHBINARY_HEADER));

    for (_uint i = 0; i < pAIScene->mNumMeshes; ++i)
    {
        const aiMesh* pAIMesh = pAIScene->mMeshes[i];

        std::vector<VTXMESH> Vertices;
        std::vector<_ulong>  Indices;
        Convert_Vertices(pAIMesh, PivotMatrix, iType, Vertices);
        Convert_Indices(pAIMesh, Indices);

        const _uint iVertexNum = _uint(Vertices.size());
        const _uint iIndexNum = _uint(Indices.size());

        fout.write(reinterpret_cast<const char*>(&iVertexNum), sizeof(_uint));
        fout.write(reinterpret_cast<const char*>(&iIndexNum), sizeof(_uint));
        fout.write(reinterpret_cast<const char*>(Vertices.data()), sizeof(VTXMESH) * iVertexNum);
        fout.write(reinterpret_cast<const char*>(Indices.data()), sizeof(_ulong) * iIndexNum);
    }

    return true;
}

#else   /* USE_ASSIMP_BAKE == 0 : assimp 없이 빌드. .bin 이 없으면 그냥 실패한다. */

bool CModelBaker::Bake_Tank_Meshes(const string& strFBXPath, const string& strOutDirectory, _fmatrix PivotMatrix)
{
    MessageBoxA(nullptr, "Model .bin is missing and USE_ASSIMP_BAKE is 0.\n"
        "Set USE_ASSIMP_BAKE to 1 in Engine_Config.h, run once, then set it back.", "ModelBaker", MB_OK);
    return false;
}

bool CModelBaker::Bake_MeshModel(const string& strFBXPath, const string& strBinPath, _fmatrix PivotMatrix, _uint iType)
{
    MessageBoxA(nullptr, "Model .bin is missing and USE_ASSIMP_BAKE is 0.\n"
        "Set USE_ASSIMP_BAKE to 1 in Engine_Config.h, run once, then set it back.", "ModelBaker", MB_OK);
    return false;
}

#endif
