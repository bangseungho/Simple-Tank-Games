#include "stdafx.h"
#include "Mesh.h"
#include "GraphicsPipeline.h"

CPolygon::CPolygon(int nVertices)
{
	m_nVertices = nVertices;
	m_vVertices.resize(nVertices);
}

void CPolygon::SetVertex(int nIndex, CVertex&& vertex)
{
	if ((0 <= nIndex) && (nIndex < m_nVertices)) {
		m_vVertices[nIndex] = vertex;
	}
}

CMesh::CMesh(int nPolygons)
{
	m_nPolygons = nPolygons;
	m_vpPolygons.resize(nPolygons);
}

void CMesh::SetPolygon(int nIndex, std::unique_ptr<CPolygon> polygon)
{
	if ((0 <= nIndex) && (nIndex < m_nPolygons)) {
		m_vpPolygons[nIndex] = std::move(polygon);
	}
}

void Draw2DLine(HDC hDCFrameBuffer, XMFLOAT3& f3PreviousProject, XMFLOAT3& f3CurrentProject)
{
	XMFLOAT3 f3Previous = CGraphicsPipeline::ScreenTransform(f3PreviousProject);
	XMFLOAT3 f3Current = CGraphicsPipeline::ScreenTransform(f3CurrentProject);
	::MoveToEx(hDCFrameBuffer, (long)f3Previous.x, (long)f3Previous.y, NULL);
	::LineTo(hDCFrameBuffer, (long)f3Current.x, (long)f3Current.y);
}

void CMesh::Render(HDC hDCFrameBuffer)
{
	XMFLOAT3 f3InitialProject, f3PreviousProject;
	bool bPreviousInside = false, bInitialInside = false, bCurrentInside = false, bIntersectInside = false;
	for (auto& polygon : m_vpPolygons) {
		int nVertices = polygon.get()->m_nVertices;
		CVertex* pVertices = polygon.get()->m_vVertices.data();

		f3PreviousProject = f3InitialProject = CGraphicsPipeline::Project(pVertices[0].m_xmf3Position);
		bPreviousInside = bInitialInside = (-1.f <= f3InitialProject.x) && (f3InitialProject.x <= 1.f) && (-1.f <= f3InitialProject.y) && (f3InitialProject.y <= 1.f);
		for (int i = 1; i < nVertices; i++)
		{
			XMFLOAT3 f3CurrentProject = CGraphicsPipeline::Project(pVertices[i].m_xmf3Position);
			bCurrentInside = (-1.f <= f3CurrentProject.x) && (f3CurrentProject.x <= 1.f) && (-1.f <= f3CurrentProject.y) && (f3CurrentProject.y <= 1.f);
			if (((0.f <= f3CurrentProject.z) && (f3CurrentProject.z <= 1.f)) && ((bCurrentInside || bPreviousInside))) ::Draw2DLine(hDCFrameBuffer, f3PreviousProject, f3CurrentProject);
			f3PreviousProject = f3CurrentProject;
			bPreviousInside = bCurrentInside;
		}
		if (((0.f <= f3InitialProject.z) && (f3InitialProject.z <= 1.f)) && ((bInitialInside || bPreviousInside))) ::Draw2DLine(hDCFrameBuffer, f3PreviousProject, f3InitialProject);
	}
}

BOOL CMesh::RayIntersectionByTriangle(XMVECTOR& xmRayOrigin, XMVECTOR& xmRayDirection, XMVECTOR v0, XMVECTOR v1, XMVECTOR v2, float* pfNearHitDistance)
{
	float fHitDistance;
	BOOL bIntersected = TriangleTests::Intersects(xmRayOrigin, xmRayDirection, v0, v1, v2, fHitDistance);
	if (bIntersected && (fHitDistance < *pfNearHitDistance)) *pfNearHitDistance = fHitDistance;

	return bIntersected;
}

