#pragma once

#define CHILD_NODE_COUNT 8

#include "GameObject.h"

class COctreeNode : public std::enable_shared_from_this<COctreeNode>{
public:
	COctreeNode();
	~COctreeNode();

public:
	void AddChildNode(std::shared_ptr<COctreeNode> child);
	void AddObject(std::shared_ptr<CGameObject> object);
	
	void SetParent(std::shared_ptr<COctreeNode> parent);
	void SetCenter(Vec3 center);
	void SetRadius(float halfWidth);

	void ReleaseNode();

	int GetChildCount() const { return _childs.size(); }
	Vec3 GetCenter() const;
	COctreeNode* GetChildNode(size_t idx) const;
	COctreeNode* GetParentNode() const;
	BoundingBox GetBoundingBox() const;


private:
	BoundingBox _boundingBox;
	std::vector<std::shared_ptr<COctreeNode>> _childs;
	std::list<std::shared_ptr<CGameObject>> _objects;
	std::shared_ptr<COctreeNode> _parent;

	Vec3 _center;
	float _halfWidth;
};

