#pragma once

#include "Engine/Math/Vector3.hpp"

class QHFace;

struct HalfEdge
{
	HalfEdge* m_prev;
	HalfEdge* m_next;
	HalfEdge* m_twin;
	Vector3 m_tail;
	QHFace* m_parentFace = nullptr;		// weak pointer to belonging face

	//bool m_markDelete = false;

	HalfEdge(HalfEdge* prev, HalfEdge* next, HalfEdge* twin, Vector3 tail)
		: m_prev(prev), m_next(next), m_twin(twin), m_tail(tail){}
	HalfEdge(Vector3 tail) 
		: m_tail(tail) 
	{
		m_prev = nullptr;
		m_next = nullptr;
		m_twin = nullptr;
		m_parentFace = nullptr;
	}
	~HalfEdge(){}

	void SetTwin(HalfEdge* value) { m_twin = value; }
	bool IsTwin(HalfEdge* value)
	{
		return (value->m_next->m_tail == m_tail
			&& value->m_tail == m_next->m_tail);
	}
};