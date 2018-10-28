#include "Engine/Math/QuickHull.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/Util/DataUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Renderer/Renderable.hpp"

#include <cassert>


QHFace::QHFace(int num, Vector3* sample)
{
	vert_num = num;

	for (int i = 0; i < num; ++i)
	{
		Vector3 vertex = *sample;
		verts.push_back(vertex);
		sample++;
	}

	ConstructFeatureID();

	// half edge
	HalfEdge* he1 = new HalfEdge(verts[0]);
	HalfEdge* he2 = new HalfEdge(verts[1]);
	HalfEdge* he3 = new HalfEdge(verts[2]);
	he1->m_prev = he3; he1->m_next = he2;
	he2->m_prev = he1; he2->m_next = he3;
	he3->m_prev = he2; he3->m_next = he1;
	m_entry = he1;
}

// Note: This constructor is ONLY for book keeping purpose, to record
// the closest feature when working with closest-point algorithms.
// That said, it suffices for this constructor to only take down vertices that this face is consisted of.
QHFace::QHFace(const Vector3& v1, const Vector3& v2, const Vector3& v3)
{
	verts.resize(3);
	verts[0] = v1;
	verts[1] = v2;
	verts[2] = v3;

	ConstructFeatureID();
}

QHFace::QHFace(HalfEdge* he, const Vector3& pt)
{
	// 3 vert for sure in this case
	vert_num = 3;

	// record vertices
	Vector3 vert1 = he->m_tail;
	Vector3 vert2 = he->m_next->m_tail;
	verts.push_back(vert1);
	verts.push_back(vert2);
	verts.push_back(pt);

	ConstructFeatureID();

	// set up tail, prev and next for half edges
	// twin and parent face NOT set up yet, EXCEPT for half edge 1
	//HalfEdge* he1 = new HalfEdge(vert1); he1->m_twin = he.m_twin;
	HalfEdge* he2 = new HalfEdge(vert2);
	HalfEdge* he3 = new HalfEdge(pt);
	he->m_prev  = he3; he->m_next  = he2;	// rearrange next and prev of horizon
	he2->m_prev = he; he2->m_next  = he3;
	he3->m_prev = he2; he3->m_next = he;
	m_entry = he;
}


QHFace::~QHFace()
{
	FlushFaceNormalMesh();

	if (faceMesh != nullptr)
	{
		delete faceMesh;
		faceMesh = nullptr;
	}

	// conflict points are usually orphaned points that will be adopted
	// so we DO NOT delete them here
}

void QHFace::AddConflictPoint(QHVert* pt)
{
	// check out/inwardness
	bool outbound = g_hull->PointOutBoundFace(pt->GetVertRef(), *this);
	ASSERT_RECOVERABLE(outbound, "Conflict point must be outbound to the face it belongs to");

	conflicts.push_back(pt);
	pt->ChangeColor(normColor);
}


QHVert* QHFace::GetFarthestConflictPoint(float& dist) const
{
	dist = -INFINITY;
	QHVert* res = nullptr;

	for (QHVert* pt : conflicts)
	{
		float pt_dist = DistPointToPlaneSigned(pt->vert, verts[0], verts[1], verts[2]);
		if (pt_dist > dist)
		{
			dist = pt_dist;
			res = pt;
		}
	}

	return res;
}

bool QHFace::FindTwinAgainstFace(QHFace* face)
{
	HalfEdge* it_he = m_entry;
	bool he1_found = FindEdgeTwinAgainstFace(face, it_he);

	it_he = it_he->m_next;
	bool he2_found = FindEdgeTwinAgainstFace(face, it_he);

	it_he = it_he->m_next;
	bool he3_found = FindEdgeTwinAgainstFace(face, it_he);

	if (!IsTriangle())
	{
		//it_he = it_he->m_next;
		//bool he4_found = FindEdgeTwinAgainstFace(face, it_he);
		ASSERT_OR_DIE(false, "Polygon face now well supported");
	}

	if (he1_found || he2_found || he3_found)
		return true;

	ASSERT_RECOVERABLE(false, "Failed to find twin edge for this face pair, check if they are neighbor");
	return false;
}

/*
 * @PRE: entry is one of the half edge of this face.
 * Find the twin of specified half edge in the given face.
 * If found, set the twin of entry and return true. If not, return false.
 * @face: QHface we use to inspect half edge candidates with.
 * @entry: the half edge which we need to find twin for.
 * @return: bool indicating whether or not we found the twin.
 */
bool QHFace::FindEdgeTwinAgainstFace(QHFace* face, HalfEdge* entry)
{
	HalfEdge* otherEntry = face->m_entry;
	HalfEdge* it_he = otherEntry;

	if (entry->IsTwin(it_he))
	{
		entry->SetTwin(it_he);
		return true;
	}

	it_he = it_he->m_next;
	while (it_he != otherEntry)
	{
		if (entry->IsTwin(it_he))
		{
			entry->SetTwin(it_he);
			return true;
		}

		it_he = it_he->m_next;
	}

	return false;
}

