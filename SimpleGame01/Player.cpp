#include "stdafx.h"
#include "Player.h"

void CPlayer::SetPosition(float x, float y, float z)
{
	m_xmf3Position = XMFLOAT3(x, y, z);
	CGameObject::SetPosition(x, y, z);
}

void CPlayer::SetPosition(XMFLOAT3& xmf3Position)
{
	m_xmf3Position = xmf3Position;
	CGameObject::SetPosition(m_xmf3Position);
}

void CPlayer::SetRotation(XMFLOAT4X4& xmf4x4Rotation)
{
	m_xmf3Right = Vector3::Normalize(XMFLOAT3(xmf4x4Rotation._11, xmf4x4Rotation._12, xmf4x4Rotation._13));
	m_xmf3Up = Vector3::Normalize(XMFLOAT3(xmf4x4Rotation._21, xmf4x4Rotation._22, xmf4x4Rotation._23));
	m_xmf3Look = Vector3::Normalize(XMFLOAT3(xmf4x4Rotation._31, xmf4x4Rotation._32, xmf4x4Rotation._33));
}

void CPlayer::SetCameraOffset(XMFLOAT3& xmf3CameraOffset)
{
	m_xmf3CameraOffset = xmf3CameraOffset;
	m_pCamera->SetLookAt(Vector3::Add(m_xmf3Position, m_xmf3CameraOffset), m_xmf3Position, m_xmf3Up);
	m_pCamera->GenerateViewMatrix();
}

void CPlayer::LookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up)
{
	XMFLOAT4X4 xmf4x4View = Matrix4x4::LookAtLH(m_xmf3Position, xmf3LookAt, xmf3Up);
	m_xmf3Right = Vector3::Normalize(XMFLOAT3(xmf4x4View._11, xmf4x4View._21, xmf4x4View._31));
	m_xmf3Up = Vector3::Normalize(XMFLOAT3(xmf4x4View._12, xmf4x4View._22, xmf4x4View._32));
	m_xmf3Look = Vector3::Normalize(XMFLOAT3(xmf4x4View._13, xmf4x4View._23, xmf4x4View._33));
}

void CPlayer::Move(DWORD dwDirection, float fDistance)
{
	if (dwDirection){
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
		if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);
		m_xmf3Shift = xmf3Shift;
		Move(xmf3Shift, true);
	}
}

void CPlayer::Move(XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity){
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else{
		m_xmf3Position = Vector3::Add(xmf3Shift, m_xmf3Position);
		m_pCamera->Move(xmf3Shift);
	}
}

void CPlayer::Move(float x, float y, float z)
{
	Move(XMFLOAT3(x, y, z), false);
}

void CPlayer::Rotate(float fPitch, float fYaw, float fRoll)
{
	m_pCamera->Rotate(fPitch, fYaw, fRoll);
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

void CPlayer::Update(float fTimeElapsed)
{
	Move(m_xmf3Velocity, false);

	// 카메라 회전 업데이트
	m_pCamera->Update(this, m_xmf3Position, fTimeElapsed);
	m_pCamera->GenerateViewMatrix();

	XMFLOAT3 xmf3Deceleration = Vector3::Normalize(Vector3::ScalarProduct(m_xmf3Velocity, -1.0f));
	float fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = m_fFriction * fTimeElapsed;
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Deceleration, fDeceleration);
}

void CPlayer::OnUpdateTransform()
{
	m_xmf4x4Transform._11 = m_xmf3Right.x; m_xmf4x4Transform._12 = m_xmf3Right.y; m_xmf4x4Transform._13 = m_xmf3Right.z;
	m_xmf4x4Transform._21 = m_xmf3Up.x; m_xmf4x4Transform._22 = m_xmf3Up.y; m_xmf4x4Transform._23 = m_xmf3Up.z;
	m_xmf4x4Transform._31 = m_xmf3Look.x; m_xmf4x4Transform._32 = m_xmf3Look.y; m_xmf4x4Transform._33 = m_xmf3Look.z;
	m_xmf4x4Transform._41 = m_xmf3Position.x; m_xmf4x4Transform._42 = m_xmf3Position.y; m_xmf4x4Transform._43 = m_xmf3Position.z;
}

