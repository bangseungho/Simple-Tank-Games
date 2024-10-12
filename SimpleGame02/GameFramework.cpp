#include "stdafx.h"
#include "GameFramework.h"
#include "Camera.h"

CGameFramework::CGameFramework()
{
	// 프로그램 종료 시점에 ReportLiveObjects를 호출한다.
	std::atexit(ReportLiveObjects);

	_swapChainBufferIndex = 0;
	_rtvDescIncrementSize = 0;
	_dsvDescIncrementSize = 0;

	for (int i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
		_fenceValue[i] = 0;

	_wndClientWidth = FRAME_BUFFER_WIDTH;
	_wndClientHeight = FRAME_BUFFER_HEIGHT;
	_aspectRatio = float(_wndClientWidth) / float(_wndClientHeight);

	_tcscpy_s(_frameRate, _T("LapProject ("));
}

CGameFramework::~CGameFramework()
{
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	_hInstance = hInstance;
	_hWnd = hMainWnd;

	CreateDevice();
	CreateCmdQueueAndList();
	CreateRtvAndDsvDescHeaps();
	CreateSwapChain();
	CreateDSV();

	BuildObjects();

	return true;
}

void CGameFramework::OnDestroy()
{
	WaitSync();

	ReleaseObjects();

	::CloseHandle(_fenceEvent);

	_swapChain->SetFullscreenState(FALSE, nullptr);
}

void CGameFramework::ReportLiveObjects()
{
#if defined(_DEBUG)
	ComPtr<IDXGIDebug1> dxgiDebug;
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiDebug.GetAddressOf()));
	dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
#endif
}

void CGameFramework::CreateDevice()
{
	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	ComPtr<ID3D12Debug> d3dDebug;
	D3D12GetDebugInterface(IID_PPV_ARGS(d3dDebug.GetAddressOf()));
	if (d3dDebug) d3dDebug->EnableDebugLayer();
	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(_factory.GetAddressOf()));

	ComPtr<IDXGIAdapter1> adapter;
	for (UINT i = 0; _factory->EnumAdapters1(i, adapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND; ++i) {
		DXGI_ADAPTER_DESC1 ad;
		adapter->GetDesc1(&ad);
		if (ad.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&_device)))) break;
	}

	if (!adapter) {
		_factory->EnumWarpAdapter(IID_PPV_ARGS(adapter.GetAddressOf()));
		D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(_device.GetAddressOf()));
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevels;
	msaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	msaaQualityLevels.SampleCount = 4;
	msaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msaaQualityLevels.NumQualityLevels = 0;
	_device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	_msaa4xQualityLevels = msaaQualityLevels.NumQualityLevels;

	_msaa4xEanble = (_msaa4xQualityLevels > 1) ? true : false;

	_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence.GetAddressOf()));

	for (int i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
		_fenceValue[i] = 0;

	_fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

void CGameFramework::CreateCmdQueueAndList()
{
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	_device->CreateCommandQueue(&d3dCommandQueueDesc, IID_PPV_ARGS(&_cmdQueue));
	_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(_cmdAlloc.GetAddressOf()));
	_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAlloc.Get(), nullptr, IID_PPV_ARGS(_cmdList.GetAddressOf()));

	_cmdList->Close();
}

void CGameFramework::CreateSwapChain()
{
	RECT clientRect;
	GetClientRect(_hWnd, &clientRect);
	_wndClientWidth = clientRect.right - clientRect.left;
	_wndClientHeight = clientRect.bottom - clientRect.top;

	DXGI_SWAP_CHAIN_DESC sd;
	::ZeroMemory(&sd, sizeof(DXGI_SWAP_CHAIN_DESC));
	sd.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
	sd.BufferDesc.Width = _wndClientWidth;
	sd.BufferDesc.Height = _wndClientHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.OutputWindow = _hWnd;
	sd.SampleDesc.Count = (_msaa4xEanble) ? 4 : 1;
	sd.SampleDesc.Quality = (_msaa4xEanble) ? (_msaa4xQualityLevels - 1) : 0;
	sd.Windowed = true;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	_factory->CreateSwapChain(_cmdQueue.Get(), &sd, (IDXGISwapChain**)_swapChain.GetAddressOf());

	_swapChainBufferIndex = _swapChain->GetCurrentBackBufferIndex();

	_factory->MakeWindowAssociation(_hWnd, DXGI_MWA_NO_ALT_ENTER);

#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRTV();
#endif 
}