void QHFace::SetParentHalfEdge()
{
	HalfEdge* myEntry = m_entry;
	HalfEdge* it_he = myEntry;

	if (it_he->m_parentFace == nullptr)
		it_he->m_parentFace = this;

	it_he = it_he->m_next;
	while (it_he != myEntry)
	{
		if (it_he->m_parentFace == nullptr)
			it_he->m_parentFace = this;
		it_he = it_he->m_next;
	}
}

const Vector3& QHFace::GetVert4() const
{
	ASSERT_OR_DIE(verts.size() == 4, "Face needs to be a quad in order to get fourth vertex");

	return verts[3];
}

Vector3 QHFace::GetFaceCentroid() const
{
	return ComputeTriangleCenter(verts[0], verts[1], verts[2]);
}

void QHFace::CreateFaceNormalMesh(const Rgba& theColor)
{
	Vector3 start = GetFaceCentroid();
	Vector3 end = start + normal * 5.f;
	if (normMesh == nullptr)
		normMesh = Mesh::CreateLineImmediate(VERT_PCU, start, end, theColor);
}

void QHFace::FlushFaceNormalMesh()
{
	if (normMesh != nullptr)
	{
		delete normMesh;
		normMesh = nullptr;
	}
}

void QHFace::GenerateEdges()
{

}

bool QHFace::ShareEdge(QHFace* other)
{
	// NOTE: Assuming there is only one shared edge between two faces
	HalfEdge* otherEntry = other->m_entry;
	HalfEdge* it_he_other = otherEntry;

	if (IsEdgeShared(it_he_other))
		return true;
	it_he_other = it_he_other->m_next;

	while (it_he_other != otherEntry)
	{
		if (IsEdgeShared(it_he_other))
			return true;
		it_he_other = it_he_other->m_next;
	}

	return false;
}

bool QHFace::IsEdgeShared(HalfEdge* he)
{
	HalfEdge* myEntry = m_entry;
	HalfEdge* it_he_my = m_entry;

	if (it_he_my->IsTwin(he))
	{
		it_he_my->m_twin = he;
		return true;
	}
	it_he_my = it_he_my->m_next;

	while (it_he_my != myEntry)
	{
		if (it_he_my->IsTwin(he))
		{
			it_he_my->m_twin = he;
			return true;
		}
		it_he_my = it_he_my->m_next;
	}

	return false;
}

void QHFace::ConstructFeatureID()
{
	SetFeatureID(QH_FACE);
}

void QHFace::DrawFace(Renderer* renderer)
{
	if (faceMesh != nullptr)
	{
		Shader* shader = renderer->CreateOrGetShader("wireframe_color");
		renderer->UseShader(shader);

		Texture* texture = renderer->CreateOrGetTexture("Data/Images/white.png");
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());
		glLineWidth(2.f);

		renderer->m_objectData.model = Matrix44::IDENTITY;

		renderer->m_currentShader->m_state.m_depthCompare = COMPARE_LESS;
		renderer->m_currentShader->m_state.m_cullMode = CULLMODE_BACK;
		renderer->m_currentShader->m_state.m_windOrder = WIND_COUNTER_CLOCKWISE;

		renderer->DrawMesh(faceMesh, false);
	}

	if (normMesh != nullptr)
	{
		Shader* shader = renderer->CreateOrGetShader("wireframe_color");
		renderer->UseShader(shader);

		Texture* texture = renderer->CreateOrGetTexture("Data/Images/white.png");
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());
		glLineWidth(2.f);

		renderer->m_objectData.model = Matrix44::IDENTITY;

		renderer->m_currentShader->m_state.m_depthCompare = COMPARE_LESS;
		renderer->m_currentShader->m_state.m_cullMode = CULLMODE_BACK;
		renderer->m_currentShader->m_state.m_windOrder = WIND_COUNTER_CLOCKWISE;

		renderer->DrawMesh(normMesh);
	}
}

