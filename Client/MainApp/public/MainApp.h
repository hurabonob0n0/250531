#pragma once
#include "Client_Defines.h"
#include "Base.h"
#include "Timer.h"
#include "RawInput.h"
#include "GameInstance.h"
#include "MyPhysicsEngine.h"

BEGIN(Client)

class CMainApp : public CBase
{
	DECLARE_SINGLETON(CMainApp)
private:
	CMainApp();
	virtual ~CMainApp() = default;

public:
	int Run();

public:
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	HRESULT Initialize_MainWindow(HINSTANCE g_hInstance);
	HRESULT Initialize(HINSTANCE g_hInstance);

public:
	void Update(const CTimer* Timer);
	void Late_Update(const CTimer* Timer);

public:
	void Draw();

public: //----------- For Debug : FPS ---------------
	void CalculateFrameStats();


private:
	HINSTANCE	m_hAppInst = nullptr;
	HWND		m_hMainWnd = nullptr;

private:
	bool		m_AppPaused = false;
	bool		m_Resizing = false;
	bool		m_FullscreenState = false;

private:
	int							m_ClientWidth = 1920;
	int							m_ClientHeight = 1080;
	std::wstring                m_MainWndCaption = L"Two the Tanks";

private:
	CGameInstance* m_GameInstance = { nullptr };
	CRawInput* m_Input_Dev{ nullptr };
	CTimer* m_Timer = { nullptr };
	MyPhysicsEngine::CMyPhysicsEngine* m_PhysicsEngine = { nullptr };

public:
	virtual void Free() override;
};


END