void CPlayer::Animate(float fElapsedTime)
{
	OnUpdateTransform();
	CGameObject::Animate(fElapsedTime);
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

void CPlayer::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CGameObject::Render(hDCFrameBuffer, pCamera);
	for (int i = 0; i < BULLETS; i++) if (m_vpBullets[i]->m_bActive) m_vpBullets[i]->Render(hDCFrameBuffer, pCamera);
}

void CPlayer::Reset()
{
	SetPosition(0.0f, 0.0f, -150.0f);
	m_nLife = 10;
	m_nKill = 0;
}

std::shared_ptr<CPlayer> CPlayer::ChangeToType(std::shared_ptr<CCamera> pCamera)
{
	std::shared_ptr<CPlayer> newPlayerPtr;
	if (GetObjectType() == PlayerType::Tank) {
		auto playerPtr = std::dynamic_pointer_cast<CTank>(shared_from_this());
		std::shared_ptr<CGoliath> goliath = std::make_shared<CGoliath>();
		newPlayerPtr = std::dynamic_pointer_cast<CPlayer>(goliath);
		newPlayerPtr->SetPosition(XMFLOAT3{ playerPtr->GetPosition() });
		newPlayerPtr->SetRotation(playerPtr->m_xmf4x4Transform);
		newPlayerPtr->SetCamera(pCamera);
		newPlayerPtr->m_xmf3CameraOffset = playerPtr->m_xmf3CameraOffset;
		newPlayerPtr->m_nKill = playerPtr->m_nKill;
		newPlayerPtr->m_nLife = playerPtr->m_nLife;
	}
	if (GetObjectType() == PlayerType::Goliath) {
		auto playerPtr = std::dynamic_pointer_cast<CGoliath>(shared_from_this());
		std::shared_ptr<CTank> tank = std::make_shared<CTank>();
		newPlayerPtr = std::dynamic_pointer_cast<CPlayer>(tank);
		newPlayerPtr->SetPosition(XMFLOAT3{ playerPtr->GetPosition() });
		newPlayerPtr->SetRotation(playerPtr->m_xmf4x4Transform);
		newPlayerPtr->SetCamera(pCamera);
		newPlayerPtr->m_xmf3CameraOffset = playerPtr->m_xmf3CameraOffset;
		newPlayerPtr->m_nKill = playerPtr->m_nKill;
		newPlayerPtr->m_nLife = playerPtr->m_nLife;
	}
	return newPlayerPtr;
}

// Tank
CTank::CTank()
{
	auto pBody = std::make_shared<CCubeMesh>(5.f, 2.f, 7.f);
	SetMesh(pBody);
	SetColor(RGB(0, 0, 255));

	m_pTurret = std::make_shared<CGameObject>();
	auto pTurretMesh = std::make_shared<CCubeMesh>(4.0f, 1.5f, 4.0f);
	m_pTurret->SetMesh(pTurretMesh);
	m_pTurret->SetColor(RGB(255, 0, 0));
	m_pTurret->SetPosition(0.0f, 1.75f, -1.f);
	m_pTurret->SetLineWidth(3);
	SetChild(m_pTurret);

	m_pGun = std::make_shared<CGameObject>();
	auto pGunMesh = std::make_shared<CCubeMesh>(1.0f, 1.0f, 7.0f);
	m_pGun->SetMesh(pGunMesh);
	m_pGun->SetColor(RGB(255, 0, 255));
	m_pGun->SetPosition(0.0f, 1.25f, 4.0f);
	m_pGun->SetLineWidth(3);
	m_pTurret->SetChild(m_pGun);
	m_fVelocity = 0.15f;

	m_vpBullets.resize(BULLETS);
	auto pBulletMesh = std::make_shared<CCubeMesh>(10.0f, 10.0f, 10.0f);
	for (auto& bullet : m_vpBullets) {
		bullet = std::make_shared<CExplosiveBulletObject>(m_fBulletEffectiveRange);
		bullet->SetMesh(pBulletMesh);
		bullet->SetMovingSpeed(150.0f);
		bullet->SetActive(false);
	}
}