QuickHull::QuickHull(uint num, const Vector3& min, const Vector3& max)
{
	// initial set up
	GeneratePointSet(num, min, max);
	GenerateInitialFace();
	GenerateInitialHull();

	/*
	for (QHVert* vert : m_verts)
		AddConflictPoint(vert);

	// get the farthest conflict point 
	float farthest;
	m_eyePair = GetFarthestConflictPair(farthest);

	std::deque<QHFace*> allFaces; 
	QHFace* conflict_face = std::get<0>(m_eyePair);
	QHVert* conflict_pt = std::get<1>(m_eyePair);
	m_visibleFaces.push_back(conflict_face);
	allFaces.push_back(conflict_face);

	// pick a edge to cross, and hence a face to stepped on
	HalfEdge* he = conflict_face->m_entry;
	HalfEdge* he_twin = he->m_twin;
	QHFace* otherFace = he_twin->m_parentFace;
	HalfEdge* startEdge = he;

	while (!m_visibleFaces.empty())
	{
		bool visited = std::find(allFaces.begin(), allFaces.end(), otherFace) != allFaces.end();
		if (!visited)
		{
			if (PointOutBoundFace(conflict_pt->vert, *otherFace))
			{
				m_visibleFaces.push_back(otherFace);
				allFaces.push_back(otherFace);

				he = he_twin->m_next;
				he_twin = he->m_twin;
				otherFace = he_twin->m_parentFace;
			}
			else
			{
				m_horizon.push_back(he);
				he = he->m_next;
				he_twin = he->m_twin;
				otherFace = he_twin->m_parentFace;
			}
		}
		else
		{
			if (otherFace == conflict_face && m_visibleFaces.size() == 1U)
			{
				// we arrived at initial conflicting face
				he = he->m_next;
				m_visibleFaces.pop_back();
			}
			else
			{
				size_t size = m_visibleFaces.size();
				QHFace* second = m_visibleFaces[size - 2U];

				if (second == otherFace)
				{
					if (otherFace == conflict_face)
					{
						he = he_twin;
						he_twin = nullptr;
						otherFace = nullptr;
						m_visibleFaces.pop_back();
						// we just reached back the initial conflicting face
						// right now the only face in visibles should be the initial one
					}
					else
					{
						he = he_twin->m_next;
						he_twin = he->m_twin;
						otherFace = he_twin->m_parentFace;
						m_visibleFaces.pop_back();
					}
				}
				else
				{
					he = he->m_next;
					he_twin = he->m_twin;
					otherFace = he_twin->m_parentFace;
				}
			}
		}
	}

	// but we still want to check remaining edges until we loop back to the "startEdge"
	while (he != startEdge)
	{
		he_twin = he->m_twin;
		otherFace = he_twin->m_parentFace;

		bool visited = std::find(allFaces.begin(), allFaces.end(), otherFace) != allFaces.end();
		if (visited)
			he = he->m_next;
		else
		{
			ASSERT_OR_DIE(false, "Should not have unvisited neighbor at this stage");
		}
	}
	*/
}

QuickHull::~QuickHull()
{
	DeleteVector(m_faces);
	DeleteVector(m_verts);
}

bool QuickHull::AddConflictPointInitial(QHVert* vert)
{
	const Vector3& globalPt = vert->GetVertRef();
	float dist;
	Vector3 closest = Vector3::INVALID;

	QHFeature* closest_feature = FindClosestFeatureInitial(globalPt, dist, closest);

	bool pointRemoved = AddToFinalizedFace(closest_feature, closest, vert);
	return pointRemoved;
}

bool QuickHull::AddConflictPointGeneral(QHVert* vert, std::vector<QHFace*>& faces)
{
	const Vector3& globalPt = vert->GetVertRef();
	float dist;
	Vector3 closest = Vector3::INVALID;

	QHFeature* closest_feature = FindClosestFeatureGeneral(globalPt, dist, closest, faces);

	bool pointRemoved = AddToFinalizedFace(closest_feature, closest, vert);
	return pointRemoved;
}

void QuickHull::AddHorizonMesh(HalfEdge* horizon)
{
	Vector3 start = horizon->m_tail;
	Vector3 end = horizon->m_next->m_tail;
	Mesh* mesh = Mesh::CreateLineImmediate(VERT_PCU, start, end, Rgba::CYAN);
	m_horizon_mesh.push_back(mesh);
}

/*
 * Add point to a face based on closest feature info
 * @param, closest_feature: the closest feature the vert is to this geometry
 * @param, closest: the closest point the vert is to this geometry
 * @param, vert: the vert we wish to add to a conflict list
 * @return: if the point is removed from global list of this hull
 */