void CGameFramework::ChangeSwapChainState()
{
	WaitSync();

	BOOL bFullScreenState = FALSE;

	_swapChain->GetFullscreenState(&bFullScreenState, NULL);
	_swapChain->SetFullscreenState(!bFullScreenState, NULL);

	DXGI_MODE_DESC targetParam;
	targetParam.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	targetParam.Width = _wndClientWidth;
	targetParam.Height = _wndClientHeight;
	targetParam.RefreshRate.Numerator = 60;
	targetParam.RefreshRate.Denominator = 1;
	targetParam.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	targetParam.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	_swapChain->ResizeTarget(&targetParam);

	for (int i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
		_rtvBuffer[i]->Release();

	DXGI_SWAP_CHAIN_DESC sd;
	_swapChain->GetDesc(&sd);
	_swapChain->ResizeBuffers(SWAP_CHAIN_BUFFER_COUNT, _wndClientWidth, _wndClientHeight, sd.BufferDesc.Format, sd.Flags);

	_swapChainBufferIndex = _swapChain->GetCurrentBackBufferIndex();

	CreateRTV();
}

void CGameFramework::CreateRtvAndDsvDescHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc;
	::ZeroMemory(&descHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	descHeapDesc.NumDescriptors = SWAP_CHAIN_BUFFER_COUNT;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descHeapDesc.NodeMask = 0;
	_device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(_rtvHeap.GetAddressOf()));
	_rtvDescIncrementSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	descHeapDesc.NumDescriptors = 1;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	_device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(_dsvHeap.GetAddressOf()));
	_dsvDescIncrementSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateRTV()
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapBegin = _rtvHeap->GetCPUDescriptorHandleForHeapStart();

	for (int i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++) {
		_rtvHandle[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeapBegin, i * _rtvDescIncrementSize);

		_swapChain->GetBuffer(i, IID_PPV_ARGS(_rtvBuffer[i].GetAddressOf()));
		_device->CreateRenderTargetView(_rtvBuffer[i].Get(), nullptr, _rtvHandle[i]);
	}
}

void CGameFramework::CreateDSV()
{
	D3D12_RESOURCE_DESC resDesc;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resDesc.MipLevels = 1;
	resDesc.Alignment = 0;
	resDesc.DepthOrArraySize = 1;
	resDesc.Width = _wndClientWidth;
	resDesc.Height = _wndClientHeight;
	resDesc.SampleDesc.Count = (_msaa4xEanble) ? 4 : 1;
	resDesc.SampleDesc.Quality = (_msaa4xEanble) ? (_msaa4xQualityLevels - 1) : 0;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	// 렌더 타겟 버퍼와 뎁스 스텐실 버퍼가 아닌 경우 클리어 밸류 사용 불가
	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	clearValue.DepthStencil.Depth = 1.f;
	clearValue.DepthStencil.Stencil = 0.f;

	_device->CreateCommittedResource(
		&heapProperty,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		IID_PPV_ARGS(_dsvBuffer.GetAddressOf()));

	_dsvHandle = _dsvHeap->GetCPUDescriptorHandleForHeapStart();
	_device->CreateDepthStencilView(_dsvBuffer.Get(), nullptr, _dsvHandle);
}

void CGameFramework::UpdateShaderVariables()
{
	float fCurrentTime = _timer.GetTotalTime();
	float fElapsedTime = _timer.GetTimeElapsed();

	_cmdList->SetGraphicsRoot32BitConstants(0, 1, &fCurrentTime, 0);
	_cmdList->SetGraphicsRoot32BitConstants(0, 1, &fElapsedTime, 1);

	POINT ptCursorPos;
	::GetCursorPos(&ptCursorPos);
	::ScreenToClient(_hWnd, &ptCursorPos);

	float fxCursorPos = (ptCursorPos.x < 0) ? 0.0f : float(ptCursorPos.x);
	float fyCursorPos = (ptCursorPos.y < 0) ? 0.0f : float(ptCursorPos.y);

	_cmdList->SetGraphicsRoot32BitConstants(0, 1, &fxCursorPos, 2);
	_cmdList->SetGraphicsRoot32BitConstants(0, 1, &fyCursorPos, 3);
}

