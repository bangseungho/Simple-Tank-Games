#include "stdafx.h"
#include "GameObject.h"
#include "GraphicsPipeline.h"

XMFLOAT3 CExplosiveObject::m_pxmf3SphereVectors[EXPLOSION_DEBRISES];
std::shared_ptr<CMesh> CExplosiveObject::m_pExplosionMesh;
std::default_random_engine dre;

inline float RandF(float fMin, float fMax)
{
	std::uniform_real_distribution<float> urd{ fMin, fMax };

	return urd(dre);
}

XMVECTOR RandomUnitVectorOnSphere()
{
	XMVECTOR xmvOne = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR xmvZero = XMVectorZero();

	while (true)
	{
		XMVECTOR v = XMVectorSet(RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), 0.0f);
		if (!XMVector3Greater(XMVector3LengthSq(v), xmvOne)) 
			return(XMVector3Normalize(v));
	}
}

void CGameObject::SetActive(bool bActive)
{
	m_bActive = bActive;

	if (m_pSibling) m_pSibling->SetActive(bActive);
	if (m_pChild) m_pChild->SetActive(bActive);
}

void CGameObject::SetRotationTransform(XMFLOAT4X4* pmxf4x4Transform)
{
	m_xmf4x4Transform._11 = pmxf4x4Transform->_11; m_xmf4x4Transform._12 = pmxf4x4Transform->_12; m_xmf4x4Transform._13 = pmxf4x4Transform->_13;
	m_xmf4x4Transform._21 = pmxf4x4Transform->_21; m_xmf4x4Transform._22 = pmxf4x4Transform->_22; m_xmf4x4Transform._23 = pmxf4x4Transform->_23;
	m_xmf4x4Transform._31 = pmxf4x4Transform->_31; m_xmf4x4Transform._32 = pmxf4x4Transform->_32; m_xmf4x4Transform._33 = pmxf4x4Transform->_33;
	ComputeWorldTransform();
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4Transform._41 = x;
	m_xmf4x4Transform._42 = y;
	m_xmf4x4Transform._43 = z;
	ComputeWorldTransform();
}

void CGameObject::SetPosition(XMFLOAT3& xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void CGameObject::SetChild(std::shared_ptr<CGameObject> pChild)
{
	if (m_pChild) {
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;
	}
	else {
		m_pChild = pChild;
	}
}

XMFLOAT3 CGameObject::GetPosition()
{
	return XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
}

XMFLOAT3 CGameObject::GetLook()
{
	return XMFLOAT3(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 CGameObject::GetUp()
{
	return XMFLOAT3(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 CGameObject::GetRight()
{
	return XMFLOAT3(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

CGameObject* CGameObject::FindFrame(_TCHAR* pstrFrameName)
{
	CGameObject* pFrameObject = NULL;
	if (!_tcsncmp(m_pstrFrameName, pstrFrameName, _tcslen(pstrFrameName))) return(this);

	if (m_pSibling) if (pFrameObject = m_pSibling->FindFrame(pstrFrameName)) return(pFrameObject);
	if (m_pChild) if (pFrameObject = m_pChild->FindFrame(pstrFrameName)) return(pFrameObject);

	return(NULL);
}

void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Right, fDistance));
	SetPosition(xmf3Position);
}

void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Up, fDistance));
	SetPosition(xmf3Position);
}

void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, Vector3::Normalize(xmf3Look), fDistance);
	SetPosition(xmf3Position);
}

void CGameObject::Move(XMFLOAT3& vDirection, float fSpeed)
{
	SetPosition(m_xmf4x4World._41 + vDirection.x * fSpeed, m_xmf4x4World._42 + vDirection.y * fSpeed, m_xmf4x4World._43 + vDirection.z * fSpeed);
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMFLOAT4X4 mtxRotate = Matrix4x4::RotationYawPitchRoll(fPitch, fYaw, fRoll);
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);
	ComputeWorldTransform();
}

void CGameObject::Rotate(XMFLOAT3& xmf3Axis, float fAngle)
{
	XMFLOAT4X4 mtxRotate = Matrix4x4::RotationAxis(xmf3Axis, fAngle);
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);
	ComputeWorldTransform();
}

