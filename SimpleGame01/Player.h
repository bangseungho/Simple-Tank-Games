#pragma once
#include "GameObject.h"
#include "Camera.h"

enum class PlayerType{
	Tank,
	Goliath
};

class CPlayer : public CGameObject
{
public:
	CPlayer() { m_nLineWidth = 2; }
	virtual ~CPlayer() { }

public:
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3& xmf3Position);
	void SetRotation(XMFLOAT4X4& xmf4x4Rotation);
	void SetCameraOffset(XMFLOAT3& xmf3CameraOffset);
	void SetCamera(std::shared_ptr<CCamera> pCamera) { m_pCamera = pCamera; }
	std::shared_ptr<CCamera> GetCamera() { return m_pCamera; }

public:
	void LookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up);

	void Move(DWORD dwDirection, float fDistance);
	void Move(XMFLOAT3& xmf3Shift, bool bUpdateVelocity);
	void Move(float x, float y, float z);
	void Rotate(float fPitch = 0.f, float fYaw = 0.f, float fRoll = 0.f);

	void Update(float fTimeElapsed = 0.016f);
	void Animate(float fElapsedTime);

	void Render(HDC hDCFrameBuffer, CCamera* pCamera);

	void Reset();

	std::shared_ptr<CPlayer> ChangeToType(std::shared_ptr<CCamera> pCamera);

public:
	virtual void OnUpdateTransform();
	virtual void RotateTurret(float fAngle) = 0;
	virtual void RotateGun(float fAngle) = 0;
	virtual PlayerType GetObjectType() = 0;
	virtual void FireBullet(CGameObject* pLockedObject) = 0;

public:
	XMFLOAT3 m_xmf3Position = XMFLOAT3(0.f, 0.f, 0.f);
	XMFLOAT3 m_xmf3Right = XMFLOAT3(1.f, 0.f, 0.f);
	XMFLOAT3 m_xmf3Up = XMFLOAT3(0.f, 1.f, 0.f);
	XMFLOAT3 m_xmf3Look = XMFLOAT3(0.f, 0.f, 1.f);
	XMFLOAT3 m_xmf3Shift = XMFLOAT3(0.f, 0.f, 0.f);
	XMFLOAT3 m_xmf3CameraOffset = XMFLOAT3(0.f, 0.f, 0.f);
	XMFLOAT3 m_xmf3Velocity = XMFLOAT3(0.f, 0.f, 0.f);
			 
	float m_fFriction = 125.f;
	float m_fPitch = 0.f;
	float m_fYaw = 0.f;
	float m_fRoll = 0.f;
	float m_fBulletEffectiveRange = 150.0f;
	float m_fVelocity = 0.f;

	bool m_bCanGo = false;

	int m_nLife = 10;
	int m_nKill = 0;

	std::shared_ptr<CCamera> m_pCamera;
	std::vector<std::shared_ptr<CRotatingObject>> m_vpBullets;
};

class CTank : public CPlayer
{
public:
	CTank();
	virtual ~CTank() { };

public:
	virtual PlayerType GetObjectType() { return PlayerType::Tank; }

public:
	void RotateTurret(float fAngle) { m_pTurret->Rotate(0.0f, fAngle, 0.0f); }
	void RotateGun(float fAngle) { m_pGun->Rotate(fAngle, 0.0f, 0.0f); }
	
public:
	virtual void FireBullet(CGameObject* pLockedObject);

public:
	std::shared_ptr<CGameObject> m_pTurret;
	std::shared_ptr<CGameObject> m_pGun;
};

class CGoliath : public CPlayer
{
public:
	CGoliath();
	virtual ~CGoliath() { };

public:
	virtual PlayerType GetObjectType() { return PlayerType::Goliath; }

public:
	void RotateTurret(float fAngle) { }
	void RotateGun(float fAngle);

public:
	virtual void FireBullet(CGameObject* pLockedObject);

public:
	std::shared_ptr<CGameObject> m_pTurret;
	std::vector<std::shared_ptr<CGameObject>> m_pLegs;
	std::vector<std::shared_ptr<CGameObject>> m_pGuns;
};