bool QuickHull::AddToFinalizedFace(QHFeature* closest_feature, const Vector3& closest, QHVert* vert)
{
	const Vector3& globalPt = vert->GetVertRef();

	// point is not inside the hull
	if (closest_feature != nullptr && closest != Vector3::INVALID)
	{
		QHFace* face = dynamic_cast<QHFace*>(closest_feature);
		QHEdge* edge = dynamic_cast<QHEdge*>(closest_feature);
		QHVert* shared_vert = dynamic_cast<QHVert*>(closest_feature);

		if (face != nullptr)
		{
			const Vector3& candidate1 = face->GetVert1();
			const Vector3& candidate2 = face->GetVert2();
			const Vector3& candidate3 = face->GetVert3();

			bool found = false;
			QHFace* theFace = FindFaceGivenPts(candidate1, candidate2, candidate3, found);
			if (found)
				theFace->AddConflictPoint(vert);
		}
		else if (edge != nullptr)
		{
			bool found = false;
			const std::vector<QHFace*>& faces = FindFaceGivenSharedEdge(*edge, found);

			if (found)
			{
				QHFace* theFace1 = faces[0];
				QHFace* theFace2 = faces[1];

				// see which face is closer to the point
				float dist1 = DistPointToPlaneUnsigned(globalPt, theFace1->GetVert1(), theFace1->GetVert2(), theFace1->GetVert3());
				float dist2 = DistPointToPlaneUnsigned(globalPt, theFace2->GetVert1(), theFace2->GetVert2(), theFace2->GetVert3());

				bool out1 = PointOutBoundFace(globalPt, *theFace1);
				bool out2 = PointOutBoundFace(globalPt, *theFace2);

				if (!out1 && out2)
					theFace2->AddConflictPoint(vert);
				else if (!out2 && out1)
					theFace1->AddConflictPoint(vert);
				else if (out1 && out2)
				{
					if (dist1 < dist2)
						theFace1->AddConflictPoint(vert);
					else
						theFace2->AddConflictPoint(vert);
				}
				else
				{
					ASSERT_OR_DIE(false, "Point inbound to both faces, this should NOT happen");
				}
			}
			else
				ASSERT_RECOVERABLE(found, "No faces for shared edge feature is found");
		}
		else if (shared_vert != nullptr)
		{
			bool found = false;
			const std::vector<QHFace*>& faces = FindFaceGivenSharedVert(*shared_vert, found);

			if (found)
			{
				QHFace* theFace1 = faces[0];
				QHFace* theFace2 = faces[1];
				QHFace* theFace3 = faces[2];

				// see which face is closer to the point
				float dist1 = DistPointToPlaneUnsigned(globalPt, theFace1->GetVert1(), theFace1->GetVert2(), theFace1->GetVert3());
				float dist2 = DistPointToPlaneUnsigned(globalPt, theFace2->GetVert1(), theFace2->GetVert2(), theFace2->GetVert3());
				float dist3 = DistPointToPlaneUnsigned(globalPt, theFace3->GetVert1(), theFace3->GetVert2(), theFace3->GetVert3());

				bool out1 = PointOutBoundFace(globalPt, *theFace1);
				bool out2 = PointOutBoundFace(globalPt, *theFace2);
				bool out3 = PointOutBoundFace(globalPt, *theFace3);

				if (out1 && out2 && out3)
				{
					// 1, 2 and 3 all possible
					float minDist = min(min(dist1, dist2), dist3);
					if (minDist == dist1)
						theFace1->AddConflictPoint(vert);
					else if (minDist == dist2)
						theFace2->AddConflictPoint(vert);
					else if (minDist == dist3)
						theFace3->AddConflictPoint(vert);
				}
				else if (out1 && out2 && !out3)
				{
					if (dist1 < dist2)
						theFace1->AddConflictPoint(vert);
					else
						theFace2->AddConflictPoint(vert);
				}
				else if (out1 && !out2 && out3)
				{
					if (dist1 < dist3)
						theFace1->AddConflictPoint(vert);
					else
						theFace3->AddConflictPoint(vert);
				}
				else if (out1 && !out2 && !out3)
					theFace1->AddConflictPoint(vert);
				else if (!out1 && out2 && out3)
				{
					if (dist2 < dist3)
						theFace2->AddConflictPoint(vert);
					else
						theFace3->AddConflictPoint(vert);
				}
				else if (!out1 && out2 && !out3)
					theFace2->AddConflictPoint(vert);
				else if (!out1 && !out2 && out3)
					theFace3->AddConflictPoint(vert);
				else
					ASSERT_OR_DIE(false, "Point inbound to triple faces, this should NOT happen");
			}
			else
				ASSERT_RECOVERABLE(found, "Faces of shared vert not found");
		}

		return false;			// there is NO point removed
	}
	// point is inside the hull
	else
	{
		RemovePointGlobal(globalPt);
		return true;			// this point IS removed
	}
}

void QuickHull::GeneratePointSet(uint num, const Vector3& min, const Vector3& max)
{
	for (uint i = 0; i < num; ++i)
	{
		float x = GetRandomFloatInRange(min.x, max.x);
		float y = GetRandomFloatInRange(min.y, max.y);
		float z = GetRandomFloatInRange(min.z, max.z);
		Vector3 pos = Vector3(x, y, z);
		QHVert* vert = new QHVert(pos, Rgba::WHITE);
		m_verts.push_back(vert);
	}
}

void QuickHull::GenerateInitialFace()
{
	// points of min and max extents along each axis
	float max = -INFINITY;
	Vector3 x_max;
	for (QHVert* vert : m_verts)
	{
		Vector3 pt = vert->GetVertRef();
		if (pt.x > max)
		{
			x_max = pt;
			max = pt.x;
		}
	}

	max = -INFINITY;
	Vector3 y_max;
	for (QHVert* vert : m_verts)
	{
		Vector3 pt = vert->GetVertRef();
		if (pt.y > max)
		{
			y_max = pt;
			max = pt.y;
		}
	}

	max = -INFINITY;
	Vector3 z_max;
	for (QHVert* vert : m_verts)
	{
		Vector3 pt = vert->GetVertRef();
		if (pt.z > max)
		{
			z_max = pt;
			max = pt.z;
		}
	}

	float min = INFINITY;
	Vector3 x_min;
	for (QHVert* vert : m_verts)
	{
		Vector3 pt = vert->GetVertRef();
		if (pt.x < min)
		{
			x_min = pt;
			min = pt.x;
		}
	}

	min = INFINITY;
	Vector3 y_min;
	for (QHVert* vert : m_verts)
	{
		Vector3 pt = vert->GetVertRef();
		if (pt.y < min)
		{
			y_min = pt;
			min = pt.y;
		}
	}

	min = INFINITY;
	Vector3 z_min;
	for (QHVert* vert : m_verts)
	{
		Vector3 pt = vert->GetVertRef();
		if (pt.z < min)
		{
			z_min = pt;
			min = pt.z;
		}
	}

	// given min/max points, choose the pair farthest apart
	Vector3 v1, v2;
	max = -INFINITY;
	float x_apart = (x_max - x_min).GetLength();
	float y_apart = (y_max - y_min).GetLength();
	float z_apart = (z_max - z_min).GetLength();
	if (x_apart > max)
	{
		v1 = x_max; v2 = x_min;
		max = x_apart;
	}
	if (y_apart > max)
	{
		v1 = y_max; v2 = y_min;
		max = y_apart;
	}
	if (z_apart > max)
	{
		v1 = z_max; v2 = z_min;
		max = z_apart;
	}

	// find the farthest point to line formed by v1 and v2
	max = -INFINITY;
	Vector3 v3;
	for (QHVert* vert : m_verts)
	{
		Vector3 pt = vert->GetVertRef();
		if (pt != v1 && pt != v2)
		{
			float dist = DistPointToEdge(pt, v1, v2);
			if (dist > max)
			{
				v3 = pt;
				max = dist;
			}
		}
	}

	// generate triangle face 
	Vector3 verts[3] = {v1, v2, v3};
	QHFace* face = new QHFace(3, verts);
	face->SetParentHalfEdge();
	m_faces.push_back(face);

	// delete these points from effective point set
	RemovePointGlobal(v1);
	RemovePointGlobal(v2);
	RemovePointGlobal(v3);
	
	// create mesh
	CreateFaceMesh(*face);
	// done
}

