#include "stdafx.h"
#include "GameObject.h"
#include "Shader.h"
#include "Camera.h"

std::default_random_engine dre;
std::uniform_real_distribution<double> uidLife{ -50.f, 0.f };

CGameObject::CGameObject(int meshNum)
{
	_transform = Matrix4x4::Identity();
	_world = Matrix4x4::Identity();
	_timeAfterDeath = 0.f;
	_timeAfterTime = 0.f;
	_movingDirection = Vec3(0.f, 0.f, 1.f);
	_meshNum = meshNum;
	_meshes.resize(meshNum);
}

CGameObject::~CGameObject()
{
}


bool CGameObject::IsVisible(std::shared_ptr<CCamera> camera)
{
	return camera->IsInFrustum(_boundingBox);
}

void CGameObject::SetActive(bool bActive)
{
	_active = bActive;

	if (_sibling) _sibling->SetActive(bActive);
	if (_child) _child->SetActive(bActive);
}


void CGameObject::SetPosition(float x, float y, float z)
{
	_transform._41 = x;
	_transform._42 = y;
	_transform._43 = z;

	UpdateTransform();
}

void CGameObject::SetPosition(Vec3 position)
{
	SetPosition(position.x, position.y, position.z);
}

void CGameObject::SetRotation(float x, float y, float z)
{
	Matrix mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(x), XMConvertToRadians(y), XMConvertToRadians(z));
	_transform = Matrix4x4::Multiply(mtxRotate, _transform);

	UpdateTransform();
}

bool CGameObject::GetActive()
{
	return _active;
}

Vec3 CGameObject::GetPosition()
{
	return Vec3(_world._41, _world._42, _world._43);
}

Vec3 CGameObject::GetLookVector()
{
	return Vector3::Normalize(Vec3(_world._31, _world._32, _world._33));
}

Vec3 CGameObject::GetUpVector()
{
	return Vector3::Normalize(Vec3(_world._21, _world._22, _world._23));
}

Vec3 CGameObject::GetRightVector()
{
	return Vector3::Normalize(Vec3(_world._11, _world._12, _world._13));
}

void CGameObject::SetChild(std::shared_ptr<CGameObject> child)
{
	if (_child) {
		if (_child) child->_sibling = _child->_sibling;
		_child->_sibling = child;
	}
	else {
		_child = child;
	}
}

void CGameObject::MoveStrafe(float distance)
{
	Vec3 position = GetPosition();
	Vec3 direction = GetRightVector();
	position = Vector3::Add(position, direction, distance);
	SetPosition(position);
}

void CGameObject::MoveUp(float distance)
{
	Vec3 position = GetPosition();
	Vec3 direction = GetUpVector();
	position = Vector3::Add(position, direction, distance);
	SetPosition(position);
}

void CGameObject::MoveForward(float distance)
{
	Vec3 position = GetPosition();
	Vec3 direction = GetLookVector();
	position = Vector3::Add(position, direction, distance);
	SetPosition(position);
}

void CGameObject::LookAt(Vec3& lookAt, Vec3& up)
{
	XMFLOAT4X4 xmf4x4View = Matrix4x4::LookAtLH(GetPosition(), lookAt, up);

	_transform._11 = xmf4x4View._11; _transform._12 = xmf4x4View._21; _transform._13 = xmf4x4View._31;
	_transform._21 = xmf4x4View._12; _transform._22 = xmf4x4View._22; _transform._23 = xmf4x4View._32;
	_transform._31 = xmf4x4View._13; _transform._32 = xmf4x4View._23; _transform._33 = xmf4x4View._33;
	UpdateTransform();
}

void CGameObject::LookTo(Vec3& lookTo, Vec3& up)
{
	XMFLOAT4X4 xmf4x4View = Matrix4x4::LookToLH(GetPosition(), lookTo, up);

	_transform._11 = xmf4x4View._11; _transform._12 = xmf4x4View._21; _transform._13 = xmf4x4View._31;
	_transform._21 = xmf4x4View._12; _transform._22 = xmf4x4View._22; _transform._23 = xmf4x4View._32;
	_transform._31 = xmf4x4View._13; _transform._32 = xmf4x4View._23; _transform._33 = xmf4x4View._33;
	UpdateTransform();
}


void CGameObject::Rotate(float pitch, float yaw, float roll)
{
	Matrix mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll));
	_transform = Matrix4x4::Multiply(mtxRotate, _transform);

	UpdateTransform();
}

