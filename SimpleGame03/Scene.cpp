#include "stdafx.h"
#include "Scene.h"
#include "GameObject.h"
#include "Enemy.h"
#include "Camera.h"
#include "Player.h"

CScene::CScene()
{
}

CScene::~CScene()
{
	if (_terrain)
		delete _terrain;
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	DWORD dwDirection = 0;

	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'I': dwDirection = dwDirection |= DIR_FORWARD;
			break;
		case 'K': dwDirection = dwDirection |= DIR_BACKWARD;
			break;
		case 'J': dwDirection = dwDirection |= DIR_LEFT;
			break;
		case 'L': dwDirection = dwDirection |= DIR_RIGHT;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return(false);
}

#define _WITH_TERRAIN_PARTITION
void CScene::BuildObjects(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList)
{
	_rootSignature = CreateRootSignature(device);

	Vec3 terrainScale(10.f, 2.f, 10.f);
	Vec4 terrainColor(0.f, 0.2f, 0.f, 0.f);

#ifdef _WITH_TERRAIN_PARTITION
	_terrain = new CHeightMapTerrain(device, cmdList,
		_rootSignature, _T("Models/HeightMap.raw"), 257, 257, 17, 17, terrainScale, terrainColor);
#else
	_terrain = new CHeightMapTerrain(device, cmdList,
		_rootSignature, _T("HeightMap.raw"), 257, 257, 257, 257, terrainScale, terrainColor);
#endif

	_shaderNum = 1;
	_enemyShaders.reserve(_shaderNum);

	_enemyShaders.push_back(std::make_shared<CInstancingShader>());
	_enemyShaders[0]->CreateShader(device, _rootSignature);
	_enemyShaders[0]->BuildObjects(device, cmdList, _terrain);

	_quadTree = std::make_unique<CQuadTree>(_terrain->GetWidth(), _terrain->GetLength());
	for (auto& enemyShader : _enemyShaders) {
		for (auto& object : enemyShader->_objects) {
			_quadTree->Insert(object);
		}
	}
}

void CScene::ReleaseObjects()
{

}

void CScene::SetPlayer(CPlayer* player)
{
	_player = player;
}

void CScene::CheckObjectByPlayerCollisions()
{
	std::vector<std::shared_ptr<CEnemy>> mObjects = _quadTree->SearchQuadTree(_player->GetPosition().x, _player->GetPosition().z);

	for (auto& object : mObjects) {
		if (object->_boundingBox.Intersects(_player->_boundingBox) && !_player->_death) {
			object->_death = true;
		}
	}
}

void CScene::CheckObjectByBulletCollisions()
{
	for (auto& bullet : _player->_bullets) {
		std::vector<std::shared_ptr<CEnemy>> mObjects = _quadTree->SearchQuadTree(bullet->GetPosition().x, bullet->GetPosition().z);
			for (auto& object : mObjects) {
				if (bullet->GetActive() && object->GetActive()) {
					if (bullet->_boundingBox.Intersects(object->_boundingBox)) {
						bullet->SetActive(false);
						object->SetActive(false);
						object->_death = true;
					}
			}
		}
	}
}

void CScene::CheckBulletByPlayerCollisions()
{
	std::vector<std::shared_ptr<CEnemy>> mObjects = _quadTree->SearchQuadTree(_player->GetPosition().x, _player->GetPosition().z);
		for (auto& object : mObjects) {
			if (_player->_boundingBox.Intersects(object->_bullet->_boundingBox) && !_player->_death) {
				object->Attack();
				_player->_death = true;
			}
	}
}

void CScene::ReleaseUploadBuffers()
{
	for (auto& shader : _enemyShaders)
		shader->ReleaseUploadBuffers();

	if (_terrain)
		_terrain->ReleaseUploadBuffers();
}

bool CScene::ProcessInput()
{
	return false;
}

void CScene::AnimateObjects(float timeElapsed)
{
	for (auto& shader : _enemyShaders)
		shader->AnimateObjects(timeElapsed);

	CheckObjectByPlayerCollisions();
	CheckObjectByBulletCollisions();
	CheckBulletByPlayerCollisions();
}

void CScene::PrepareRender(ComPtr<ID3D12GraphicsCommandList> cmdList)
{
	cmdList->SetGraphicsRootSignature(_rootSignature.Get());
}

void CScene::Render(ComPtr<ID3D12GraphicsCommandList> cmdList, std::shared_ptr<CCamera> camera)
{
	camera->SetViewportsAndScissorRects(cmdList);
	camera->UpdateShaderVariables(cmdList);
	
	for (auto& shader : _enemyShaders)
		shader->Render(cmdList, camera);

	if (_terrain)
		_terrain->Render(cmdList, camera);
}

ComPtr<ID3D12RootSignature> CScene::CreateRootSignature(ComPtr<ID3D12Device> device)
{
	ComPtr<ID3D12RootSignature> rootSignature;

	D3D12_ROOT_PARAMETER rootParam[3];
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParam[0].Constants.Num32BitValues = 4;
	rootParam[0].Constants.ShaderRegister = 0;
	rootParam[0].Constants.RegisterSpace = 0;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParam[1].Constants.Num32BitValues = 17;
	rootParam[1].Constants.ShaderRegister = 1;
	rootParam[1].Constants.RegisterSpace = 0;
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParam[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParam[2].Constants.Num32BitValues = 35;
	rootParam[2].Constants.ShaderRegister = 2;
	rootParam[2].Constants.RegisterSpace = 0;
	rootParam[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS rootFlags = 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	D3D12_ROOT_SIGNATURE_DESC rootDesc;
	::ZeroMemory(&rootDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	rootDesc.NumParameters = _countof(rootParam);
	rootDesc.pParameters = rootParam;
	rootDesc.NumStaticSamplers = 0;
	rootDesc.pStaticSamplers = NULL;
	rootDesc.Flags = rootFlags;
	
	ComPtr<ID3DBlob> signatureBlob;
	ComPtr<ID3DBlob> errorBlob;

	::D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, signatureBlob.GetAddressOf(), errorBlob.GetAddressOf());

	device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(rootSignature.GetAddressOf()));

	return rootSignature;
}

ComPtr<ID3D12RootSignature> CScene::GetRootSignature()
{
	return _rootSignature;
}

std::shared_ptr<COctreeNode> CScene::BuildOctree(Vec3 center, float halfWidth, int depthLimit)
{
	if (depthLimit < 0)
		return nullptr;

	std::shared_ptr<COctreeNode> octNode = std::make_shared<COctreeNode>();
	BoundingBox boundingBox = octNode->GetBoundingBox();
	octNode->SetCenter(center);
	octNode->SetRadius(halfWidth);

	Vec3 offset;
	Vec3 childCenter;
	float step = halfWidth * 0.5f;
	
	for (UINT i = 0; i < CHILD_NODE_COUNT; ++i) {
		offset.x = (i & 1) ? step : -step;
		offset.y = (i & 4) ? step : -step;
		offset.z = (i & 2) ? step : -step;
		childCenter.x = offset.x + center.x;
		childCenter.y = offset.y + center.y;
		childCenter.z = offset.z + center.z;
		octNode->AddChildNode(BuildOctree(childCenter, step, depthLimit - 1));
	}

	return octNode;
}

