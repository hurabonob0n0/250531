#include "Client_pch.h"
#include "MainApp.h"
#include "DefaultObj.h"
#include "Camera_Free.h"
#include "BoxObj.h"
#include "Tank.h"
#include "Terrain.h"
#include "Effect.h"
#include "UICrossHair.h"
#include "WinningTeam.h"
#include "Tree.h"
#include "UIHP.h"
#include "UIReloading.h"
#include "UISelectPos.h"
#include "UIDamaged.h"
#include "UIKill.h"
#include "UITeamPercent.h"
#include "Drone.h"
#include "Camera_Drone.h"
#include "BulletPath.h"
#include "Zet.h"
#include "UINumber.h"
#include "UISkillBox.h"
#include "AirDrop.h"
#include "UI_DEFEAT.h"
#include "UI_VICTORY.h"
#include "FMOD_Manager.h"
#include "UITime.h"
/*-----------------
	For Server
-----------------*/
#include "Client_Globals.h"
#include "ThreadManager.h"
#include "Session.h"
#include "BufferReader.h"
#include "ClientPacketHandler.h"
#include "Network_Manager.h"
#include "ServiceManager.h"

/*----------------
	For Lobby
---------------*/
#include "GameLobby.h"

IMPLEMENT_SINGLETON(CMainApp)

bool RunLobbyWindowLoop(HINSTANCE hInstance, int showCmd);
LRESULT CALLBACK LobbyWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
HWND g_hWnd;



LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// CreateWindow가 완료되기 전에는 mhMainWnd는 유효하지 않습니다.
	// 하지만 CreateWindow가 완료되기 전에 메세지 (예를 들어 WM_CREATE)를
	// 받을 수 있기 때문에 hwnd를 전달합니다.
	return CMainApp::Get_Instance()->MsgProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd){
	//_CrtSetBreakAlloc(8420);

	if (!RunLobbyWindowLoop(hInstance, showCmd))
	return 0;

	CMainApp* mainApp = CMainApp::Get_Instance();
	if (FAILED(mainApp->Initialize(hInstance)))
		return 0;

	return mainApp->Run();
}

CMainApp::CMainApp() : m_Timer(CTimer::Get_Instance()), m_Input_Dev(CRawInput::Get_Instance())
{
}

static const XMFLOAT3 g_RespawnPositions[8] =
{
	XMFLOAT3(RESPAWNPOS_1),
	XMFLOAT3(RESPAWNPOS_2),
	XMFLOAT3(RESPAWNPOS_3),
	XMFLOAT3(RESPAWNPOS_4),
	XMFLOAT3(RESPAWNPOS_5),
	XMFLOAT3(RESPAWNPOS_6),
	XMFLOAT3(RESPAWNPOS_7),
	XMFLOAT3(RESPAWNPOS_8),
};


