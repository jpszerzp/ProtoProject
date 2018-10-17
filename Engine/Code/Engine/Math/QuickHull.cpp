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
	//normColor = Rgba::WHITE;

	for (int i = 0; i < num; ++i)
	{
		Vector3 vertex = *sample;
		verts.push_back(vertex);
		sample++;
	}

	ConstructFeatureID();
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

/*
QHFace::QHFace(int num, Vector3* sample, const Rgba& color)
{
	vert_num = num;
	normColor = color;

	for (int i = 0; i < num; ++i)
	{
		Vector3 vertex = *sample;
		verts.push_back(vertex);
		sample++;
	}

	ConstructFeatureID();
}
*/

QHFace::~QHFace()
{
	FlushFaceNormalMesh();
}


void QHFace::AddConflictPoint(QHVert* pt)
{
	// check out/inwardness
	bool outbound = g_hull->PointOutBoundFace(pt->GetVertRef(), *this);
	ASSERT_RECOVERABLE(outbound, "Conflict point must be outbound to the face it belongs to");

	conflicts.push_back(pt);
	pt->ChangeColor(normColor);
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
		*/
}

QuickHull::~QuickHull()
{
	DeleteVector(m_faces);
	DeleteVector(m_verts);
}

bool QuickHull::AddConflictPoint(QHVert* vert)
{
	const Vector3& globalPt = vert->GetVertRef();
	float dist;
	Vector3 closest = Vector3::INVALID;

	QHFeature* closest_feature = FindClosestFeatureInitial(globalPt, dist, closest);

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
				//theFace.ChangeConflictColor(vert);
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
				//float dist1 = DistPointToPlaneSigned(globalPt, theFace1->GetVert1(), theFace1->GetVert2(), theFace1->GetVert3());
				//float dist2 = DistPointToPlaneSigned(globalPt, theFace2->GetVert1(), theFace2->GetVert2(), theFace2->GetVert3());

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

				/*
				// if inbound to a plane, use the other directly
				if (dist1 <= 0.f)
					theFace2->AddConflictPoint(vert);
				else if (dist2 <= 0.f)
					theFace1->AddConflictPoint(vert);
				else
				{
					if (dist1 < dist2)
						theFace1->AddConflictPoint(vert);
					else
						theFace2->AddConflictPoint(vert);
				}
				*/
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
				//float dist1 = DistPointToPlaneSigned(globalPt, theFace1->GetVert1(), theFace1->GetVert2(), theFace1->GetVert3());
				//float dist2 = DistPointToPlaneSigned(globalPt, theFace2->GetVert1(), theFace2->GetVert2(), theFace2->GetVert3());
				//float dist3 = DistPointToPlaneSigned(globalPt, theFace3->GetVert1(), theFace3->GetVert2(), theFace3->GetVert3());
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


				/*
				if (dist1 <= 0.f)
				{
					if (dist2 <= 0.f)
						// 3
						theFace3->AddConflictPoint(vert);
					else if (dist3 <= 0.f)
						// 2
						theFace2->AddConflictPoint(vert);
					else
					{
						// 2 and 3 possible
						if (dist2 < dist3)
							theFace2->AddConflictPoint(vert);
						else
							theFace3->AddConflictPoint(vert);
					}
				}
				else
				{
					if (dist2 <= 0.f)
					{
						if (dist3 <= 0.f)
							// 1
							theFace1->AddConflictPoint(vert);
						else
						{
							// 1 and 3 are both possible
							if (dist1 < dist3)
								theFace1->AddConflictPoint(vert);
							else
								theFace3->AddConflictPoint(vert);
						}
					}
					else
					{
						// 1 and 2 possible
						if (dist3 <= 0.f)
						{
							if (dist1 < dist2)
								theFace1->AddConflictPoint(vert);
							else
								theFace2->AddConflictPoint(vert);
						}
						else
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
					}
				}
				*/
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
	//Rgba randColor = GetRandomColor();
	QHFace* face = new QHFace(3, verts);
	//face->GenerateEdges();
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
	Vector3 verts124[3] = {v1, v2, v4};
	//Rgba randColor124 = GetRandomColor();
	Vector3 verts134[3] = {v1, v3, v4};
	//Rgba randColor134 = GetRandomColor();
	Vector3 verts234[3] = {v2, v3, v4};
	//Rgba randColor234 = GetRandomColor();
	QHFace* face124 = new QHFace(3, verts124);
	QHFace* face134 = new QHFace(3, verts134);
	QHFace* face234 = new QHFace(3, verts234);

	// given a face and an external point, calculate the outbound normal 
	// outbound means opposite direction from the face to that external point
	GenerateOutboundNorm(v3, *face124);
	GenerateOutboundNorm(v2, *face134);
	GenerateOutboundNorm(v1, *face234);

	// store faces
	m_faces.push_back(face124);
	m_faces.push_back(face134);
	m_faces.push_back(face234);

	// remove v4 from verts
	RemovePointGlobal(v4);

	// create mesh
	CreateFaceMesh(*face124);
	CreateFaceMesh(*face134);
	CreateFaceMesh(*face234);
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
	float ext2 = DotProduct(towardExternal, norm2);
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

void QuickHull::RenderHull(Renderer* renderer)
{
	RenderFaces(renderer);
	RenderVerts(renderer);
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
