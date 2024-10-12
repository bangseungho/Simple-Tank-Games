#pragma once

class CVertex
{
public:
	CVertex() { m_xmf3Position = XMFLOAT3(0.f, 0.f, 0.f); }
	CVertex(float x, float y, float z) { m_xmf3Position = XMFLOAT3(x, y, z); }
	~CVertex() { }

public:
	XMFLOAT3 m_xmf3Position = XMFLOAT3(0.f, 0.f, 0.f);
};

class CPolygon
{
public:
	CPolygon() { }
	CPolygon(int nVertices);
	~CPolygon() { };

public:
	void SetVertex(int nIndex, CVertex&& vertex);

public:
	int m_nVertices = 0;
	std::vector<CVertex> m_vVertices;
};

class CMesh
{
public:
	CMesh() { }
	CMesh(int nPolygons);
	virtual ~CMesh() { };

public:
	void SetPolygon(int nIndex, std::unique_ptr<CPolygon> polygon);

public:
	virtual void Render(HDC hDCFrameBuffer);
	BOOL RayIntersectionByTriangle(XMVECTOR& xmRayOrigin, XMVECTOR& xmRayDirection, XMVECTOR v0, XMVECTOR v1, XMVECTOR v2, float* pfNearHitDistance);
	int CheckRayIntersection(XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection, float* pfNearHitDistance);

public:
	BoundingOrientedBox	m_xmOOBB = BoundingOrientedBox();

protected:
	int	m_nPolygons = 0;
	std::vector<std::unique_ptr<CPolygon>> m_vpPolygons;

};

class CCubeMesh : public CMesh
{
public:
	CCubeMesh(float fWidth = 4.f, float fHeight = 4.f, float fDepth = 4.f);
	virtual ~CCubeMesh() { }
};

class CFloorMesh : public CMesh
{
public:
	CFloorMesh(float fWidth = 4.f, float fDepth = 4.f, int nSubRects = 20);
	virtual ~CFloorMesh() { }
};

class CWallMesh : public CMesh
{
public:
	CWallMesh(float fWidth = 4.f, float fDepth = 4.f, int nSubRects = 20);
	virtual ~CWallMesh() { }
};

class CTransParentMesh : public CMesh
{
public:
	CTransParentMesh(float fWidth = 4.f, float fHeight = 4.f, float fDepth = 4.f);
	virtual ~CTransParentMesh() { }
};