HRESULT CMainApp::Initialize(HINSTANCE g_hInstance)
{
	Initialize_MainWindow(g_hInstance);
	m_GameInstance = CGameInstance::Get_Instance();
	m_Input_Dev->Initialize(m_hMainWnd);

	WindowInfo WI;
	WI.Wincx = m_ClientWidth;
	WI.Wincy = m_ClientHeight;
	WI.windowHandle = m_hMainWnd;
	WI.isFullScreen = m_FullscreenState;

	m_GameInstance->Initialize(WI, m_Input_Dev);



	m_PhysicsEngine = MyPhysicsEngine::CMyPhysicsEngine::Get_Instance();
	m_PhysicsEngine->Initialize_PhysX();
	m_PhysicsEngine->Add_Terrain_From_File("../bin/Models/Terrain/HeightD.png", 1.f, 1.f);
	m_PhysicsEngine->MyPhysicsEngine::CMyPhysicsEngine::Add_Tank(0.f,40.f,0.f);


	m_GameInstance->AddPrototype("TransformCom", CTransform::Create(GETDEVICE, GETCOMMANDLIST));
	m_GameInstance->AddPrototype("VIBuffer_GeosCom", CVIBuffer_Geos::Create(GETDEVICE, GETCOMMANDLIST));
	m_GameInstance->AddPrototype("TerrainCom", CVIBuffer_Terrain::Create(GETDEVICE, GETCOMMANDLIST, "../bin/Models/Terrain/TerrainVertices"));
	m_GameInstance->AddPrototype("ModelCom", CModel::Create(m_GameInstance->Get_Device(), m_GameInstance->Get_CommandList(), CModel::TYPE_ANIM, "../bin/Models/Tank/M1A2.fbx",
		XMMatrixScaling(0.01f, 0.01f, 0.01f)));
	m_GameInstance->AddPrototype("VIBuffer_QuadCom", CVIBuffer_Quad::Create(GETDEVICE, GETCOMMANDLIST));

	m_GameInstance->AddPrototype("TreeModel1", CMeshModel::Create(m_GameInstance->Get_Device(), m_GameInstance->Get_CommandList(), "../bin/Models/Tree/Dead_Trunk_1.FBX", XMMatrixRotationX(XMConvertToRadians(-90.f))));
	m_GameInstance->AddPrototype("TreeModel2", CMeshModel::Create(m_GameInstance->Get_Device(), m_GameInstance->Get_CommandList(), "../bin/Models/Tree/Dead_Trunk_2.FBX", XMMatrixRotationX(XMConvertToRadians(-90.f))));
	m_GameInstance->AddPrototype("TreeModel3", CMeshModel::Create(m_GameInstance->Get_Device(), m_GameInstance->Get_CommandList(), "../bin/Models/Tree/Dead_Tree_3.FBX", XMMatrixRotationX(XMConvertToRadians(-90.f))));
	m_GameInstance->AddPrototype("TreeModel4", CMeshModel::Create(m_GameInstance->Get_Device(), m_GameInstance->Get_CommandList(), "../bin/Models/Tree/Dead_Tree_4.FBX", XMMatrixRotationX(XMConvertToRadians(-90.f))));
	m_GameInstance->AddPrototype("TreeModel5", CMeshModel::Create(m_GameInstance->Get_Device(), m_GameInstance->Get_CommandList(), "../bin/Models/Tree/Dead_Tree_5.FBX", XMMatrixRotationX(XMConvertToRadians(-90.f))));
	m_GameInstance->AddPrototype("TreeModel6", CMeshModel::Create(m_GameInstance->Get_Device(), m_GameInstance->Get_CommandList(), "../bin/Models/Tree/Dead_Tree_6.FBX", XMMatrixRotationX(XMConvertToRadians(-90.f))));

	m_GameInstance->AddPrototype("DroneModel", CMeshModel::Create(m_GameInstance->Get_Device(), m_GameInstance->Get_CommandList(), "../bin/Models/Drone/Drone.fbx",XMMatrixScaling(0.01f,0.01f,0.01f), 1));

	m_GameInstance->AddPrototype("ZetModel", CMeshModel::Create(m_GameInstance->Get_Device(), m_GameInstance->Get_CommandList(), "../bin/Models/Zet/Zet1.fbx",  
		XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(3.141592 * 0.5f) ,1));

	m_GameInstance->Add_PrototypeObject("Camera", CCamera_Free::Create());
	m_GameInstance->Add_PrototypeObject("DefaultObject", CDefaultObj::Create());
	m_GameInstance->Add_PrototypeObject("Effect", CEffect::Create());
	m_GameInstance->Add_PrototypeObject("Tank", CTank::Create());
	m_GameInstance->Add_PrototypeObject("Terrain", CTerrain::Create());
	m_GameInstance->Add_PrototypeObject("UICrossHair", CUICrossHair::Create());
	m_GameInstance->Add_PrototypeObject("UIHP", CUIHP::Create());
	m_GameInstance->Add_PrototypeObject("UIReloading", CUIReloading::Create());
	m_GameInstance->Add_PrototypeObject("UISelectPos", CUISelectPos::Create());
	m_GameInstance->Add_PrototypeObject("UIDamaged", CUIDamaged::Create());
	m_GameInstance->Add_PrototypeObject("UIKill", CUIKill::Create());
	m_GameInstance->Add_PrototypeObject("UITeamPercent", CUITeamPercent::Create());
	m_GameInstance->Add_PrototypeObject("UINumber", CUINumber::Create());
	m_GameInstance->Add_PrototypeObject("UISkillbox", CUISkillBox::Create());
	m_GameInstance->Add_PrototypeObject("UIAirDrop", CUI_AirDrop::Create());
	m_GameInstance->Add_PrototypeObject("DefeatUI", CUI_DEFEAT::Create());
	m_GameInstance->Add_PrototypeObject("VictoryUI", CUI_VICTORY::Create());
	m_GameInstance->Add_PrototypeObject("UITime", CUITime::Create());

	m_GameInstance->Add_PrototypeObject("WinningTeam", CWinningTeam::Create());
	m_GameInstance->Add_PrototypeObject("Tree", CTree::Create());
	m_GameInstance->Add_PrototypeObject("Drone", CDrone::Create());
	m_GameInstance->Add_PrototypeObject("Zet", CZet::Create());
	m_GameInstance->Add_PrototypeObject("Camera_Drone", CCamera_Drone::Create());
	m_GameInstance->Add_PrototypeObject("BulletPath", CBulletPath::Create());
	
	auto* FM = FMOD_Manager::Get_Instance();
	if (!FM->Initialize(512, /*distanceScale*/1.0f)) {
		MessageBox(nullptr, L"FMOD init failed", L"Error", MB_OK);
		return E_FAIL;
	}


	FM->LoadSound("Tank_Engine_Sound", "../bin/Sounds/Engine.wav",  true, true, true);
	FM->LoadSound("Tank_Track_Sound", "../bin/Sounds/Track.wav",   true, true, true);

	// 3D 원샷 = 발사/폭발
	FM->LoadSound("Tank_Shot", "../bin/Sounds/CannonShot.wav", true, false, false);
	FM->LoadSound("Explosion", "../bin/Sounds/WeaponHit.wav", true, false, false);

	FM->LoadSound("AirDrop", "../bin/Sounds/AirDrop.wav",
		/*is3D=*/false, /*loop=*/false, /*stream=*/false);


	FM->LoadSound("KillSound", "../bin/Sounds/KILL.wav",
		/*is3D=*/false, /*loop=*/false, /*stream=*/false);
	FM->LoadSound("DeadSound", "../bin/Sounds/DEAD.wav",
		/*is3D=*/false, /*loop=*/false, /*stream=*/false);

	// 감쇠 세팅(필요에 맞게 조정)
	FM->SetSound3DDistance("Tank_Engine_Sound", 3.0f, 70.0f);
	FM->SetSound3DRolloff("Tank_Engine_Sound", AudioRolloff::Inverse);

	FM->SetSound3DDistance("Tank_Track_Sound", 3.0f, 30.0f);
	FM->SetSound3DRolloff("Tank_Track_Sound", AudioRolloff::Linear);

	FM->SetSound3DDistance("Tank_Shot",3.0f, 20.0f);
	FM->SetSound3DRolloff("Tank_Shot", AudioRolloff::Linear);

	FM->SetSound3DDistance("Explosion", 7.0f, 200.0f);
	FM->SetSound3DRolloff("Explosion", AudioRolloff::Inverse);



	FM->LoadSound("Drone_Fly_Sound", "../bin/Sounds/DroneFly.wav", true, true, true);
	FM->SetSound3DDistance("Drone_Fly_Sound", 3.0f, 70.0f);
	FM->SetSound3DRolloff("Drone_Fly_Sound", AudioRolloff::Inverse);





	m_GameInstance->AddObject("Terrain", "Terrain", nullptr);

	if (Network_Manager::GetInstance()->isConnected()) {

		const auto& roomPlayers = Network_Manager::GetInstance()->GetRoomPlayers();
		int myPos = Network_Manager::GetInstance()->GetMyInGame_Data().Position; // 내 포지션

		// map<조종수 포지션, 해당 탱크에 탑승할 플레이어들>
		std::map<int, std::vector<Room_Ready_Data>> tankSlotMap;

		for (size_t i = 0; i < roomPlayers.size(); ++i)
		{
			const Room_Ready_Data& player = roomPlayers[i];

			int pos = player.Position;
			int keyPos = (pos % 2 == 0) ? pos - 1 : pos; // 짝수는 조종수 포지션으로 매핑
			tankSlotMap[keyPos].push_back(player);
		}

		int tankIndex = 0;

		for (std::map<int, std::vector<Room_Ready_Data>>::iterator it = tankSlotMap.begin(); it != tankSlotMap.end(); ++it)
		{
			int driverPosition = it->first;
			const std::vector<Room_Ready_Data>& playerList = it->second;

			// 팀 구분
			bool isBlue = (driverPosition <= 8);

			// 탱크 위치 설정
			float x = 50.f * tankIndex;
			float y = 40.f;
			float z = isBlue ? 50.f : 150.f;

			_matrix tankMat = XMMatrixTranslation(x, y, z);
			m_GameInstance->AddObject("Tank", "Tank", &tankMat);

			_matrix DroneMat = XMMatrixTranslation(x, y + 50.f, z);
			m_GameInstance->AddObject("Drone", "Drone", &DroneMat);
		

			CTank* tank = dynamic_cast<CTank*>(m_GameInstance->GetGameObject("Tank", tankIndex));
			CDrone* drone = dynamic_cast<CDrone*>(m_GameInstance->GetGameObject("Drone", tankIndex));

			if (tank && drone)
			{
				tank->Set_Team(isBlue + 1);

				// 탑승자 중 내 플레이어 포지션 확인
				for (size_t j = 0; j < playerList.size(); ++j)
				{
					const Room_Ready_Data& player = playerList[j];
					if (player.Position == myPos)  // <-- ID 대신 포지션 비교
					{
						drone->Set_My_Drone();
						tank->set_MyPlayer();

						tank->Set_Team(3);
						if (player.Position % 2 == 0) {

							Network_Manager::GetInstance()->MyPosMode = POS_POSU;
							CStateMgr::Get_Instance()->Set_GameMode(GM_FPS);
							Network_Manager::GetInstance()->MyControlTarget = CONTROL_POSIN;
						}
						else {

							Network_Manager::GetInstance()->MyPosMode = POS_DRIVER;
							CStateMgr::Get_Instance()->Set_GameMode(GM_TPS);
							Network_Manager::GetInstance()->MyControlTarget = CONTROL_TANK;
						}

						if (player.Position == 1) {
							Network_Manager::GetInstance()->MyPosMode = POS_MASTER;
							CStateMgr::Get_Instance()->Set_GameMode(GM_TPS);
							Network_Manager::GetInstance()->MyControlTarget = CONTROL_TANK;
						}

						Network_Manager::GetInstance()->SetMyTankIndex(tankIndex);
						if (tankIndex >= 0 && tankIndex < 8)
						{

							const XMFLOAT3& pos = g_RespawnPositions[tankIndex];
							tank->Set_MyPos(pos.x, pos.y, pos.z);
							XMFLOAT4X4 worldFloat4x4;
							XMStoreFloat4x4(&worldFloat4x4, tank->Get_WorldMatrix());
							drone->Set_My_DronePos_OnTank(worldFloat4x4);
						}
					}
				}
			}

			++tankIndex;
		}

	}
	else {
		Network_Manager::GetInstance()->MyPosMode = POS_MASTER;

		CStateMgr::Get_Instance()->Set_GameMode(GM_TPS);

		_matrix mat1 = XMMatrixTranslation(10.f, 40.f, 10.f);
		m_GameInstance->AddObject("Tank", "Tank", &mat1);
		dynamic_cast<CTank*>(m_GameInstance->GetGameObject("Tank", 0))->set_MyPlayer();
		dynamic_cast<CTank*>(m_GameInstance->GetGameObject("Tank", 0))->Set_MyPos(10.f, 40.f, 10.f);

		_matrix mat2 = XMMatrixTranslation(30.f, 40.f, 10.f);
		m_GameInstance->AddObject("Tank", "Tank", &mat2);



		CTank* tank = dynamic_cast<CTank*>(m_GameInstance->GetGameObject("Tank", 0));
		_matrix mat3 = XMMatrixTranslation(10.f, 80.f, 10.f);

		m_GameInstance->AddObject("Drone", "Drone", &mat3);
		dynamic_cast<CDrone*>(m_GameInstance->GetGameObject("Drone", 0))->Set_My_Drone();
		XMFLOAT4X4 worldFloat4x4;
		XMStoreFloat4x4(&worldFloat4x4, tank->Get_WorldMatrix());
		dynamic_cast<CDrone*>(m_GameInstance->GetGameObject("Drone", 0))->Set_My_DronePos_OnTank(worldFloat4x4);
	}


	_matrix matCamera = XMMatrixTranslation(0.f, 200.f, 0.f);
	m_GameInstance->AddObject("Camera", "Camera", &matCamera);

	m_GameInstance->AddObject("Camera_Drone", "Camera", nullptr);


	//TODOUI -> (모든 상황 전체 Render) 숫자 넣으면 그 숫자에 따라 출력되는 UI 만들기 (NumUI60X90),(NumUI30X40)
	//			TeamPercentUI변경
	//TODOUI -> (POSIN모드) -> AIR_STRIKE 스킬 UI, USEDRONE UI, AIRSTRIKE위에 쿨타임에 맞게 Render되는 빨간 쿨타임 박스 
	//TODOUI -> (Drone모드) -> FOLLW TANK UI, Drone화면UI, Drone모드에서 탱크 조준UI 지우기

	m_GameInstance->AddObject("UICrossHair", "UI", nullptr);
	m_GameInstance->AddObject("UIReloading", "UI", nullptr);
	m_GameInstance->AddObject("UIKill", "UI", nullptr);
	m_GameInstance->AddObject("UIDamaged", "UI", nullptr); //TODOUI Damaged와 Kill UI 줄이고, 이미지가 서로 바뀌어야 함
	m_GameInstance->AddObject("UITeamPercent", "UI", nullptr);
	m_GameInstance->AddObject("UISelectPos", "UI", nullptr);
	m_GameInstance->AddObject("UISkillbox", "UI", nullptr);
	m_GameInstance->AddObject("UIAirDrop", "UI", nullptr);
	m_GameInstance->AddObject("VictoryUI", "UI", nullptr);
	m_GameInstance->AddObject("DefeatUI", "UI", nullptr);
	m_GameInstance->AddObject("UINumber", "UINumber", nullptr); // 10	min
	m_GameInstance->AddObject("UINumber", "UINumber", nullptr);	// 1	min
	m_GameInstance->AddObject("UINumber", "UINumber", nullptr);	// 10	sec
	m_GameInstance->AddObject("UINumber", "UINumber", nullptr);	// 1	sec
	m_GameInstance->AddObject("UITime", "UI", nullptr);

	m_GameInstance->AddObject("UINumber", "UINumber", nullptr);	// 100	hp
	m_GameInstance->AddObject("UINumber", "UINumber", nullptr);	// 10	hp
	m_GameInstance->AddObject("UINumber", "UINumber", nullptr);	// 1	hp
	m_GameInstance->AddObject("UIHP", "UI", nullptr);//TODOUI -> HP바 변경, HP숫자 나오기

	//m_GameInstance->AddObject("Effect", "Effect", &mat3);
	//_matrix mat4 = XMMatrixScaling(30.f,30.f,30.f) * XMMatrixTranslation(0.f, 100.f, 0.f);
	//m_GameInstance->AddObject("WinningTeam", "WinningTeam", &mat3);


	m_GameInstance->AddObject("Tree", "Tree", nullptr);



	XMMATRIX matZet = XMMatrixTranslation(20.f, 60.f, 20.f);
	m_GameInstance->AddObject("Zet", "Zet", &matZet);

	m_GameInstance->m_ShadowMap->Set_My_Tank_Index(Network_Manager::GetInstance()->GetMyTankIndex());

	m_GameInstance->Execute_CommandList();

	m_GameInstance->Flush_CommandQueue();
	
	//((CUI_VICTORY*)m_GameInstance->GetGameObject("UI", 9))->set_render();
	//((CUI_DEFEAT*)m_GameInstance->GetGameObject("UI",10))->set_render();


	if (Network_Manager::GetInstance()->isConnected()) {
		auto sendBuffer = ClientPacketHandler::Make_C_LOADING_FINISH(1);
		Network_Manager::GetInstance()->Send(sendBuffer);
	}


	return S_OK;
}