void QuickHull::GenerateInitialHull()
{
	// first triangle
	assert(m_faces[0]->vert_num == 3);
	Vector3 v1 = m_faces[0]->verts[0];
	Vector3 v2 = m_faces[0]->verts[1];
	Vector3 v3 = m_faces[0]->verts[2];

	// say triangle has v1, v2, v3, which are removed from global vert list
	// to have initial hull, we need a v4
	float max = -INFINITY;
	Vector3 v4;
	for (QHVert* vert : m_verts)
	{
		Vector3 pt = vert->GetVertRef();
		float dist = DistPointToPlaneUnsigned(pt, v1, v2, v3);

		if (dist > max)
		{
			v4 = pt;
			max = dist;
		}
	}

	// get a temp normal for initial face,
	// regardless of direction of normal (in or outward)
	GenerateOutboundNorm(v4, *m_faces[0]);

	// build new faces
	Vector3 verts214[3] = {v2, v1, v4};
	Vector3 verts134[3] = {v1, v3, v4};
	Vector3 verts243[3] = {v2, v4, v3};
	QHFace* face214 = new QHFace(3, verts214); face214->SetParentHalfEdge();
	QHFace* face134 = new QHFace(3, verts134); face134->SetParentHalfEdge();
	QHFace* face243 = new QHFace(3, verts243); face243->SetParentHalfEdge();

	// fill twin for 123
	bool found = m_faces[0]->FindTwinAgainstFace(face214);	
	found = m_faces[0]->FindTwinAgainstFace(face134);		
	found = m_faces[0]->FindTwinAgainstFace(face243);		

	// fill twin for 214
	face214->FindTwinAgainstFace(m_faces[0]);	
	face214->FindTwinAgainstFace(face134);		
	face214->FindTwinAgainstFace(face243);		

	// fill twin for 134
	face134->FindTwinAgainstFace(m_faces[0]); 
	face134->FindTwinAgainstFace(face214);	  
	face134->FindTwinAgainstFace(face243);	  

	// fill twin for 243
	face243->FindTwinAgainstFace(m_faces[0]);
	face243->FindTwinAgainstFace(face214);	 
	face243->FindTwinAgainstFace(face134);	 

	// given a face and an external point, calculate the outbound normal 
	// outbound means opposite direction from the face to that external point
	GenerateOutboundNorm(v3, *face214);
	GenerateOutboundNorm(v2, *face134);
	GenerateOutboundNorm(v1, *face243);

	// store faces
	m_faces.push_back(face214);
	m_faces.push_back(face134);
	m_faces.push_back(face243);

	// remove v4 from verts
	RemovePointGlobal(v4);

	// create mesh
	CreateFaceMesh(*face214);
	CreateFaceMesh(*face134);
	CreateFaceMesh(*face243);
	// done
}


void QuickHull::GenerateFaceNorms(Vector3& norm1, Vector3& norm2, const QHFace& face)
{
	const Vector3& v1 = face.verts[0];
	const Vector3& v2 = face.verts[1];
	const Vector3& v3 = face.verts[2];

	const Vector3& side1 = v2 - v1;
	const Vector3& side2 = v3 - v1;

	norm1 = side1.Cross(side2).GetNormalized();
	norm2 = side2.Cross(side1).GetNormalized();
}

void QuickHull::GenerateOutboundNorm(const Vector3& external, QHFace& face)
{
	// get a temp normal for initial face,
	// regardless of direction of normal (in or outward)
	Vector3 norm1;
	Vector3 norm2;
	GenerateFaceNorms(norm1, norm2, face);

	// decide on initial face normal based on v4
	Vector3 towardExternal = external - face.verts[0];
	float ext1 = DotProduct(towardExternal, norm1);
	//float ext2 = DotProduct(towardExternal, norm2);
	Vector3 norm = (ext1 < 0.f) ? norm1 : norm2;
	face.normal = norm;
}

