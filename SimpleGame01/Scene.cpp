#include "stdafx.h"
#include "Scene.h"
#include "GraphicsPipeline.h"
#include "GameObject.h"

CScene::CScene(std::shared_ptr<CPlayer> pPlayer)
{
	m_pPlayer = pPlayer;
}

void CScene::BuildObjects()
{
	CExplosiveObject::PrepareExplosion();

	float fHalfWidth = 45.f, fHalfHeight = 45.f, fHalfDepth = 200.f;
	auto pFloorMesh = std::make_shared<CFloorMesh>(fHalfWidth * 2.0f, fHalfDepth * 2.0f, 30);
	m_pFloorObject = std::make_unique<CFloorObject>();
	m_pFloorObject->SetPosition(0.0f, 0.0f, 0.0f);
	m_pFloorObject->SetMesh(pFloorMesh);
	m_pFloorObject->SetColor(RGB(0, 0, 0));
	m_pFloorObject->m_pxmf4FloorPlane = XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);


	m_vpWallObjects.resize(2);
	fHalfWidth = 10.f, fHalfHeight = 45.f, fHalfDepth = 200.f;
	auto pWallMesh = std::make_shared<CWallMesh>(fHalfWidth * 2.0f, fHalfDepth * 2.0f, 10);

	int LRFlag = 1;
	for (auto& wallObject : m_vpWallObjects) {
		wallObject = std::make_shared<CWallObject>();
		wallObject->Rotate(XMFLOAT3{ 0, 0, 1 }, LRFlag * 90.f);
		wallObject->SetPosition(LRFlag * fHalfHeight, fHalfWidth, 0.0f);
		wallObject->SetMesh(pWallMesh);
		wallObject->m_pxmf4WallPlanes = XMFLOAT4(LRFlag * -1.0f, 0.0f, 0.0f, LRFlag * fHalfHeight);
		wallObject->SetColor(RGB(0, 0, 0));
		LRFlag *= -1;
	}
}

void CScene::ReleaseObjects()
{
	// 스마트 포인터 사용하였기 때문에 Pass
}

void CScene::CheckObjectByBulletCollisions()
{
	for (auto& enemyObject : m_vpEnemyObjects) {
		for (auto& bulletObject : m_pPlayer->m_vpBullets) {
			if (bulletObject->m_bActive && enemyObject->m_xmOOBB.Intersects(bulletObject->m_xmOOBB)) {
				if (bulletObject->m_bValid) {
					enemyObject->m_nlife--;
					if (enemyObject->m_nlife <= 0) {
						m_pPlayer->m_nKill++;
						enemyObject->Reset();
						enemyObject->m_nlife = 3;
					}
				}
				bulletObject->m_bValid = false;
			}
		}
	}
}

void CScene::CheckObjectByWallCollisions()
{
	for (auto& wallObject : m_vpWallObjects)
		wallObject->m_pObjectCollided = nullptr;

	for (auto& wallObject : m_vpWallObjects) {
		for (auto& enemyObject : m_vpEnemyObjects) {
			if (wallObject->m_xmOOBB.Intersects(enemyObject->m_xmOOBB)) {
				XMVECTOR xmvNormal = XMVectorSet(wallObject->m_pxmf4WallPlanes.x, 
					wallObject->m_pxmf4WallPlanes.y, 
					wallObject->m_pxmf4WallPlanes.z, 0.0f);
				XMVECTOR xmvReflect = XMVector3Reflect(XMLoadFloat3(&enemyObject->GetLook()), xmvNormal);
				XMFLOAT3 xmf3Reflect = Vector3::XMVectorToFloat3(xmvReflect);
				enemyObject->Move(xmf3Reflect, 1.f);
				enemyObject->m_xmf3ScoutPoint = XMFLOAT3{ Vector3::Add(enemyObject->m_xmf3Position, 
					Vector3::ScalarProduct(xmf3Reflect, 10.f)) };
			}
		}
	}
}

void CScene::CheckWallByPlayerCollisions()
{
	for (auto& object : m_vpWallObjects)
		object->m_pObjectCollided = nullptr;

	for (auto& object : m_vpWallObjects) {
		if (object->m_xmOOBB.Intersects(m_pPlayer->m_xmOOBB)) {
			XMFLOAT3 inverseLook = Vector3::ScalarProduct(m_pPlayer->m_xmf3Shift, -1);
			m_pPlayer->Move(inverseLook, false);
		}
	}
}

