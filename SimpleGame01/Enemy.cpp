#include "stdafx.h"
#include "Enemy.h"
default_random_engine m_dre;
uniform_int_distribution m_urd{-10, 10};
std::uniform_int_distribution m_urdSpawnX{ -40, 40 };
std::uniform_int_distribution m_urdSpawnZ{ -90, 90 };

CEnemy::CEnemy(float x, float y, float z) : m_xmf3Position{ x, y, z }
{
	XMFLOAT3 newPointValue = XMFLOAT3{ (float)m_urd(m_dre), 0, (float)m_urd(m_dre) };
	m_xmf3ScoutPoint = Vector3::Add(m_xmf3Position, newPointValue);
}

void CEnemy::SetPosition(float x, float y, float z)
{
	m_xmf3Position = XMFLOAT3(x, y, z);
	CGameObject::SetPosition(x, y, z);
}

void CEnemy::SetPosition(XMFLOAT3& xmf3Position)
{
	m_xmf3Position = xmf3Position;
	CGameObject::SetPosition(m_xmf3Position);
}

void CEnemy::SetRotation(XMFLOAT4X4& xmf4x4Rotation)
{
	m_xmf3Right = Vector3::Normalize(XMFLOAT3(xmf4x4Rotation._11, xmf4x4Rotation._12, xmf4x4Rotation._13));
	m_xmf3Up = Vector3::Normalize(XMFLOAT3(xmf4x4Rotation._21, xmf4x4Rotation._22, xmf4x4Rotation._23));
	m_xmf3Look = Vector3::Normalize(XMFLOAT3(xmf4x4Rotation._31, xmf4x4Rotation._32, xmf4x4Rotation._33));
}

void CEnemy::LookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up)
{
	XMFLOAT4X4 xmf4x4View = Matrix4x4::LookAtLH(m_xmf3Position, xmf3LookAt, xmf3Up);
	m_xmf3Right = Vector3::Normalize(XMFLOAT3(xmf4x4View._11, xmf4x4View._21, xmf4x4View._31));
	m_xmf3Up = Vector3::Normalize(XMFLOAT3(xmf4x4View._12, xmf4x4View._22, xmf4x4View._32));
	m_xmf3Look = Vector3::Normalize(XMFLOAT3(xmf4x4View._13, xmf4x4View._23, xmf4x4View._33));
}

void CEnemy::Move(XMFLOAT3& xmf3Direction, float fMovingSpeed)
{
	m_xmf3Position = Vector3::Add(m_xmf3Position, Vector3::ScalarProduct(xmf3Direction, fMovingSpeed));
}

void CEnemy::Rotate(float fPitch, float fYaw, float fRoll)
{
	if (fPitch != 0.f)
	{
		XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(fPitch));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, mtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, mtxRotate);
	}
	if (fYaw != 0.f)
	{
		XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(fYaw));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, mtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, mtxRotate);
	}
	if (fRoll != 0.f)
	{
		XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(fRoll));
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, mtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, mtxRotate);
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Up, m_xmf3Look));
	m_xmf3Up = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Look, m_xmf3Right));
}

void CEnemy::Reset()
{
	SetPosition(XMFLOAT3{ (float)m_urdSpawnX(m_dre), 0, (float)m_urdSpawnZ(m_dre) });
	SetActive(true);
}

void CEnemy::OnUpdateTransform()
{
	m_xmf4x4Transform._11 = m_xmf3Right.x; m_xmf4x4Transform._12 = m_xmf3Right.y; m_xmf4x4Transform._13 = m_xmf3Right.z;
	m_xmf4x4Transform._21 = m_xmf3Up.x; m_xmf4x4Transform._22 = m_xmf3Up.y; m_xmf4x4Transform._23 = m_xmf3Up.z;
	m_xmf4x4Transform._31 = m_xmf3Look.x; m_xmf4x4Transform._32 = m_xmf3Look.y; m_xmf4x4Transform._33 = m_xmf3Look.z;
	m_xmf4x4Transform._41 = m_xmf3Position.x; m_xmf4x4Transform._42 = m_xmf3Position.y; m_xmf4x4Transform._43 = m_xmf3Position.z;
}

void CEnemy::Animate(float fElapsedTime)
{
	OnUpdateTransform();
	CGameObject::Animate(fElapsedTime);
}

void CEnemy::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CGameObject::Render(hDCFrameBuffer, pCamera);
}

CETank::CETank(float x, float y, float z) : CEnemy(x, y, z)
{
	auto pBody = std::make_shared<CCubeMesh>(6.f, 2.f, 6.f);
	SetMesh(pBody);
	SetColor(RGB(255, 0, 0));
	SetRotationSpeed(60.f);

	m_pTurret = std::make_shared<CGameObject>();
	auto pTurretMesh = std::make_shared<CCubeMesh>(4.0f, 1.5f, 4.0f);
	m_pTurret->SetMesh(pTurretMesh);
	m_pTurret->SetColor(RGB(255, 0, 0));
	m_pTurret->SetPosition(0.0f, 1.75f, 0.0f);
	SetChild(m_pTurret);

	m_pGun = std::make_shared<CGameObject>();
	auto pGunMesh = std::make_shared<CCubeMesh>(1.0f, 1.0f, 8.0f);
	m_pGun->SetMesh(pGunMesh);
	m_pGun->SetColor(RGB(255, 0, 0));
	m_pGun->SetPosition(0.0f, 1.25f, 4.0f);
	m_pTurret->SetChild(m_pGun);

	root->addChild(sequence1);
	root->addChild(scoutOn);
	sequence1->addChild(checkPlayer);
	sequence1->addChild(sequence2);
	sequence2->addChild(turnToPlayer);
	sequence2->addChild(reLoad);
	sequence2->addChild(attackPlayer);

	m_vpBullets.resize(BULLETS);
	auto pBulletMesh = std::make_shared<CCubeMesh>(5.0f, 5.0f, 5.0f);
	for (auto& bullet : m_vpBullets) {
		bullet = std::make_shared<CExplosiveBulletObject>(m_fBulletEffectiveRange);
		bullet->SetMesh(pBulletMesh);
		bullet->SetMovingSpeed(30.0f);
		bullet->SetActive(false);
	}
}

