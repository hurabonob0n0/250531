#include "Texture.h"
#include "GameInstance.h"

CTexture::CTexture() : CBase()
{
}

void CTexture::Initialize_Prototype(const wchar_t* Filename)
{
    CreateDDSTextureFromFile12(GETDEVICE,GETCOMMANDLIST, Filename, &m_Resource, &m_UploadHeap);
}

CTexture* CTexture::Create(const wchar_t* Filename)
{
    CTexture* pInstance = new CTexture();
    pInstance->Initialize_Prototype(Filename);
    return pInstance;
}

void CTexture::Free()
{
    Safe_Release(m_Resource);
    Safe_Release(m_UploadHeap);
}