void QuickHull::RemovePointGlobal(const Vector3& pt)
{
	for (std::vector<QHVert*>::size_type idx = 0; idx < m_verts.size(); ++idx)
	{
		if (m_verts[idx]->GetVertRef() == pt)
		{
			QHVert* delVert = m_verts[idx];

			// adjust vector 
			std::vector<QHVert*>::iterator it = m_verts.begin();
			m_verts.erase(it + idx);
			idx--;

			// delete vert
			delete delVert;
		}
	}
}

bool QuickHull::PointOutBoundFace(const Vector3& pt, const QHFace& face)
{
	Vector3 disp = pt - face.verts[0];
	bool outbound = false;
	float extension = DotProduct(disp, face.normal);
	
	if (extension > 0.f)
		outbound = true;

	return outbound;
}

void QuickHull::RenderCurrentHalfEdge(Renderer* renderer)
{
	if (m_test_he_mesh != nullptr)
	{
		Shader* shader = renderer->CreateOrGetShader("wireframe_color");
		renderer->UseShader(shader);

		Texture* texture = renderer->CreateOrGetTexture("Data/Images/white.png");
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());
		glLineWidth(10.f);

		renderer->m_objectData.model = Matrix44::IDENTITY;

		renderer->m_currentShader->m_state.m_depthCompare = COMPARE_LESS;
		renderer->m_currentShader->m_state.m_cullMode = CULLMODE_BACK;
		renderer->m_currentShader->m_state.m_windOrder = WIND_COUNTER_CLOCKWISE;

		renderer->DrawMesh(m_test_he_mesh);
	}
}

void QuickHull::CreateNormalMeshes()
{
	for (std::vector<QHFace*>::size_type idx = 0; idx < m_faces.size(); ++idx)
	{
		const Rgba& theColor = color_list[idx];
		m_faces[idx]->normColor = theColor;
		m_faces[idx]->CreateFaceNormalMesh(theColor);
	}
}

void QuickHull::FlushNormalMeshes()
{
	for (QHFace* face : m_faces)
		face->FlushFaceNormalMesh();
}

/*
* Find the closest QH feature the given point is subject to the hull.
* @param pt: position of point of interest. 
* @param theDist: reference to closest distance.
* @param closest: reference to closest point.
* @return: closest feature
*/
QHFeature* QuickHull::FindClosestFeatureInitial(const Vector3& pt, float& theDist, Vector3& closest)
{
	float minDist = INFINITY;
	QHFeature* res = nullptr;

	// initial hull should have 4 faces
	assert(m_faces.size() == 4);

	if (PointOutBoundFace(pt, *m_faces[0]))
	{
		Vector3 closest_f1;
		float dist_f1 = 0.f;
		QHFeature* closest_feature_f1 = nullptr;

		if (m_faces[0]->IsTriangle())
			closest_feature_f1 = DistPointToTriangleHull(pt, m_faces[0]->GetVert1(),
				m_faces[0]->GetVert2(), m_faces[0]->GetVert3(), dist_f1, closest_f1);
		else
			//dist_f1 = DistPointToQuadHull(pt, m_faces[0].GetVert1(), m_faces[0].GetVert2(),
			//	m_faces[0].GetVert3(), m_faces[0].GetVert4(), closest_feature_f1, closest_f1);
			ASSERT_RECOVERABLE(false, "Initial hull should not have a quad face");

		if (dist_f1 < minDist)
		{
			minDist = dist_f1;	
			theDist = dist_f1;
			closest = closest_f1;
			res = closest_feature_f1;
		}
	}

	if (PointOutBoundFace(pt, *m_faces[1]))
	{
		Vector3 closest_f2;
		float dist_f2 = 0.f;
		QHFeature* closest_feature_f2 = nullptr;

		if (m_faces[1]->IsTriangle())
			closest_feature_f2 = DistPointToTriangleHull(pt, m_faces[1]->GetVert1(),
				m_faces[1]->GetVert2(), m_faces[1]->GetVert3(), dist_f2, closest_f2);
		else
			//dist_f2 = DistPointToQuadHull(pt, m_faces[1].GetVert1(), m_faces[1].GetVert2(),
			//	m_faces[1].GetVert3(), m_faces[1].GetVert4(), closest_feature_f2, closest_f2);
			ASSERT_RECOVERABLE(false, "Initial hull should not have a quad face");

		if (dist_f2 < minDist)
		{
			minDist = dist_f2;
			theDist = dist_f2;
			closest = closest_f2;
			res = closest_feature_f2;
		}
	}

	if (PointOutBoundFace(pt, *m_faces[2]))
	{
		Vector3 closest_f3;
		float dist_f3 = 0.f;
		QHFeature* closest_feature_f3 = nullptr;

		if (m_faces[2]->IsTriangle())
			closest_feature_f3 = DistPointToTriangleHull(pt, m_faces[2]->GetVert1(),
				m_faces[2]->GetVert2(), m_faces[2]->GetVert3(), dist_f3, closest_f3);
		else
			//dist_f3 = DistPointToQuadHull(pt, m_faces[2].GetVert1(), m_faces[2].GetVert2(),
			//	m_faces[2].GetVert3(), m_faces[2].GetVert4(), closest_feature_f3, closest_f3);
			ASSERT_RECOVERABLE(false, "Initial hull should not have a quad face");

		if (dist_f3 < minDist)
		{
			minDist = dist_f3;
			theDist = dist_f3;
			closest = closest_f3;
			res = closest_feature_f3;
		}
	}

	if (PointOutBoundFace(pt, *m_faces[3]))
	{
		Vector3 closest_f4;
		float dist_f4 = 0.f;
		QHFeature* closest_feature_f4 = nullptr;

		if (m_faces[3]->IsTriangle())
			closest_feature_f4 = DistPointToTriangleHull(pt, m_faces[3]->GetVert1(),
				m_faces[3]->GetVert2(), m_faces[3]->GetVert3(), dist_f4, closest_f4);
		else
			//dist_f4 = DistPointToQuadHull(pt, m_faces[3].GetVert1(), m_faces[3].GetVert2(),
			//	m_faces[3].GetVert3(), m_faces[3].GetVert4(), closest_feature_f4, closest_f4);
			ASSERT_RECOVERABLE(false, "Initial hull should not have a quad face");

		if (dist_f4 < minDist)
		{
			minDist = dist_f4;
			theDist = dist_f4;
			closest = closest_f4;
			res = closest_feature_f4;
		}
	}

	return res;
}

