#include "stdafx.h"
#include "GameFramework.h"

void CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	::srand(timeGetTime());

	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	BuildFrameBuffer();

	BuildObjects();

	_tcscpy_s(m_pszFrameRate, _T("LabProject ("));
}

void CGameFramework::OnDestroy()
{
	ReleaseObjects();

	if (m_hBitmapFrameBuffer) ::DeleteObject(m_hBitmapFrameBuffer);
	if (m_hDCFrameBuffer) ::DeleteDC(m_hDCFrameBuffer);
}

void CGameFramework::BuildFrameBuffer()
{
	::GetClientRect(m_hWnd, &m_rcClient);

	HDC hDC = ::GetDC(m_hWnd);

	m_hDCFrameBuffer = ::CreateCompatibleDC(hDC);
	m_hBitmapFrameBuffer = ::CreateCompatibleBitmap(hDC, m_rcClient.right - m_rcClient.left, m_rcClient.bottom - m_rcClient.top);
	::SelectObject(m_hDCFrameBuffer, m_hBitmapFrameBuffer);

	::ReleaseDC(m_hWnd, hDC);
	::SetBkMode(m_hDCFrameBuffer, TRANSPARENT);
}

void CGameFramework::ClearFrameBuffer(DWORD dwColor)
{
	HPEN hPen = ::CreatePen(PS_SOLID, 0, dwColor);
	HPEN hOldPen = (HPEN)::SelectObject(m_hDCFrameBuffer, hPen);
	HBRUSH hBrush = ::CreateSolidBrush(dwColor);
	HBRUSH hOldBrush = (HBRUSH)::SelectObject(m_hDCFrameBuffer, hBrush);
	::Rectangle(m_hDCFrameBuffer, m_rcClient.left, m_rcClient.top, m_rcClient.right, m_rcClient.bottom);
	::SelectObject(m_hDCFrameBuffer, hOldBrush);
	::SelectObject(m_hDCFrameBuffer, hOldPen);
	::DeleteObject(hPen);
	::DeleteObject(hBrush);
}

void CGameFramework::PresentFrameBuffer()
{
	HDC hDC = ::GetDC(m_hWnd);
	::BitBlt(hDC, m_rcClient.left, m_rcClient.top, m_rcClient.right - m_rcClient.left, m_rcClient.bottom - m_rcClient.top, m_hDCFrameBuffer, m_rcClient.left, m_rcClient.top, SRCCOPY);
	::ReleaseDC(m_hWnd, hDC);
}

void CGameFramework::BuildObjects()
{
	m_pCamera = std::make_shared<CCamera>();
	m_pCamera->SetViewport(0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
	m_pCamera->GeneratePerspectiveProjectionMatrix(1.01f, 500.0f, 60.0f);
	m_pCamera->SetFOVAngle(60.0f);

	m_pCamera->GenerateOrthographicProjectionMatrix(1.01f, 50.0f, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);

	m_pPlayer = std::make_shared<CTank>();
	m_pPlayer->SetPosition(0.0f, 0.0f, -150.0f);
	m_pPlayer->SetCamera(m_pCamera);
	m_pPlayer->SetCameraOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));

	m_pScene = std::make_shared<CScene>(m_pPlayer);
	m_pScene->BuildObjects();

	m_nObjects = 10;
	m_vpEnemyObjects.reserve(m_nObjects);

	auto enemy1 = std::make_shared<CETank>(35.f, 0.f, -100.f);
	enemy1->SetTarget(m_pPlayer);

	auto enemy2 = std::make_shared<CETank>(-19.f, 0.f, -100.f);
	enemy2->SetTarget(m_pPlayer);

	auto enemy3 = std::make_shared<CETank>(25.f, 0.f, -50.f);
	enemy3->SetTarget(m_pPlayer);

	auto enemy4 = std::make_shared<CETank>(-15.f, 0.f, 50.f);
	enemy4->SetTarget(m_pPlayer);

	auto enemy5 = std::make_shared<CETank>(25.f, 0.f, 150.f);
	enemy5->SetTarget(m_pPlayer);

	auto enemy6 = std::make_shared<CETank>(35.f, 0.f, 170.f);
	enemy6->SetTarget(m_pPlayer);
	
	auto enemy7 = std::make_shared<CETank>(-10.f, 0.f, -110.f);
	enemy7->SetTarget(m_pPlayer);
	
	auto enemy8 = std::make_shared<CETank>(35.f, 0.f, -70.f);
	enemy8->SetTarget(m_pPlayer); 
	
	auto enemy9 = std::make_shared<CETank>(-25.f, 0.f, -10.f);
	enemy9->SetTarget(m_pPlayer);

	auto enemy10 = std::make_shared<CETank>(0.f, 0.f, 10.f);
	enemy10->SetTarget(m_pPlayer);

	m_vpEnemyObjects.emplace_back(enemy1);
	m_vpEnemyObjects.emplace_back(enemy2);
	m_vpEnemyObjects.emplace_back(enemy3);
	m_vpEnemyObjects.emplace_back(enemy4);	
	m_vpEnemyObjects.emplace_back(enemy5);
	m_vpEnemyObjects.emplace_back(enemy6);
	m_vpEnemyObjects.emplace_back(enemy7);
	m_vpEnemyObjects.emplace_back(enemy8);
	m_vpEnemyObjects.emplace_back(enemy9);
	m_vpEnemyObjects.emplace_back(enemy10);

	m_pScene->SetEnemy(m_vpEnemyObjects);
}

