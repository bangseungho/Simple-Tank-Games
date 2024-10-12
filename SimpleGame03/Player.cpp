#include "stdafx.h"
#include "Player.h"
#include "Shader.h"

CPlayer::CPlayer(int meshNum) : CGameObject(meshNum)
{
	_camera = nullptr;
	_position = Vec3(0.0f, 0.0f, 0.0f);
	_right = Vec3(1.0f, 0.0f, 0.0f);
	_up = Vec3(0.0f, 1.0f, 0.0f);
	_look = Vec3(0.0f, 0.0f, 1.0f);
	_velocity = Vec3(0.0f, 0.0f, 0.0f);
	_gravity = Vec3(0.0f, 0.0f, 0.0f);
	_maxVelocityXZ = 0.0f; 
	_rotationAxis = Vec3(0.f, 1.f, 0.f);
	_maxVelocityY = 0.0f;
	_friction = 0.0f;
	_pitch = 0.0f;
	_roll = 0.0f;
	_yaw = 0.0f;
	_playerUpdatedContext = NULL;
	_cameraUpdatedContext = NULL;
	_type = PT_NORMAL;

	_death = false;
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();
	for (auto& bullet : _bullets)
		delete bullet;
}

// 플레이어 키 입력 함수에서 호출되어 플레이어의 위치를 결정한다.
void CPlayer::Move(ULONG direction, float distance, bool updateVelocity)
{
	if (direction)
	{
		Vec3 shift = Vec3(0, 0, 0);
		if (direction & DIR_FORWARD) shift = Vector3::Add(shift, _look, distance);
		if (direction & DIR_BACKWARD) shift = Vector3::Add(shift, _look, -distance);
		if (direction & DIR_RIGHT && _type == PT_NORMAL) shift = Vector3::Add(shift, _right, distance);
		if (direction & DIR_LEFT && _type == PT_NORMAL) shift = Vector3::Add(shift, _right, -distance);
		if (direction & DIR_RIGHT && _type == PT_TIGER) RotateYawBody(0.3f);
		if (direction & DIR_LEFT && _type == PT_TIGER) RotateYawBody(-0.3f);
		if (direction & DIR_UP) shift = Vector3::Add(shift, _up, distance);
		if (direction & DIR_DOWN) shift = Vector3::Add(shift, _up, -distance);
		Move(shift, updateVelocity);
	}
}

// 플레이어 업데이트 함수에서 한 프레임마다 호출되어 플레이어를 이동한다.
void CPlayer::Move(const Vec3& shift, bool updateVelocity)
{
	//updateVelocity가 참이면 플레이어를 이동하지 않고 속도 벡터를 변경한다. 
	if (updateVelocity) {
		//플레이어의 속도 벡터를 shift 벡터만큼 변경한다. 
		_velocity = Vector3::Add(_velocity, shift);
	}
	else {
		//플레이어를 현재 위치 벡터에서 shift 벡터만큼 이동한다. 
		_position = Vector3::Add(_position, shift);
		//플레이어의 위치가 변경되었으므로 카메라의 위치도 shift 벡터만큼 이동한다. 
		if (_camera) _camera->Move(shift);
	}
}

void CPlayer::RotateYawBody(float angle)
{
	Matrix xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&_up), XMConvertToRadians(angle));
	_right = Vector3::TransformNormal(_right, xmmtxRotate);
	_look = Vector3::TransformNormal(_look, xmmtxRotate);
	_up = Vector3::TransformNormal(_up, xmmtxRotate);

	_look = Vector3::Normalize(_look);
	_right = Vector3::CrossProduct(_up, _look, true);
	_up = Vector3::CrossProduct(_look, _right, true);
}



