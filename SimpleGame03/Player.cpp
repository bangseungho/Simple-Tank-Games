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

// �÷��̾� Ű �Է� �Լ����� ȣ��Ǿ� �÷��̾��� ��ġ�� �����Ѵ�.
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

// �÷��̾� ������Ʈ �Լ����� �� �����Ӹ��� ȣ��Ǿ� �÷��̾ �̵��Ѵ�.
void CPlayer::Move(const Vec3& shift, bool updateVelocity)
{
	//updateVelocity�� ���̸� �÷��̾ �̵����� �ʰ� �ӵ� ���͸� �����Ѵ�. 
	if (updateVelocity) {
		//�÷��̾��� �ӵ� ���͸� shift ���͸�ŭ �����Ѵ�. 
		_velocity = Vector3::Add(_velocity, shift);
	}
	else {
		//�÷��̾ ���� ��ġ ���Ϳ��� shift ���͸�ŭ �̵��Ѵ�. 
		_position = Vector3::Add(_position, shift);
		//�÷��̾��� ��ġ�� ����Ǿ����Ƿ� ī�޶��� ��ġ�� shift ���͸�ŭ �̵��Ѵ�. 
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
		/*���� x-���� �߽����� ȸ���ϴ� ���� ���� �յڷ� ���̴� ���ۿ� �ش��Ѵ�. �׷��Ƿ� x-���� �߽����� ȸ���ϴ�
		������ -89.0~+89.0�� ���̷� �����Ѵ�. x�� ������ m_fPitch���� ���� ȸ���ϴ� �����̹Ƿ� x��ŭ ȸ���� ����
		Pitch�� +89�� ���� ũ�ų� -89�� ���� ������ m_fPitch�� +89�� �Ǵ� -89���� �ǵ��� ȸ������(x)�� �����Ѵ�.*/
		if (x != 0.0f)
		{
			_pitch += x;
			if (_pitch > +89.0f) { x -= (_pitch - 89.0f); _pitch = +89.0f; }
			if (_pitch < -89.0f) { x -= (_pitch + 89.0f); _pitch = -89.0f; }
		}
		if (y != 0.0f)
		{
			//���� y-���� �߽����� ȸ���ϴ� ���� ������ ������ ���̹Ƿ� ȸ�� ������ ������ ����. m_fYaw += y;
			if (_yaw > 360.0f) _yaw -= 360.0f;
			if (_yaw < 0.0f) _yaw += 360.0f;
		}
		if (z != 0.0f)
		{
			/*���� z-���� �߽����� ȸ���ϴ� ���� ������ �¿�� ����̴� ���̹Ƿ� ȸ�� ������ -20.0~+20.0�� ���̷� ���ѵ�
			��. z�� ������ m_fRoll���� ���� ȸ���ϴ� �����̹Ƿ� z��ŭ ȸ���� ���� m_fRoll�� +20�� ���� ũ�ų� -20������
			������ m_fRoll�� +20�� �Ǵ� -20���� �ǵ��� ȸ������(z)�� �����Ѵ�.*/
			_roll += z;
			if (_roll > +20.0f) { z -= (_roll - 20.0f); _roll = +20.0f; }
			if (_roll < -20.0f) { z -= (_roll + 20.0f); _roll = -20.0f; }
		}
		//ī�޶� x, y, z ��ŭ ȸ���Ѵ�. �÷��̾ ȸ���ϸ� ī�޶� ȸ���ϰ� �ȴ�. 
		_camera->Rotate(x, y, z);

		/*�÷��̾ ȸ���Ѵ�. 1��Ī ī�޶� �Ǵ� 3��Ī ī�޶󿡼� �÷��̾��� ȸ���� ���� y-�࿡���� �Ͼ��. �÷��̾�
		�� ���� y-��(Up ����)�� �������� ���� z-��(Look ����)s�� ���� x-��(Right ����)�� ȸ����Ų��. �⺻������ Up ��
		�͸� �������� ȸ���ϴ� ���� �÷��̾ �ȹٷ� ���ִ� ���� �����Ѵٴ� �ǹ��̴�.*/
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
		/*�����̽�-�� ī�޶󿡼� �÷��̾��� ȸ���� ȸ�� ������ ������ ����. �׸��� ��� ���� �߽����� ȸ���� �� �� ��
		��.*/
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

	/*ȸ������ ���� �÷��̾��� ���� x-��, y-��, z-���� ���� �������� ���� �� �����Ƿ� z-��(Look ����)�� ��������
	�Ͽ� ���� �����ϰ� �������Ͱ� �ǵ��� �Ѵ�.*/
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

	//ī�޶� ��尡 3��Ī�̸� �÷��̾� ��ü�� �������Ѵ�. 
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
		//�÷��̾��� Ư���� 1��Ī ī�޶� ��忡 �°� �����Ѵ�. �߷��� �������� �ʴ´�. 
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
		//�÷��̾��� Ư���� �����̽�-�� ī�޶� ��忡 �°� �����Ѵ�. �߷��� �������� �ʴ´�. 
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
		//�÷��̾��� Ư���� 3��Ī ī�޶� ��忡 �°� �����Ѵ�. ���� ȿ���� ī�޶� �������� �����Ѵ�. 
		SetFriction(250.0f);
		SetGravity(XMFLOAT3(0.0f, -100.0f, 0.0f));
		SetMaxVelocityXZ(125.0f);
		SetMaxVelocityY(400.0f);
		_camera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode, _turret.get());
		//3��Ī ī�޶��� ���� ȿ���� �����Ѵ�. ���� 0.25f ��ſ� 0.0f�� 1.0f�� ������ ����� ���ϱ� �ٶ���. 
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
	//�÷��̾ �ð��� ����� ���� ����(��ġ�� ������ ����: �ӵ�, ������, �߷� ���� ó��)�Ѵ�. 

	Update(fTimeElapsed);

	return _camera;
}