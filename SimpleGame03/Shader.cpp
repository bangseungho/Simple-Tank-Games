#include "stdafx.h"
#include "Shader.h"
#include "Mesh.h"

std::default_random_engine dreS;
std::uniform_real_distribution<double> urd{ 0.01, 0.99 };
std::uniform_real_distribution<double> uid{ 0, 360 };

CShader::CShader()
{
}

CShader::~CShader()
{
}

void CShader::SetPlayer(CPlayer* player)
{
	_player = player;
}

D3D12_RASTERIZER_DESC CShader::CreateRasterizerState()
{
	// Rasterizer
	D3D12_RASTERIZER_DESC rasterizerDesc;
	::ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.f;
	rasterizerDesc.SlopeScaledDepthBias = 0.f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_DEPTH_STENCIL_DESC CShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	::ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = 0x00;
	depthStencilDesc.StencilWriteMask = 0x00;
	depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return depthStencilDesc;
}

D3D12_BLEND_DESC CShader::CreateBlendState()
{
	// Blend
	D3D12_BLEND_DESC blendDesc;
	::ZeroMemory(&blendDesc, sizeof(D3D12_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}

D3D12_INPUT_LAYOUT_DESC CShader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = NULL;
	inputLayoutDesc.NumElements = 0;

	return inputLayoutDesc;
}

D3D12_SHADER_BYTECODE CShader::CreateVertexShader(ComPtr<ID3DBlob>& shaderBlob)
{
	D3D12_SHADER_BYTECODE shaderByteCode;
	shaderByteCode.BytecodeLength = 0;
	shaderByteCode.pShaderBytecode = NULL;

	return shaderByteCode;
}

D3D12_SHADER_BYTECODE CShader::CreatePixelShader(ComPtr<ID3DBlob>& shaderBlob)
{
	D3D12_SHADER_BYTECODE shaderByteCode;
	shaderByteCode.BytecodeLength = 0;
	shaderByteCode.pShaderBytecode = NULL;

	return shaderByteCode;
}

D3D12_SHADER_BYTECODE CShader::CompileShaderFromFile(WCHAR* fileName, LPCSTR shaderName, LPCSTR shaderProfile, ComPtr<ID3DBlob>& shaderBlob)
{
	UINT compileFlags = 0;
#if defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	if (FAILED(D3DCompileFromFile(fileName, nullptr, nullptr, shaderName, shaderProfile, compileFlags, 0, &shaderBlob, nullptr))) {
		::MessageBoxA(nullptr, "Shader Create Failed!", nullptr, MB_OK);
	}

	D3D12_SHADER_BYTECODE shaderByteCode;
	shaderByteCode.BytecodeLength = shaderBlob->GetBufferSize();
	shaderByteCode.pShaderBytecode = shaderBlob->GetBufferPointer();

	return shaderByteCode;
}

D3D12_SHADER_BYTECODE CShader::ReadCompiledShaderFile(WCHAR* fileName, ComPtr<ID3DBlob>& shaderBlob)
{
	std::ifstream in{ fileName, std::ios::binary | std::ios::ate };
	UINT readBytes = (UINT)in.tellg();
	BYTE* byteCode = new BYTE[readBytes];
	in.seekg(0);
	in.read((char*)byteCode, readBytes);
	
	D3D12_SHADER_BYTECODE d3dByteCode;
	if (shaderBlob) {
		D3DCreateBlob(readBytes, shaderBlob.GetAddressOf());
		memcpy(shaderBlob->GetBufferPointer(), byteCode, readBytes);
		d3dByteCode.BytecodeLength = shaderBlob->GetBufferSize();
		d3dByteCode.pShaderBytecode = shaderBlob->GetBufferPointer();
	}
	else {
		d3dByteCode.BytecodeLength = readBytes;
		d3dByteCode.pShaderBytecode = byteCode;
	}

	return d3dByteCode;
}

