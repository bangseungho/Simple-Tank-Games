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

	if (_vertexUploadBuffer)
		_vertexUploadBuffer->Release();
	_vertexUploadBuffer = nullptr;
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
			if (str == "<Vertices>:") {
				in >> _vertexNum;
				_positions.resize(_vertexNum);
				for (auto& position : _positions)
					in >> position.x >> position.y >> position.z;
			}
			else if (str == "<Normals>:") {
				in >> strToken;
				_normals.resize(_vertexNum);
				for (auto& normal : _normals)
					in >> normal.x >> normal.y >> normal.z;
			}
			else if (str == "<TextureCoords>:") {
				in >> strToken;
				_textureCoords.resize(_vertexNum);
				for (auto& textureCoord : _textureCoords)
					in >> textureCoord.x >> textureCoord.y;
			}
			else if (str == "<Indices>:") {
				in >> _indexNum;
				_indices.resize(_indexNum);
				for (auto& indice : _indices)
					in >> indice;
			}
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
		// Vertices
		in.read((char*)&strLength, sizeof(BYTE));
		in.read((char*)&strToken, sizeof(char) * 11);
		in.read((char*)&_vertexNum, sizeof(int) * 1);
		_positions.resize(_vertexNum);
		in.read((char*)_positions.data(), sizeof(float) * 3 * _vertexNum);
		// Normals
		in.read((char*)&strLength, sizeof(BYTE));
		in.read((char*)&strToken, sizeof(char) * 10);
		in.read((char*)&_vertexNum, sizeof(int) * 1);
		_normals.resize(_vertexNum);
		in.read((char*)_normals.data(), sizeof(float) * 3 * _vertexNum);
		// Normals
		in.read((char*)&strLength, sizeof(BYTE));
		in.read((char*)&strToken, sizeof(char) * 16);
		in.read((char*)&_vertexNum, sizeof(int) * 1);
		_textureCoords.resize(_vertexNum);
		in.read((char*)_textureCoords.data(), sizeof(float) * 2 * _vertexNum);
		// Normals
		in.read((char*)&strLength, sizeof(BYTE));
		in.read((char*)&strToken, sizeof(char) * 10);
		in.read((char*)&_indexNum, sizeof(int) * 1);
		_indices.resize(_indexNum);
		in.read((char*)_indices.data(), sizeof(UINT) * _indexNum);
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

	_normalBuffer = ::CreateBufferResource(
		device,
		cmdList,
		_normals.data(),
		_normals.size() * sizeof(Vec3),
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		&_normalUploadBuffer
	);

	_textureCoordBuffer = ::CreateBufferResource(
		device,
		cmdList,
		_textureCoords.data(),
		_textureCoords.size() * sizeof(Vec2),
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		&_textureCoordUploadBuffer
	);

	_vertexBufferViewNum = 4;
	_vertexBufferViews.resize(_vertexBufferViewNum);
	
	_vertexBufferViews[0].BufferLocation = _positionBuffer->GetGPUVirtualAddress();
	_vertexBufferViews[0].StrideInBytes = sizeof(Vec3);
	_vertexBufferViews[0].SizeInBytes = sizeof(Vec3) * _vertexNum;

	_vertexBufferViews[1].BufferLocation = _normalBuffer->GetGPUVirtualAddress();
	_vertexBufferViews[1].StrideInBytes = sizeof(Vec3);
	_vertexBufferViews[1].SizeInBytes = sizeof(Vec3) * _vertexNum;

	_vertexBufferViews[2].BufferLocation = _textureCoordBuffer->GetGPUVirtualAddress();
	_vertexBufferViews[2].StrideInBytes = sizeof(Vec2);
	_vertexBufferViews[2].SizeInBytes = sizeof(Vec2) * _vertexNum;

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