void CGameObject::LookTo(XMFLOAT3& xmf3LookTo, XMFLOAT3& xmf3Up)
{
	XMFLOAT4X4 xmf4x4View = Matrix4x4::LookToLH(GetPosition(), xmf3LookTo, xmf3Up);
	m_xmf4x4Transform._11 = xmf4x4View._11; m_xmf4x4Transform._12 = xmf4x4View._21; m_xmf4x4Transform._13 = xmf4x4View._31;
	m_xmf4x4Transform._21 = xmf4x4View._12; m_xmf4x4Transform._22 = xmf4x4View._22; m_xmf4x4Transform._23 = xmf4x4View._32;
	m_xmf4x4Transform._31 = xmf4x4View._13; m_xmf4x4Transform._32 = xmf4x4View._23; m_xmf4x4Transform._33 = xmf4x4View._33;
	ComputeWorldTransform();
}

void CGameObject::LookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up)
{
	XMFLOAT4X4 xmf4x4View = Matrix4x4::LookAtLH(GetPosition(), xmf3LookAt, xmf3Up);
	m_xmf4x4Transform._11 = xmf4x4View._11; m_xmf4x4Transform._12 = xmf4x4View._21; m_xmf4x4Transform._13 = xmf4x4View._31;
	m_xmf4x4Transform._21 = xmf4x4View._12; m_xmf4x4Transform._22 = xmf4x4View._22; m_xmf4x4Transform._23 = xmf4x4View._32;
	m_xmf4x4Transform._31 = xmf4x4View._13; m_xmf4x4Transform._32 = xmf4x4View._23; m_xmf4x4Transform._33 = xmf4x4View._33;
	ComputeWorldTransform();
}

void CGameObject::UpdateBoundingBox()
{
	if (m_pMesh) {
		m_pMesh->m_xmOOBB.Transform(m_xmOOBB, XMLoadFloat4x4(&m_xmf4x4World));
		XMStoreFloat4(&m_xmOOBB.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBB.Orientation)));
	}

	if (m_pSibling) m_pSibling->UpdateBoundingBox();
	if (m_pChild) m_pChild->UpdateBoundingBox();
}

void CGameObject::Animate(float fElapsedTime)
{
	if (m_fMovingSpeed != 0.f) Move(m_xmf3MovingDirection, m_fMovingSpeed * fElapsedTime);

	if (m_pSibling) m_pSibling->Animate(fElapsedTime);
	if (m_pChild) m_pChild->Animate(fElapsedTime);
}

void CGameObject::Render(HDC hDCFrameBuffer, XMFLOAT4X4* pxmf4x4World, CMesh* pMesh)
{
	if (pMesh && m_bActive)
	{
		CGraphicsPipeline::SetWorldTransform(pxmf4x4World);
		HPEN hPen = ::CreatePen(PS_SOLID, m_nLineWidth, m_dwColor);
		HPEN hOldPen = (HPEN)::SelectObject(hDCFrameBuffer, hPen);
		pMesh->Render(hDCFrameBuffer);
		::SelectObject(hDCFrameBuffer, hOldPen);
		::DeleteObject(hPen);
	}
}

void CGameObject::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	if (pCamera->IsInFrustum(m_xmOOBB)) CGameObject::Render(hDCFrameBuffer, &m_xmf4x4World, m_pMesh.get());

	if (m_pSibling) m_pSibling->Render(hDCFrameBuffer, pCamera);
	if (m_pChild) m_pChild->Render(hDCFrameBuffer, pCamera);
}

void CGameObject::GenerateRayForPicking(XMVECTOR& xmvPickPosition, XMMATRIX& xmmtxView, XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection)
{
	XMMATRIX xmmtxToModel = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4World) * xmmtxView);

	XMFLOAT3 xmf3CameraOrigin(0.0f, 0.0f, 0.0f);
	xmvPickRayOrigin = XMVector3TransformCoord(XMLoadFloat3(&xmf3CameraOrigin), xmmtxToModel);
	xmvPickRayDirection = XMVector3TransformCoord(xmvPickPosition, xmmtxToModel);
	xmvPickRayDirection = XMVector3Normalize(xmvPickRayDirection - xmvPickRayOrigin);
}

int CGameObject::PickObjectByRayIntersection(XMVECTOR& xmvPickPosition, XMMATRIX& xmmtxView, float* pfHitDistance)
{
	int nIntersected = 0;
	if (m_pMesh)
	{
		XMVECTOR xmvPickRayOrigin, xmvPickRayDirection;
		GenerateRayForPicking(xmvPickPosition, xmmtxView, xmvPickRayOrigin, xmvPickRayDirection);
		nIntersected = m_pMesh->CheckRayIntersection(xmvPickRayOrigin, xmvPickRayDirection, pfHitDistance);
	}
	return nIntersected;
}

void CGameObject::ComputeWorldTransform(XMFLOAT4X4* pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4Transform, *pxmf4x4Parent) : m_xmf4x4Transform;
	if (m_pSibling) m_pSibling->ComputeWorldTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->ComputeWorldTransform(&m_xmf4x4World);
}

