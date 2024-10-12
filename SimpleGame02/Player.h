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

	/*플레이어의 위치를 position 위치로 설정한다. position 벡터에서 현재 플레이어의 위치 벡터를 빼면 현
	재 플레이어의 위치에서 position 방향으로의 벡터가 된다. 현재 플레이어의 위치에서 이 벡터 만큼 이동한다.*/
	void SetPosition(Vec3& position) { Move(Vec3(position.x - _position.x, position.y - _position.y, position.z - _position.z), false); CGameObject::SetPosition(_position); }

	float GetYaw() { return _yaw; }
	float GetPitch() { return _pitch; }
	float GetRoll() { return _roll; }

	std::shared_ptr<CCamera> GetCamera() { return _camera; }
	void SetCamera(std::shared_ptr<CCamera> camera) { _camera = camera; }

	//플레이어를 이동하는 함수이다. 
	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(const Vec3& xmf3Shift, bool bVelocity = false);

	//플레이어를 회전하는 함수이다. 
	void RotateBody(float x, float y, float z);
	void Rotate(float x, float y, float z);

	//플레이어의 위치와 회전 정보를 경과 시간에 따라 갱신하는 함수이다. 
	virtual void Update(float fTimeElapsed);
	virtual void Animate(float timeElapsed, XMFLOAT4X4* parent = NULL);

	//플레이어의 위치가 바뀔 때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수이다.
	virtual void OnPlayerUpdateCallback(float fTimeElapsed) { }
	void SetPlayerUpdatedContext(LPVOID pContext) { _playerUpdatedContext = pContext; }

	//카메라의 위치가 바뀔 때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수이다. 
	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	void SetCameraUpdatedContext(LPVOID pContext) { _cameraUpdatedContext = pContext; }

	virtual void SetShader(std::shared_ptr<CShader> shader);
	virtual void CreateShaderVariables(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ComPtr<ID3D12GraphicsCommandList> cmdList);

	//카메라를 변경하기 위하여 호출하는 함수이다. 
	std::shared_ptr<CCamera> OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode, CGameObject* target);
	virtual std::shared_ptr<CCamera> ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) { return nullptr; }

	//플레이어의 위치와 회전축으로부터 월드 변환 행렬을 생성하는 함수이다.
	virtual void OnPrepareRender();

	//플레이어의 카메라가 3인칭 카메라일 때 플레이어(메쉬)를 렌더링한다. 
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

	//플레이어의 위치가 바뀔 때마다 호출되는 OnPlayerUpdateCallback() 함수에서 사용하는 데이터이다.
	LPVOID _playerUpdatedContext;
	//카메라의 위치가 바뀔 때마다 호출되는 OnCameraUpdateCallback() 함수에서 사용하는 데이터이다. 
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