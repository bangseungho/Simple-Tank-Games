#pragma once
#include "Mesh.h"

class CShader;
class CCamera;

class CGameObject {
public:
	CGameObject();
	virtual ~CGameObject();

public:
	bool IsVisible(std::shared_ptr<CCamera> camera);
	void SetActive(bool bActive);
	void SetPosition(float x, float y, float z);
	void SetPosition(Vec3 position);
	void SetRotation(float x, float y, float z);

	bool GetActive();
	Vec3 GetPosition();
	Vec3 GetRightVector();
	Vec3 GetUpVector();
	Vec3 GetLookVector();

	void SetChild(std::shared_ptr<CGameObject> child);

	void MoveStrafe(float distance = 1.f);
	void MoveUp(float distance = 1.f);
	void MoveForward(float distance = 1.f);

	void LookAt(Vec3& lookTo, Vec3& up);

	virtual void Rotate(float pitch = 10.f, float yaw = 10.f, float roll = 10.f);
	virtual void Rotate(Vec3* axis, float angle);
	virtual void Rotate(Vec4* quaternion);

	virtual void SetMesh(std::shared_ptr<CMesh> mesh);
	virtual void SetColor(Vec3 color);
	void SetMovingDirection(Vec3& movingDirection) { _movingDirection = Vector3::Normalize(movingDirection); }
	void SetMovingSpeed(float speed) { _movingSpeed = speed; }

	virtual void CreateShaderVariables(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList);
	virtual void UpdateShaderVariables(ComPtr<ID3D12GraphicsCommandList> cmdList);
	virtual void ReleaseShaderVariables();
	virtual void ReleaseUploadBuffers();
	
	virtual void Animate(float timeElapsed, XMFLOAT4X4* parent);
	void UpdateBoundingBox();
	virtual void OnPrepareRender();
	virtual void Render(ComPtr<ID3D12GraphicsCommandList> cmdList, std::shared_ptr<CCamera> camera);
	virtual void Render(ComPtr<ID3D12GraphicsCommandList> cmdList, std::shared_ptr<CCamera> camera,
		UINT instance, D3D12_VERTEX_BUFFER_VIEW instancingBufferView);

	void UpdateTransform(Vec4x4* parent = nullptr);

	void SetTimeAfterDeath(float timeElapsed);
	void Reset();

public:
	Vec4x4 _world;
	Vec4x4 _transform;
	BoundingBox _boundingBox = BoundingBox();

	std::shared_ptr<CShader> _shader;
	std::shared_ptr<CMesh> _mesh;
	std::shared_ptr<CGameObject> _sibling;
	std::shared_ptr<CGameObject> _child;
	std::shared_ptr<CGameObject> _parent;

	Vec3 _color = Vec3{ 1.0f, 1.0f, 1.0f };

	Vec3 _movingDirection;
	float _movingDistance = 0.0f;
	float _movingSpeed = 0.f;

	bool _active = true;
	float _timeAfterDeath;
	float _timeAfterTime;

	bool _death;
};

class CRotatingObject : public CGameObject {
public:
	CRotatingObject();
	virtual ~CRotatingObject();

public:
	void SetRotationSpeed(float rotationSpeed) { _rotationSpeed = rotationSpeed; }
	void SetRotationAxis(Vec3 rotationAxis) { _rotationAxis = rotationAxis; }
	virtual void Animate(float fTimeElapsed);

private:
	Vec3 _rotationAxis;
	float _rotationSpeed;
};

class CBulletObject : public CGameObject
{
public:
	CBulletObject();
	virtual ~CBulletObject();

public:
	void SetFirePosition(XMFLOAT3 firePosition);
	void Reset();
	virtual void SetShader(std::shared_ptr<CShader> shader);


	virtual void Animate(float elapsedTime);
	virtual void OnPrepareRender();

public:
	XMFLOAT3 _firePosition = XMFLOAT3(0.0f, 0.0f, 1.0f);
	float _elapsedTimeAfterFire = 0.0f;
	float _lifeTime;
};

class CEBulletObject : public CBulletObject
{
public:
	CEBulletObject();
	virtual ~CEBulletObject();

public:
	virtual void Animate(float elapsedTime);
};