void CFloorObject::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CGameObject::Render(hDCFrameBuffer, &m_xmf4x4World, m_pMesh.get());
}


void CWallObject::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CGameObject::Render(hDCFrameBuffer, &m_xmf4x4World, m_pMesh.get());
}

void CRotatingObject::Animate(float fElapsedTime)
{
	if (m_fRotationSpeed != 0.f) Rotate(m_xmf3RotationAxis, m_fRotationSpeed * fElapsedTime);

	CGameObject::Animate(fElapsedTime);
}

void CExplosiveObject::PrepareExplosion()
{
	for (int i = 0; i < EXPLOSION_DEBRISES; i++) {
		XMStoreFloat3(&m_pxmf3SphereVectors[i], RandomUnitVectorOnSphere());
	}


	m_pExplosionMesh = std::make_shared<CCubeMesh>(0.5f, 0.5f, 0.5f);
}

void CExplosiveObject::Animate(float fElapsedTime)
{
	if (m_bBlowingUp)
	{
		m_fElapsedTimes += fElapsedTime;
		if (m_fElapsedTimes <= m_fDuration)
		{
			XMFLOAT3 xmf3Position = GetPosition();
			for (int i = 0; i < EXPLOSION_DEBRISES; i++)
			{
				m_pxmf4x4Transforms[i] = Matrix4x4::Identity();
				m_pxmf4x4Transforms[i]._41 = xmf3Position.x + m_pxmf3SphereVectors[i].x * m_fExplosionSpeed * m_fElapsedTimes;
				m_pxmf4x4Transforms[i]._42 = xmf3Position.y + m_pxmf3SphereVectors[i].y * m_fExplosionSpeed * m_fElapsedTimes;
				m_pxmf4x4Transforms[i]._43 = xmf3Position.z + m_pxmf3SphereVectors[i].z * m_fExplosionSpeed * m_fElapsedTimes;
				m_pxmf4x4Transforms[i] = Matrix4x4::Multiply(Matrix4x4::RotationAxis(m_pxmf3SphereVectors[i], m_fExplosionRotation * m_fElapsedTimes), m_pxmf4x4Transforms[i]);
			}
		}
		else
		{
			m_bBlowingUp = false;
			m_fElapsedTimes = 0.0f;
			SetColor(m_dwDefaultColor);
		}
	}
	else
	{
		CRotatingObject::Animate(fElapsedTime);
	}
}

void CExplosiveObject::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	if (m_bBlowingUp)
	{
		for (int i = 0; i < EXPLOSION_DEBRISES; i++)
		{
			CGameObject::Render(hDCFrameBuffer, &m_pxmf4x4Transforms[i], m_pExplosionMesh.get());
		}
	}
	else
	{
		CGameObject::Render(hDCFrameBuffer, &m_xmf4x4Transform, m_pMesh.get());
	}
}

CBulletObject::CBulletObject(float fEffectiveRange)
{
	m_fBulletEffectiveRange = fEffectiveRange;
}

CBulletObject::~CBulletObject()
{
}

void CBulletObject::SetFirePosition(XMFLOAT3 xmf3FirePosition)
{
	m_xmf3FirePosition = xmf3FirePosition;
	SetPosition(xmf3FirePosition);
}

void CBulletObject::Reset()
{
	m_pLockedObject = NULL;
	m_fElapsedTimeAfterFire = 0;
	m_fMovingDistance = 0;
	m_fRotationAngle = 0.0f;
	m_bActive = false;
	m_bValid = true;
}