int CMainApp::Run()
{
	MSG msg = { 0 };

	m_Timer->Reset();

	while (msg.message != WM_QUIT)
	{
		// 처리해야할 윈도우 메세지들이 있는지 확인합니다.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// 처리해야할 메세지가 없는 경우, 에니메이션과 게임을 처리합니다.
		else
		{
			m_Timer->Tick();

			bool isConnected = Network_Manager::GetInstance()->isConnected();
			bool isGameStart = Network_Manager::GetInstance()->GetGameStart();
			if (!m_AppPaused && (!isConnected || (isConnected && isGameStart)))
			{
				CalculateFrameStats();

				if (isConnected)
				{
					Network_Manager::GetInstance()->Dispatch(PacketQueueType::INGAME);
				}
				
				//CStateMgr::Set_GameMode((GameMode)(((int)CStateMgr::Get_GameMode()+1)%3));

				Update(m_Timer);
				m_PhysicsEngine->CMyPhysicsEngine::Update_PhysX(m_Timer->DeltaTime());
				Late_Update(m_Timer);
				UpdateFMODListener();
				FMOD_Manager::Get_Instance()->Update();
				Draw();

				m_Input_Dev->UpdateKeyStates();
				m_Input_Dev->ResetPerFrame();


				//RECT rect;
				//GetClientRect(m_hMainWnd, &rect);         // 클라이언트 영역 좌표
				//POINT center = {
				//	(rect.right - rect.left) / 2,
				//	(rect.bottom - rect.top) / 2
				//};
				// 
				//// 클라이언트 좌표 → 스크린 좌표로 변환
				//ClientToScreen(m_hMainWnd, &center);

				//SetCursorPos(center.x, center.y);
			}
			else
			{
				Sleep(100);
			}
		}
	}
	RELEASE_INSTANCE(CMainApp);


	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	return (int)msg.wParam;
}