void CTank::FireBullet(CGameObject* pLockedObject)
{
	// 발사하지 않은 불릿을 찾아 발사될 불릿에 설정 
	// 발사될 불릿이 있다면 불릿의 발사 포인트를 설정한다.
	std::shared_ptr<CExplosiveBulletObject> pBulletObject = NULL;
	
	for (auto& bullet : m_vpBullets) {
		if (!bullet->m_bActive) {
			pBulletObject = std::dynamic_pointer_cast<CExplosiveBulletObject>(bullet);
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
		pBulletObject->SetColor(RGB(100, 200, 80));
		pBulletObject->SetActive(true);
	}
}

// Goliath
CGoliath::CGoliath()
{
	auto pBody = std::make_shared<CTransParentMesh>(5.f, 7.f, 7.f);
	SetMesh(pBody);

	m_pTurret = std::make_shared<CGameObject>();
	auto pTurretMesh = std::make_shared<CCubeMesh>(2.f, 4.f, 2.f);
	m_pTurret->SetMesh(pTurretMesh);
	m_pTurret->SetColor(RGB(255, 0, 0));
	m_pTurret->SetPosition(0.0f, 3.f, 0.0f);
	m_fVelocity = 0.4f;
	m_pTurret->SetLineWidth(3);
	SetChild(m_pTurret);

	int LRFlag = 1;
	m_pGuns.resize(2);
	auto pGunMesh = std::make_shared<CCubeMesh>(1.f, 1.f, 5.f);
	for (auto& gun : m_pGuns) {
		gun = std::make_shared<CGameObject>();
		gun->SetMesh(pGunMesh);
		gun->SetColor(RGB(255, 150, 0));
		gun->SetPosition(LRFlag * -1.5f, 0.f, 1.0f);
		gun->SetLineWidth(3);
		m_pTurret->SetChild(gun);
		LRFlag *= -1;
	}

	LRFlag = 1;
	m_pLegs.resize(2);
	auto pLegMesh = std::make_shared<CCubeMesh>(1.f, 4.f, 1.f);
	for (auto& leg : m_pLegs) {
		leg = std::make_shared<CGameObject>();
		leg->SetMesh(pLegMesh);
		leg->SetColor(RGB(0, 0, 255));
		leg->SetPosition(LRFlag * -1.5f, -3.5f, 0.0f);
		leg->SetLineWidth(3);
		m_pTurret->SetChild(leg);
		LRFlag *= -1;
	}

	m_vpBullets.resize(BULLETS);
	auto pBulletMesh = std::make_shared<CCubeMesh>(2.0f, 2.0f, 2.0f);
	for (auto& bullet : m_vpBullets) {
		bullet = std::make_shared<CBulletObject>(m_fBulletEffectiveRange);
		bullet->SetMesh(pBulletMesh);
		bullet->SetMovingSpeed(100.0f);
		bullet->SetActive(false);
	}
}

void CGoliath::RotateGun(float fAngle)
{
	for (auto& gun : m_pGuns) {
		gun->Rotate(fAngle, 0.f, 0.f);
	}
}

void CGoliath::FireBullet(CGameObject* pLockedObject)
{
	// 발사하지 않은 불릿을 찾아 발사될 불릿에 설정 
	// 발사될 불릿이 있다면 불릿의 발사 포인트를 설정한다.
	std::shared_ptr<CBulletObject> pBulletObject = NULL;

	for (auto& bullet : m_vpBullets) {
		if (!bullet->m_bActive) {
			pBulletObject = std::dynamic_pointer_cast<CBulletObject>(bullet);
			break;
		}
	}

	static int RotateGun{};
	if (pBulletObject)
	{
		XMFLOAT3 xmf3Position = m_pGuns[RotateGun]->GetPosition();
		XMFLOAT3 xmf3Direction = m_pGuns[RotateGun]->GetLook();

		XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 6.0f, false));

		pBulletObject->m_xmf4x4Transform = m_pGuns[RotateGun]->m_xmf4x4World;
		pBulletObject->SetFirePosition(xmf3FirePosition);
		pBulletObject->SetMovingDirection(xmf3Direction);
		pBulletObject->SetColor(RGB(100, 200, 80));
		pBulletObject->SetActive(true);

		if (pLockedObject)
		{
			pBulletObject->m_pLockedObject = pLockedObject;
			pBulletObject->SetColor(RGB(0, 0, 255));
		}
		RotateGun = (RotateGun + 1) % 2;
	}
}