void CBulletObject::Animate(float fElapsedTime)
{
	m_fElapsedTimeAfterFire += fElapsedTime;

	float fDistance = m_fMovingSpeed * fElapsedTime;

	if ((m_fElapsedTimeAfterFire > m_fLockingDelayTime) && m_pLockedObject)
	{
		XMFLOAT3 xmf3Position = GetPosition();
		XMVECTOR xmvPosition = XMLoadFloat3(&xmf3Position);

		XMFLOAT3 xmf3LockedObjectPosition = m_pLockedObject->GetPosition();
		XMVECTOR xmvLockedObjectPosition = XMLoadFloat3(&xmf3LockedObjectPosition);
		XMVECTOR xmvToLockedObject = xmvLockedObjectPosition - xmvPosition;
		xmvToLockedObject = XMVector3Normalize(xmvToLockedObject);

		XMVECTOR xmvMovingDirection = XMLoadFloat3(&m_xmf3MovingDirection);
		xmvMovingDirection = XMVector3Normalize(XMVectorLerp(xmvMovingDirection, xmvToLockedObject, 0.25f));
		XMStoreFloat3(&m_xmf3MovingDirection, xmvMovingDirection);
	}

	XMMATRIX xmRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.0f), XMConvertToRadians(m_fRotationSpeed * fElapsedTime), XMConvertToRadians(0.0f));
	XMStoreFloat4x4(&m_xmf4x4World, xmRotate * XMLoadFloat4x4(&m_xmf4x4World));
	XMFLOAT3 xmf3Position = GetPosition();
	XMStoreFloat3(&xmf3Position, XMLoadFloat3(&xmf3Position) + (XMLoadFloat3(&m_xmf3MovingDirection) * fDistance));
	m_xmf4x4World._41 = xmf3Position.x; m_xmf4x4World._42 = xmf3Position.y; m_xmf4x4World._43 = xmf3Position.z;
	m_fMovingDistance += fDistance;

	CGameObject::Animate(fElapsedTime);

	if ((m_fMovingDistance > m_fBulletEffectiveRange) || (m_fElapsedTimeAfterFire > m_fLockingTime)) Reset();
}


CExplosiveBulletObject::CExplosiveBulletObject(float fEffectiveRange)
{
	m_fBulletEffectiveRange = fEffectiveRange;
}

CExplosiveBulletObject::~CExplosiveBulletObject()
{
}

void CExplosiveBulletObject::SetFirePosition(XMFLOAT3 xmf3FirePosition)
{
	m_xmf3FirePosition = xmf3FirePosition;
	SetPosition(xmf3FirePosition);
}

void CExplosiveBulletObject::Reset()
{
	m_pLockedObject = NULL;
	m_fElapsedTimeAfterFire = 0;
	m_fMovingDistance = 0;
	m_fRotationAngle = 0.0f;
	m_bBlowingUp = false;
	m_bActive = false;
	m_bValid = true;
}

void CExplosiveBulletObject::Animate(float fElapsedTime)
{
	m_fElapsedTimeAfterFire += fElapsedTime;

	float fDistance = m_fMovingSpeed * fElapsedTime;

	XMFLOAT3 xmf3Position = GetPosition();

	// 중력 적용
	if (!m_bBlowingUp){
		XMFLOAT3 gravityDirection{ 0, -1, 0 };
		float gravity = cGravity * 0.5f * pow(m_fElapsedTimeAfterFire, 2) * m_fMess;
		gravityDirection = Vector3::ScalarProduct(gravityDirection, gravity);
		xmf3Position = Vector3::Add(xmf3Position, gravityDirection);
	}

	XMStoreFloat3(&xmf3Position, XMLoadFloat3(&xmf3Position) + (XMLoadFloat3(&m_xmf3MovingDirection) * fDistance));
	m_xmf4x4World._41 = xmf3Position.x; m_xmf4x4World._42 = xmf3Position.y; m_xmf4x4World._43 = xmf3Position.z;
	m_fMovingDistance += fDistance;

	// 폭발물의 애니메이션
	CRotatingObject::Animate(fElapsedTime);

	// 범위 벗어나면 폭발 후 리셋
	if (xmf3Position.y <= -10.f) {
		m_bBlowingUp = true;
		m_xmf3MovingDirection = XMFLOAT3{ 0.f, 0.f, 0.f };
	}

	if (m_bBlowingUp)
	{
		m_fElapsedTimes += fElapsedTime;
		if (m_fElapsedTimes <= m_fDuration)
		{
			XMFLOAT3 xmf3Position = GetPosition();
			for (int i = 0; i < EXPLOSION_DEBRISES; i++)
			{
				m_pxmf4x4Transforms[i] = Matrix4x4::Identity();
				m_pxmf4x4Transforms[i]._41 = xmf3Position.x + m_pxmf3SphereVectors[i].x * m_fExplosionSpeed * m_fElapsedTimes;
				m_pxmf4x4Transforms[i]._42 = xmf3Position.y + m_pxmf3SphereVectors[i].y * m_fExplosionSpeed * m_fElapsedTimes;
				m_pxmf4x4Transforms[i]._43 = xmf3Position.z + m_pxmf3SphereVectors[i].z * m_fExplosionSpeed * m_fElapsedTimes;
				m_pxmf4x4Transforms[i] = Matrix4x4::Multiply(Matrix4x4::RotationAxis(m_pxmf3SphereVectors[i], m_fExplosionRotation * m_fElapsedTimes), m_pxmf4x4Transforms[i]);
			}
		}
		else
		{
			m_fElapsedTimes = 0.f;
			SetColor(RGB(0, 255, 0));
			Reset();
		}
	}
}