void CGameFramework::BuildObjects()
{
	_cmdList->Reset(_cmdAlloc.Get(), nullptr);

	_scene = std::make_shared<CScene>();
	_scene->BuildObjects(_device, _cmdList);
	
	_player = new CTigerPlayer(_device, _cmdList, _scene->GetRootSignature());
	_camera = _player->GetCamera();
	_scene->SetPlayer(_player);

	_cmdList->Close();
	ID3D12CommandList* cmdListArr[]{ _cmdList.Get() };
	_cmdQueue->ExecuteCommandLists(_countof(cmdListArr), cmdListArr);

	WaitSync();

	if (_scene) _scene->ReleaseUploadBuffers();
	if (_player) _player->ReleaseUploadBuffers();

	_timer.Reset();
}

void CGameFramework::ReleaseObjects()
{
	delete _player;

	if (_scene) _scene->ReleaseObjects();
}

void CGameFramework::ProcessInput()
{
	static UCHAR pKeyBuffer[256];
	DWORD dwDirection = 0;
	/*키보드의 상태 정보를 반환한다. 화살표 키(‘→’, ‘←’, ‘↑’, ‘↓’)를 누르면 플레이어를 오른쪽/왼쪽(로컬 x-축), 앞/
	뒤(로컬 z-축)로 이동한다. ‘Page Up’과 ‘Page Down’ 키를 누르면 플레이어를 위/아래(로컬 y-축)로 이동한다.*/

	if (::GetAsyncKeyState('W')) dwDirection |= DIR_FORWARD;
	if (::GetAsyncKeyState('S')) dwDirection |= DIR_BACKWARD;
	if (::GetAsyncKeyState('A')) dwDirection |= DIR_LEFT;
	if (::GetAsyncKeyState('D')) dwDirection |= DIR_RIGHT;

	if (::GetKeyboardState(pKeyBuffer))
	{
		if (pKeyBuffer[VK_UP] & 0xF0) dwDirection |= DIR_FORWARD;
		if (pKeyBuffer[VK_DOWN] & 0xF0) dwDirection |= DIR_BACKWARD;
		if (pKeyBuffer[VK_LEFT] & 0xF0) dwDirection |= DIR_LEFT;
		if (pKeyBuffer[VK_RIGHT] & 0xF0) dwDirection |= DIR_RIGHT;
		if (pKeyBuffer[VK_PRIOR] & 0xF0) dwDirection |= DIR_UP;
		if (pKeyBuffer[VK_NEXT] & 0xF0) dwDirection |= DIR_DOWN;
	}
	float cxDelta = 0.0f, cyDelta = 0.0f;
	POINT ptCursorPos;
	/*마우스를 캡쳐했으면 마우스가 얼마만큼 이동하였는 가를 계산한다. 마우스 왼쪽 또는 오른쪽 버튼이 눌러질 때의
	메시지(WM_LBUTTONDOWN, WM_RBUTTONDOWN)를 처리할 때 마우스를 캡쳐하였다. 그러므로 마우스가 캡쳐된
	것은 마우스 버튼이 눌려진 상태를 의미한다. 마우스 버튼이 눌려진 상태에서 마우스를 좌우 또는 상하로 움직이면 플
	레이어를 x-축 또는 y-축으로 회전한다.*/
	if (::GetCapture() == _hWnd)
	{
		//마우스 커서를 화면에서 없앤다(보이지 않게 한다).
		::SetCursor(NULL);
		//현재 마우스 커서의 위치를 가져온다. 
		::GetCursorPos(&ptCursorPos);
		//마우스 버튼이 눌린 상태에서 마우스가 움직인 양을 구한다. 
		cxDelta = (float)(ptCursorPos.x - _oldCursorPos.x) / 3.0f;
		cyDelta = (float)(ptCursorPos.y - _oldCursorPos.y) / 3.0f;
		//마우스 커서의 위치를 마우스가 눌려졌던 위치로 설정한다. 
		::SetCursorPos(_oldCursorPos.x, _oldCursorPos.y);
	}
	//마우스 또는 키 입력이 있으면 플레이어를 이동하거나(dwDirection) 회전한다(cxDelta 또는 cyDelta).
	if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
	{
		if (cxDelta || cyDelta)
		{
			/*cxDelta는 y-축의 회전을 나타내고 cyDelta는 x-축의 회전을 나타낸다. 오른쪽 마우스 버튼이 눌려진 경우
			cxDelta는 z-축의 회전을 나타낸다.*/
			if (pKeyBuffer[VK_RBUTTON] & 0xF0)
				_player->Rotate(cyDelta, 0.0f, -cxDelta);
			else
				_player->Rotate(cyDelta, cxDelta, 0.0f);
		}
		/*플레이어를 dwDirection 방향으로 이동한다(실제로는 속도 벡터를 변경한다). 이동 거리는 시간에 비례하도록 한다. 플레이어의 이동 속력은 (50/초)로 가정한다.*/
		if (dwDirection) _player->Move(dwDirection, 20.0f * _timer.GetTimeElapsed(), true);
	}
	//플레이어를 실제로 이동하고 카메라를 갱신한다. 중력과 마찰력의 영향을 속도 벡터에 적용한다. 
	_player->Update(_timer.GetTimeElapsed());
}

