#pragma once
class GameLobby
{
public:
	GameLobby();
	~GameLobby();
public:
	void Initialize(HWND hand);
	void Update(void);
	void Late_Update(void);
	void Render(void);
	void Release(void);

public:
	bool GameStart() {
		if (is_start)
			return true;

		return false;
	};

	void InitBmps(HWND hand);

private:
	HDC			m_hDC;
	HWND		m_hWnd;

	int Timer = 0;
	bool		is_start = false;
	bool		is_Connected = false;
};