LRESULT CMainApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// WM_ACTIVATE는 윈도우가 활성화 되거나 비활성화 될때 보내집니다.
		// 윈도우가 비활성화 되었을 때는 게임을 중지시키고
		// 다시 활성화 되었을 때는 게임을 다시 재게합니다.
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			m_AppPaused = true;
			m_Timer->Stop();
		}
		else
		{
			m_AppPaused = false;
			m_Timer->Start();
		}
		return 0;

		// WM_SIZE는 사용자가 윈도우 크기를 변경할 때 보내집니다.
	case WM_SIZE:
		// 새로운 윈도우 크기를 저장합니다.
		m_ClientWidth = LOWORD(lParam);
		m_ClientHeight = HIWORD(lParam);
		//m_FullscreenState = !m_FullscreenState;
		/*if(m_GameInstance)
			m_GameInstance->OnResize();*/
		return 0;

		// 윈도우가 파괴될 때 WM_DESTORY가 보내집니다.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// WM_MENUCAR는 메뉴가 활성화 될때 보내집니다.
		// 사용자가 니모닉, 가속기 키에 해당하지 않는 키를 누릅니다.
	case WM_MENUCHAR:
		// 알트 + 엔터를 입력시 비프음이 발생하는것을 방지합니다.
		return MAKELRESULT(0, MNC_CLOSE);

	case WM_INPUT:
		m_Input_Dev->Update_InputDev(lParam);
		return 0;

	case WM_KEYUP:
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}


