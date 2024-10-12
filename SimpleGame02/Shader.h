#pragma once
#include "GameObject.h"
#include "Camera.h"
#include "Enemy.h"

struct CB_GAMEOBJECT_INFO {
	Vec4 _world;
};

struct VS_VB_INSTANCE {
	Vec4x4 _transform;
	float _deathTime;

};

class CShader {
public:
	CShader();
	virtual ~CShader();

public:
	void SetPlayer(CPlayer* player);

	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_BLEND_DESC CreateBlendState();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ComPtr<ID3DBlob>& shaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ComPtr<ID3DBlob>& shaderBlob);
	D3D12_SHADER_BYTECODE CompileShaderFromFile(WCHAR* fileName, LPCSTR shaderName, LPCSTR shaderProfile, ComPtr<ID3DBlob>& shaderBlob);
	D3D12_SHADER_BYTECODE ReadCompiledShaderFile(WCHAR* fileName, ComPtr<ID3DBlob>& shaderBlob);

	virtual void CreateShader(ComPtr<ID3D12Device> device, ComPtr<ID3D12RootSignature> rootSignature);
	
	virtual void CreateShaderVariables(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList);
	virtual void UpdateShaderVariables(ComPtr<ID3D12GraphicsCommandList> cmdList);
	virtual void UpdateShaderVariables(ComPtr<ID3D12GraphicsCommandList> cmdList, Vec4x4* world);
	virtual void ReleaseShaderVariables();
	virtual void ReleaseUploadBuffers();

	virtual void BuildObjects(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList);
	virtual void AnimateObjects(float timeElapsed);
	virtual void ReleaseObjects();

	virtual void OnPrepareRender(ComPtr<ID3D12GraphicsCommandList> cmdList);
	virtual void Render(ComPtr<ID3D12GraphicsCommandList> cmdList, std::shared_ptr<CCamera> camera);

protected:
	CPlayer* _player;

	std::vector<ComPtr<ID3D12PipelineState>> _pipelineStates;
	ComPtr<ID3D12RootSignature> _rootSignature;

	int _pipelineStateNum;

};

class CDiffusedShader : public CShader {
public:
	CDiffusedShader();
	virtual ~CDiffusedShader();

public:
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ComPtr<ID3DBlob>& shaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ComPtr<ID3DBlob>& shaderBlob);

	virtual void CreateShader(ComPtr<ID3D12Device> device, ComPtr<ID3D12RootSignature> rootSignature);
};

class CObjectsShader : public CShader {
public:
	CObjectsShader();
	virtual ~CObjectsShader();

public:
	std::vector<std::shared_ptr<CEnemy>> mObjects;
};

class CEnemyShader : public CObjectsShader {
public:
	CEnemyShader();
	virtual ~CEnemyShader();

public:
	virtual void BuildObjects(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList);
};

class CInstancingShader : public CObjectsShader {
public:
	CInstancingShader();
	virtual ~CInstancingShader();

protected:
	// 인스턴스 정점 버퍼와 정점 버퍼 뷰
	ComPtr<ID3D12Resource>		mCbGameObjects;
	VS_VB_INSTANCE*				mCbMappedGameObjects;
	D3D12_VERTEX_BUFFER_VIEW	mInstancingBufferView;
};