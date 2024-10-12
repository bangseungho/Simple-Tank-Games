#include "stdafx.h"
#include "Player.h"
#include "Camera.h"

CCamera::CCamera()
{
	_view = Matrix4x4::Identity();
	_projection = Matrix4x4::Identity();
	_viewport = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.f, 1.f };
	_scissorRect = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
	_position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	_right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	_look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	_up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	_pitch = 0.0f;
	_roll = 0.0f;
	_yaw = 0.0f;
	_offset = XMFLOAT3(0.0f, 0.0f, 0.0f);
	_timeLag = 0.0f;
	_lookAtWorld = XMFLOAT3(0.0f, 0.0f, 0.0f);
	_mode = 0x00;
}

CCamera::CCamera(std::shared_ptr<CCamera> camera)
{
	if (camera) {
		*this = *camera;
	}
	else {
		_view = Matrix4x4::Identity();
		_projection = Matrix4x4::Identity();
		_viewport = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.f, 1.f };
		_scissorRect = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
		_position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		_right = XMFLOAT3(1.0f, 0.0f, 0.0f);
		_look = XMFLOAT3(0.0f, 0.0f, 1.0f);
		_up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		_pitch = 0.0f;
		_roll = 0.0f;
		_yaw = 0.0f;
		_offset = XMFLOAT3(0.0f, 0.0f, 0.0f);
		_timeLag = 0.0f;
		_lookAtWorld = XMFLOAT3(0.0f, 0.0f, 0.0f);
		_mode = 0x00;
	}
}

CCamera::~CCamera()
{
}

void CCamera::SetViewport(int xTopLeft, int yTopLeft, int width, int height, float minZ, float maxZ)
{
	_viewport.TopLeftX = float(xTopLeft);
	_viewport.TopLeftY = float(yTopLeft);
	_viewport.Width = float(width);
	_viewport.Height = float(height);
	_viewport.MinDepth = minZ;
	_viewport.MaxDepth = maxZ;
}

void CCamera::SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom)
{
	_scissorRect.left = xLeft;
	_scissorRect.top = yTop;
	_scissorRect.right = xRight;
	_scissorRect.bottom = yBottom;
}

void CCamera::SetViewportsAndScissorRects(ComPtr<ID3D12GraphicsCommandList> cmdList)
{
	cmdList->RSSetViewports(1, &_viewport);
	cmdList->RSSetScissorRects(1, &_scissorRect);
}

void CCamera::CreateShaderVariables(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList)
{
}

void CCamera::ReleaseShaderVariables()
{
}

void CCamera::UpdateShaderVariables(ComPtr<ID3D12GraphicsCommandList> cmdList)
{
	// 카메라 행렬의 전치 행렬을 저장하여 루트 상수에 바로 셋한다.
	// 쉐이더는 열우선 행렬이기 때문에 전치 행렬을 만들어줘야 한다.
	Vec4x4 view;
	XMStoreFloat4x4(&view, XMMatrixTranspose(XMLoadFloat4x4(&_view)));
	cmdList->SetGraphicsRoot32BitConstants(2, 16, &view, 0);

	Vec4x4 projection;
	XMStoreFloat4x4(&projection, XMMatrixTranspose(XMLoadFloat4x4(&_projection)));
	cmdList->SetGraphicsRoot32BitConstants(2, 16, &projection, 16);

	cmdList->SetGraphicsRoot32BitConstants(2, 3, &_position, 32);
}

void CCamera::GenerateViewMatrix(Vec3 position, Vec3 LookAt, Vec3 up)
{
	_position = position;
	_lookAtWorld = LookAt;
	_up = up;
	
	GenerateViewMatrix();
}

void CCamera::GenerateViewMatrix()
{
	_view = Matrix4x4::LookAtLH(_position, _lookAtWorld, _up);
}

