#pragma once
#include "GameObject.h"
#include <iostream>
using namespace std;

class CETank;

struct Status {
	float elapsedTime;
	float coolTime;
	float checkTarget;
};

class Node {
public:
	Node() { }
	Node(Status* status, CETank* pSelf) : status(status), m_pSelf(pSelf) { }
	virtual bool run() = 0;

protected:
	Status* status;
	CETank* m_pSelf;
};

// Selector 노드와 Sequence 노드는 Composite 노드를 상속받으며 
// 자식을 관리할 수 있는 노드가 된다.
class CompositeNode : public Node {
private:
	std::list<Node*> children;
public:
	const std::list<Node*>& getChildren() const { return children; }
	void addChild(Node* child) { children.emplace_back(child); }
};

// Selector 노드는 자식들을 하나씩 실행하며 하나라도 성공했을 경우 true를 리턴한다.
class Selector : public CompositeNode {
public:
	virtual bool run() override {
		for (Node* child : getChildren()) {
			if (child->run())
				return true;
		}
		return false;
	}
};

// Sequence 노드는 자식들을 하나씩 실행하며 하나라도 실패했을 경우 false를 리턴한다.
class Sequence : public CompositeNode {
public:
	virtual bool run() override {
		for (Node* child : getChildren()) {
			if (!child->run())
				return false;
		}
		return true;
	}
};

class BT_ScoutOn : public Node {
private:
public:
	BT_ScoutOn(Status* status, CETank* pSelf) : Node{ status, pSelf } { }
	virtual bool run() override;
};

class BT_CheckPlayer : public Node {
public:
	BT_CheckPlayer(Status* status, CETank* pSelf) : Node{ status, pSelf } { }
	virtual bool run() override;
};

class BT_TurnToPlayer : public Node {
public:
	BT_TurnToPlayer(Status* status, CETank* pSelf) : Node{ status, pSelf } { }
	virtual bool run() override;
};

class BT_Attack : public Node {
public:
	BT_Attack(Status* status, CETank* pSelf) : Node{ status, pSelf } { }
	virtual bool run() override;
};

class BT_Reload : public Node {
public:
	BT_Reload(Status* status, CETank* pSelf) : Node{ status, pSelf } { }
	virtual bool run() override;
};

class CEnemy : public CExplosiveObject
{
public:
	CEnemy(float x, float y, float z);
	virtual ~CEnemy() { };

public:
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3& xmf3Position);
	void SetRotation(XMFLOAT4X4& xmf4x4Rotation);
	void SetTarget(std::shared_ptr<CGameObject> pTarget) { m_pTarget = pTarget; }

public:
	void LookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up);
	void Move(XMFLOAT3& xmf3Direction, float fMovingSpeed);
	void Rotate(float fPitch, float fYaw, float fRoll);
	void Reset();

public:
	virtual void OnUpdateTransform();
	virtual void Animate(float fElapsedTime);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);
	virtual void RotateTurret(float fAngle) = 0;
	virtual void RotateGun(float fAngle) = 0;

public:
	int m_nlife = 3;
	XMFLOAT3 m_xmf3Position = XMFLOAT3(0.f, 0.f, 0.f);
	XMFLOAT3 m_xmf3Right = XMFLOAT3(1.f, 0.f, 0.f);
	XMFLOAT3 m_xmf3Up = XMFLOAT3(0.f, 1.f, 0.f);
	XMFLOAT3 m_xmf3Look = XMFLOAT3(0.f, 0.f, 1.f);
	XMFLOAT3 m_xmf3Velocity = XMFLOAT3(0.f, 0.f, 0.f);
	XMFLOAT3 m_xmf3FindNewPosition = XMFLOAT3(0.f, 0.f, 0.f);
	XMFLOAT3 m_xmf3ScoutPoint = XMFLOAT3{ 0.f, 0.f, 0.f };
	std::shared_ptr<CGameObject> m_pTarget;
	std::vector<std::shared_ptr<CExplosiveBulletObject>> m_vpBullets;
};

class CETank : public CEnemy
{
public:
	CETank(float x, float y, float z);
	virtual ~CETank();

public:
	void RotateTurret(float fAngle) { m_pTurret->Rotate(0.0f, fAngle, 0.0f); }
	void RotateGun(float fAngle) { m_pGun->Rotate(fAngle, 0.0f, 0.0f); }
	void FireBullet();
	void Scout();

public:
	virtual void Animate(float fElapsedTime);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);

public:
	std::shared_ptr<CGameObject> m_pTurret;
	std::shared_ptr<CGameObject> m_pGun;
	float m_fDistanceToTarget = 0.f;
	float m_fBulletEffectiveRange = 150.0f;
	float m_fSpySpeed = 0.05f;

private:
	Selector* root = new Selector;
	Sequence* sequence1 = new Sequence, * sequence2 = new Sequence;
	Status* status = new Status{0.f, 3.f, false};
	BT_CheckPlayer* checkPlayer = new BT_CheckPlayer(status, this);
	BT_ScoutOn* scoutOn = new BT_ScoutOn(status, this);
	BT_TurnToPlayer* turnToPlayer = new BT_TurnToPlayer(status, this);
	BT_Reload* reLoad = new BT_Reload(status, this);
	BT_Attack* attackPlayer = new BT_Attack(status, this);
};