#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/Renderer.hpp"

class QHFace;

struct HalfEdge
{
	HalfEdge* m_prev;
	HalfEdge* m_next;
	HalfEdge* m_twin;
	Vector3 m_tail;
	QHFace* m_parentFace = nullptr;		// weak pointer to belonging face

	Mesh* m_body_mesh;
	Mesh* m_arrow_mesh[3];

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

	~HalfEdge()
	{
		m_prev = nullptr;
		m_next = nullptr;
		m_twin = nullptr;
		m_parentFace = nullptr;
		FlushMeshes();
	}

	void FlushMeshes();

	void Draw(Renderer* renderer);

	void SetTwin(HalfEdge* value) { m_twin = value; }

	void FillBodyMesh();
	void FillArrowMeshes(Vector3 arrow_end[3]);
	void CreateArrowMeshes();

	void FillArrowMeshesOffset(Vector3 arrow_end[3], const Vector3& head, const Rgba& color);
	void FillBodyMeshOffset(float percentage, const Vector3& centroid, const Rgba& color = Rgba::WHITE);
	void CreateArrowMeshesOffset(float percentage, const Vector3& centroid, const Rgba& color = Rgba::WHITE);
	void GetOffsetTailHead(float percentage, const Vector3& centroid, Vector3& tail, Vector3& head);

	void DebugUpdateTwin();
	void SwapMesh(const Rgba& color);
	void SwapMeshTwin(bool twinSet);		// we do not care if this is current_he of hull
	void SwapMeshTwinGeneral(bool twinSet);		// we do care if this is current_he of hull, because it could have been intialized or changed
	void SwapMeshTwinHorizon();

	bool IsTwin(HalfEdge* value) { return (value->m_next->m_tail == m_tail && value->m_tail == m_next->m_tail); }
	bool HasTwin() const { return m_twin != nullptr; }
	Vector3 GetHeadPos() const;

	// assumes this HE is horizon
	void VerifyHorizonTwin();
};