void CShader::CreateShader(ComPtr<ID3D12Device> device, ComPtr<ID3D12RootSignature> rootSignature)
{
	ComPtr<ID3D12PipelineState> pipelineState;

	ComPtr<ID3DBlob> vertexShaderBlob;
	ComPtr<ID3DBlob> pixelShaderBlob;

	// PipelineState
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc;
	::ZeroMemory(&pipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	pipelineStateDesc.pRootSignature = rootSignature.Get();
	pipelineStateDesc.VS = CreateVertexShader(vertexShaderBlob);
	pipelineStateDesc.PS = CreatePixelShader(pixelShaderBlob);
	pipelineStateDesc.RasterizerState = CreateRasterizerState();
	pipelineStateDesc.BlendState = CreateBlendState();
	pipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	pipelineStateDesc.InputLayout = CreateInputLayout();
	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.NumRenderTargets = 1;
	pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	pipelineStateDesc.SampleDesc.Count = 1;
	pipelineStateDesc.SampleDesc.Quality = 0;
	pipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	HRESULT hr = device->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(pipelineState.GetAddressOf()));

	if (SUCCEEDED(hr))
		_pipelineStates.push_back(std::move(pipelineState));

	if (pipelineStateDesc.InputLayout.pInputElementDescs)
		delete[] pipelineStateDesc.InputLayout.pInputElementDescs;

	_rootSignature = rootSignature;
}

void CShader::CreateShaderVariables(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList)
{
}

void CShader::UpdateShaderVariables(ComPtr<ID3D12GraphicsCommandList> cmdList)
{
}

void CShader::UpdateShaderVariables(ComPtr<ID3D12GraphicsCommandList> cmdList, Vec4x4* pWorld)
{
}

void CShader::ReleaseShaderVariables()
{
}

void CShader::ReleaseUploadBuffers()
{

}

void CShader::BuildObjects(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList, void* pContext)
{

}

void CShader::ReleaseObjects()
{

}

void CShader::AnimateObjects(float timeElapsed)
{

}

void CShader::OnPrepareRender(ComPtr<ID3D12GraphicsCommandList> cmdList)
{
	cmdList->SetPipelineState(_pipelineStates[0].Get());
}

void CShader::Render(ComPtr<ID3D12GraphicsCommandList> cmdList, std::shared_ptr<CCamera> camera)
{
	OnPrepareRender(cmdList);
}

// CDiffusedShader
CDiffusedShader::CDiffusedShader()
{
}

CDiffusedShader::~CDiffusedShader()
{
}

D3D12_INPUT_LAYOUT_DESC CDiffusedShader::CreateInputLayout()
{
	UINT inputElementDescNum = 3;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[inputElementDescNum];

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = inputElementDescNum;

	return inputLayoutDesc;
}

D3D12_SHADER_BYTECODE CDiffusedShader::CreateVertexShader(ComPtr<ID3DBlob>& shaderBlob)
{
	return ReadCompiledShaderFile(L"VS_Shader.cso", shaderBlob);
	//return CompileShaderFromFile(L"VS_Shader.hlsl", "VS_Diffused", "vs_5_1", shaderBlob);
}

D3D12_SHADER_BYTECODE CDiffusedShader::CreatePixelShader(ComPtr<ID3DBlob>& shaderBlob)
{
	return ReadCompiledShaderFile(L"PS_Shader.cso", shaderBlob);
	//return CompileShaderFromFile(L"PS_Shader.hlsl", "PS_Diffused", "ps_5_1", shaderBlob);
}

void CDiffusedShader::CreateShader(ComPtr<ID3D12Device> device, ComPtr<ID3D12RootSignature> rootSignature)
{
	_pipelineStateNum = 1;
	_pipelineStates.reserve(_pipelineStateNum);

	CShader::CreateShader(device, rootSignature);
}

// CObjectsShader

CObjectsShader::CObjectsShader()
{
}

CObjectsShader::~CObjectsShader()
{
}

D3D12_INPUT_LAYOUT_DESC CObjectsShader::CreateInputLayout()
{
	UINT inputElementDescNum = 3;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[inputElementDescNum];

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = inputElementDescNum;

	return inputLayoutDesc;
}


D3D12_SHADER_BYTECODE CObjectsShader::CreateVertexShader(ComPtr<ID3DBlob>& shaderBlob)
{
	return ReadCompiledShaderFile(L"VS_Shader.cso", shaderBlob);
}

D3D12_SHADER_BYTECODE CObjectsShader::CreatePixelShader(ComPtr<ID3DBlob>& shaderBlob)
{
	return ReadCompiledShaderFile(L"PS_Shader.cso", shaderBlob);
}

