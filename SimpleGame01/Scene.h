#pragma once
#include "GameObject.h"
#include "Camera.h"
#include "Player.h"
#include "Enemy.h"

class CScene
{
public:
	CScene(std::shared_ptr<CPlayer> pPlayer);
	virtual ~CScene() { };

public:
	void SetPlayer(std::shared_ptr<CPlayer> pPlayer) { m_pPlayer = pPlayer; }
	void SetEnemy(vector<std::shared_ptr<CEnemy>>& vpEnemyObjects) { m_vpEnemyObjects = vpEnemyObjects; }

public:
	void CheckObjectByBulletCollisions();
	void CheckObjectByWallCollisions();
	void CheckWallByPlayerCollisions();
	void CheckBulletByPlayerCollisions();
	CGameObject* PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera);

public:
	virtual void BuildObjects();
	virtual void ReleaseObjects();

	virtual void Animate(float fElapsedTime);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);

	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

private:
	std::vector<std::shared_ptr<CEnemy>> m_vpEnemyObjects;
	std::vector<std::shared_ptr<CWallObject>> m_vpWallObjects;
	std::shared_ptr<CFloorObject> m_pFloorObject;
	std::shared_ptr<CPlayer> m_pPlayer;
};
