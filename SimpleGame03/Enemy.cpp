#include "stdafx.h"
#include "Enemy.h"
#include "Shader.h"
#include "Player.h"

CEnemy::CEnemy()
{
	_type = ET_NORMAL;
	_death = false;
}

CEnemy::~CEnemy()
{
	ReleaseShaderVariables();
}

void CEnemy::Render(ComPtr<ID3D12GraphicsCommandList> cmdList, std::shared_ptr<CCamera> camera)
{
	CGameObject::Render(cmdList, camera);
}

void CEnemy::Render(ComPtr<ID3D12GraphicsCommandList> cmdList, std::shared_ptr<CCamera> camera, UINT instance, D3D12_VERTEX_BUFFER_VIEW instancingBufferView)
{
	CGameObject::Render(cmdList, camera, instance, instancingBufferView);
}

// FlyerShip
CFlyerShip::CFlyerShip()
{
	_type = ET_FLYERSHIP;
}

CFlyerShip::~CFlyerShip()
{
}

void CFlyerShip::Animate(float timeElapsed, XMFLOAT4X4* parent)
{
	if (_death) SetTimeAfterDeath(timeElapsed);

	XMMATRIX xmmtxTranslate = XMMatrixTranslation(0.f, 0.f, 0.0f * timeElapsed);
	_transform = Matrix4x4::Multiply(xmmtxTranslate, _transform);

	CGameObject::Animate(timeElapsed, parent);
}


// IS7
CIS7Enemy::CIS7Enemy()
{
	_rotateGunSpeed = 10.f;
	_type = ET_IS7;

	_bullet = new CEBulletObject();
	_bullet->SetActive(false);
	_bullet->SetMovingSpeed(30.f);
}

CIS7Enemy::~CIS7Enemy()
{
	delete _bullet;
}

void CIS7Enemy::Attack()
{
	Vec3 position = _child->_child->GetPosition();
	Vec3 direction = _child->_child->GetLookVector();
	Vec3 firePosition = Vector3::Add(position, Vector3::ScalarProduct(direction, 6.0f, false));
	_bullet->_transform = _child->_child->_world;
	_bullet->SetActive(true);
	_bullet->SetFirePosition(firePosition);
	_bullet->SetMovingDirection(direction);
}

void CIS7Enemy::Animate(float timeElapsed, XMFLOAT4X4* parent)
{
	if (_death)
		SetTimeAfterDeath(timeElapsed);

	if (!_bullet->GetActive() && GetActive())
		Attack();

	if (_bullet->GetActive()) {
		_bullet->Animate(timeElapsed);
		_bullet->UpdateBoundingBox();
	}

	CGameObject::Animate(timeElapsed, parent);
}

void CIS7Enemy::Render(ComPtr<ID3D12GraphicsCommandList> cmdList, std::shared_ptr<CCamera> camera, UINT instance, D3D12_VERTEX_BUFFER_VIEW instancingBufferView)
{
	CGameObject::Render(cmdList, camera, instance, instancingBufferView);
}

void CIS7Enemy::ReleaseUploadBuffers()
{
	CGameObject::ReleaseUploadBuffers();

	for (auto& mesh : _bullet->_meshes)
		if(mesh) mesh->ReleaseUploadBuffers();
}


