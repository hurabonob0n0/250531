#pragma once
#include "Client_Defines.h"
#include "Base.h"
#include "Timer.h"
#include "RawInput.h"
#include "GameInstance.h"
#include "MyPhysicsEngine.h"
#include "StateMgr.h"




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
	/*  창 모드 <-> 전체 화면 (Alt+Enter)

		★ 렌더 해상도(m_ClientWidth/Height = 백버퍼)는 건드리지 않는다.
		   창 크기만 바꾸고 1920x1080 백버퍼를 DXGI 가 늘려 그리게 둔다.
		   UI 좌표와 뷰포트가 전부 1920x1080 기준으로 짜여 있어서,
		   백버퍼를 같이 줄이면 UI 가 전부 어긋난다.                        */
	void Toggle_Fullscreen();
	void Apply_Fullscreen(bool isFullscreen);

private:
	/* 이 모니터에서 16:9 를 유지한 채 들어갈 수 있는 최대 창 크기 */
	void Calc_WindowedSize(int* pOutWidth, int* pOutHeight) const;

public:
	void Update(const CTimer* Timer);
	void Late_Update(const CTimer* Timer);

public:
	void Draw();

public: //----------- For Debug : FPS ---------------
	void CalculateFrameStats();

public://---For Server_Connect
	void ConnectServer();

	bool CanStart = false;
	void UpdateFMODListener();
	static void ExtractFMODPoseFromWorld(const DirectX::XMMATRIX& world,
		FMOD_VECTOR& outPos,
		FMOD_VECTOR& outForward,
		FMOD_VECTOR& outUp);

private:
	HINSTANCE	m_hAppInst = nullptr;
	HWND		m_hMainWnd = nullptr;

private:
	bool		m_AppPaused = false;
	bool		m_Resizing = false;
	bool		m_FullscreenState = false;

private:
	/*  ★ 이 둘은 '렌더 해상도(백버퍼)' 다. 창 크기가 아니다.
		한 번 정해지면 실행 내내 안 바뀐다. 창이 이보다 작으면 DXGI 가 줄여 그린다.  */
	int							m_ClientWidth = 1920;
	int							m_ClientHeight = 1080;
	std::wstring                m_MainWndCaption = L"Two the Tanks";

	/* 창 모드일 때의 창 크기와 위치. 전체 화면에서 돌아올 때 그대로 복원한다. */
	int							m_WindowedWidth = 1920;
	int							m_WindowedHeight = 1080;
	int							m_WindowedPosX = 0;
	int							m_WindowedPosY = 0;

private:
	CGameInstance* m_GameInstance = { nullptr };
	CRawInput* m_Input_Dev{ nullptr };
	CTimer* m_Timer = { nullptr };
	MyPhysicsEngine::CMyPhysicsEngine* m_PhysicsEngine = { nullptr };
	CStateMgr* GameModeMgr{ nullptr };

public:
	virtual void Free() override;
};

END