CETank::~CETank()
{
	delete root;
	delete sequence1;
	delete status;
	delete checkPlayer;
	delete scoutOn;
	delete turnToPlayer;
	delete reLoad;
	delete attackPlayer;
}

void CETank::Animate(float fElapsedTime)
{
	if (m_bActive) {
		CEnemy::Animate(fElapsedTime);

		status->elapsedTime = fElapsedTime;
		status->coolTime += fElapsedTime;
		root->run();

		UpdateBoundingBox();
		ComputeWorldTransform(NULL);

		for (auto& bullet : m_vpBullets) {
			if (bullet->m_bActive) {
				bullet->Animate(fElapsedTime);
				bullet->ComputeWorldTransform(NULL);
				bullet->UpdateBoundingBox();
			}
		}
	}
	else {
		Reset();
	}
}

void CETank::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CEnemy::Render(hDCFrameBuffer, pCamera);

	for (int i = 0; i < BULLETS; i++) if (m_vpBullets[i]->m_bActive) m_vpBullets[i]->Render(hDCFrameBuffer, pCamera);
}

void CETank::FireBullet()
{
	std::shared_ptr<CExplosiveBulletObject> pBulletObject = NULL;

	for (auto& bullet : m_vpBullets) {
		if (!bullet->m_bActive) {
			pBulletObject = bullet;
			break;
		}
	}

	if (pBulletObject)
	{
		XMFLOAT3 xmf3Position = m_pGun->GetPosition();
		XMFLOAT3 xmf3Direction = m_pGun->GetLook();
		XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 6.0f, false));
		pBulletObject->m_xmf4x4Transform = m_pGun->m_xmf4x4World;
		pBulletObject->SetFirePosition(xmf3FirePosition);
		pBulletObject->SetMovingDirection(xmf3Direction);
		pBulletObject->SetColor(RGB(255, 0, 255));
		pBulletObject->SetActive(true);
	}
	status->coolTime = 0.f;
}

void CETank::Scout()
{
	// 정찰중일 경우 새로운 정찰 좌표만큼 회전하고 회전이 끝났을시 이동한다.

	XMFLOAT3 ScoutPoint = Vector3::Subtract(m_xmf3ScoutPoint, m_xmf3Position);
	XMFLOAT3 ScoutDirection = Vector3::Normalize(ScoutPoint);
	
	// 스칼라 삼중적의 부호 확인 0보다 작을 경우 반시계 회전
	float scalarTripleProduct = Vector3::CrossProduct(m_xmf3Look, ScoutDirection, false).y;
	float angle = XMConvertToDegrees(Vector3::Angle(m_xmf3Look, ScoutDirection));
	if (angle > 1.f) {
		if (scalarTripleProduct < 0.f)
			Rotate(0, -m_fRotationSpeed * status->elapsedTime, 0);
		else
			Rotate(0, m_fRotationSpeed * status->elapsedTime, 0);
	}
	else {
		if (Vector3::Length(ScoutPoint) < 0.3f) {
			XMFLOAT3 newPointValue = XMFLOAT3{ (float)m_urd(m_dre), 0, (float)m_urd(m_dre) };
			m_xmf3ScoutPoint = Vector3::Add(m_xmf3ScoutPoint, newPointValue);
		}
		Move(m_xmf3Look, m_fSpySpeed);
	}
}

bool BT_ScoutOn::run()
{
	if (!status->checkTarget) {
		m_pSelf->Scout();
	}

	return true;
}

bool BT_CheckPlayer::run()
{
	float distance = Vector3::Distance(m_pSelf->GetPosition(), m_pSelf->m_pTarget->GetPosition());

	if (distance < 50.f) {
		status->checkTarget = true;
		return true;
	}

	status->checkTarget = false;
	return false;
}

bool BT_TurnToPlayer::run()
{
	XMFLOAT3 turretLook = m_pSelf->m_pTurret->GetLook();
	XMFLOAT3 turretDirection = Vector3::Normalize(Vector3::Subtract(m_pSelf->m_pTarget->GetPosition(), m_pSelf->GetPosition()));

	// 스칼라 삼중적의 부호 확인 0보다 작을 경우 반시계 회전
	float scalarTripleProduct = Vector3::CrossProduct(turretLook, turretDirection, false).y;
	// 적탱크의 터렛 look과 플레이어와의 각도 계산
	float angle = XMConvertToDegrees(Vector3::Angle(turretDirection, turretLook));

	// 포신을 모두 돌리지 않았다면 포신 회전
	if (angle > 3.f) {
		if (status->coolTime < 3.f)
			return false;

		if(scalarTripleProduct < 0.f)
			m_pSelf->m_pTurret->Rotate(0, -m_pSelf->m_fRotationSpeed * status->elapsedTime, 0);
		else
			m_pSelf->m_pTurret->Rotate(0, m_pSelf->m_fRotationSpeed * status->elapsedTime, 0);
		return false;
	}
	return true;
}

bool BT_Reload::run()
{
	// 3초마다 총알 발사
	if (status->coolTime > 3.f) {
		return true;
	}

	return false;
}


bool BT_Attack::run()
{
	m_pSelf->FireBullet();
		
	return true;
}