void CPlayer::Rotate(float x, float y, float z)
{
	DWORD nCameraMode = _camera->GetMode();

	if ((nCameraMode == FIRST_PERSON_CAMERA) || (nCameraMode == THIRD_PERSON_CAMERA))
	{
		/*로컬 x-축을 중심으로 회전하는 것은 고개를 앞뒤로 숙이는 동작에 해당한다. 그러므로 x-축을 중심으로 회전하는
		각도는 -89.0~+89.0도 사이로 제한한다. x는 현재의 m_fPitch에서 실제 회전하는 각도이므로 x만큼 회전한 다음
		Pitch가 +89도 보다 크거나 -89도 보다 작으면 m_fPitch가 +89도 또는 -89도가 되도록 회전각도(x)를 수정한다.*/
		if (x != 0.0f)
		{
			_pitch += x;
			if (_pitch > +89.0f) { x -= (_pitch - 89.0f); _pitch = +89.0f; }
			if (_pitch < -89.0f) { x -= (_pitch + 89.0f); _pitch = -89.0f; }
		}
		if (y != 0.0f)
		{
			//로컬 y-축을 중심으로 회전하는 것은 몸통을 돌리는 것이므로 회전 각도의 제한이 없다. m_fYaw += y;
			if (_yaw > 360.0f) _yaw -= 360.0f;
			if (_yaw < 0.0f) _yaw += 360.0f;
		}
		if (z != 0.0f)
		{
			/*로컬 z-축을 중심으로 회전하는 것은 몸통을 좌우로 기울이는 것이므로 회전 각도는 -20.0~+20.0도 사이로 제한된
			다. z는 현재의 m_fRoll에서 실제 회전하는 각도이므로 z만큼 회전한 다음 m_fRoll이 +20도 보다 크거나 -20도보다
			작으면 m_fRoll이 +20도 또는 -20도가 되도록 회전각도(z)를 수정한다.*/
			_roll += z;
			if (_roll > +20.0f) { z -= (_roll - 20.0f); _roll = +20.0f; }
			if (_roll < -20.0f) { z -= (_roll + 20.0f); _roll = -20.0f; }
		}
		//카메라를 x, y, z 만큼 회전한다. 플레이어를 회전하면 카메라가 회전하게 된다. 
		_camera->Rotate(x, y, z);

		/*플레이어를 회전한다. 1인칭 카메라 또는 3인칭 카메라에서 플레이어의 회전은 로컬 y-축에서만 일어난다. 플레이어
		의 로컬 y-축(Up 벡터)을 기준으로 로컬 z-축(Look 벡터)s와 로컬 x-축(Right 벡터)을 회전시킨다. 기본적으로 Up 벡
		터를 기준으로 회전하는 것은 플레이어가 똑바로 서있는 것을 가정한다는 의미이다.*/
		if (y != 0.0f)
		{
			if (_type == PT_NORMAL) {
				Matrix xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&_up), XMConvertToRadians(y));
				_look = Vector3::TransformNormal(_look, xmmtxRotate);
				_right = Vector3::TransformNormal(_right, xmmtxRotate);
			}
			else if (_type == PT_TIGER) {
				CTigerPlayer* tigerPlayer = dynamic_cast<CTigerPlayer*>(this);
				if (tigerPlayer != nullptr) {
					Matrix rotate = XMMatrixRotationAxis(XMLoadFloat3(&_up), XMConvertToRadians(y / tigerPlayer->_rotateGunSpeed));
					tigerPlayer->_turret->_transform = Matrix4x4::Multiply(rotate, tigerPlayer->_turret->_transform);
				}
			}
		}
	}
	else if (nCameraMode == SPACESHIP_CAMERA)
	{
		/*스페이스-쉽 카메라에서 플레이어의 회전은 회전 각도의 제한이 없다. 그리고 모든 축을 중심으로 회전을 할 수 있
		다.*/
		_camera->Rotate(x, y, z);
		if (x != 0.0f)
		{
			Matrix xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&_right), XMConvertToRadians(x));
			_look = Vector3::TransformNormal(_look, xmmtxRotate);
			_up = Vector3::TransformNormal(_up, xmmtxRotate);
		}
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&_up), XMConvertToRadians(y));
			_look = Vector3::TransformNormal(_look, xmmtxRotate);
			_right = Vector3::TransformNormal(_right, xmmtxRotate);
		}
		if (z != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&_look), XMConvertToRadians(z));
			_up = Vector3::TransformNormal(_up, xmmtxRotate);
			_right = Vector3::TransformNormal(_right, xmmtxRotate);
		}
	}

	/*회전으로 인해 플레이어의 로컬 x-축, y-축, z-축이 서로 직교하지 않을 수 있으므로 z-축(Look 벡터)을 기준으로
	하여 서로 직교하고 단위벡터가 되도록 한다.*/
	_look = Vector3::Normalize(_look);
	_right = Vector3::CrossProduct(_up, _look, true);
	_up = Vector3::CrossProduct(_look, _right, true);
}