void CGameFramework::AnimateObjects()
{
	if (_player) _player->Animate(_timer.GetTimeElapsed(), nullptr);
	if (_scene) _scene->AnimateObjects(_timer.GetTimeElapsed());
}

//#define _WITH_PLAYER_TOP
void CGameFramework::FrameAdvance()
{
	_timer.Tick(0.f);

	ProcessInput();
	
	AnimateObjects();

	_cmdAlloc->Reset();
	_cmdList->Reset(_cmdAlloc.Get(), nullptr);

	// 명령 리스트가 Reset 될 때마다 뷰포트와 시저 렉트를 다시 설정해야 함

	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		_rtvBuffer[_swapChainBufferIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);
	_cmdList->ResourceBarrier(1, &barrier);

	_cmdList->ClearRenderTargetView(_rtvHandle[_swapChainBufferIndex], Colors::Black, 0, nullptr);
	_cmdList->ClearDepthStencilView(_dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);
	_cmdList->OMSetRenderTargets(1, &_rtvHandle[_swapChainBufferIndex], TRUE, &_dsvHandle);

	// Render begin
	if (_scene) _scene->PrepareRender(_cmdList);
	UpdateShaderVariables();
	if (_scene) _scene->Render(_cmdList.Get(), _camera);

#ifdef _WITH_PLAYER_TOP
	_cmdList->ClearDepthStencilView(_dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0.f, 0.f, nullptr);
#endif

	if (_player) _player->Render(_cmdList, _camera);

	// Render end

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		_rtvBuffer[_swapChainBufferIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);
	_cmdList->ResourceBarrier(1, &barrier);

	_cmdList->Close();

	ID3D12CommandList* cmdListArr[] = { _cmdList.Get() };
	_cmdQueue->ExecuteCommandLists(1, cmdListArr);

	WaitSync();

	_swapChain->Present(0, 0);

	MoveToNextFrame();

	_timer.GetFrameRate(_frameRate + 12, 37);
	::SetWindowText(_hWnd, _frameRate);
}

void CGameFramework::WaitSync()
{
	UINT64 fenceValue = ++_fenceValue[_swapChainBufferIndex];

	_cmdQueue->Signal(_fence.Get(), fenceValue);

	if (_fence->GetCompletedValue() < fenceValue) {
		_fence->SetEventOnCompletion(fenceValue, _fenceEvent);
		::WaitForSingleObject(_fenceEvent, INFINITE);
	}
}

void CGameFramework::MoveToNextFrame()
{
	_swapChainBufferIndex = _swapChain->GetCurrentBackBufferIndex();

	UINT64 fenceValue = ++_fenceValue[_swapChainBufferIndex];

	_cmdQueue->Signal(_fence.Get(), fenceValue);

	if (_fence->GetCompletedValue() < fenceValue) {
		_fence->SetEventOnCompletion(fenceValue, _fenceEvent);
		::WaitForSingleObject(_fenceEvent, INFINITE);
	}
}


void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		::SetCapture(hWnd);
		::GetCursorPos(&_oldCursorPos);
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		::ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (_scene) _scene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_F1:
		case VK_F2:
		case VK_F3:
			if (_player)
				_camera = _player->ChangeCamera((DWORD)(wParam - VK_F1 + 1), _timer.GetTimeElapsed());
			break;
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			break;
		case VK_F8:
			break;
		case VK_F9:
			ChangeSwapChainState();
			break;
		case VK_CONTROL:
			if(!_player->_death)
				_player->Attack();
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

LRESULT CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_SIZE:
	{
		_wndClientWidth = LOWORD(lParam);
		_wndClientHeight = HIWORD(lParam);
		break;
	}
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return 0;
}

