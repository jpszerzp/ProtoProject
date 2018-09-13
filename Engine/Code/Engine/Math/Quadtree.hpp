#pragma once

#include "Engine/Physics/2D/Entity.hpp"

class Quadtree
{
public:
	static const int MAX_ENTITIES = 4;
	static const int MAX_DEPTH = 4;

	int m_depth;
	std::vector<Entity*> m_entities;
	AABB2 m_boundary;
	std::vector<Quadtree*> m_nodes;

public:
	Quadtree();
	Quadtree(int depth, AABB2 bound);
	~Quadtree();

	void ClearTree();
	void SplitTree();
	int  GetQuadrant(Entity* entity);
	void InsertTree(Entity* entity);
	void RetrieveEntity(std::vector<Entity*>& entities, Entity* ent);
};