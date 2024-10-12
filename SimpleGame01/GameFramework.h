#pragma once
#include "Player.h"
#include "Scene.h"
#include "Timer.h"


class CGameFramework
{
public:
	CGameFramework() { }
	~CGameFramework() { }

public:
	void SetActive(bool bActive) { m_bActive = bActive; }

public:
	void OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void BuildFrameBuffer();
	void ClearFrameBuffer(DWORD dwColor);
	void PresentFrameBuffer();

	void BuildObjects();
	void ReleaseObjects();

	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);


private:
	HINSTANCE m_hInstance = NULL;
	HWND m_hWnd = NULL;

	int	m_nObjects = 0;
	bool m_bActive = true;

	RECT m_rcClient;

	HDC	m_hDCFrameBuffer = NULL;
	HBITMAP	m_hBitmapFrameBuffer = NULL;
	HBITMAP	m_hBitmapSelect = NULL;

	std::shared_ptr<CPlayer> m_pPlayer;
	std::shared_ptr<CScene> m_pScene;
	std::shared_ptr<CCamera> m_pCamera;
	std::vector<std::shared_ptr<CEnemy>> m_vpEnemyObjects;

	CGameObject* m_pLockedObject = NULL;

	CTimer m_Timer;

	POINT m_ptOldCursorPos;

	_TCHAR m_pszFrameRate[50];
};