int CMesh::CheckRayIntersection(XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection, float* pfNearHitDistance)
{
	int nIntersections = 0;
	bool bIntersected = m_xmOOBB.Intersects(xmvPickRayOrigin, xmvPickRayDirection, *pfNearHitDistance);

	if (bIntersected) {
		for (auto& polygon : m_vpPolygons) {
			switch (polygon.get()->m_nVertices) {
			case 3: {
				XMVECTOR v0 = XMLoadFloat3(&polygon.get()->m_vVertices[0].m_xmf3Position);
				XMVECTOR v1 = XMLoadFloat3(&polygon.get()->m_vVertices[1].m_xmf3Position);
				XMVECTOR v2 = XMLoadFloat3(&polygon.get()->m_vVertices[2].m_xmf3Position);
				bool bIntersected = RayIntersectionByTriangle(xmvPickRayOrigin, xmvPickRayDirection, v0, v1, v2, pfNearHitDistance);
				if (bIntersected) nIntersections++;
				break;
			}
			case 4: {
				XMVECTOR v0 = XMLoadFloat3(&polygon.get()->m_vVertices[0].m_xmf3Position);
				XMVECTOR v1 = XMLoadFloat3(&polygon.get()->m_vVertices[1].m_xmf3Position);
				XMVECTOR v2 = XMLoadFloat3(&polygon.get()->m_vVertices[2].m_xmf3Position);
				bool bIntersected = RayIntersectionByTriangle(xmvPickRayOrigin, xmvPickRayDirection, v0, v1, v2, pfNearHitDistance);
				if (bIntersected) nIntersections++;
				v0 = XMLoadFloat3(&polygon.get()->m_vVertices[0].m_xmf3Position);
				v1 = XMLoadFloat3(&polygon.get()->m_vVertices[2].m_xmf3Position);
				v2 = XMLoadFloat3(&polygon.get()->m_vVertices[3].m_xmf3Position);
				bIntersected = RayIntersectionByTriangle(xmvPickRayOrigin, xmvPickRayDirection, v0, v1, v2, pfNearHitDistance);
				if (bIntersected)
					nIntersections++;
				break;
			}
			}
		}
	}
	return nIntersections;
}

CCubeMesh::CCubeMesh(float fWidth, float fHeight, float fDepth) : CMesh(6)
{
	float fHalfWidth = fWidth * 0.5f;
	float fHalfHeight = fHeight * 0.5f;
	float fHalfDepth = fDepth * 0.5f;

	auto pFrontFace = std::make_unique<CPolygon>(4);
	pFrontFace->SetVertex(0, CVertex{ -fHalfWidth, +fHalfHeight, -fHalfDepth });
	pFrontFace->SetVertex(1, CVertex{ -fHalfWidth, +fHalfHeight, -fHalfDepth });
	pFrontFace->SetVertex(2, CVertex{ -fHalfWidth, +fHalfHeight, -fHalfDepth });
	pFrontFace->SetVertex(3, CVertex{ -fHalfWidth, +fHalfHeight, -fHalfDepth });
	SetPolygon(0, std::move(pFrontFace));

	auto pTopFace = std::make_unique<CPolygon>(4);
	pTopFace->SetVertex(0, CVertex{ -fHalfWidth, +fHalfHeight, +fHalfDepth });
	pTopFace->SetVertex(1, CVertex{ +fHalfWidth, +fHalfHeight, +fHalfDepth });
	pTopFace->SetVertex(2, CVertex{ +fHalfWidth, +fHalfHeight, -fHalfDepth });
	pTopFace->SetVertex(3, CVertex{ -fHalfWidth, +fHalfHeight, -fHalfDepth });
	SetPolygon(1, std::move(pTopFace));

	auto pBackFace = std::make_unique<CPolygon>(4);
	pBackFace->SetVertex(0, CVertex{ -fHalfWidth, -fHalfHeight, +fHalfDepth });
	pBackFace->SetVertex(1, CVertex{ +fHalfWidth, -fHalfHeight, +fHalfDepth });
	pBackFace->SetVertex(2, CVertex{ +fHalfWidth, +fHalfHeight, +fHalfDepth });
	pBackFace->SetVertex(3, CVertex{ -fHalfWidth, +fHalfHeight, +fHalfDepth });
	SetPolygon(2, std::move(pBackFace));

	auto pBottomFace = std::make_unique<CPolygon>(4);
	pBottomFace->SetVertex(0, CVertex{ -fHalfWidth, -fHalfHeight, -fHalfDepth });
	pBottomFace->SetVertex(1, CVertex{ +fHalfWidth, -fHalfHeight, -fHalfDepth });
	pBottomFace->SetVertex(2, CVertex{ +fHalfWidth, -fHalfHeight, +fHalfDepth });
	pBottomFace->SetVertex(3, CVertex{ -fHalfWidth, -fHalfHeight, +fHalfDepth });
	SetPolygon(3, std::move(pBottomFace));

	auto pLeftFace = std::make_unique<CPolygon>(4);
	pLeftFace->SetVertex(0, CVertex{ -fHalfWidth, +fHalfHeight, +fHalfDepth });
	pLeftFace->SetVertex(1, CVertex{ -fHalfWidth, +fHalfHeight, -fHalfDepth });
	pLeftFace->SetVertex(2, CVertex{ -fHalfWidth, -fHalfHeight, -fHalfDepth });
	pLeftFace->SetVertex(3, CVertex{ -fHalfWidth, -fHalfHeight, +fHalfDepth });
	SetPolygon(4, std::move(pLeftFace));

	auto pRightFace = std::make_unique<CPolygon>(4);
	pRightFace->SetVertex(0, CVertex{ +fHalfWidth, +fHalfHeight, -fHalfDepth });
	pRightFace->SetVertex(1, CVertex{ +fHalfWidth, +fHalfHeight, +fHalfDepth });
	pRightFace->SetVertex(2, CVertex{ +fHalfWidth, -fHalfHeight, +fHalfDepth });
	pRightFace->SetVertex(3, CVertex{ +fHalfWidth, -fHalfHeight, -fHalfDepth });
	SetPolygon(5, std::move(pRightFace));

	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fHalfWidth, fHalfHeight, fHalfDepth), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

