#pragma once
#include "Timer.h"
#include "Shader.h"
#include "OctreeNode.h"
#include "QuadTreeNode.h"

class CGameObject;
class CEnemy;
class CPlayer;

class CScene {
public:
	CScene();
	~CScene();

public:
	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void BuildObjects(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList);
	void ReleaseObjects();
	void SetPlayer(CPlayer* player);

	void CheckObjectByPlayerCollisions();
	void CheckObjectByBulletCollisions();
	void CheckBulletByPlayerCollisions();

	bool ProcessInput();
	void AnimateObjects(float fTimeElapsed);
	void PrepareRender(ComPtr<ID3D12GraphicsCommandList> cmdList);
	void Render(ComPtr<ID3D12GraphicsCommandList> cmdList, std::shared_ptr<CCamera> camera);

	void ReleaseUploadBuffers();

	ComPtr<ID3D12RootSignature> CreateRootSignature(ComPtr<ID3D12Device> device);
	ComPtr<ID3D12RootSignature> GetRootSignature();
	
	std::shared_ptr<COctreeNode> BuildOctree(Vec3 center, float halfWidth, int depthLimit);
	CHeightMapTerrain* GetTerrain() { return _terrain; }
protected:
	CPlayer* _player;

	std::vector<std::shared_ptr<CObjectsShader>> _enemyShaders;
	int _shaderNum = 0;

	ComPtr<ID3D12RootSignature> _rootSignature;
	CHeightMapTerrain* _terrain;

	std::unique_ptr<CQuadTree> _quadTree;
	//std::shared_ptr<COctreeNode> _octree;
};