void CCamera::GenerateFrustum()
{
	//원근 투영 변환 행렬에서 절두체를 생성한다(절두체는 카메라 좌표계로 표현된다).
	_frustum.CreateFromMatrix(_frustum, XMLoadFloat4x4(&_projection));
	//카메라 변환 행렬의 역행렬을 구한다. 
	XMMATRIX xmmtxInversView = XMMatrixInverse(NULL, XMLoadFloat4x4(&_view));
	//절두체를 카메라 변환 행렬의 역행렬로 변환한다(이제 절두체는 월드 좌표계로 표현된다).
	_frustum.Transform(_frustum, xmmtxInversView);
}

void CCamera::RegenerateViewMatrix()
{
	_look = Vector3::Normalize(_look);
	_right = Vector3::CrossProduct(_up, _look, true);
	_up = Vector3::CrossProduct(_look, _right, true);

	_view._11 = _right.x; _view._12 = _up.x; _view._13 = _look.x;
	_view._21 = _right.y; _view._22 = _up.y; _view._23 = _look.y;
	_view._31 = _right.z; _view._32 = _up.z; _view._33 = _look.z;
	_view._41 = -Vector3::DotProduct(_position, _right);
	_view._42 = -Vector3::DotProduct(_position, _up);
	_view._43 = -Vector3::DotProduct(_position, _look);

	GenerateFrustum();
}

bool CCamera::IsInFrustum(BoundingBox& boundingBox)
{
	return _frustum.Intersects(boundingBox);
}

void CCamera::GenerateProjectionMatrix(float nearPlaneDistance, float farPlaneDistance, float aspectRatio, float FOVAngle)
{
	_projection = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(FOVAngle), aspectRatio, nearPlaneDistance, farPlaneDistance);
}

CSpaceShipCamera::CSpaceShipCamera(std::shared_ptr<CCamera> camera) : CCamera(camera)
{
	_mode = SPACESHIP_CAMERA;
}

void CSpaceShipCamera::Rotate(float pitch, float yaw, float roll)
{
	if (_player && (pitch != 0.0f))
	{
		//플레이어의 로컬 x-축에 대한 x 각도의 회전 행렬을 계산한다. 
		Vec3 right = _player->GetRightVector();
		Matrix mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&right), XMConvertToRadians(pitch));

		// 카메라의 x, y, z축 회전
		_right = Vector3::TransformNormal(_right, mtxRotate);
		_up = Vector3::TransformNormal(_up, mtxRotate);
		_look = Vector3::TransformNormal(_look, mtxRotate);

		// 카메라의 원점을 플레이어에 맞추고 플레이어 중심으로 회전한 후 다시 제자리에 원상복귀
		_position = Vector3::Subtract(_position, _player->GetPosition());
		_position = Vector3::TransformCoord(_position, mtxRotate);
		_position = Vector3::Add(_position, _player->GetPosition());
	}
	if (_player && (yaw != 0.0f))
	{
		Vec3 up = _player->GetUpVector();
		Matrix mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&up), XMConvertToRadians(yaw));

		_right = Vector3::TransformNormal(_right, mtxRotate);
		_up = Vector3::TransformNormal(_up, mtxRotate);
		_look = Vector3::TransformNormal(_look, mtxRotate);

		_position = Vector3::Subtract(_position, _player->GetPosition());
		_position = Vector3::TransformCoord(_position, mtxRotate);
		_position = Vector3::Add(_position, _player->GetPosition());
	}
	if (_player && (roll != 0.0f))
	{
		Vec3 look = _player->GetLookVector();
		Matrix mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&look), XMConvertToRadians(roll));

		_right = Vector3::TransformNormal(_right, mtxRotate);
		_up = Vector3::TransformNormal(_up, mtxRotate);
		_look = Vector3::TransformNormal(_look, mtxRotate);

		_position = Vector3::Subtract(_position, _player->GetPosition());
		_position = Vector3::TransformCoord(_position, mtxRotate);
		_position = Vector3::Add(_position, _player->GetPosition());
	}
}