void CObjectsShader::CreateShader(ComPtr<ID3D12Device> device, ComPtr<ID3D12RootSignature> rootSignature)
{
	_pipelineStateNum = 1;
	_pipelineStates.reserve(_pipelineStateNum);

	CShader::CreateShader(device, rootSignature);
}

void CObjectsShader::BuildObjects(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList, void* pContext)
{
	std::shared_ptr<CMesh> bodyMesh = std::make_shared<CMesh>(device, cmdList, "Models/IS7/Body.bin", false);
	std::shared_ptr<CMesh> turretMesh = std::make_shared<CMesh>(device, cmdList, "Models/IS7/Turret.bin", false);
	std::shared_ptr<CMesh> gunMesh = std::make_shared<CMesh>(device, cmdList, "Models/IS7/Gun.bin", false);

	_objectNum = 5;
	_objects.resize(_objectNum);

	for (int i = 0; i < _objectNum; ++i) {
		_objects[i] = std::make_shared<CIS7Enemy>();
		std::shared_ptr<CGameObject> _turret = std::make_shared<CGameObject>();
		std::shared_ptr<CGameObject> _gun = std::make_shared<CGameObject>();

		_objects[i]->SetMesh(0, bodyMesh);
		_turret->SetMesh(0, turretMesh);
		_gun->SetMesh(0, gunMesh);

		_turret->SetChild(_gun); 
		_objects[i]->SetChild(_turret);

		_objects[i]->SetRotation(0.f, 180.f, 0.f);
	}
	_objects[0]->SetPosition(Vec3(-10.f, 1.f, 20.f));
	_objects[1]->SetPosition(Vec3(+10.f, 1.f, 20.f));
	_objects[2]->SetPosition(Vec3(+20.f, 1.f, 20.f));
	_objects[3]->SetPosition(Vec3(-20.f, 1.f, 20.f));
	_objects[4]->SetPosition(Vec3(0.f, 1.f, 20.f));


	CreateShaderVariables(device, cmdList);
}

void CObjectsShader::AnimateObjects(float timeElapsed)
{
	for (auto& object : _objects) {
		object->Animate(timeElapsed, nullptr);
		object->UpdateTransform();
		object->UpdateBoundingBox();
	}
}

void CObjectsShader::ReleaseObjects()
{
}

void CObjectsShader::ReleaseUploadBuffers()
{
	for (auto& object : _objects)
		object->ReleaseUploadBuffers();
}

void CObjectsShader::Render(ComPtr<ID3D12GraphicsCommandList> cmdList, std::shared_ptr<CCamera> camera)
{
	CShader::Render(cmdList, camera);

	for (auto& object : _objects)
		object->Render(cmdList, camera);
}

// CEnemyShader
CEnemyShader::CEnemyShader()
{
}

CEnemyShader::~CEnemyShader()
{
}

void CEnemyShader::BuildObjects(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList)
{
	std::shared_ptr<CMesh> bodyMesh = std::make_shared<CMesh>(device, cmdList, "Models/IS7/Body.bin", false);
	std::shared_ptr<CMesh> turretMesh = std::make_shared<CMesh>(device, cmdList, "Models/IS7/Turret.bin", false);
	std::shared_ptr<CMesh> gunMesh = std::make_shared<CMesh>(device, cmdList, "Models/IS7/Gun.bin", false);
	std::shared_ptr<CMesh> flyerShipMesh = std::make_shared<CMesh>(device, cmdList, "Models/FlyerShip.bin", false);

	_objectNum = 6;
	_objects.resize(_objectNum);


	for (int i = 0; i < _objectNum - 1; ++i) {
		_objects[i] = std::make_shared<CIS7Enemy>();
		std::shared_ptr<CGameObject> _turret = std::make_shared<CGameObject>();
		std::shared_ptr<CGameObject> _gun = std::make_shared<CGameObject>();
		_objects[i]->SetMesh(0, bodyMesh);
		_turret->SetMesh(0, turretMesh);
		_gun->SetMesh(0, gunMesh);
		_turret->SetChild(_gun);
		_objects[i]->SetChild(_turret);
		_objects[i]->SetRotation(0.f, 180.f, 0.f);
	}
	_objects[5] = std::make_shared<CFlyerShip>();
	_objects[5]->SetMesh(0, flyerShipMesh);

	_objects[0]->SetPosition(Vec3(-10.f, 1.f, 20.f));
	_objects[1]->SetPosition(Vec3(+10.f, 1.f, 20.f));
	_objects[2]->SetPosition(Vec3(+20.f, 1.f, 20.f));
	_objects[3]->SetPosition(Vec3(-20.f, 1.f, 20.f));
	_objects[4]->SetPosition(Vec3(0.f, 1.f, 20.f));
	_objects[5]->SetPosition(Vec3(0.f, 10.f, 20.f));
	_objects[5]->SetRotation(0.f, 180.f, 0.f);

	CreateShaderVariables(device, cmdList);
}