void CPlayer::Update(float timeElapsed)
{
	_velocity = Vector3::Add(_velocity, Vector3::ScalarProduct(_gravity, timeElapsed, false));

	float length = sqrtf(_velocity.x * _velocity.x + _velocity.z * _velocity.z);
	float maxVelocityXZ = _maxVelocityXZ * timeElapsed;
	if (length > _maxVelocityXZ) {
		_velocity.x *= maxVelocityXZ / length;
		_velocity.z *= maxVelocityXZ / length;
	}

	float maxVelocityY = _maxVelocityY * timeElapsed;
	length = sqrtf(_velocity.y * _velocity.y);
	if (length > _maxVelocityY) 
		_velocity.y *= (maxVelocityY / length);

	Move(_velocity, false);
	Animate(timeElapsed, nullptr);

	if (_playerUpdatedContext)
		OnPlayerUpdateCallback(timeElapsed);

	DWORD cameraMode = _camera->GetMode();
	
	if (cameraMode == THIRD_PERSON_CAMERA)
		_camera->Update(_position, timeElapsed);
	
	if (_cameraUpdatedContext)
		OnCameraUpdateCallback(timeElapsed);

	if (cameraMode == THIRD_PERSON_CAMERA)
		_camera-> SetLookAt(_position);

	_camera->RegenerateViewMatrix();

	length = Vector3::Length(_velocity);
	float deceleration = _friction * timeElapsed;
	if (deceleration > length)
		deceleration = length;
	_velocity = Vector3::Add(_velocity, Vector3::ScalarProduct(_velocity, -deceleration, true));
}

void CPlayer::Animate(float timeElapsed, XMFLOAT4X4* parent)
{
	if (_death) SetTimeAfterDeath(timeElapsed);

	UpdateBoundingBox();
}

void CPlayer::SetShader(std::shared_ptr<CShader> shader)
{
	if (_shader) _shader.reset();
	_shader = shader;
}

void CPlayer::CreateShaderVariables(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList)
{
	CGameObject::CreateShaderVariables(device, cmdList);

	if (_camera) _camera->CreateShaderVariables(device, cmdList);
}

void CPlayer::ReleaseShaderVariables()
{
	CGameObject::ReleaseShaderVariables();

	if (_camera) _camera->ReleaseShaderVariables();

	for (auto& bullet : _bullets)
		bullet->ReleaseUploadBuffers();
}

void CPlayer::UpdateShaderVariables(ComPtr<ID3D12GraphicsCommandList> cmdList)
{
	CGameObject::UpdateShaderVariables(cmdList);
}

std::shared_ptr<CCamera> CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode, CGameObject* target)
{
	std::shared_ptr<CCamera> newCamera;
	
	switch (nNewCameraMode) {
	case FIRST_PERSON_CAMERA:
		newCamera = std::make_shared<CFirstPersonCamera>(_camera);
		break;
	case THIRD_PERSON_CAMERA:
		newCamera = std::make_shared<CThirdPersonCamera>(_camera);
		break;
	case SPACESHIP_CAMERA:
		newCamera = std::make_shared<CSpaceShipCamera>(_camera);
		break;
	}

	if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		_right = Vector3::Normalize(Vec3(_right.x, 0.0f, _right.z));
		_up = Vector3::Normalize(Vec3(0.0f, 1.0f, 0.0f));
		_look = Vector3::Normalize(Vec3(_look.x, 0.0f, _look.z));
		_pitch = 0.0f;
		_roll = 0.0f;

		_yaw = Vector3::Angle(Vec3(0.0f, 0.0f, 1.0f), _look);
		if (_look.x < 0.0f) _yaw = -_yaw;
	}
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && _camera) {
		_right = _camera->GetRightVector();
		_up = _camera->GetUpVector();
		_look = _camera->GetLookVector();
	}

	if (newCamera) {
		newCamera->SetMode(nNewCameraMode);
		newCamera->SetPlayer(target);
	}

	if (_camera)
		_camera.reset();

	return newCamera;
}