CFloorMesh::CFloorMesh(float fWidth, float fDepth, int nSubRects) : CMesh(nSubRects* nSubRects)
{
	float fHalfWidth = fWidth * 0.5f;
	float fHalfDepth = fDepth * 0.5f;
	float fCellWidth = fWidth * (1.f / nSubRects);
	float fCellDepth = fDepth * (1.f / nSubRects);

	int idx = 0;

	for (int i = 0; i < nSubRects; ++i) {
		for (int j = 0; j < nSubRects; ++j) {
			auto pFace = std::make_unique<CPolygon>(4);
			pFace->SetVertex(0, CVertex{ -fHalfWidth + (i * fCellWidth), 0.0f, -fHalfDepth + (j * fCellDepth) });
			pFace->SetVertex(1, CVertex{ -fHalfWidth + ((i + 1) * fCellWidth), 0.0f, -fHalfDepth + (j * fCellDepth) });
			pFace->SetVertex(2, CVertex{ -fHalfWidth + ((i + 1) * fCellWidth), 0.0f, -fHalfDepth + ((j + 1) * fCellDepth) });
			pFace->SetVertex(3, CVertex{ -fHalfWidth + (i * fCellWidth), 0.0f, -fHalfDepth + ((j + 1) * fCellDepth) });
			SetPolygon(idx++, std::move(pFace));
		}
	}
}

CWallMesh::CWallMesh(float fWidth, float fDepth, int nSubRects) : CMesh(nSubRects* nSubRects)
{
	float fHalfWidth = fWidth * 0.5f;
	float fHalfDepth = fDepth * 0.5f;
	float fCellWidth = fWidth * (1.f / nSubRects);
	float fCellDepth = fDepth * (1.f / nSubRects);

	int idx = 0;

	for (int i = 0; i < nSubRects; ++i) {
		for (int j = 0; j < nSubRects; ++j) {
			auto pFace = std::make_unique<CPolygon>(4);
			pFace->SetVertex(0, CVertex{ -fHalfWidth + (i * fCellWidth), 0.0f, -fHalfDepth + (j * fCellDepth) });
			pFace->SetVertex(1, CVertex{ -fHalfWidth + ((i + 1) * fCellWidth), 0.0f, -fHalfDepth + (j * fCellDepth) });
			pFace->SetVertex(2, CVertex{ -fHalfWidth + ((i + 1) * fCellWidth), 0.0f, -fHalfDepth + ((j + 1) * fCellDepth) });
			pFace->SetVertex(3, CVertex{ -fHalfWidth + (i * fCellWidth), 0.0f, -fHalfDepth + ((j + 1) * fCellDepth) });
			SetPolygon(idx++, std::move(pFace));
		}
	}
	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fHalfWidth, 0.f, fHalfDepth), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

CTransParentMesh::CTransParentMesh(float fWidth, float fHeight, float fDepth)
{
	float fHalfWidth = fWidth * 0.5f;
	float fHalfHeight = fHeight * 0.5f;
	float fHalfDepth = fDepth * 0.5f;

	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fHalfWidth, fHalfHeight, fHalfDepth), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}
