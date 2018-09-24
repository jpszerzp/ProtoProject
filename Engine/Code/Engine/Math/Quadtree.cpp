#include "Engine/Math/Quadtree.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Physics/3D/Entity3.hpp"

Quadtree::Quadtree()
{

}

Quadtree::Quadtree(int depth, AABB2 bound)
{
	m_depth = depth;
	m_boundary = bound;
}

Quadtree::~Quadtree()
{
	ClearTree();
}

void Quadtree::ClearTree()
{
	m_entities.clear();

	if (!m_nodes.empty())
	{
		for each (Quadtree* node in m_nodes)
		{
			delete node;
			node = nullptr;
		}

		m_nodes.clear();
	}
}

void Quadtree::SplitTree()
{
	float halfWidth = m_boundary.GetDimensions().x / 2.f;
	float halfHeight = m_boundary.GetDimensions().y / 2.f;
	Vector2 parentBoundMin = m_boundary.mins;
	Vector2 parentBoundMax = m_boundary.maxs;

	Vector2 childBoundMin1 = parentBoundMin + Vector2(halfWidth, halfHeight);
	Vector2 childBoundMax1 = parentBoundMax;
	Vector2 childBoundMin2 = parentBoundMin + Vector2(0.f, halfHeight);
	Vector2 childBoundMax2 = childBoundMin2 + Vector2(halfWidth, halfHeight);
	Vector2 childBoundMin3 = parentBoundMin;
	Vector2 childBoundMax3 = childBoundMin3 + Vector2(halfWidth, halfHeight);
	Vector2 childBoundMin4 = parentBoundMin + Vector2(halfWidth, 0.f);
	Vector2 childBoundMax4 = childBoundMin4 + Vector2(halfWidth, halfHeight);
	Quadtree* firstQuadrant = new Quadtree(m_depth + 1, AABB2(childBoundMin1, childBoundMax1));
	Quadtree* secondQuadrant = new Quadtree(m_depth + 1, AABB2(childBoundMin2, childBoundMax2));
	Quadtree* thirdQuadrant = new Quadtree(m_depth + 1, AABB2(childBoundMin3, childBoundMax3));
	Quadtree* fourthQuadrant = new Quadtree(m_depth + 1, AABB2(childBoundMin4, childBoundMax4));
	m_nodes.push_back(firstQuadrant);
	m_nodes.push_back(secondQuadrant);
	m_nodes.push_back(thirdQuadrant);
	m_nodes.push_back(fourthQuadrant);
}

int Quadtree::GetQuadrant(Entity* entity)
{
	AABB2 entityBound = entity->m_boundAABB; 

	int quadrant = -1;
	float halfWidth = m_boundary.GetDimensions().x / 2.f;
	float halfHeight = m_boundary.GetDimensions().y / 2.f;
	float midPointX = m_boundary.mins.x + halfWidth;
	float midPointY = m_boundary.mins.y + halfHeight;

	// top or bottom (or in between)
	bool top = entityBound.mins.y > midPointY;
	bool bottom = entityBound.maxs.y < midPointY;

	// left or right (or in between)
	bool left = entityBound.maxs.x < midPointX;
	bool right = entityBound.mins.x > midPointX;

	if (left)
	{
		if (top)
		{
			quadrant = 1;
		}
		else if (bottom)
		{
			quadrant = 2;
		}
	}
	else if (right)
	{
		if (top)
		{
			quadrant = 0;
		}
		else if (bottom)
		{
			quadrant = 3;
		}
	}
	return quadrant;
}

void Quadtree::InsertTree(Entity* entity)
{
	if (!m_nodes.empty())
	{
		int quadIndex = GetQuadrant(entity);

		if (quadIndex != -1)
		{
			Quadtree* tree = m_nodes[quadIndex];
			tree->InsertTree(entity);

			return;
		}
	}

	m_entities.push_back(entity);
	if (m_entities.size() > MAX_ENTITIES && m_depth < MAX_DEPTH)
	{
		// make sure this is a child node
		if (m_nodes.empty())
		{
			SplitTree();
		}

		for (int entIdx = (int)(m_entities.size()- 1U); entIdx >= 0; --entIdx)
		{
			Entity* ent = m_entities[entIdx];
			int quadIndex = GetQuadrant(ent);
			if (quadIndex != -1)
			{
				Quadtree* tree = m_nodes[quadIndex];

				// remove in old, insert in new
				m_entities[entIdx] = m_entities[m_entities.size() - 1U];
				m_entities.pop_back();
				tree->InsertTree(ent);
			}
			// else the entity does not COMPLETELY fall in any of the quadrants, hence remain in current level
		}
	}
}

void Quadtree::RetrieveEntity(std::vector<Entity*>& entities, Entity* ent)
{
	int quadIndex = GetQuadrant(ent);
	if (quadIndex != -1 && !m_nodes.empty())
	{
		Quadtree* tree = m_nodes[quadIndex];
		tree->RetrieveEntity(entities, ent);
	}

	// could include ent itself, need to prune when processing pairs
	for each (Entity* myEnt in m_entities)
	{
		entities.push_back(myEnt);
	}
}

Quadtree3::Quadtree3()
{

}