void CPlayer::OnPrepareRender()
{
	_transform._11 = _right.x; _transform._12 = _right.y; _transform._13 = _right.z;
	_transform._21 = _up.x; _transform._22 = _up.y; _transform._23 = _up.z;
	_transform._31 = _look.x; _transform._32 = _look.y; _transform._33 = _look.z;
	_transform._41 = _position.x; _transform._42 = _position.y; _transform._43 = _position.z;

	UpdateTransform();
}

void CPlayer::Render(ComPtr<ID3D12GraphicsCommandList> cmdList, std::shared_ptr<CCamera> camera)
{
	DWORD nCameraMode = (_camera) ? _camera->GetMode() : 0x00;

	//카메라 모드가 3인칭이면 플레이어 객체를 렌더링한다. 
	if (nCameraMode == THIRD_PERSON_CAMERA){
		CGameObject::Render(cmdList, camera);
	}
}

// Tiger
CTigerPlayer::CTigerPlayer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList, ComPtr<ID3D12RootSignature> rootSignature, void* pContext, int meshNum) : CPlayer(meshNum)
{
	std::shared_ptr<CMesh> bodyMesh = std::make_shared<CMesh>(device, cmdList, "Models/Tiger/Body.bin", false);
	std::shared_ptr<CMesh> turretMesh = std::make_shared<CMesh>(device, cmdList, "Models/Tiger/Turret.bin", false);
	std::shared_ptr<CMesh> gunMesh = std::make_shared<CMesh>(device, cmdList, "Models/Tiger/Gun.bin", false);
	std::shared_ptr<CMesh> bulletMesh = std::make_shared<CMesh>(device, cmdList, "Models/Bullet.bin", false);

	_turret = std::make_shared<CGameObject>();
	_turret->SetMesh(0, turretMesh);
	_gun = std::make_shared<CGameObject>();
	_gun->SetMesh(0, gunMesh);
	SetChild(_turret);
	_turret->SetChild(_gun);
	SetMesh(0, bodyMesh);

	_camera = ChangeCamera(THIRD_PERSON_CAMERA, 0.f);

	CHeightMapTerrain* terrain = (CHeightMapTerrain*)pContext;
	float height = terrain->GetHeight(terrain->GetWidth() * 0.5f, terrain->GetLength() * 0.5f);
	SetPosition(Vec3(terrain->GetWidth() * 0.5f, height + 100.f, terrain->GetLength() * 0.5f));
	SetPlayerUpdatedContext(terrain);
	SetCameraUpdatedContext(terrain);

	_rotateGunSpeed = 10.f;
	_type = PT_TIGER;

	std::shared_ptr<CShader> shader = std::make_shared<CDiffusedShader>();
	shader->CreateShader(device, rootSignature);
	shader->CreateShaderVariables(device, cmdList);
	SetShader(shader);

	_bullets.resize(10);
	for (auto& bullet : _bullets) {
		bullet = new CBulletObject();
		bullet->SetMesh(0, bulletMesh);
		bullet->SetActive(false);
		bullet->SetShader(shader);
		bullet->SetMovingSpeed(20.f);
	}

	CreateShaderVariables(device, cmdList);
}

void CTigerPlayer::Render(ComPtr<ID3D12GraphicsCommandList> cmdList, std::shared_ptr<CCamera> camera)
{
	CPlayer::Render(cmdList, camera);
	
	for (auto& bullet : _bullets) {
		if (bullet->GetActive()) {
			bullet->Render(cmdList, camera);
		}
	}
}

