#include "stdafx.h"
#include "Mesh.h"

CMesh::CMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList)
{
}

CMesh::CMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList, const char* fileName, bool bTextFile)
{
	if (fileName) LoadMeshFromFile(device, cmdList, fileName, bTextFile);
}

CMesh::~CMesh()
{
}

void CMesh::ReleaseUploadBuffers()
{
	if (_positionUploadBuffer)
		_positionUploadBuffer->Release();
	_positionUploadBuffer = nullptr;

	if (_normalUploadBuffer)
		_normalUploadBuffer->Release();
	_normalUploadBuffer = nullptr;

	if (_textureCoordUploadBuffer)
		_textureCoordUploadBuffer->Release();
	_textureCoordUploadBuffer = nullptr;

	if (_indexUploadBuffer)
		_indexUploadBuffer->Release();
	_indexUploadBuffer = nullptr;
}

void CMesh::Render(ComPtr<ID3D12GraphicsCommandList> cmdList)
{
	cmdList->IASetVertexBuffers(_slot, _vertexBufferViewNum, _vertexBufferViews.data());
	Render(cmdList, 1);
}

void CMesh::Render(ComPtr<ID3D12GraphicsCommandList> cmdList, UINT instance, D3D12_VERTEX_BUFFER_VIEW instancingBufferView)
{
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[] = {
	_vertexBufferViews[0],
	_vertexBufferViews[1],
	_vertexBufferViews[2],
	instancingBufferView,
	};

	cmdList->IASetVertexBuffers(_slot, _countof(vertexBufferViews), vertexBufferViews);
	Render(cmdList, instance);
}

void CMesh::Render(ComPtr<ID3D12GraphicsCommandList> cmdList, UINT instance)
{
	cmdList->IASetPrimitiveTopology(_primitiveTopology);
	if (_indexBuffer) {
		cmdList->IASetIndexBuffer(&_indexBufferView);
		cmdList->DrawIndexedInstanced(_indexNum, instance, 0, 0, 0);
	}
	else {
		cmdList->DrawInstanced(_vertexNum, instance, _offset, 0);
	}
}

void CMesh::LoadMeshFromFile(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList, const char* fileName, bool bTextFile)
{
	if (bTextFile) {
		std::ifstream in{ fileName };
		std::string strToken;
		std::string str;
		while (in >> str) {
			if (str == "<Bounds>:") {
				in >> _boundingBox.Center.x >> _boundingBox.Center.y >> _boundingBox.Center.z;
				in >> _boundingBox.Extents.x >> _boundingBox.Center.y >> _boundingBox.Extents.z;
			}
			// ... 이하 생략
		}
	}
	else {
		std::ifstream in{ fileName, std::ios::binary };
		BYTE strLength = 0;
		char strToken[64];
		// Bounding box
		in.read((char*)&strLength, sizeof(BYTE));
		in.read((char*)&strToken, sizeof(char) * 14);
		in.read((char*)&_boundingBox.Center, sizeof(float) * 3);
		in.read((char*)&_boundingBox.Extents, sizeof(float) * 3);
		// ... 이하 생략
	}

	_positionBuffer = ::CreateBufferResource(
		device,
		cmdList,
		_positions.data(),
		_positions.size() * sizeof(Vec3),
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		&_positionUploadBuffer
	);
	// ... 이하 생략

	_vertexBufferViewNum = 4;
	_vertexBufferViews.resize(_vertexBufferViewNum);
	_vertexBufferViews[0].BufferLocation = _positionBuffer->GetGPUVirtualAddress();
	_vertexBufferViews[0].StrideInBytes = sizeof(Vec3);
	_vertexBufferViews[0].SizeInBytes = sizeof(Vec3) * _vertexNum;
	// ... 이하 생략

	_indexBuffer = ::CreateBufferResource(
		device,
		cmdList,
		_indices.data(),
		_indexNum * sizeof(UINT),
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_INDEX_BUFFER,
		&_indexUploadBuffer
	);

	_indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
	_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	_indexBufferView.SizeInBytes = _indexNum * sizeof(UINT);
}
