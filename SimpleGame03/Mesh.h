#pragma once

class CVertex {
public:
	CVertex() { _position = Vec3{ 0.f, 0.f, 0.f }; }
	CVertex(Vec3 position) { _position = position; }

	~CVertex() { }

protected:
	Vec3 _position;
};

class CDiffusedVertex : public CVertex {
public:
	CDiffusedVertex() { _position = Vec3{ 0.f, 0.f, 0.f }; _diffuse = Vec4{ 0.f, 0.f, 0.f, 0.f }; }
	CDiffusedVertex(float x, float y, float z, Vec4 diffuse) { _position = Vec3{ x, y, z }; _diffuse = diffuse; }
	CDiffusedVertex(Vec3 position, Vec4 diffuse) { _position = position; _diffuse = diffuse; }

	~CDiffusedVertex() { }

protected:
	Vec4 _diffuse;
};

class CMesh {
public:
	CMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList);
	CMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList, const char* fileName, bool bTextFile);
	virtual ~CMesh();

public:
	void ReleaseUploadBuffers();

	virtual void Render(ComPtr<ID3D12GraphicsCommandList> cmdList);
	virtual void Render(ComPtr<ID3D12GraphicsCommandList> cmdList, UINT instance);
	virtual void Render(ComPtr<ID3D12GraphicsCommandList> cmdList, UINT instance, D3D12_VERTEX_BUFFER_VIEW instancingBufferView);

	void LoadMeshFromFile(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList, const char* fileName, bool bTextFile);
	std::vector<Vec3>& GetPosition() { return _positions; }
	std::vector<Vec3>& GetNormal() { return _normals; }

public:
	BoundingBox _boundingBox;

protected:
	std::vector<Vec3> _positions;
	ComPtr<ID3D12Resource> _positionBuffer;
	ID3D12Resource* _positionUploadBuffer = nullptr;
	
	std::vector<Vec3> _normals;
	ComPtr<ID3D12Resource> _normalBuffer;
	ID3D12Resource* _normalUploadBuffer = nullptr;

	std::vector<Vec2> _textureCoords;
	ComPtr<ID3D12Resource> _textureCoordBuffer;
	ID3D12Resource* _textureCoordUploadBuffer = nullptr;

	UINT _vertexBufferViewNum = 0;
	std::vector<D3D12_VERTEX_BUFFER_VIEW> _vertexBufferViews;

	UINT _slot = 0;
	UINT _vertexNum = 0;
	UINT _stride = 0;
	UINT _offset = 0;

	D3D12_PRIMITIVE_TOPOLOGY _primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
protected:
	ComPtr<ID3D12Resource> _vertexBuffer;
	ID3D12Resource* _vertexUploadBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;

	std::vector<UINT> _indices;
	ComPtr<ID3D12Resource> _indexBuffer;
	ID3D12Resource* _indexUploadBuffer = nullptr;
	D3D12_INDEX_BUFFER_VIEW _indexBufferView;

	UINT _indexNum = 0;
	UINT _startIndex = 0;
	int _baseVertex = 0;
};

class CHeightMapImage
{
public:
	CHeightMapImage(LPCTSTR fileName, int nWidth, int nLength, Vec3 xmf3Scale);
	~CHeightMapImage();

public:
	float GetHeight(float fx, float fz);
	Vec3 GetHeightMapNormal(int fx, int fz);
	Vec3 GetScale() { return _xmf3Scale; }
	std::vector<BYTE>& GetHeightMapPixels() { return _heightMapPixels; }
	int GetHeightMapWidth() { return _nWidth; }
	int GetHeightMapLength() { return _nLength; }

private:
	std::vector<BYTE> _heightMapPixels;
	int _nWidth;
	int _nLength;
	Vec3 _xmf3Scale;
};

class CHeightMapGridMesh : public CMesh
{
public:
	CHeightMapGridMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList, int xStart, int zStart, int nWidth, int nLength,
		Vec3 scale = Vec3(1.f, 1.f, 1.f), Vec4 color = Vec4(1.f, 1.f, 0.f, 0.f), void* pContext = nullptr);
	virtual ~CHeightMapGridMesh() { }

public:
	Vec3 GetScale() const { return _xmf3Scale; }
	int GetWidth() const { return _nWidth; }
	int GetLength() const { return _nLength; }

	virtual float OnGetHeight(int x, int z, void* pContext);
	virtual Vec4 OnGetColor(int x, int z, void* pContext);

	virtual void Render(ComPtr<ID3D12GraphicsCommandList> cmdList);

protected:
	int _nWidth;
	int _nLength;
	Vec3 _xmf3Scale;
};