QHFeature* QuickHull::FindClosestFeatureGeneral(const Vector3& pt, float& dist, Vector3& closest, std::vector<QHFace*>& faces)
{
	float minDist = INFINITY;
	QHFeature* res = nullptr;

	for (QHFace* face : faces)
	{
		if (PointOutBoundFace(pt, *face))
		{
			Vector3 closest_f;
			float dist_f = 0.f;
			QHFeature* closest_feature_f = nullptr;

			if (face->IsTriangle())
				closest_feature_f = DistPointToTriangleHull(pt, face->GetVert1(),
					face->GetVert2(), face->GetVert3(), dist_f, closest_f);
			else
				ASSERT_RECOVERABLE(false, "Initial hull should not have a quad face");

			if (dist_f < minDist)
			{
				minDist = dist_f;	
				dist = dist_f;
				closest = closest_f;
				res = closest_feature_f;
			}
		}
	}

	return res;
}

QHFace* QuickHull::FindFaceGivenPts(const Vector3& v1, const Vector3& v2, const Vector3& v3, bool& found)
{
	QHFace* theFace = nullptr;

	for ( QHFace* face : m_faces )
	{
		bool has1 = (std::find(face->verts.begin(), face->verts.end(), v1) != face->verts.end());
		bool has2 = (std::find(face->verts.begin(), face->verts.end(), v2) != face->verts.end());
		bool has3 = (std::find(face->verts.begin(), face->verts.end(), v3) != face->verts.end());

		if (has1 && has2 && has3)
		{
			found = true;
			theFace = face;
			break;
		}
	}

	return theFace;
}

const std::vector<QHFace*> QuickHull::FindFaceGivenSharedEdge(const QHEdge& edge, bool& found)
{
	std::vector<QHFace*> share_faces;
	bool f1_found = false;
	bool f2_found = false;

	for (QHFace* face : m_faces)
	{
		bool v1_found = (std::find(face->verts.begin(), face->verts.end(), edge.v1) != face->verts.end());
		bool v2_found = (std::find(face->verts.begin(), face->verts.end(), edge.v2) != face->verts.end());

		if (v1_found && v2_found)
		{
			if (!f1_found)
			{
				f1_found = true;
				share_faces.push_back(face);
			}
			else if (!f2_found)
			{
				f2_found = true;
				share_faces.push_back(face);
				break;
			}
		}
	}

	found = f1_found && f2_found;

	return share_faces;
}

const std::vector<QHFace*> QuickHull::FindFaceGivenSharedVert(const QHVert& theVert, bool& found)
{
	std::vector<QHFace*> share_faces;
	bool found1 = false;
	bool found2 = false;
	bool found3 = false;

	for (QHFace* face : m_faces)
	{
		bool vert_found = (std::find(face->verts.begin(), face->verts.end(), theVert.vert) != face->verts.end());

		if (vert_found)
		{
			if (!found1)
			{
				found1 = true;
				share_faces.push_back(face);
			}
			else if (!found2)
			{
				found2 = true;
				share_faces.push_back(face);
			}
			else if (!found3)
			{
				found3 = true;
				share_faces.push_back(face);
				break;							// all three relevant faces found, stop processing
			}
		}
	}

	found = found1 && found2 && found3;

	return share_faces;
}

std::tuple<QHFace*, QHVert*> QuickHull::GetFarthestConflictPair(float& dist) const
{
	std::tuple<QHFace*, QHVert*> vert_pair;
	QHVert* winner_vert = nullptr;
	QHFace* winner_face = nullptr;
	dist = -INFINITY;

	for (QHFace* face : m_faces)
	{
		float conflict_dist;
		QHVert* conflict = face->GetFarthestConflictPoint(conflict_dist);
		if (conflict_dist > dist)
		{
			dist = conflict_dist;
			winner_vert = conflict;
			winner_face = face;
		}
	}

	vert_pair = std::make_tuple(winner_face, winner_vert);
	return vert_pair;
}

