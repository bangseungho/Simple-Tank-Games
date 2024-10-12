#pragma once
#include "GameObject.h"

class CPlayer;

enum EnemyType {
	ET_NORMAL = 1,
	ET_IS7 = 2,
	ET_FLYERSHIP = 3,
};

class CEnemy : public CGameObject {
public:
	CEnemy();
	virtual ~CEnemy();

public:
	void SetTarget(CPlayer* target) { _target = target; }
	virtual void Attack() {}
	virtual void Render(ComPtr<ID3D12GraphicsCommandList> cmdList, std::shared_ptr<CCamera> camera = nullptr);
	virtual void Render(ComPtr<ID3D12GraphicsCommandList> cmdList, std::shared_ptr<CCamera> camera, UINT instance, D3D12_VERTEX_BUFFER_VIEW instancingBufferView);

public:
	EnemyType _type;
	CPlayer* _target = nullptr;
	CBulletObject* _bullet;
};

class CFlyerShip : public CEnemy {
public:
	CFlyerShip();
	virtual ~CFlyerShip();

public:
	virtual void Animate(float timeElapsed, XMFLOAT4X4* parent = NULL);
};

class CIS7Enemy : public CEnemy {
public:
	CIS7Enemy();
	virtual ~CIS7Enemy();
	virtual void Attack();

public:
	virtual void Animate(float timeElapsed, XMFLOAT4X4* parent = NULL);
	virtual void Render(ComPtr<ID3D12GraphicsCommandList> cmdList, std::shared_ptr<CCamera> camera, UINT instance, D3D12_VERTEX_BUFFER_VIEW instancingBufferView);
	virtual void ReleaseUploadBuffers();

public:
	std::shared_ptr<CGameObject> _turret;
	std::shared_ptr<CGameObject> _gun;
	float _rotateGunSpeed;
};