CFirstPersonCamera::CFirstPersonCamera(std::shared_ptr<CCamera> camera) : CCamera(camera)
{
	_mode = FIRST_PERSON_CAMERA;
	if (camera) {
		if (camera->GetMode() == SPACESHIP_CAMERA) {
			_up = Vec3(0.f, 1.f, 0.f);
			_right.y = 0.f;
			_look.y = 0.f;
			_right = Vector3::Normalize(_right);
			_look = Vector3::Normalize(_look);
		}
	}

}

void CFirstPersonCamera::Rotate(float pitch, float yaw, float roll)
{
	if (pitch != 0.f) {
		Matrix mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&_right), XMConvertToRadians(pitch));
		_right = Vector3::TransformNormal(_right, mtxRotate);
		_up = Vector3::TransformNormal(_up, mtxRotate);
		_look = Vector3::TransformNormal(_look, mtxRotate);
	}
	if (_player && (yaw != 0.f)) {
		Vec3 up = _player->GetUpVector();
		Matrix mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&up), XMConvertToRadians(yaw));
		_right = Vector3::TransformNormal(_right, mtxRotate);
		_up = Vector3::TransformNormal(_up, mtxRotate);
		_look = Vector3::TransformNormal(_look, mtxRotate);
	}
	if (_player && (roll != 0.f)) {
		Vec3 look = _player->GetLookVector();
		Matrix mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&look), XMConvertToRadians(roll));
		_position = Vector3::Subtract(_position, _player->GetPosition());
		_position = Vector3::TransformCoord(_position, mtxRotate);
		_position = Vector3::Add(_position, _player->GetPosition());
		_right = Vector3::TransformNormal(_right, mtxRotate);
		_up = Vector3::TransformNormal(_up, mtxRotate);
		_look = Vector3::TransformNormal(_look, mtxRotate);
	}
}

CThirdPersonCamera::CThirdPersonCamera(std::shared_ptr<CCamera> camera) : CCamera(camera)
{
	_mode = THIRD_PERSON_CAMERA;
	if (camera) {
		if (camera->GetMode() == SPACESHIP_CAMERA) {
			_up = Vec3(0.f, 1.f, 0.f);
			_right.y = 0.f;
			_look.y = 0.f;
			_right = Vector3::Normalize(_right);
			_look = Vector3::Normalize(_look);
		}
	}
}

void CThirdPersonCamera::Update(Vec3& lookAt, float timeElapsed)
{
	if (_player) {
		Vec4x4 mtxRotate = Matrix4x4::Identity();
		Vec3 right = _player->GetRightVector();
		Vec3 up = _player->GetUpVector();
		Vec3 look = _player->GetLookVector();
					 
		mtxRotate._11 = right.x; mtxRotate._21 = up.x; mtxRotate._31 = look.x;
		mtxRotate._12 = right.y; mtxRotate._22 = up.y; mtxRotate._32 = look.y;
		mtxRotate._13 = right.z; mtxRotate._23 = up.z; mtxRotate._33 = look.z;

		Vec3 offset = Vector3::TransformCoord(_offset, mtxRotate);
		Vec3 position = Vector3::Add(_player->GetPosition(), offset);
		Vec3 direction = Vector3::Subtract(position, _position);

		float length = Vector3::Length(direction);
		direction = Vector3::Normalize(direction);

		float timeLagScale = _timeLag ? timeElapsed * (1.f / _timeLag) : 1.f;
		float distance = length * timeLagScale;
		
		if (distance > length) distance = length;
		if (length < 0.01f) distance = length;
		if (distance > 0) {
			_position = Vector3::Add(_position, direction, distance);
			SetLookAt(lookAt);
		}
	}
}

void CThirdPersonCamera::SetLookAt(Vec3& lookAt)
{
	Vec4x4 mtxLookAt = Matrix4x4::LookAtLH(_position, lookAt, _player->GetUpVector());
	_right = Vec3(mtxLookAt._11, mtxLookAt._21, mtxLookAt._31);
	_up = Vec3(mtxLookAt._12, mtxLookAt._22, mtxLookAt._32);
	_look = Vec3(mtxLookAt._13, mtxLookAt._23, mtxLookAt._33);
}