void QuickHull::ChangeCurrentHalfEdge()
{
	if (m_test_he_mesh != nullptr)
	{
		delete m_test_he_mesh;
		m_test_he_mesh = nullptr;
	}

	if (test_he != nullptr)
	{
		Vector3 start = test_he->m_tail;
		Vector3 end = test_he->m_twin->m_tail;
		m_test_he_mesh = Mesh::CreateLineImmediate(VERT_PCU, start, end, Rgba::MEGENTA);
	}
	else 
		ASSERT_OR_DIE(false, "Half edge is null, cannot generate mesh for it");
}

void QuickHull::RenderHull(Renderer* renderer)
{
	RenderFaces(renderer);
	RenderVerts(renderer);
	RenderCurrentHalfEdge(renderer);
	RenderHorizon(renderer);
}

void QuickHull::RenderFaces(Renderer* renderer)
{
	for (QHFace* face : m_faces)
		face->DrawFace(renderer);
}


void QuickHull::RenderVerts(Renderer* renderer)
{
	for (QHVert* vert : m_verts)
		vert->DrawVert(renderer);
}

void QuickHull::RenderHorizon(Renderer* renderer)
{
	for (Mesh* mesh : m_horizon_mesh)
	{
		if (mesh != nullptr)
		{
			Shader* shader = renderer->CreateOrGetShader("wireframe_color");
			renderer->UseShader(shader);

			Texture* texture = renderer->CreateOrGetTexture("Data/Images/white.png");
			renderer->SetTexture2D(0, texture);
			renderer->SetSampler2D(0, texture->GetSampler());
			glLineWidth(20.f);

			renderer->m_objectData.model = Matrix44::IDENTITY;

			renderer->m_currentShader->m_state.m_depthCompare = COMPARE_LESS;
			renderer->m_currentShader->m_state.m_cullMode = CULLMODE_BACK;
			renderer->m_currentShader->m_state.m_windOrder = WIND_COUNTER_CLOCKWISE;

			renderer->DrawMesh(mesh);
		}
	}
}

void QuickHull::CreateFaceMesh(QHFace& face)
{
	if (face.vert_num == 3)
	{
		const Vector3& v1 = face.verts[0];
		const Vector3& v2 = face.verts[1];
		const Vector3& v3 = face.verts[2];

		Mesh* mesh = Mesh::CreateTriangleImmediate(VERT_PCU, Rgba::WHITE, v1, v2, v3);
		face.faceMesh = mesh;
	}
	else 
	{
		ASSERT_RECOVERABLE(false, "Quad face is not fully supported");

		const Vector3& v1 = face.verts[0];
		const Vector3& v2 = face.verts[1];
		const Vector3& v3 = face.verts[2];
		const Vector3& v4 = face.verts[3];

		Mesh* mesh = Mesh::CreateQuadImmediate(VERT_PCU, v1, v2, v3, v4, Rgba::WHITE);
		face.faceMesh = mesh;
	}
}

QHEdge::QHEdge(const Vector3& vert1, const Vector3& vert2)
{
	v1 = vert1;
	v2 = vert2;

	ConstructFeatureID();
}

QHEdge::QHEdge(const Vector3& vert1, const Vector3& vert2, Rgba theColor)
{
	v1 = vert1;
	v2 = vert2;
	color = theColor;

	edgeMesh = Mesh::CreateLineImmediate(VERT_PCU, v1, v2, color);

	ConstructFeatureID();
}

QHEdge::~QHEdge()
{
	if (edgeMesh != nullptr)
	{
		delete edgeMesh;
		edgeMesh = nullptr;
	}
}

void QHEdge::ConstructFeatureID()
{
	SetFeatureID(QH_EDGE);
}

QHVert::QHVert(const Vector3& pt)
{
	vert = pt;

	ConstructFeatureID();
}

QHVert::QHVert(const Vector3& pt, const Rgba& theColor)
{
	vert = pt;
	color = theColor;

	vertMesh = Mesh::CreatePointImmediate(VERT_PCU, vert, color);

	ConstructFeatureID();
}

QHVert::~QHVert()
{
	if (vertMesh != nullptr)
	{
		delete vertMesh;
		vertMesh = nullptr;
	}
}

void QHVert::ChangeColor(const Rgba& theColor)
{
	if (vertMesh != nullptr)
	{
		delete vertMesh;
		vertMesh = nullptr;
	}

	vertMesh = Mesh::CreatePointImmediate(VERT_PCU, vert, theColor);
}

void QHVert::ConstructFeatureID()
{
	SetFeatureID(QH_VERT);
}

void QHVert::DrawVert(Renderer* renderer) const
{
	if (vertMesh != nullptr)
	{
		Shader* shader = renderer->CreateOrGetShader("wireframe_color");
		renderer->UseShader(shader);

		Texture* texture = renderer->CreateOrGetTexture("Data/Images/white.png");
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());
		glPointSize(10.f);

		renderer->m_objectData.model = Matrix44::IDENTITY;

		renderer->m_currentShader->m_state.m_depthCompare = COMPARE_LESS;
		renderer->m_currentShader->m_state.m_cullMode = CULLMODE_BACK;
		renderer->m_currentShader->m_state.m_windOrder = WIND_COUNTER_CLOCKWISE;

		renderer->DrawMesh(vertMesh);
	}
}