void CGameObject::Rotate(Vec3* axis, float angle)
{
	Matrix mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(axis), XMConvertToRadians(angle));
	_transform = Matrix4x4::Multiply(mtxRotate, _transform);

	UpdateTransform();
}

void CGameObject::Rotate(Vec4* quaternion)
{
	Matrix mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(quaternion));
	_transform = Matrix4x4::Multiply(mtxRotate, _transform);

	UpdateTransform();
}

void CGameObject::SetShader(std::shared_ptr<CShader> shader)
{
	if (_shader) _shader.reset();
	_shader = shader;
}

void CGameObject::SetMesh(int index, std::shared_ptr<CMesh> mesh)
{
	if (index >= 0 && index < _meshNum) {
		if (_meshes[index]) _meshes[index].reset();
			_meshes[index] = mesh;
	}
}

void CGameObject::SetColor(Vec3 color)
{
	_color = color;
}

void CGameObject::CreateShaderVariables(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList)
{
}

void CGameObject::UpdateShaderVariables(ComPtr<ID3D12GraphicsCommandList> cmdList)
{
	Vec4x4 world;
	XMStoreFloat4x4(&world, XMMatrixTranspose(XMLoadFloat4x4(&_world)));
	cmdList->SetGraphicsRoot32BitConstants(1, 16, &world, 0);
	cmdList->SetGraphicsRoot32BitConstants(1, 1, &_timeAfterDeath, 16);
}

void CGameObject::ReleaseShaderVariables()
{
}

void CGameObject::ReleaseUploadBuffers()
{
	for (auto& mesh : _meshes)
		if (mesh) mesh->ReleaseUploadBuffers();

	if (_sibling) _sibling->ReleaseUploadBuffers();
	if (_child) _child->ReleaseUploadBuffers();
}

void CGameObject::Animate(float timeElapsed, XMFLOAT4X4* parent)
{
	if (_sibling) _sibling->Animate(timeElapsed, parent);
	if (_child) _child->Animate(timeElapsed, &_world);
}

void CGameObject::UpdateBoundingBox()
{
	_boundingBox.Center = Vec3(_world._41, _world._42, _world._43);

	if (_sibling) _sibling->UpdateBoundingBox();
	if (_child) _child->UpdateBoundingBox();
}

void CGameObject::OnPrepareRender()
{
}

void CGameObject::Render(ComPtr<ID3D12GraphicsCommandList> cmdList, std::shared_ptr<CCamera> camera)
{
	OnPrepareRender();
	UpdateShaderVariables(cmdList);
	if (_shader) _shader->Render(cmdList, camera);

	for (auto& mesh : _meshes)
		if (mesh) mesh->Render(cmdList);

	if (_sibling) _sibling->Render(cmdList, camera);
	if (_child) _child->Render(cmdList, camera);
}

void CGameObject::Render(ComPtr<ID3D12GraphicsCommandList> cmdList, std::shared_ptr<CCamera> camera, UINT instance, D3D12_VERTEX_BUFFER_VIEW instancingBufferView)
{
	OnPrepareRender();

	for (auto& mesh : _meshes)
		if (mesh) mesh->Render(cmdList, instance, instancingBufferView);

	if (_sibling) _sibling->Render(cmdList, camera, instance, instancingBufferView);
	if (_child) _child->Render(cmdList, camera, instance, instancingBufferView);
}


void CGameObject::UpdateTransform(Vec4x4* parent)
{
	_world = parent ? Matrix4x4::Multiply(_transform, *parent) : _transform;

	if (_sibling) _sibling->UpdateTransform(parent);
	if (_child) _child->UpdateTransform(&_world);
}

void CGameObject::SetTimeAfterDeath(float timeElapsed)
{
	_timeAfterDeath += timeElapsed * 4.f;
	_timeAfterTime += timeElapsed;
	
	if (_timeAfterTime > 5.f)
		SetActive(false);

	if (_timeAfterTime > 10.f)
		Reset();

	if (_sibling) _sibling->SetTimeAfterDeath(timeElapsed);
	if (_child) _child->SetTimeAfterDeath(timeElapsed);
}

void CGameObject::Reset()
{
	_timeAfterTime = 0.f;
	_timeAfterDeath = 0.f;
	SetActive(true);
	_death = false;
}