// CInstancingShader
CInstancingShader::CInstancingShader()
{
}

CInstancingShader::~CInstancingShader()
{
}

D3D12_INPUT_LAYOUT_DESC CInstancingShader::CreateInputLayout()
{
	UINT inputElementDescNum = 8;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[inputElementDescNum];

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[3] = { "WORLDMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[4] = { "WORLDMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 3, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[5] = { "WORLDMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 3, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[6] = { "WORLDMATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 3, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[7] = { "DEATHTIME", 0, DXGI_FORMAT_R32_FLOAT, 3, 64, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = inputElementDescNum;
	return inputLayoutDesc;
}

D3D12_SHADER_BYTECODE CInstancingShader::CreateVertexShader(ComPtr<ID3DBlob>& shaderBlob)
{
	return ReadCompiledShaderFile(L"VS_Instance_Shader.cso", shaderBlob);
}

D3D12_SHADER_BYTECODE CInstancingShader::CreatePixelShader(ComPtr<ID3DBlob>& shaderBlob)
{
	return ReadCompiledShaderFile(L"PS_Instance_Shader.cso", shaderBlob);
}

void CInstancingShader::CreateShader(ComPtr<ID3D12Device> device, ComPtr<ID3D12RootSignature> rootSignature)
{
	_pipelineStateNum = 1;
	_pipelineStates.reserve(_pipelineStateNum);

	CShader::CreateShader(device, rootSignature);
}

void CInstancingShader::CreateShaderVariables(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList)
{
	// 인스턴스 정보를 저장할 정점 버퍼를 업로드 힙으로 생성
	_cbGameObjects = ::CreateBufferResource(
		device,
		cmdList,
		nullptr,
		sizeof(VS_VB_INSTANCE) * _objectNum,
		D3D12_HEAP_TYPE_UPLOAD,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		nullptr
	);

	// 정점 버퍼에 대한 포인터를 저장 
	_cbGameObjects->Map(0, nullptr, (void**)&_cbMappedGameObjects);

	// 정점 버퍼 뷰 생성
	_instancingBufferView.BufferLocation = _cbGameObjects->GetGPUVirtualAddress();
	_instancingBufferView.StrideInBytes = sizeof(VS_VB_INSTANCE);
	_instancingBufferView.SizeInBytes = sizeof(VS_VB_INSTANCE) * _objectNum;
}

void CInstancingShader::UpdateShaderVariables(ComPtr<ID3D12GraphicsCommandList> cmdList)
{
	for (int i = 0; i < _objectNum; ++i) {
		XMStoreFloat4x4(&_cbMappedGameObjects[i]._transform, XMMatrixTranspose(XMLoadFloat4x4(&_objects[i]->_world)));
		_cbMappedGameObjects[i]._deathTime = _objects[i]->_timeAfterDeath;
	}
}

void CInstancingShader::ReleaseShaderVariables()
{
	_cbGameObjects->Unmap(0, nullptr);
}

void CInstancingShader::BuildObjects(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList, void* pContext)
{
	CHeightMapTerrain* terrain = (CHeightMapTerrain*)pContext;
	float terrainWidth = terrain->GetWidth(), terrainLength = terrain->GetLength();
	
	std::shared_ptr<CMesh> bodyMesh = std::make_shared<CMesh>(device, cmdList, "Models/IS7/Body.bin", false);
	std::shared_ptr<CMesh> turretMesh = std::make_shared<CMesh>(device, cmdList, "Models/IS7/Turret.bin", false);
	std::shared_ptr<CMesh> gunMesh = std::make_shared<CMesh>(device, cmdList, "Models/IS7/Gun.bin", false);
	std::shared_ptr<CMesh> bulletMesh = std::make_shared<CMesh>(device, cmdList, "Models/Bullet.bin", false);
	std::shared_ptr<CShader> shader = std::make_shared<CDiffusedShader>();
	shader->CreateShader(device, _rootSignature);
	shader->CreateShaderVariables(device, cmdList);
	
	_objectNum = 900;

	UINT xValue = sqrt(_objectNum);
	UINT yValue = sqrt(_objectNum);
	_objects.resize(_objectNum);

	Vec3 terrainPosition = terrain->GetPosition();
	for (int i = 0; i < xValue; ++i) {
		for (int j = 0; j < yValue; ++j) {
			_objects[i * xValue + j] = std::make_shared<CIS7Enemy>();
			std::shared_ptr<CGameObject> _turret = std::make_shared<CGameObject>();
			std::shared_ptr<CGameObject> _gun = std::make_shared<CGameObject>();
			_turret->SetChild(_gun);
			_objects[i * xValue + j]->SetChild(_turret);

			int randValueX = terrainWidth * urd(dreS);
			int randValueZ = terrainLength * urd(dreS);
			float fHeight = terrain->GetHeight(randValueX, randValueZ);
			_objects[i * xValue + j]->SetPosition(Vec3(randValueX, fHeight, randValueZ));

			Vec3 surfaceNormal = terrain->GetNormal(randValueX, randValueZ);
			Vec3 rotateAxis = Vector3::CrossProduct(Vec3(0.f, 1.f, 0.f), surfaceNormal);
			if (Vector3::IsZero(rotateAxis))
				rotateAxis = Vec3(0.f, 1.f, 0.f);
			Vec3 upVector = Vec3(0.f, 1.f, 0.f);
			float angle = acos(Vector3::DotProduct(Vec3(0.f, 1.f, 0.f), surfaceNormal));
			_objects[i * xValue + j]->Rotate(&upVector, uid(dreS));
			_objects[i * xValue + j]->Rotate(&rotateAxis, XMConvertToDegrees(angle));

			_objects[i * xValue + j]->_bullet->SetMesh(0, bulletMesh);
			_objects[i * xValue + j]->_bullet->SetShader(shader);
		}
	}

	_objects[0]->SetMesh(0, bodyMesh);
	_objects[0]->_child->SetMesh(0, turretMesh);
	_objects[0]->_child->_child->SetMesh(0, gunMesh);

	for (int i = 1; i < _objectNum; ++i) {
		_objects[i]->_boundingBox = _objects[0]->_meshes[0]->_boundingBox;
	}

	CreateShaderVariables(device, cmdList);
}

void CInstancingShader::Render(ComPtr<ID3D12GraphicsCommandList> cmdList, std::shared_ptr<CCamera> camera)
{
	CShader::Render(cmdList, camera);

	UpdateShaderVariables(cmdList);

	_objects[0]->Render(cmdList, camera, _objectNum, _instancingBufferView);

	for (auto& object : _objects) {
		if(object->_bullet->_lifeTime > 0)
			object->_bullet->Render(cmdList, camera);
	}
}

CTerrainShader::CTerrainShader()
{
}

CTerrainShader::~CTerrainShader()
{
}

D3D12_INPUT_LAYOUT_DESC CTerrainShader::CreateInputLayout()
{
	UINT inputElementDescNum = 2;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[inputElementDescNum];

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = inputElementDescNum;

	return inputLayoutDesc;
}

D3D12_SHADER_BYTECODE CTerrainShader::CreateVertexShader(ComPtr<ID3DBlob>& shaderBlob)
{
	return ReadCompiledShaderFile(L"VS_Terrain.cso", shaderBlob);
}

D3D12_SHADER_BYTECODE CTerrainShader::CreatePixelShader(ComPtr<ID3DBlob>& shaderBlob)
{
	return ReadCompiledShaderFile(L"PS_Terrain.cso", shaderBlob);
}

void CTerrainShader::CreateShader(ComPtr<ID3D12Device> device, ComPtr<ID3D12RootSignature> rootSignature)
{
	_pipelineStateNum = 1;
	_pipelineStates.reserve(_pipelineStateNum);

	CShader::CreateShader(device, rootSignature);
}
