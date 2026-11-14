#include "Texture.h"
#include "GameInstance.h"

CTexture::CTexture() : CBase()
{
}

void CTexture::Initialize_Prototype(const wchar_t* Filename)
{
    HRESULT hr = CreateDDSTextureFromFile12(GETDEVICE,GETCOMMANDLIST, Filename, &m_Resource, &m_UploadHeap);
    if (FAILED(hr))
    {
        // 경로 오류, BC 포맷인데 가로/세로가 4의 배수가 아님 등
        wchar_t msg[512];
        swprintf_s(msg, L"%s\nHRESULT = 0x%08X", Filename, static_cast<unsigned int>(hr));
        MessageBoxW(nullptr, msg, L"Texture Load Failed", MB_OK | MB_ICONERROR);
    }

    CGameInstance::Get_Instance()->Execute_Flush_and_Reset();

    Safe_Release(m_UploadHeap);
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