void CScene::CheckBulletByPlayerCollisions()
{
	for (auto& enemyObject : m_vpEnemyObjects)
		for (auto& bulletObject : enemyObject->m_vpBullets)
			bulletObject->m_pObjectCollided = nullptr;

	for (auto& enemyObject : m_vpEnemyObjects)
		for (auto& bulletObject : enemyObject->m_vpBullets)
			if (bulletObject->m_bActive && bulletObject->m_xmOOBB.Intersects(m_pPlayer->m_xmOOBB)) {
				if (bulletObject->m_bValid) {
					m_pPlayer->m_nLife--;
					bulletObject->m_bValid = false;
					if (m_pPlayer->m_nLife <= 0)
						m_pPlayer->Reset();
				}
			}
}

void CScene::Animate(float fElapsedTime)
{
	m_pFloorObject->Animate(fElapsedTime);
	m_pFloorObject->ComputeWorldTransform();

	for (auto& object : m_vpEnemyObjects) {
		object->Animate(fElapsedTime);
		object->ComputeWorldTransform();
		object->UpdateBoundingBox();
	}

	for (const auto& object : m_vpWallObjects) {
		object->Animate(fElapsedTime);
		object->ComputeWorldTransform();
		object->UpdateBoundingBox();
	}

	CheckWallByPlayerCollisions();
	CheckObjectByWallCollisions();
	CheckObjectByBulletCollisions();
	CheckBulletByPlayerCollisions();
}

void CScene::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CGraphicsPipeline::SetViewport(&pCamera->m_Viewport);
	CGraphicsPipeline::SetViewPerspectiveProjectTransform(&pCamera->m_xmf4x4ViewPerspectiveProject);

	m_pFloorObject->Render(hDCFrameBuffer, pCamera);
	for (const auto& object : m_vpEnemyObjects) {
		object->Render(hDCFrameBuffer, pCamera);
	}

	for (const auto& object : m_vpWallObjects) {
		object->Render(hDCFrameBuffer, pCamera);
	}

	TCHAR strKill[30];
	wsprintf(strKill, TEXT("KILL : %d"), m_pPlayer->m_nKill);
	TextOut(hDCFrameBuffer, 10, 10, strKill, lstrlen(strKill));
	
	TCHAR strLife[30];
	wsprintf(strLife, TEXT("LIFE : %d"), m_pPlayer->m_nLife);
	TextOut(hDCFrameBuffer, 10, 30, strLife, lstrlen(strLife));



	if (m_pPlayer) m_pPlayer->Render(hDCFrameBuffer, pCamera);
}

void CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
}

void CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			//CExplosiveObject* pExplosiveObject = (CExplosiveObject*)m_ppObjects[int(wParam - '1')];
			//pExplosiveObject->m_bBlowingUp = true;
			break;
		}
		default:
			break;
		}
		break;
	default:
		break;
	}
}

CGameObject* CScene::PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera)
{
	XMFLOAT3 xmf3PickPosition;
	xmf3PickPosition.x = (((2.0f * xClient) / (float)pCamera->m_Viewport.m_nWidth) - 1) / pCamera->m_xmf4x4PerspectiveProject._11;
	xmf3PickPosition.y = -(((2.0f * yClient) / (float)pCamera->m_Viewport.m_nHeight) - 1) / pCamera->m_xmf4x4PerspectiveProject._22;
	xmf3PickPosition.z = 1.0f;

	XMVECTOR xmvPickPosition = XMLoadFloat3(&xmf3PickPosition);
	XMMATRIX xmmtxView = XMLoadFloat4x4(&pCamera->m_xmf4x4View);

	int nIntersected = 0;
	float fNearestHitDistance = FLT_MAX;
	CGameObject* pNearestObject = NULL;
	for (auto& object : m_vpEnemyObjects) {
		float fHitDistance = FLT_MAX;
		nIntersected = object->PickObjectByRayIntersection(xmvPickPosition, xmmtxView, &fHitDistance);
		int a = 3;
		if ((nIntersected > 0) && (fHitDistance < fNearestHitDistance))
		{
			fNearestHitDistance = fHitDistance;
			pNearestObject = object.get();
		}
	}

	return pNearestObject;
}
