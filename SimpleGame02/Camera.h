#pragma once

#define FIRST_PERSON_CAMERA 0x01
#define SPACESHIP_CAMERA 0x02
#define THIRD_PERSON_CAMERA 0x03

class CPlayer;

struct VS_CB_CAMERA_INFO{
	Vec4x4 _view;
	Vec4x4 _projection;
};

class CCamera{
public:
	CCamera();
	CCamera(std::shared_ptr<CCamera> camera);
	virtual ~CCamera();

public:
	void SetViewport(int xTopLeft, int yTopLeft, int width, int height, float minZ = 0.0f, float maxZ = 1.0f);
	void SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom);
	
	void SetPlayer(CGameObject* target) { _player = target; }
	CGameObject* GetPlayer() { return _player; }

	void SetMode(DWORD mode) { _mode = mode; }
	DWORD GetMode() { return _mode; }

	void SetPosition(Vec3 position) { _position = position; }
	Vec3& GetPosition() { return _position; }

	void SetLookAtPosition(Vec3 lookAtWorld) { _lookAtWorld = lookAtWorld; }
	Vec3& GetLookAtPosition() { return _lookAtWorld; }

	Vec3& GetLookVector() { return _look; }
	Vec3& GetUpVector() { return _up; }
	Vec3& GetRightVector() { return _right; }

	float& GetPitch() { return _pitch; }
	float& GetRoll() { return _roll; }
	float& GetYaw() { return _yaw; }

	void SetOffset(Vec3 offset) { _offset = offset; }
	Vec3& GetOffset() { return _offset; }

	void SetTimeLag(float timeLag) { _timeLag = timeLag; }
	float GetTimeLag() { return _timeLag; }

	Vec4x4 GetViewMatrix() { return _view; }
	Vec4x4 GetProjectionMatrix() { return _projection; }
	D3D12_VIEWPORT GetViewport() { return _viewport; }
	D3D12_RECT GetScissorRect() { return _scissorRect; }

	virtual void SetViewportsAndScissorRects(ComPtr<ID3D12GraphicsCommandList> cmdList);

	// 카메라의 정보를 셰이더에 전달하기 위한 상수 버퍼를 생성하고 갱신
	virtual void CreateShaderVariables(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ComPtr<ID3D12GraphicsCommandList> cmdList);

	// 카메라 변환 행렬 생성
	void GenerateViewMatrix(Vec3 position, Vec3 LookAt, Vec3 up);
	void GenerateViewMatrix();
	void GenerateFrustum();

	// 카메라의 누적 연산으로 인한 부정확성으로 각 축들이 다시 직교하도록 만들어 주는 함수
	void RegenerateViewMatrix();

	bool IsInFrustum(BoundingBox& boundingBox);

	// 투영변환 행렬 생성
	void GenerateProjectionMatrix(float nearPlaneDistance, float farPlaneDistance, float aspectRatio, float FOVAngle);

	virtual void Move(const Vec3& shift) {
		_position.x += shift.x;
		_position.y += shift.y; 
		_position.z += shift.z;
	}

	virtual void Rotate(float pitch = 0.0f, float yaw = 0.0f, float roll = 0.0f) { }
	virtual void Update(Vec3& lookAt, float timeElapsed) { }
	virtual void SetLookAt(Vec3& lookAt) { }

protected:
	BoundingFrustum _frustum;

	D3D12_VIEWPORT _viewport;
	D3D12_RECT _scissorRect;

	Vec3 _position;
	
	Vec3 _right;
	Vec3 _up;
	Vec3 _look;

	float _pitch;
	float _roll;
	float _yaw;

	DWORD _mode;

	Vec3 _lookAtWorld;
	Vec3 _offset;

	float _timeLag;

	Vec4x4 _view;
	Vec4x4 _projection;

	CGameObject* _player;
};

class CSpaceShipCamera : public CCamera {
public:
	CSpaceShipCamera(std::shared_ptr<CCamera> camera);
	virtual ~CSpaceShipCamera() { }

	virtual void Rotate(float pitch = 0.f, float yaw = 0.f, float roll = 0.f);
};

class CFirstPersonCamera : public CCamera{
public:
	CFirstPersonCamera(std::shared_ptr<CCamera> camera);
	virtual ~CFirstPersonCamera() { }

	virtual void Rotate(float pitch = 0.f, float yaw = 0.f, float roll = 0.f);
};

class CThirdPersonCamera : public CCamera{
public:
	CThirdPersonCamera(std::shared_ptr<CCamera> camera);
	virtual ~CThirdPersonCamera() { }

	virtual void Update(Vec3& lookAt, float timeElapsed);

	virtual void SetLookAt(Vec3& lookAt);
};