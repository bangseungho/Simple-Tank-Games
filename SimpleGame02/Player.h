#pragma once

#include "GameObject.h"
#include "Camera.h"

enum PlayerType {
	PT_NORMAL = 1,
	PT_TIGER = 2,
};

class CPlayer : public CGameObject {
public:
	CPlayer();
	virtual ~CPlayer();

public:
	Vec3 GetPosition() { return _position; }
	Vec3 GetLookVector() { return _look; }
	Vec3 GetUpVector() { return _up; }
	Vec3 GetRightVector() { return _right; }

	void SetFriction(float friction) { _friction = friction; }
	void SetGravity(Vec3& gravity) { _gravity = gravity; }

	void SetMaxVelocityXZ(float maxVelocity) { _maxVelocityXZ = maxVelocity; }
	void SetMaxVelocityY(float maxVelocity) { _maxVelocityY = maxVelocity; }

	void SetVelocity(Vec3& velocity) { _velocity = velocity; }
	Vec3& GetVelocity() { return _velocity; }

	/*�÷��̾��� ��ġ�� position ��ġ�� �����Ѵ�. position ���Ϳ��� ���� �÷��̾��� ��ġ ���͸� ���� ��
	�� �÷��̾��� ��ġ���� position ���������� ���Ͱ� �ȴ�. ���� �÷��̾��� ��ġ���� �� ���� ��ŭ �̵��Ѵ�.*/
	void SetPosition(Vec3& position) { Move(Vec3(position.x - _position.x, position.y - _position.y, position.z - _position.z), false); CGameObject::SetPosition(_position); }

	float GetYaw() { return _yaw; }
	float GetPitch() { return _pitch; }
	float GetRoll() { return _roll; }

	std::shared_ptr<CCamera> GetCamera() { return _camera; }
	void SetCamera(std::shared_ptr<CCamera> camera) { _camera = camera; }

	//�÷��̾ �̵��ϴ� �Լ��̴�. 
	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(const Vec3& xmf3Shift, bool bVelocity = false);

	//�÷��̾ ȸ���ϴ� �Լ��̴�. 
	void RotateBody(float x, float y, float z);
	void Rotate(float x, float y, float z);

	//�÷��̾��� ��ġ�� ȸ�� ������ ��� �ð��� ���� �����ϴ� �Լ��̴�. 
	virtual void Update(float fTimeElapsed);
	virtual void Animate(float timeElapsed, XMFLOAT4X4* parent = NULL);

	//�÷��̾��� ��ġ�� �ٲ� ������ ȣ��Ǵ� �Լ��� �� �Լ����� ����ϴ� ������ �����ϴ� �Լ��̴�.
	virtual void OnPlayerUpdateCallback(float fTimeElapsed) { }
	void SetPlayerUpdatedContext(LPVOID pContext) { _playerUpdatedContext = pContext; }

	//ī�޶��� ��ġ�� �ٲ� ������ ȣ��Ǵ� �Լ��� �� �Լ����� ����ϴ� ������ �����ϴ� �Լ��̴�. 
	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	void SetCameraUpdatedContext(LPVOID pContext) { _cameraUpdatedContext = pContext; }

	virtual void SetShader(std::shared_ptr<CShader> shader);
	virtual void CreateShaderVariables(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ComPtr<ID3D12GraphicsCommandList> cmdList);

	//ī�޶� �����ϱ� ���Ͽ� ȣ���ϴ� �Լ��̴�. 
	std::shared_ptr<CCamera> OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode, CGameObject* target);
	virtual std::shared_ptr<CCamera> ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) { return nullptr; }

	//�÷��̾��� ��ġ�� ȸ�������κ��� ���� ��ȯ ����� �����ϴ� �Լ��̴�.
	virtual void OnPrepareRender();

	//�÷��̾��� ī�޶� 3��Ī ī�޶��� �� �÷��̾�(�޽�)�� �������Ѵ�. 
	virtual void Render(ComPtr<ID3D12GraphicsCommandList> cmdList, std::shared_ptr<CCamera> camera = nullptr);

	virtual void Attack() { }

protected:
	Vec3 _position;
	Vec3 _right;
	Vec3 _up;
	Vec3 _look;
	
	float _pitch;
	float _roll;
	float _yaw;

	Vec3 _gravity;

	Vec3 _velocity;
	float _maxVelocityXZ;
	float _maxVelocityY;

	float _friction;

	//�÷��̾��� ��ġ�� �ٲ� ������ ȣ��Ǵ� OnPlayerUpdateCallback() �Լ����� ����ϴ� �������̴�.
	LPVOID _playerUpdatedContext;
	//ī�޶��� ��ġ�� �ٲ� ������ ȣ��Ǵ� OnCameraUpdateCallback() �Լ����� ����ϴ� �������̴�. 
	LPVOID _cameraUpdatedContext;

	std::shared_ptr<CCamera> _camera;
	std::shared_ptr<CCamera> _topCamera;

	PlayerType _type;

public:
	std::vector<CBulletObject*> _bullets;
};

class CTigerPlayer : public CPlayer
{
public:
	CTigerPlayer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList, ComPtr<ID3D12RootSignature> rootSignature);
	virtual ~CTigerPlayer();
	
public:
	virtual void OnPrepareRender();
	virtual std::shared_ptr<CCamera> ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);
	virtual void Render(ComPtr<ID3D12GraphicsCommandList> cmdList, std::shared_ptr<CCamera> camera = nullptr);
	virtual void Attack();
	virtual void Animate(float timeElapsed, XMFLOAT4X4* parent = NULL);

public:
	std::shared_ptr<CGameObject> _turret;
	std::shared_ptr<CGameObject> _gun;
	float _rotateGunSpeed;


};