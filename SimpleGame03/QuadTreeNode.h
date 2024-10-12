#pragma once
#include "Enemy.h"

struct Point {
    Point(float x, float z) : _x(x), _z(z) {}

    float _x;
    float _z;
};

class CQuadTreeNode{
public:
    CQuadTreeNode(Point _position, float _size) : position(_position), size(_size) { }
    ~CQuadTreeNode() { }

public:
    Point position;
    float size;
    std::vector<std::shared_ptr<CQuadTreeNode>> mChildren;
    std::vector<std::shared_ptr<CEnemy>>        mObjects;
};

class CQuadTree {
public:
    CQuadTree(float width, float height) : root(nullptr), worldWidth(width), worldHeight(height) {}
    ~CQuadTree() { }

public:
    void Insert(std::shared_ptr<CEnemy> object) {
        if (!root)
            root = std::make_shared<CQuadTreeNode>(Point(worldWidth / 2.0f, worldHeight / 2.0f), max(worldWidth, worldHeight));

        InsertRecursive(root, object);
    }

    void InsertRecursive(std::shared_ptr<CQuadTreeNode> node, std::shared_ptr<CEnemy> object) {
        // 현재 노드의 사이즈가 1 이하이면 종료

        if (node->size <= 100.0f) {
            node->mObjects.push_back(object);
            return;
        }

        if (node->position._x - node->size / 2 > object->GetPosition().x || node->position._x + node->size / 2 < object->GetPosition().x ||
            node->position._z - node->size / 2 > object->GetPosition().z || node->position._z + node->size / 2 < object->GetPosition().z)
            return;

        // 현재 노드에 자식 노드가 없으면 생성
        if (node->mChildren.empty()) {
            float childSize = node->size / 2.0f;
            float childX = node->position._x - childSize / 2.0f;
            float childY = node->position._z - childSize / 2.0f;

            node->mChildren.emplace_back(std::make_shared<CQuadTreeNode>(Point(childX, childY), childSize));
            node->mChildren.emplace_back(std::make_shared<CQuadTreeNode>(Point(childX + childSize, childY), childSize));
            node->mChildren.emplace_back(std::make_shared<CQuadTreeNode>(Point(childX, childY + childSize), childSize));
            node->mChildren.emplace_back(std::make_shared<CQuadTreeNode>(Point(childX + childSize, childY + childSize), childSize));
        }

        // 자식 노드에 포인트 삽입
        for (auto& child : node->mChildren) {
            InsertRecursive(child, object);
        }
    }

    std::vector<std::shared_ptr<CEnemy>> SearchQuadTree(float x, float z) {
        return SearchObject(x, z, root);
    }

    std::vector<std::shared_ptr<CEnemy>> SearchObject(float x, float z, std::shared_ptr<CQuadTreeNode> node) {
        std::vector<std::shared_ptr<CEnemy>> objects;
        // ... 종료 로직 생략 
        // 자식 노드에 대해 재귀적으로 호출하여 오브젝트들을 가져옴
        for (auto& child : node->mChildren) {
            std::vector<std::shared_ptr<CEnemy>> childObjects = SearchObject(x, z, child);
            objects.insert(objects.end(), childObjects.begin(), childObjects.end());
        }

        return objects;
    }

private:
    std::shared_ptr<CQuadTreeNode> root;
    float worldWidth;
    float worldHeight;
};