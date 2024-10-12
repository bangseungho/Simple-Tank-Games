#include "stdafx.h"
#include "OctreeNode.h"

COctreeNode::COctreeNode()
{
	_center = Vec3(1.0f, 1.0f, 1.0f);
	_halfWidth = 0.f;
}

COctreeNode::~COctreeNode()
{
}

void COctreeNode::AddChildNode(std::shared_ptr<COctreeNode> child)
{
	_childs.push_back(child);

	if(child)
		child->SetParent(shared_from_this());
}

void COctreeNode::AddObject(std::shared_ptr<CGameObject> object)
{
	_objects.push_back(object);
}

void COctreeNode::SetParent(std::shared_ptr<COctreeNode> parent)
{
	_parent = parent;
}

void COctreeNode::SetCenter(Vec3 center)
{
	_center = center;
	_boundingBox.Center = _center;
}

void COctreeNode::SetRadius(float halfWidth)
{
	_halfWidth = halfWidth;
	_boundingBox.Extents = Vec3(halfWidth, halfWidth, halfWidth);
}

void COctreeNode::ReleaseNode()
{
}

Vec3 COctreeNode::GetCenter() const
{
	return _center;
}

COctreeNode* COctreeNode::GetChildNode(size_t idx) const
{
	return _childs[idx].get();
}

COctreeNode* COctreeNode::GetParentNode() const
{
	return _parent.get();
}

BoundingBox COctreeNode::GetBoundingBox() const
{
	return _boundingBox;
}