Quadtree3::Quadtree3(int depth, AABB3 bound)
{
	m_depth = depth;
	m_boundary = bound;
}

Quadtree3::~Quadtree3()
{
	ClearTree();
}

void Quadtree3::ClearTree()
{
	m_entities.clear();

	if (!m_nodes.empty())
	{
		for each (Quadtree3* node in m_nodes)
		{
			delete node;
			node = nullptr;
		}

		m_nodes.clear();
	}
}

void Quadtree3::SplitTree()
{
	float halfWidth = m_boundary.GetDimensions().x / 2.f;
	float halfHeight = m_boundary.GetDimensions().y / 2.f;
	float depth = m_boundary.GetDimensions().z;
	Vector3 parentBoundMin = m_boundary.m_min;
	Vector3 parentBoundMax = m_boundary.m_max;

	Vector3 childBoundMin1 = parentBoundMin + Vector3(halfWidth, halfHeight, 0.f);
	Vector3 childBoundMax1 = parentBoundMax;
	Vector3 childBoundMin2 = parentBoundMin + Vector3(0.f, halfHeight, 0.f);
	Vector3 childBoundMax2 = childBoundMin2 + Vector3(halfWidth, halfHeight, depth);
	Vector3 childBoundMin3 = parentBoundMin;
	Vector3 childBoundMax3 = childBoundMin3 + Vector3(halfWidth, halfHeight, depth);
	Vector3 childBoundMin4 = parentBoundMin + Vector3(halfWidth, 0.f, 0.f);
	Vector3 childBoundMax4 = childBoundMin4 + Vector3(halfWidth, halfHeight, depth);
	Quadtree3* firstQuadrant = new Quadtree3(m_depth + 1, AABB3(childBoundMin1, childBoundMax1));
	Quadtree3* secondQuadrant = new Quadtree3(m_depth + 1, AABB3(childBoundMin2, childBoundMax2));
	Quadtree3* thirdQuadrant = new Quadtree3(m_depth + 1, AABB3(childBoundMin3, childBoundMax3));
	Quadtree3* fourthQuadrant = new Quadtree3(m_depth + 1, AABB3(childBoundMin4, childBoundMax4));
	m_nodes.push_back(firstQuadrant);
	m_nodes.push_back(secondQuadrant);
	m_nodes.push_back(thirdQuadrant);
	m_nodes.push_back(fourthQuadrant);
}

int Quadtree3::GetQuadrant(Entity3* ent)
{
	// sphere BV 
	const BoundingSphere& bv = ent->GetBoundingSphere();
	Sphere3 entityBound = Sphere3(bv.GetCenter(), bv.GetRadius());

	int quadrant = -1;
	float halfWidth = m_boundary.GetDimensions().x / 2.f;
	float halfHeight = m_boundary.GetDimensions().y / 2.f;
	float midPointX = m_boundary.m_min.x + halfWidth;
	float midPointY = m_boundary.m_min.y + halfHeight;

	// top or bottom (or in between)
	bool top = entityBound.GetCenter().y - entityBound.GetRadius() > midPointY;
	bool bottom = entityBound.GetCenter().y + entityBound.GetRadius() < midPointY;

	// left or right (or in between)
	bool left = entityBound.GetCenter().x + entityBound.GetRadius() < midPointX;
	bool right = entityBound.GetCenter().x - entityBound.GetRadius() > midPointX;

	if (left)
	{
		if (top)
			quadrant = 1;
		else if (bottom)
			quadrant = 2;
	}
	else if (right)
	{
		if (top)
			quadrant = 0;
		else if (bottom)
			quadrant = 3;
	}
	return quadrant;
}

void Quadtree3::InsertTree(Entity3* ent)
{
	if (!m_nodes.empty())
	{
		int quadIndex = GetQuadrant(ent);

		if (quadIndex != -1)
		{
			Quadtree3* tree = m_nodes[quadIndex];
			tree->InsertTree(ent);

			return;
		}
	}

	m_entities.push_back(ent);
	if (m_entities.size() > MAX_ENTITIES && m_depth < MAX_DEPTH)
	{
		// make sure this is a child node
		if (m_nodes.empty())
			SplitTree();

		for (int entIdx = (int)(m_entities.size()- 1U); entIdx >= 0; --entIdx)
		{
			Entity3* ent = m_entities[entIdx];
			int quadIndex = GetQuadrant(ent);
			if (quadIndex != -1)
			{
				Quadtree3* tree = m_nodes[quadIndex];

				// remove in old, insert in new
				m_entities[entIdx] = m_entities[m_entities.size() - 1U];
				m_entities.pop_back();
				tree->InsertTree(ent);
			}
			// else the entity does not COMPLETELY fall in any of the quadrants, hence remain in current level
		}
	}
}

void Quadtree3::RetrieveEntity(std::vector<Entity3*>& entities, Entity3* ent)
{
	int quadIndex = GetQuadrant(ent);
	// get to the bottom of the tree
	if (quadIndex != -1 && !m_nodes.empty())
	{
		Quadtree3* tree = m_nodes[quadIndex];
		tree->RetrieveEntity(entities, ent);
	}

	// include all ents with potential collisions
	// could include ent itself, need to prune when processing pairs
	for each (Entity3* myEnt in m_entities)
		entities.push_back(myEnt);
}