// CHeightMapImage
CHeightMapImage::CHeightMapImage(LPCTSTR fileName, int nWidth, int nLength, Vec3 xmf3Scale)
{
	_nWidth = nWidth;
	_nLength = nLength;
	_xmf3Scale = xmf3Scale;

	std::vector<BYTE> heightMapPixels(_nWidth * _nLength);
	HANDLE hFile = ::CreateFile(fileName, GENERIC_READ, 0, nullptr, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, NULL);
	DWORD dwBytesRead;
	::ReadFile(hFile, heightMapPixels.data(), (_nWidth * _nLength), &dwBytesRead, NULL);
	::CloseHandle(hFile);

	_heightMapPixels.resize(_nWidth * _nLength);
	for (int y = 0; y < _nLength; ++y) {
		for (int x = 0; x < _nWidth; ++x) {
			_heightMapPixels[x + ((_nLength - 1 - y) * _nWidth)] = (BYTE)heightMapPixels[x + (y * _nWidth)];
		}
	}
}

CHeightMapImage::~CHeightMapImage()
{
}

#define _WITH_APPROXIMATE_OPPOSITE_CORNER
float CHeightMapImage::GetHeight(float fx, float fz)
{
	if ((fx < 0.f) || (fz < 0.f) || (fx >= _nWidth) || (fz >= _nLength))
		return 0.f;

	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;

	float fBottomLeft = (float)_heightMapPixels[x + (z * _nWidth)];
	float fBottomRight = (float)_heightMapPixels[(x + 1) + (z * _nWidth)];
	float fTopLeft = (float)_heightMapPixels[x + ((z + 1) * _nWidth)];
	float fTopRight = (float)_heightMapPixels[(x + 1) + ((z + 1) * _nWidth)];

#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
	bool bRightToLeft = ((z % 2) != 0);
	if (bRightToLeft) {
		if (fzPercent >= fxPercent)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else {
		if (fzPercent < (1.f - fxPercent))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}
#endif // #define _WITH_APPROXIMATE_OPPOSITE_CORNER

	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	return fHeight;
}

Vec3 CHeightMapImage::GetHeightMapNormal(int fx, int fz)
{
	if ((fx < 0.f) || (fz < 0.f) || (fx >= _nWidth) || (fz >= _nLength))
		return Vec3(0.f, 1.f, 0.f);

	int nHeightMapIndex = fx + (fz * _nWidth);
	int xHeightMapAdd = (fx < _nWidth - 1) ? 1 : -1;
	int zHeightMapAdd = (fz < _nLength - 1) ? _nWidth : -_nWidth;

	float y1 = (float)_heightMapPixels[nHeightMapIndex] * _xmf3Scale.y;
	float y2 = (float)_heightMapPixels[nHeightMapIndex + xHeightMapAdd] * _xmf3Scale.y;
	float y3 = (float)_heightMapPixels[nHeightMapIndex + zHeightMapAdd] * _xmf3Scale.y;

	Vec3 xmf3Edge1 = Vec3(0.0f, y3 - y1, _xmf3Scale.z);
	Vec3 xmf3Edge2 = Vec3(_xmf3Scale.x, y2 - y1, 0.0f);
	Vec3 xmf3Normal = Vector3::CrossProduct(xmf3Edge1, xmf3Edge2, true);

	return xmf3Normal;
}

CHeightMapGridMesh::CHeightMapGridMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList, int xStart, int zStart, int nWidth, int nLength, Vec3 scale, Vec4 color, void* pContext) : CMesh(device, cmdList)
{
	_vertexNum = nWidth * nLength;
	_stride = sizeof(CDiffusedVertex);
	_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	_nWidth = nWidth;
	_nLength = nLength;
	_xmf3Scale = scale;

	// Verices
	std::vector<CDiffusedVertex> pVertices(_vertexNum);

	float fHeight = 0.0f, fMinHeight = +FLT_MAX, fMaxHeight = -FLT_MAX;
	for (int i = 0, z = zStart; z < (zStart + nLength); z++)
	{
		for (int x = xStart; x < (xStart + nWidth); x++, i++)
		{
			//정점의 높이와 색상을 높이 맵으로부터 구한다. 
			Vec3 xmf3Position = Vec3((x * _xmf3Scale.x), OnGetHeight(x, z, pContext), (z * _xmf3Scale.z));
			Vec4 xmf3Color = Vector4::Add(OnGetColor(x, z, pContext), color);
			pVertices[i] = CDiffusedVertex(xmf3Position, xmf3Color);
			if (fHeight < fMinHeight) fMinHeight = fHeight;
			if (fHeight > fMaxHeight) fMaxHeight = fHeight;
		}
	}

	_vertexBuffer = ::CreateBufferResource(
		device,
		cmdList,
		pVertices.data(),
		_stride * _vertexNum,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		&_vertexUploadBuffer
	);

	_vertexBufferView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
	_vertexBufferView.StrideInBytes = _stride;
	_vertexBufferView.SizeInBytes = _stride * _vertexNum;

	// Indices
	_indexNum = ((nWidth * 2) * (nLength - 1)) + ((nLength - 1) - 1);
	std::vector<UINT> pIndices(_indexNum);

	for (int j = 0, z = 0; z < nLength - 1; z++)
	{
		if ((z % 2) == 0)
		{
			for (int x = 0; x < nWidth; x++)
			{
				if ((x == 0) && (z > 0)) pIndices[j++] = (UINT)(x + (z * nWidth));
				pIndices[j++] = (UINT)(x + (z * nWidth));
				pIndices[j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
		else
		{
			for (int x = nWidth - 1; x >= 0; x--)
			{
				if (x == (nWidth - 1)) pIndices[j++] = (UINT)(x + (z * nWidth));
				pIndices[j++] = (UINT)(x + (z * nWidth));
				pIndices[j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
	}

	_indexBuffer = ::CreateBufferResource(
		device,
		cmdList,
		pIndices.data(),
		_indexNum * sizeof(UINT),
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_INDEX_BUFFER,
		&_indexUploadBuffer
	);

	_indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
	_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	_indexBufferView.SizeInBytes = _indexNum * sizeof(UINT);
}

float CHeightMapGridMesh::OnGetHeight(int x, int z, void* pContext)
{
	CHeightMapImage* pHeightMapImage = (CHeightMapImage*)pContext;

	BYTE* pHeightMapPixels = pHeightMapImage->GetHeightMapPixels().data();
	Vec3 xmf3Scale = pHeightMapImage->GetScale();

	int nWidth = pHeightMapImage->GetHeightMapWidth();
	float fHeight = pHeightMapPixels[x + (z * nWidth)] * xmf3Scale.y;

	return fHeight;
}

Vec4 CHeightMapGridMesh::OnGetColor(int x, int z, void* pContext)
{
	// 조명의 방향 벡터
	Vec3 xmf3LightDirection = Vec3(-1.0f, 1.0f, 1.0f);
	xmf3LightDirection = Vector3::Normalize(xmf3LightDirection);
	CHeightMapImage* pHeightMapImage = (CHeightMapImage*)pContext;
	Vec3 xmf3Scale = pHeightMapImage->GetScale();

	// 조명의 색상
	Vec4 xmf4IncidentLightColor(0.9f, 0.8f, 0.4f, 1.0f);

	// 주변 인접한 3개의 정점의 법선 벡터와 조명 방향 벡터를 내적을 평균하여 구한다.
	float fScale = Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x, z), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x + 1, z), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x + 1, z + 1), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x, z + 1), xmf3LightDirection);
	fScale = (fScale / 4.0f) + 0.05f;

	if (fScale > 1.0f) fScale = 1.0f;
	if (fScale < 0.25f) fScale = 0.25f;

	Vec4 xmf4Color = Vector4::Multiply(fScale, xmf4IncidentLightColor);

	return xmf4Color;
}

void CHeightMapGridMesh::Render(ComPtr<ID3D12GraphicsCommandList> cmdList)
{
	cmdList->IASetVertexBuffers(_slot, 1, &_vertexBufferView);
	CMesh::Render(cmdList, 1);
}