HRESULT CMainApp::Initialize_MainWindow(HINSTANCE g_hInstance)
{
	m_hAppInst = g_hInstance;

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hAppInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"MainWnd";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// 클라이언트의 크기를 기반으로 윈도우 사각형을 계산합니다.
	/*RECT R = { 0, 0, m_ClientWidth, m_ClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;*/

	/*DWORD style = WS_VISIBLE | WS_POPUP;

	m_hMainWnd = CreateWindow(L"MainWnd", m_MainWndCaption.c_str(),
		style & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, m_ClientWidth, m_ClientHeight, 0, 0, m_hAppInst, 0);*/

	RECT R = { 0, 0, m_ClientWidth, m_ClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	m_hMainWnd = CreateWindow(L"MainWnd", m_MainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, m_hAppInst, 0);

	if (!m_hMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return E_FAIL;
	}

	ShowWindow(m_hMainWnd, SW_SHOW);
	UpdateWindow(m_hMainWnd);

	return S_OK;
}



void CMainApp::Update(const CTimer* Timer)
{
	m_GameInstance->Update(m_Timer);
}

void CMainApp::Late_Update(const CTimer* Timer)
{
	m_GameInstance->Late_Update(m_Timer);
}

void CMainApp::Draw()
{
	m_GameInstance->Draw();
}

void CMainApp::CalculateFrameStats()
{
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	++frameCnt;

	// 1초 동안의 평균을 계산합니다.
	if ((m_Timer->TotalTime() - timeElapsed >= 1.0f))
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1;
		float mspf = 1000.0f / fps;

		wstring fpsStr = to_wstring(fps);
		wstring mpsfStr = to_wstring(mspf);
		wstring ClientId = to_wstring(g_PlayerID);

		wstring windowText = m_MainWndCaption +
			L"    fps: " + fpsStr +
			L"   mfps: " + mpsfStr +
			L"   ClientID: " + ClientId;

		SetWindowText(m_hMainWnd, windowText.c_str());

		// 다음 평균을 위해 초기화 합니다.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}



void CMainApp::Free()
{
	m_GameInstance->Release_Engine();
	m_PhysicsEngine->Release_Instance();

	Safe_Release(m_Timer);
	Safe_Release(m_Input_Dev);
	Safe_Release(m_GameInstance);
}

bool RunLobbyWindowLoop(HINSTANCE hInstance, int showCmd)
{
	HWND hWnd;
	MSG msg = { 0 };

	// 윈도우 클래스 등록
	WNDCLASS wc = {};
	wc.lpfnWndProc = LobbyWndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"LobbyWnd";
	RegisterClass(&wc);

	RECT rc{ 0,0,LOBBY_WINCX,LOBBY_WINCY };

	hWnd = CreateWindow(L"LobbyWnd", L"게임 로비", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);

	ShowWindow(hWnd, showCmd);
	UpdateWindow(hWnd);

	g_hWnd = hWnd;
	
	GameLobby Game_Lobby;

	Game_Lobby.Initialize(hWnd);

	constexpr double desiredFPS = 60.0;
	constexpr double desiredFrameTime = 1000.0 / desiredFPS; // 16.66 ms

	bool bStartGame = false;

	while (true)
	{
		auto frameStart = chrono::high_resolution_clock::now();
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Game_Lobby.Update();
			Game_Lobby.Late_Update();
			Game_Lobby.Render();
			if (Game_Lobby.GameStart())
			{
				DestroyWindow(hWnd);
				bStartGame = true;
				break;
			}
		}
		//auto frameEnd = chrono::high_resolution_clock::now();
		//auto elapsed = chrono::duration_cast<chrono::milliseconds>(frameEnd - frameStart).count();

		//if (elapsed < desiredFrameTime)
		//{
		//	Sleep(static_cast<DWORD>(desiredFrameTime - elapsed));
		//}
	}

	Game_Lobby.Release();
	return bStartGame;
}

LRESULT CALLBACK LobbyWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		//PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}


void CMainApp::UpdateFMODListener()
{
    const bool isDrone = (Network_Manager::GetInstance()->MyControlTarget == CONTROL_DRONE);

    XMMATRIX worldMat;
    if (isDrone) {
        auto* cam = static_cast<CCamera_Drone*>(m_GameInstance->GetGameObject("Camera", 1));
        if (!cam) return;
        worldMat = cam->Get_WorldMatrix();
    } else {
        auto* cam = static_cast<CCamera_Free*>(m_GameInstance->GetGameObject("Camera", 0));
        if (!cam) return;
        worldMat = cam->Get_WorldMatrix();
    }

    // 2) 월드 행렬 → 오디오 포즈 (FMOD_VECTOR)
    FMOD_VECTOR pos{}, fwd{}, up{};
    ExtractFMODPoseFromWorld(worldMat, pos, fwd, up);

    // 3) 도플러용 속도 (카메라 전환 시 스파이크 방지)
    static FMOD_VECTOR prevPos{0,0,0};
    static bool hasPrev = false;
    static bool prevDrone = isDrone;
    FMOD_VECTOR vel{0,0,0};

    if (prevDrone != isDrone) {
        prevPos = pos;
        hasPrev = true;
        prevDrone = isDrone;
    }

    float dt = m_Timer->DeltaTime();
    if (hasPrev && dt > 0.f) {
        vel.x = (pos.x - prevPos.x) / dt;
        vel.y = (pos.y - prevPos.y) / dt;
        vel.z = (pos.z - prevPos.z) / dt;
    }
    prevPos = pos; hasPrev = true;

    // 4) FMOD 리스너 갱신 (오버로드 추가 필요)
    FMOD_Manager::Get_Instance()->SetListener(pos, vel, fwd, up);
}


void CMainApp::ExtractFMODPoseFromWorld(const XMMATRIX& world,
	FMOD_VECTOR& outPos,
	FMOD_VECTOR& outForward,
	FMOD_VECTOR& outUp)
{
	XMFLOAT4X4 m; XMStoreFloat4x4(&m, world);
	outPos = { m._41, m._42, m._43 };
	outForward = { m._31, m._32, m._33 }; // +Z forward 기준
	outUp = { m._21, m._22, m._23 };

	// 필요시 정규화:
	auto norm = [](FMOD_VECTOR& v) {
		float L = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
		if (L > 1e-6f) { v.x /= L; v.y /= L; v.z /= L; }
		};
	norm(outForward);
	norm(outUp);
}