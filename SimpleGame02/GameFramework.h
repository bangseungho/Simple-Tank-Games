#pragma once
#include "Timer.h"
#include "Scene.h"
#include "Player.h"

class CCamera;

class CGameFramework {
public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();
	static void ReportLiveObjects();

	void CreateDevice();
	void CreateCmdQueueAndList();
	void CreateSwapChain();
	void ChangeSwapChainState();

	void CreateRtvAndDsvDescHeaps();
	void CreateRTV();
	void CreateDSV();

	virtual void UpdateShaderVariables();

public:
	void BuildObjects();
	void ReleaseObjects();

	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();
	void WaitSync();
	void MoveToNextFrame();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

public:
	CPlayer* _player;
	POINT _oldCursorPos;

private:
	HINSTANCE _hInstance;
	HWND _hWnd;
	int _wndClientWidth;
	int _wndClientHeight;
	float _aspectRatio;

	bool _msaa4xEanble = false;
	UINT _msaa4xQualityLevels = 0;

	UINT _swapChainBufferIndex;

	ComPtr<IDXGIFactory4> _factory;
	ComPtr<ID3D12Device> _device;
	ComPtr<IDXGISwapChain3> _swapChain;

	ComPtr<ID3D12Resource> _rtvBuffer[SWAP_CHAIN_BUFFER_COUNT];
	D3D12_CPU_DESCRIPTOR_HANDLE _rtvHandle[SWAP_CHAIN_BUFFER_COUNT];
	ComPtr<ID3D12DescriptorHeap> _rtvHeap;
	UINT _rtvDescIncrementSize;

	ComPtr<ID3D12Resource> _dsvBuffer;
	D3D12_CPU_DESCRIPTOR_HANDLE _dsvHandle;
	ComPtr<ID3D12DescriptorHeap> _dsvHeap;
	UINT _dsvDescIncrementSize;

	ComPtr<ID3D12CommandQueue> _cmdQueue;
	ComPtr<ID3D12CommandAllocator> _cmdAlloc;
	ComPtr<ID3D12GraphicsCommandList> _cmdList;

	ComPtr<ID3D12PipelineState> _pipelineState;

	ComPtr<ID3D12Fence> _fence;
	UINT64 _fenceValue[SWAP_CHAIN_BUFFER_COUNT];
	HANDLE _fenceEvent;

	CGameTimer _timer;
	_TCHAR _frameRate[50];

private:
	std::shared_ptr<CScene> _scene;
	std::shared_ptr<CCamera> _camera;
};