CRotatingObject::CRotatingObject(int meshNum) : CGameObject(meshNum)
{
	_rotationAxis = Vec3(0.f, 1.f, 0.f);
	_rotationSpeed = 90.f;
}

CRotatingObject::~CRotatingObject()
{
}

void CRotatingObject::Animate(float fTimeElapsed)
{
	CGameObject::Rotate(&_rotationAxis, _rotationSpeed * fTimeElapsed);
}

// bulletObject
CBulletObject::CBulletObject(int meshNum) : CGameObject(meshNum)
{
	SetActive(false);
	Rotate(0, 0.f, 0);
}

CBulletObject::~CBulletObject()
{
}

void CBulletObject::SetFirePosition(XMFLOAT3 firePosition)
{
	_firePosition = firePosition;
	SetPosition(firePosition);
}

void CBulletObject::Reset()
{
	_elapsedTimeAfterFire = 0.f;
	_movingDistance = 0.f;
	_lifeTime = 0.f;
	SetActive(false);
}

void CBulletObject::Animate(float elapsedTime)
{
	_elapsedTimeAfterFire += elapsedTime;
	
	Vec3 position = GetPosition();
	float distance = _movingSpeed * elapsedTime;

	XMStoreFloat3(&position, XMLoadFloat3(&position) + (XMLoadFloat3(&_movingDirection) * distance));
	SetPosition(position);
	_movingDistance += distance;

	if (_elapsedTimeAfterFire > 5.f)
		Reset();
}

void CBulletObject::SetShader(std::shared_ptr<CShader> shader)
{
	if (_shader) _shader.reset();
		_shader = shader;
}

void CBulletObject::OnPrepareRender()
{
	CGameObject::OnPrepareRender();
}

// bulletObject
CEBulletObject::CEBulletObject(int meshNum) : CBulletObject(meshNum)
{
	_lifeTime = uidLife(dre);
	SetActive(false);
}

CEBulletObject::~CEBulletObject()
{
}

void CEBulletObject::Animate(float elapsedTime)
{
	_elapsedTimeAfterFire += elapsedTime;
	_lifeTime += elapsedTime;

	if (_lifeTime > 0) {
		Vec3 position = GetPosition();
		float distance = _movingSpeed * elapsedTime;

		XMStoreFloat3(&position, XMLoadFloat3(&position) + (XMLoadFloat3(&_movingDirection) * distance));
		SetPosition(position);
		
		float movingDistance = _movingSpeed * _lifeTime;

		if (_lifeTime > 8.f)
			Reset();
	}
}

CHeightMapTerrain::~CHeightMapTerrain()
{
}

void CHeightMapTerrain::UpdateShaderVariables(ComPtr<ID3D12GraphicsCommandList> cmdList)
{
	Vec4x4 world;
	XMStoreFloat4x4(&world, XMMatrixTranspose(XMLoadFloat4x4(&_world)));
	cmdList->SetGraphicsRoot32BitConstants(1, 16, &world, 0);
}

CHeightMapTerrain::CHeightMapTerrain(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList, ComPtr<ID3D12RootSignature> rootSignature, LPCTSTR fileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, Vec3 xmf3Scale, Vec4 xmf4Color)
{
	_nWidth = nWidth;
	_nLength = nLength;

	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;

	_xmf3Scale = xmf3Scale;

	_heightMapImage = std::make_shared<CHeightMapImage>(fileName, nWidth, nLength, xmf3Scale);

	long cxBlocks = (_nWidth - 1) / cxQuadsPerBlock;
	long czBlocks = (_nLength - 1) / czQuadsPerBlock;

	_meshNum = cxBlocks * czBlocks;

	_meshes.resize(_meshNum);

	std::shared_ptr<CHeightMapGridMesh> heightMapGridMesh;
	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			//지형의 일부분을 나타내는 격자 메쉬의 시작 위치(좌표)이다. 
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			//지형의 일부분을 나타내는 격자 메쉬를 생성하여 지형 메쉬에 저장한다. 
			heightMapGridMesh = std::make_shared<CHeightMapGridMesh>(device, cmdList, xStart,
				zStart, nBlockWidth, nBlockLength, xmf3Scale, xmf4Color, _heightMapImage.get());
			SetMesh(x + (z * cxBlocks), heightMapGridMesh);
		}
	}

	std::shared_ptr<CShader> shader = std::make_shared<CTerrainShader>();
	shader->CreateShader(device, rootSignature);
	SetShader(shader);
}