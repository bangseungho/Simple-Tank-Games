#pragma once
#include "Mesh.h"
#include "Camera.h"

class CGameObject : public std::enable_shared_from_this<CGameObject>
{
public:
	CGameObject() { }
	virtual ~CGameObject() { };

public:
	void SetActive(bool bActive);
	void SetMesh(std::shared_ptr<CMesh> pMesh) { m_pMesh = pMesh; }
	void SetDefaultColor(DWORD dwColor) { m_dwColor = m_dwDefaultColor = dwColor; }
	void SetColor(DWORD dwColor) { m_dwColor = dwColor; }
	void SetLineWidth(UINT nWidth) { m_nLineWidth = nWidth; }
	void SetRotationTransform(XMFLOAT4X4* pmxf4x4Transform);
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3& xmf3Position);
	void SetMovingDirection(XMFLOAT3& xmf3MovingDirection) { m_xmf3MovingDirection = Vector3::Normalize(xmf3MovingDirection); }
	void SetMovingSpeed(float fSpeed) { m_fMovingSpeed = fSpeed; }
	void SetMovingRange(float fRange) { m_fMovingRange = fRange; }
	void SetChild(std::shared_ptr<CGameObject> pChild);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	std::shared_ptr<CGameObject> GetParent() { return m_pParent; }
public:
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	void Move(XMFLOAT3& vDirection, float fSpeed);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3& xmf3Axis, float fAngle);

	void LookTo(XMFLOAT3& xmf3LookTo, XMFLOAT3& xmf3Up);
	void LookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up);

	void UpdateBoundingBox();

	void ComputeWorldTransform(XMFLOAT4X4* pxmf4x4Parent = nullptr);

	void GenerateRayForPicking(XMVECTOR& xmvPickPosition, XMMATRIX& xmmtxView, XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection);
	int PickObjectByRayIntersection(XMVECTOR& xmvPickPosition, XMMATRIX& xmmtxView, float* pfHitDistance);
	
	void Render(HDC hDCFrameBuffer, XMFLOAT4X4* pxmf4x4World, CMesh* pMesh);

	CGameObject* FindFrame(_TCHAR* pstrFrameName);

public:
	virtual void Animate(float fElapsedTime);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);

public:
	bool m_bActive = true;
	TCHAR m_pstrFrameName[256];
	DWORD m_dwDefaultColor = RGB(255, 0, 0);
	DWORD m_dwColor = RGB(255, 0, 0);

	std::shared_ptr<CMesh> m_pMesh;

	BoundingOrientedBox	m_xmOOBB = BoundingOrientedBox();
	std::shared_ptr<CGameObject> m_pObjectCollided;

	XMFLOAT4X4 m_xmf4x4Transform = Matrix4x4::Identity();
	XMFLOAT4X4 m_xmf4x4World = Matrix4x4::Identity();

	XMFLOAT3 m_xmf3MovingDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);
	float m_fMovingSpeed = 0.f;
	float m_fMovingRange = 0.f;
	UINT m_nLineWidth = 0.f;

	std::shared_ptr<CGameObject> m_pParent;
	std::shared_ptr<CGameObject> m_pChild;
	std::shared_ptr<CGameObject> m_pSibling;
};

class CFloorObject : public CGameObject
{
public:
	CFloorObject() { };
	virtual ~CFloorObject() { };

public:
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);

public:
	XMFLOAT4 m_pxmf4FloorPlane;
};

class CWallObject : public CGameObject
{
public:
	CWallObject() { };
	virtual ~CWallObject() { };

public:
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);

public:
	XMFLOAT4 m_pxmf4WallPlanes;
};

class CRotatingObject : public CGameObject
{
public:
	CRotatingObject() { };
	virtual ~CRotatingObject() { };

public:
	void SetRotationAxis(XMFLOAT3& xmf3RotationAxis) { m_xmf3RotationAxis = Vector3::Normalize(xmf3RotationAxis); }
	void SetRotationSpeed(float fSpeed) { m_fRotationSpeed = fSpeed; }
	
public:
	virtual void Animate(float fElapsedTime);

public:
	XMFLOAT3 m_xmf3RotationAxis = XMFLOAT3(0.f, 1.f, 0.f);
	bool m_bValid = true;
	float m_fRotationSpeed = 0.f;
};

class CExplosiveObject : public CRotatingObject
{
public:
	CExplosiveObject() { };
	virtual ~CExplosiveObject() { };

public:
	static void PrepareExplosion();

public:
	virtual void Animate(float fElapsedTime);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);

public:
	static std::shared_ptr<CMesh> m_pExplosionMesh;
	static XMFLOAT3	m_pxmf3SphereVectors[EXPLOSION_DEBRISES];

public:
	XMFLOAT4X4 m_pxmf4x4Transforms[EXPLOSION_DEBRISES];
	bool m_bBlowingUp = false;
	float m_fElapsedTimes = 0.f;
	float m_fDuration = 0.5f;
	float m_fExplosionSpeed = 100.f;
	float m_fExplosionRotation = 720.f;
};

class CBulletObject : public CRotatingObject
{
public:
	CBulletObject(float fEffectiveRange);
	virtual ~CBulletObject();

public:
	void SetFirePosition(XMFLOAT3 xmf3FirePosition);

public:
	void Reset();

public:
	virtual void Animate(float fElapsedTime);

public:
	CGameObject* m_pLockedObject = NULL;
	XMFLOAT3 m_xmf3FirePosition = XMFLOAT3(0.0f, 0.0f, 1.0f);
			 
	float m_fElapsedTimeAfterFire = 0.0f;
	float m_fLockingDelayTime = 0.3f;
	float m_fLockingTime = 4.0f;

	float m_fBulletEffectiveRange = 50.0f;
	float m_fMovingDistance = 0.0f;
	float m_fRotationAngle = 0.0f;
};

class CExplosiveBulletObject : public CExplosiveObject
{
public:
	CExplosiveBulletObject(float fEffectiveRange);
	virtual ~CExplosiveBulletObject();

public:
	void SetFirePosition(XMFLOAT3 xmf3FirePosition);

public:
	void Reset();

public:
	virtual void Animate(float fElapsedTime);

public:
	CGameObject* m_pLockedObject = NULL;
	XMFLOAT3 m_xmf3FirePosition = XMFLOAT3(0.0f, 0.0f, 1.0f);

	float m_fElapsedTimeAfterFire = 0.0f;
	float m_fLockingDelayTime = 0.3f;
	float m_fLockingTime = 4.0f;

	float m_fBulletEffectiveRange = 50.0f;
	float m_fMovingDistance = 0.0f;
	float m_fRotationAngle = 0.0f;

	float m_fMess = 0.5f;
};

class CFountainObject : public CGameObject
{
public:
	CFountainObject(float fEffectiveRange);
	virtual ~CFountainObject();

public:
	void Reset();

public:
	virtual void Animate(float fElapsedTime);

public:
	float m_fMess = 0.5f;
};
