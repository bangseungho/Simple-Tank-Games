#include "stdafx.h"

ComPtr<ID3D12Resource> CreateBufferResource(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList, void* pData, UINT nBytes, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resState, ID3D12Resource** uploadBuffer)
{
	ComPtr<ID3D12Resource> buffer;

	// CD3D12 구조체를 이용해서 쉽게 초기화 가능
	// D3D12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	// D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(nBytes);

	D3D12_HEAP_PROPERTIES heapProperty;
	::ZeroMemory(&heapProperty, sizeof(D3D12_HEAP_PROPERTIES));
	heapProperty.Type = heapType;
	heapProperty.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperty.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperty.CreationNodeMask = 1;
	heapProperty.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resDesc;
	::ZeroMemory(&resDesc, sizeof(D3D12_RESOURCE_DESC));
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Alignment = 0;
	resDesc.Width = nBytes;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_RESOURCE_STATES resInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;
	// 업로드 힙의 초기 상태는 GENERIC_READ, 리드백 힙의 초기 상태는 COPY_DEST
	if (heapType == D3D12_HEAP_TYPE_UPLOAD)
		resInitialStates = D3D12_RESOURCE_STATE_GENERIC_READ;
	else if (heapType == D3D12_HEAP_TYPE_READBACK)
		resInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;

	device->CreateCommittedResource(
		&heapProperty,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		resInitialStates,
		nullptr,
		IID_PPV_ARGS(buffer.GetAddressOf())
	);

	if (pData) {
		switch (heapType) {
		case D3D12_HEAP_TYPE_DEFAULT:
		{
			// 업로드 버퍼 생성
			heapProperty.Type = D3D12_HEAP_TYPE_UPLOAD;
			device->CreateCommittedResource(
				&heapProperty,
				D3D12_HEAP_FLAG_NONE,
				&resDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(uploadBuffer)
			);

			// 업로드 버퍼를 매핑하여 초기화 데이터를 업로드 버퍼에 복사
			D3D12_RANGE readRange = { 0, 0 };
			UINT8* bufferDataBegin = nullptr;
			(*uploadBuffer)->Map(0, &readRange, (void**)&bufferDataBegin);
			memcpy(bufferDataBegin, pData, nBytes);
			(*uploadBuffer)->Unmap(0, nullptr);

			// 업로드 버퍼의 내용을 디폴트 버퍼에 복사
			cmdList->CopyResource(buffer.Get(), *uploadBuffer);

			D3D12_RESOURCE_BARRIER barrier;
			::ZeroMemory(&barrier, sizeof(D3D12_RESOURCE_BARRIER));
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = buffer.Get();
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			barrier.Transition.StateAfter = resState;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			cmdList->ResourceBarrier(1, &barrier);
			break;
		}
		case D3D12_HEAP_TYPE_UPLOAD:
		{
			D3D12_RANGE readRange = { 0, 0 };
			UINT8* bufferDataBegin = nullptr;
			buffer->Map(0, &readRange, (void**)&bufferDataBegin);
			memcpy(bufferDataBegin, pData, nBytes);
			buffer->Unmap(0, nullptr);
			break;
		}
		case D3D12_HEAP_TYPE_READBACK:
			break;
		}
	}

	return buffer;
}