void CGameFramework::ReleaseObjects()
{

}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScene) m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);

	switch (nMessageID)
	{
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
		::SetCapture(hWnd);
		::GetCursorPos(&m_ptOldCursorPos);
		if (nMessageID == WM_RBUTTONDOWN)
		{
			m_pLockedObject = m_pScene->PickObjectPointedByCursor(LOWORD(lParam), HIWORD(lParam), m_pPlayer->m_pCamera.get());
			if (m_pLockedObject) m_pLockedObject->SetColor(RGB(0, 0, 0));
		}
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		::ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScene) m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);

	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			break;
		case 'E':
			m_pPlayer.get()->RotateTurret(+5.0f);
			break;
		case 'Q':
			m_pPlayer.get()->RotateTurret(-5.0f);
			break;
		case 'S':
			m_pPlayer.get()->RotateGun(+1.0f);
			break;
		case 'W':
			m_pPlayer.get()->RotateGun(-1.0f);
			break;
		case 'R':
			m_pPlayer->Reset();
			break;
		case 'T': {
			m_pPlayer = m_pPlayer->ChangeToType(m_pCamera);
			for (auto& enemyObject : m_vpEnemyObjects)
				enemyObject->SetTarget(m_pPlayer);
			m_pScene->SetPlayer(m_pPlayer);
			break;
		}
		case VK_CONTROL:
			m_pPlayer.get()->FireBullet(m_pLockedObject);
			m_pLockedObject = NULL;
			break;
		default:
			m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
			break;
		}
		break;
	default:
		break;
	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
			m_Timer.Stop();
		else
			m_Timer.Start();
		break;
	}
	case WM_SIZE:
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return(0);
}

void CGameFramework::ProcessInput()
{
	static UCHAR pKeyBuffer[256];
	if (GetKeyboardState(pKeyBuffer))
	{
		DWORD dwDirection = 0;
		if (pKeyBuffer[VK_UP] & 0xF0) dwDirection |= DIR_FORWARD;
		if (pKeyBuffer[VK_DOWN] & 0xF0) dwDirection |= DIR_BACKWARD;
		if (pKeyBuffer[VK_LEFT] & 0xF0) dwDirection |= DIR_LEFT;
		if (pKeyBuffer[VK_RIGHT] & 0xF0) dwDirection |= DIR_RIGHT;
		if (pKeyBuffer[VK_PRIOR] & 0xF0) dwDirection |= DIR_UP;
		if (pKeyBuffer[VK_NEXT] & 0xF0) dwDirection |= DIR_DOWN;
		if (dwDirection) m_pPlayer->Move(dwDirection, m_pPlayer->m_fVelocity);
	}

	if (GetCapture() == m_hWnd)
	{
		SetCursor(NULL);
		POINT ptCursorPos;
		GetCursorPos(&ptCursorPos);
		float cxMouseDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
		float cyMouseDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
		SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		if (cxMouseDelta || cyMouseDelta)
		{
			if (pKeyBuffer[VK_RBUTTON] & 0xF0) {
				m_pPlayer->m_pCamera->RoundRotate(cxMouseDelta);
			}
			else
				m_pPlayer->Rotate(0.0f, cxMouseDelta, 0.0f);
		}
	}

	m_pPlayer->Update(m_Timer.GetTimeElapsed());
}

void CGameFramework::AnimateObjects()
{
	float fTimeElapsed = m_Timer.GetTimeElapsed();
	if (m_pPlayer) m_pPlayer->Animate(fTimeElapsed);
	if (m_pScene) m_pScene->Animate(fTimeElapsed);
}

void CGameFramework::FrameAdvance()
{
	m_Timer.Tick(60.0f);

	ProcessInput();

	AnimateObjects();

	ClearFrameBuffer(RGB(255, 255, 255));

	std::shared_ptr<CCamera> pCamera{ m_pPlayer->GetCamera() };
	if (m_pScene) m_pScene->Render(m_hDCFrameBuffer, pCamera.get());

	PresentFrameBuffer();

	m_Timer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}