CTigerPlayer::~CTigerPlayer()
{
}

void CTigerPlayer::Attack()
{
	CBulletObject* crtBullet = nullptr;
	
	for (auto& bullet : _bullets) {
		if (!bullet->GetActive()) {
			crtBullet = bullet;
			break;
		}
	}
	if (crtBullet) {
		Vec3 position = _gun->GetPosition();
		position.y += 2.f;
		Vec3 direction = _gun->GetLookVector();
		Vec3 firePosition = Vector3::Add(position, Vector3::ScalarProduct(direction, 9.0f, false));
		crtBullet->SetActive(true);
		crtBullet->_transform = _gun->_world;
		crtBullet->SetFirePosition(firePosition);
		crtBullet->SetMovingDirection(direction);
	}
}

void CTigerPlayer::Animate(float timeElapsed, XMFLOAT4X4* parent)
{
	CPlayer::Animate(timeElapsed, parent);
	for (auto& bullet : _bullets) {
		if (bullet->GetActive()) {
			bullet->Animate(timeElapsed);
			bullet->UpdateBoundingBox();
		}
	}
}

void CTigerPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	Vec3 playerPosition = GetPosition();
	CHeightMapTerrain* terrain = (CHeightMapTerrain*)_playerUpdatedContext;
	float terrainHeight = terrain->GetHeight(playerPosition.x, playerPosition.z) + 0.5f;

	if (playerPosition.y < terrainHeight) {
		Vec3 playerVelocity = GetVelocity();
		playerVelocity.y = 0.f;
		SetVelocity(playerVelocity);
		playerPosition.y = terrainHeight;
		SetPosition(playerPosition);

	}
}

void CTigerPlayer::OnCameraUpdateCallback(float fTimeElapsed)
{
}

void CTigerPlayer::OnPrepareRender()
{
	CPlayer::OnPrepareRender();

	Matrix mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&_rotationAxis), _surfaceAngle);
	_world = Matrix4x4::Multiply(mtxRotate, _world);
}

std::shared_ptr<CCamera> CTigerPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (_camera) ? _camera->GetMode() : 0x00;

	if (nCurrentCameraMode == nNewCameraMode) 
		return _camera;

	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		//플레이어의 특성을 1인칭 카메라 모드에 맞게 변경한다. 중력은 적용하지 않는다. 
		SetFriction(200.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(125.0f);
		SetMaxVelocityY(400.0f);
		_camera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode, this);
		_camera->SetTimeLag(0.0f);
		_camera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
		_camera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		_camera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		_camera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case SPACESHIP_CAMERA:
		//플레이어의 특성을 스페이스-쉽 카메라 모드에 맞게 변경한다. 중력은 적용하지 않는다. 
		SetFriction(125.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(400.0f);
		SetMaxVelocityY(400.0f);
		_camera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode, this);
		_camera->SetTimeLag(0.0f);
		_camera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
		_camera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		_camera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		_camera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		//플레이어의 특성을 3인칭 카메라 모드에 맞게 변경한다. 지연 효과와 카메라 오프셋을 설정한다. 
		SetFriction(250.0f);
		SetGravity(XMFLOAT3(0.0f, -100.0f, 0.0f));
		SetMaxVelocityXZ(125.0f);
		SetMaxVelocityY(400.0f);
		_camera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode, _turret.get());
		//3인칭 카메라의 지연 효과를 설정한다. 값을 0.25f 대신에 0.0f와 1.0f로 설정한 결과를 비교하기 바란다. 
		_camera->SetTimeLag(0.25f);
		_camera->SetOffset(XMFLOAT3(0.0f, 10.0f, -20.0f));
		_camera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		_camera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		_camera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}
	_camera->SetPosition(Vector3::Add(_position, _camera->GetOffset()));
	//플레이어를 시간의 경과에 따라 갱신(위치와 방향을 변경: 속도, 마찰력, 중력 등을 처리)한다. 

	Update(fTimeElapsed);

	return _camera;
}