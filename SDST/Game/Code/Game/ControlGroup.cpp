#include "Game/ControlGroup.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Primitive/Sphere.hpp"
#include "Engine/Core/Primitive/Quad.hpp"
#include "Engine/Core/Primitive/Box.hpp"
#include "Engine/Core/Primitive/Line.hpp"
#include "Engine/Core/Util/DataUtils.hpp"
#include "Engine/Core/Util/RenderUtil.hpp"
#include "Engine/Core/Primitive/Line.hpp"
#include "Engine/Core/HullObject.hpp"
#include "Engine/Physics/3D/SphereRB3.hpp"
#include "Engine/Physics/3D/QuadRb3.hpp"
#include "Engine/Physics/3D/BoxRB3.hpp"
#include "Engine/Physics/3D/GJK3.hpp"
#include "Engine/Physics/3D/GJK3Simplex.hpp"
#include "Engine/Physics/3D/EPA3.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/Renderable.hpp"


ControlGroup::ControlGroup(GameObject* go1, GameObject* go2, const eControlID& id, const Vector3& observation)
{
	m_gos.push_back(go1);
	m_gos.push_back(go2);
	m_id = id;

	Window* window = Window::GetInstance();
	float width = window->GetWindowWidth();
	float height = window->GetWindowHeight();
	m_textHeight = height / 50.f;
	m_startMin = Vector2(-width / 2.f, height / 2.f - m_textHeight);

	m_observation_pos = observation;
}

static QuickHull* fake_hull = nullptr;		// holder for mksi hull
static Vector3 net_disp;

// stat control of gjk algorithm
static eGJKStatus gjk_stat = GJK_FIND_SUPP_INITIAL;
static float gjk_closest_dist;

// support point
static Vector3 gjk_supp;
static Mesh* gjk_supp_mesh;

// support direction
static Line3 gjk_supp_dir;
static Mesh* gjk_supp_dir_mesh;

// gjk simplex
static std::set<Vector3> gjk_simplex;
static eGJKSimplex gjk_simplex_stat= GJK_SIMPLEX_NONE;
static Mesh* gjk_simplex_mesh =nullptr;

// gjk normal base
static Mesh* gjk_normal_base_mesh = nullptr;
static Vector3 gjk_normal_base;
static Vector3 gjk_last_normal_base;

// epa stat
static eEPAStat epa_stat = EPA_DELETE_GJK_REF;

// epa helper
static Mesh* epa_face_centroid;
static float epa_threshold = 1.f;	// hardcoded threshold, could be adjusted
static float epa_close_dist;

// epa simplex
static sEPASimplex* epa_simplex;
static sEPAFace* epa_close_face;

// epa support data
static Mesh* epa_support_anchor;
static Mesh* epa_support_line;
static Mesh* epa_support_pt;
static Vector3 epa_support_pt_pos;
static Vector3 epa_last_support_pt_pos = Vector3::ZERO;		// might be wrong in the corner case where the origin lies ON one of the verts of hull
static Vector3 epa_support_anchor_pos;

void ControlGroup::ProcessInput()
{
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_C))
	{
		for (GameObject* go : m_gos)
			go->m_debugOn = !go->m_debugOn;
	}

	GameObject* g0 = m_gos[0];
	GameObject* g1 = m_gos[1];

	if (g0->GetEntity() != nullptr)
	{
		if (g_input->IsKeyDown(InputSystem::KEYBOARD_UP_ARROW))
			g0->GetEntity()->SetLinearVelocity(Vector3(0.f, 0.f, 5.f));
		else if (g_input->IsKeyDown(InputSystem::KEYBOARD_DOWN_ARROW))
			g0->GetEntity()->SetLinearVelocity(Vector3(0.f, 0.f, -5.f));
		else if (g_input->IsKeyDown(InputSystem::KEYBOARD_LEFT_ARROW))
			g0->GetEntity()->SetLinearVelocity(Vector3(-5.f, 0.f, 0.f));
		else if (g_input->IsKeyDown(InputSystem::KEYBOARD_RIGHT_ARROW))
			g0->GetEntity()->SetLinearVelocity(Vector3(5.f, 0.f, 0.f));
		else if (g_input->IsKeyDown(InputSystem::KEYBOARD_PAGEUP))
			g0->GetEntity()->SetLinearVelocity(Vector3(0.f, 5.f, 0.f));
		else if (g_input->IsKeyDown(InputSystem::KEYBOARD_PAGEDOWN))
			g0->GetEntity()->SetLinearVelocity(Vector3(0.f, -5.f, 0.f));
		else
			g0->GetEntity()->SetLinearVelocity(Vector3::ZERO);

		Rigidbody3* rb0 = static_cast<Rigidbody3*>(g0->GetEntity());
		if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_1))
			rb0->SetAngularVelocity(Vector3(30.f, 0.f, 0.f));
		else if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_4))
			rb0->SetAngularVelocity(Vector3(0.f, 30.f, 0.f));
		else if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_7))
			rb0->SetAngularVelocity(Vector3(0.f, 0.f, 30.f));
		else
			rb0->SetAngularVelocity(Vector3::ZERO);
	}
	else
	{
		// for line control
		Line* line = dynamic_cast<Line*>(g0);
		if (line != nullptr)
		{
			if (g_input->IsKeyDown(InputSystem::KEYBOARD_UP_ARROW))
			{
				line->m_start += Vector3(0.f, 0.f, .01f);
				if (line->m_renderable->m_mesh != nullptr)
				{
					delete line->m_renderable->m_mesh;
					line->m_renderable->m_mesh = nullptr;
				}
				line->m_renderable->m_mesh = Mesh::CreateLineImmediate(VERT_PCU, line->m_start, line->m_end, Rgba::WHITE);
			}
			else if (g_input->IsKeyDown(InputSystem::KEYBOARD_DOWN_ARROW))
			{
				line->m_start -= Vector3(0.f, 0.f, .01f);
				if (line->m_renderable->m_mesh != nullptr)
				{
					delete line->m_renderable->m_mesh;
					line->m_renderable->m_mesh = nullptr;
				}
				line->m_renderable->m_mesh = Mesh::CreateLineImmediate(VERT_PCU, line->m_start, line->m_end, Rgba::WHITE);
			}
			else if (g_input->IsKeyDown(InputSystem::KEYBOARD_LEFT_ARROW))
			{
				line->m_start -= Vector3(.01f, 0.f, 0.f);
				if (line->m_renderable->m_mesh != nullptr)
				{
					delete line->m_renderable->m_mesh;
					line->m_renderable->m_mesh = nullptr;
				}
				line->m_renderable->m_mesh = Mesh::CreateLineImmediate(VERT_PCU, line->m_start, line->m_end, Rgba::WHITE);
			}
			else if (g_input->IsKeyDown(InputSystem::KEYBOARD_RIGHT_ARROW))
			{
				line->m_start += Vector3(.01f, 0.f, 0.f);
				if (line->m_renderable->m_mesh != nullptr)
				{
					delete line->m_renderable->m_mesh;
					line->m_renderable->m_mesh = nullptr;
				}
				line->m_renderable->m_mesh = Mesh::CreateLineImmediate(VERT_PCU, line->m_start, line->m_end, Rgba::WHITE);
			}
			else if (g_input->IsKeyDown(InputSystem::KEYBOARD_PAGEUP))
			{
				line->m_start += Vector3(0.f, .01f, 0.f);
				if (line->m_renderable->m_mesh != nullptr)
				{
					delete line->m_renderable->m_mesh;
					line->m_renderable->m_mesh = nullptr;
				}
				line->m_renderable->m_mesh = Mesh::CreateLineImmediate(VERT_PCU, line->m_start, line->m_end, Rgba::WHITE);
			}
			else if (g_input->IsKeyDown(InputSystem::KEYBOARD_PAGEDOWN))
			{
				line->m_start -= Vector3(0.f, .01f, 0.f);
				if (line->m_renderable->m_mesh != nullptr)
				{
					delete line->m_renderable->m_mesh;
					line->m_renderable->m_mesh = nullptr;
				}
				line->m_renderable->m_mesh = Mesh::CreateLineImmediate(VERT_PCU, line->m_start, line->m_end, Rgba::WHITE);
			}
		}

		// the object may be a hull, handle it here
		HullObject* hull = dynamic_cast<HullObject*>(g0);
		if (hull != nullptr)
		{
			if (g_input->IsKeyDown(InputSystem::KEYBOARD_UP_ARROW))
			{
				Vector3 pos = hull->m_renderable->m_transform.GetLocalPosition();
				pos += Vector3(0.f, 0.f, 1.f);
				hull->m_renderable->m_transform.SetLocalPosition(pos);
				net_disp = Vector3(0.f, 0.f, 1.f);
			}
			if (g_input->IsKeyDown(InputSystem::KEYBOARD_DOWN_ARROW))
			{
				Vector3 pos = hull->m_renderable->m_transform.GetLocalPosition();
				pos -= Vector3(0.f, 0.f, 1.f);
				hull->m_renderable->m_transform.SetLocalPosition(pos);
				net_disp = -Vector3(0.f, 0.f, 1.f);
			}
			if (g_input->IsKeyDown(InputSystem::KEYBOARD_LEFT_ARROW))
			{
				Vector3 pos = hull->m_renderable->m_transform.GetLocalPosition();
				pos -= Vector3(1.f, 0.f, 0.f);
				hull->m_renderable->m_transform.SetLocalPosition(pos);
				net_disp = -Vector3(1.f, 0.f, 0.f);
			}
			if (g_input->IsKeyDown(InputSystem::KEYBOARD_RIGHT_ARROW))
			{
				Vector3 pos = hull->m_renderable->m_transform.GetLocalPosition();
				pos += Vector3(1.f, 0.f, 0.f);
				hull->m_renderable->m_transform.SetLocalPosition(pos);
				net_disp = Vector3(1.f, 0.f, 0.f);
			}
			if (g_input->IsKeyDown(InputSystem::KEYBOARD_PAGEUP))
			{
				Vector3 pos = hull->m_renderable->m_transform.GetLocalPosition();
				pos += Vector3(0.f, 1.f, 0.f);
				hull->m_renderable->m_transform.SetLocalPosition(pos);
				net_disp = Vector3(0.f, 1.f, 0.f);
			}
			if (g_input->IsKeyDown(InputSystem::KEYBOARD_PAGEDOWN))
			{
				Vector3 pos = hull->m_renderable->m_transform.GetLocalPosition();
				pos -= Vector3(0.f, 1.f, 0.f);
				hull->m_renderable->m_transform.SetLocalPosition(pos);
				net_disp = -Vector3(0.f, 1.f, 0.f);
			}
			if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_1))
			{
				Vector3 rot = hull->m_renderable->m_transform.GetLocalRotation();
				rot += Vector3(1.f, 0.f, 0.f);
				hull->m_renderable->m_transform.SetLocalRotation(rot);
			}
			if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_4))
			{
				Vector3 rot = hull->m_renderable->m_transform.GetLocalRotation();
				rot += Vector3(0.f, 1.f, 0.f);
				hull->m_renderable->m_transform.SetLocalRotation(rot);
			}
			if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_7))
			{
				Vector3 rot = hull->m_renderable->m_transform.GetLocalRotation();
				rot += Vector3(0.f, 0.f, 1.f);
				hull->m_renderable->m_transform.SetLocalRotation(rot);
			}
		}
	}

	if (g1->GetEntity() != nullptr)
	{
		if (g_input->IsKeyDown(InputSystem::KEYBOARD_I))
			g1->GetEntity()->SetLinearVelocity(Vector3(0.f, 0.f, 5.f));
		else if (g_input->IsKeyDown(InputSystem::KEYBOARD_K))
			g1->GetEntity()->SetLinearVelocity(Vector3(0.f, 0.f, -5.f));
		else if (g_input->IsKeyDown(InputSystem::KEYBOARD_J))
			g1->GetEntity()->SetLinearVelocity(Vector3(-5.f, 0.f, 0.f));
		else if (g_input->IsKeyDown(InputSystem::KEYBOARD_L))
			g1->GetEntity()->SetLinearVelocity(Vector3(5.f, 0.f, 0.f));
		else if (g_input->IsKeyDown(InputSystem::KEYBOARD_U))
			g1->GetEntity()->SetLinearVelocity(Vector3(0.f, 5.f, 0.f));
		else if (g_input->IsKeyDown(InputSystem::KEYBOARD_O))
			g1->GetEntity()->SetLinearVelocity(Vector3(0.f, -5.f, 0.f));
		else
			g1->GetEntity()->SetLinearVelocity(Vector3::ZERO);

		Rigidbody3* rb1 = static_cast<Rigidbody3*>(g1->GetEntity());
		if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_2))
			rb1->SetAngularVelocity(Vector3(30.f, 0.f, 0.f));
		else if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_5))
			rb1->SetAngularVelocity(Vector3(0.f, 30.f, 0.f));
		else if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_8))
			rb1->SetAngularVelocity(Vector3(0.f, 0.f, 30.f));
		else 
			rb1->SetAngularVelocity(Vector3::ZERO);
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_0))
	{
		// first generate the minkowski hull if the group is all about hull object
		if (m_id == CONTROL_HULL_HULL)
		{
			if (fake_hull == nullptr)
			{
				HullObject* hull_0 = dynamic_cast<HullObject*>(g0);
				HullObject* hull_1 = dynamic_cast<HullObject*>(g1);

				QuickHull* qh_0 = hull_0->GetHullPrimitive();
				QuickHull* qh_1 = hull_1->GetHullPrimitive();

				fake_hull = QuickHull::GenerateMinkowskiHull(qh_0, qh_1);
				//std::vector<Vector3>& verts = fake_hull->m_vertices;
				//for each (const Vector3& vert in verts)
				//	DebugRenderPoint(1000.f, 20.f, vert, Rgba::GREEN, Rgba::GREEN, DEBUG_RENDER_USE_DEPTH);
			}
		}
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_3))
	{
		if (fake_hull != nullptr)
		{
			switch (gjk_stat)
			{
			case GJK_FIND_SUPP_INITIAL:
			{
				gjk_supp = fake_hull->GetRandomPt();
				if (gjk_supp_mesh != nullptr)
				{
					delete gjk_supp_mesh;
					gjk_supp_mesh= nullptr;
				}
				gjk_supp_mesh = Mesh::CreatePointImmediate(VERT_PCU, gjk_supp, Rgba::RED);

				// update simplex
				gjk_simplex.insert(gjk_supp);
				gjk_simplex_stat = GJK_UpdateSimplex(gjk_simplex);
				if (gjk_simplex_mesh != nullptr)
				{
					delete gjk_simplex_mesh;
					gjk_simplex_mesh = nullptr;
				}
				gjk_simplex_mesh = GJK_CreateSimplexMesh(gjk_simplex, gjk_simplex_stat);

				gjk_stat = GJK_FIND_DIRECTION_INITIAL;
			}
				break;
			case GJK_FIND_DIRECTION_INITIAL:
			{
				// given the supp point, find the direction to start with iterations
				gjk_supp_dir = Line3(gjk_supp, Vector3::ZERO);
				if (gjk_supp_dir_mesh != nullptr)
				{
					delete gjk_supp_dir_mesh;
					gjk_supp_dir_mesh = nullptr;
				}
				gjk_supp_dir_mesh = Mesh::CreateLineImmediate(VERT_PCU, gjk_supp,Vector3::ZERO, Rgba::CYAN);
				
				gjk_stat = GJK_FIND_SUPP;
			}
				break;
			case GJK_FIND_SUPP:
			{
				gjk_supp = GJK_FindSupp(fake_hull, gjk_supp_dir);
				if (gjk_supp_mesh != nullptr)
				{
					delete gjk_supp_mesh;
					gjk_supp_mesh = nullptr;
				}
				gjk_supp_mesh = Mesh::CreatePointImmediate(VERT_PCU, gjk_supp, Rgba::RED);

				// update simplex
				std::vector<Vector3> gjk_simplex_snapshot = ConvertToVectorFromSet(gjk_simplex);
				gjk_simplex.insert(gjk_supp);
				gjk_simplex_stat = GJK_UpdateSimplex(gjk_simplex);
				if (gjk_simplex_mesh != nullptr)
				{
					delete gjk_simplex_mesh;
					gjk_simplex_mesh = nullptr;
				}
				gjk_simplex_mesh = GJK_CreateSimplexMesh(gjk_simplex, gjk_simplex_stat);

				// TODO: when the simplex is a tetrahedron, we check if the origin is in it; if yes we abort and the two hulls intersect
				// there may be a better early out check on this using an IsPointContainedHull(Vector3::ZERO)
				if (gjk_simplex.size() == 4)
				{
					// Note: this also means that when gjk ends the simplex is mostly a tetrahedron? (not a triangle or line)
					// when it is triangle it is most when the origin is outside the mksi_hull
					GJK3SimplexTetra tetra = GJK3SimplexTetra(gjk_simplex);
					if (tetra.IsPointInTetra(Vector3::ZERO))
					{
						gjk_closest_dist = -INFINITY;
						gjk_stat = GJK_COMPLETE;
					}
					else
						gjk_stat = GJK_UPDATE_MIN_NORMAL;
				}
				else
					gjk_stat = GJK_UPDATE_MIN_NORMAL;
			}
				break;
			case GJK_UPDATE_MIN_NORMAL:
			{
				// find the min normal of current simplex
				float dist;
				gjk_normal_base= GJK_FindMinNormalBase(gjk_simplex, gjk_simplex_stat, dist);

				// there is a chance where finding the normal base would reduce the simplex
				if (gjk_simplex_mesh != nullptr)
				{
					delete gjk_simplex_mesh;
					gjk_simplex_mesh = nullptr;
				}
				gjk_simplex_mesh = GJK_CreateSimplexMesh(gjk_simplex, gjk_simplex_stat);

				if (gjk_normal_base_mesh != nullptr)
				{
					delete gjk_normal_base_mesh;
					gjk_normal_base_mesh = nullptr;
				}
				gjk_normal_base_mesh = Mesh::CreatePointImmediate(VERT_PCU, gjk_normal_base, Rgba::YELLOW);

				// if the normal base remains the same, gjk ends
				if (gjk_normal_base == gjk_last_normal_base)
				{
					gjk_closest_dist = dist;
					gjk_stat = GJK_COMPLETE;
				}
				else
				{
					gjk_last_normal_base = gjk_normal_base;
					gjk_stat = GJK_FIND_DIRECTION;
				}
			}
				break;
			case GJK_FIND_DIRECTION:
			{
				gjk_supp_dir = Line3(gjk_normal_base, Vector3::ZERO);
				if (gjk_supp_dir_mesh != nullptr)
				{
					delete gjk_supp_dir_mesh;
					gjk_supp_dir_mesh= nullptr;
				}
				gjk_supp_dir_mesh = Mesh::CreateLineImmediate(VERT_PCU, gjk_normal_base,Vector3::ZERO, Rgba::CYAN);

				gjk_stat = GJK_FIND_SUPP;
			}
				break;
			case GJK_COMPLETE:
			{

			}
				break;
			default:
				break;
			}
		}
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_6))
	{
		if (fake_hull != nullptr && gjk_stat == GJK_COMPLETE)
		{
			switch (epa_stat)
			{
			case EPA_DELETE_GJK_REF:
			{
				if (gjk_supp_mesh != nullptr)
				{
					delete gjk_supp_mesh;
					gjk_supp_mesh = nullptr;
				}
				if (gjk_supp_dir_mesh != nullptr)
				{
					delete gjk_supp_dir_mesh;
					gjk_supp_dir_mesh = nullptr;
				}
				if (gjk_simplex_mesh != nullptr)
				{
					delete gjk_simplex_mesh;
					gjk_simplex_mesh = nullptr;
				}
				if (gjk_normal_base_mesh != nullptr)
				{
					delete gjk_normal_base_mesh;
					gjk_normal_base_mesh = nullptr;
				}

				epa_stat = EPA_CREATE_SIMPLEX;
			}
				break;
			case EPA_CREATE_SIMPLEX:
			{
				epa_simplex = new sEPASimplex(gjk_simplex);

				epa_stat = EPA_FIND_FACE;
			}
				break;
			case EPA_FIND_FACE:
			{
				//sEPAFace* close_face = epa_simplex->SelectClosestFaceToOrigin();
				epa_close_face = epa_simplex->SelectClosestFaceToOrigin();
				//float to_face;
				//epa_support_anchor_pos = ProjectPointToPlane(Vector3::ZERO, close_face->m_verts[0], close_face->m_verts[1], close_face->m_verts[2], to_face);
				epa_support_anchor_pos = ProjectPointToPlane(Vector3::ZERO, epa_close_face->m_verts[0], epa_close_face->m_verts[1], epa_close_face->m_verts[2], epa_close_dist);
				//if (epa_threshold == 0.f)
				//	epa_threshold = to_face;		// lower bound of the epa algorithm, could be adjusted later

				Vector3 centroid = (epa_close_face->m_verts[0] + epa_close_face->m_verts[1] + epa_close_face->m_verts[2]) / 3.f;
				if (epa_face_centroid != nullptr)
				{
					delete epa_face_centroid;
					epa_face_centroid = nullptr;
				}
				epa_face_centroid = Mesh::CreatePointImmediate(VERT_PCU, centroid, Rgba::GREEN);

				if (epa_support_anchor != nullptr)
				{
					delete epa_support_anchor;
					epa_support_anchor = nullptr;
				}
				epa_support_anchor = Mesh::CreatePointImmediate(VERT_PCU, epa_support_anchor_pos, Rgba::BLUE);

				if (epa_support_line != nullptr)
				{
					delete epa_support_line;
					epa_support_line = nullptr;
				}
				epa_support_line = Mesh::CreateLineImmediate(VERT_PCU, Vector3::ZERO, epa_support_anchor_pos, Rgba::CYAN);
				
				// finally, the support point should be on the hull
				Line3 epa_support_dir = Line3(Vector3::ZERO, epa_support_anchor_pos);
				epa_support_pt_pos = EPA_FindSupp(fake_hull, epa_support_dir);

				// if the support point remains the same, we've reached end
				if (epa_support_pt_pos != epa_last_support_pt_pos)
					epa_last_support_pt_pos = epa_support_pt_pos;
				else
					epa_stat = EPA_COMPLETE;

				if (epa_stat != EPA_COMPLETE)
				{
					// IMPORTANT: got the newest support point, will abort if dist from this point to origin
					// MINUS the threshold is smaller than the distance from closest face to origin
					if (((epa_support_pt_pos - Vector3::ZERO).GetLength() - epa_threshold) < epa_close_dist)
						epa_stat = EPA_COMPLETE;
					else 
					{
						if (epa_support_pt != nullptr)
						{
							delete epa_support_pt;
							epa_support_pt = nullptr;
						}
						epa_support_pt = Mesh::CreatePointImmediate(VERT_PCU, epa_support_pt_pos, Rgba::RED);

						epa_stat = EPA_DELETE_VISIBLE;
					}
				}
			}
				break;
			case EPA_DELETE_VISIBLE:
			{
				bool deleted = epa_simplex->DeleteVisibleFacesForPt(epa_support_pt_pos);

				// delete old debug draws
				if (epa_support_anchor != nullptr)
				{
					delete epa_support_anchor;
					epa_support_anchor = nullptr;
				}
				if (epa_support_line != nullptr)
				{
					delete epa_support_line;
					epa_support_line = nullptr;
				}
				if (epa_face_centroid != nullptr)
				{
					delete epa_face_centroid;
					epa_face_centroid = nullptr;
				}
				
				epa_stat = EPA_FORM_NEW_FACE;
			}
				break;
			case EPA_FORM_NEW_FACE:
			{
				// generate new faces between the supp point and edges with 1 ref count
				std::vector<sEPAEdgeRef*> new_edges;
				for (sEPAEdgeRef* edge : epa_simplex->m_edge_refs)
				{
					if (edge->ref_count == 1)
						epa_simplex->FormNewFace(epa_support_pt_pos, edge, epa_support_anchor_pos, new_edges);
				}
				epa_simplex->AppendNewEdges(new_edges);
				new_edges.clear();

				//std::vector<sEPAEdgeRef*> new_edges_non_dup;
				//for (std::vector<sEPAEdgeRef*>::size_type idx = 0; idx < new_edges.size(); ++idx)
				//{

				//}

				epa_stat = EPA_FIND_FACE;
			}
				break;
			case EPA_COMPLETE:
			{
				// in the step of completion, release all unnecessary heap memory
				if (epa_face_centroid != nullptr)
				{
					delete epa_face_centroid;
					epa_face_centroid = nullptr;
				}

				if (epa_support_anchor != nullptr)
				{
					delete epa_support_anchor;
					epa_support_anchor = nullptr;
				}

				if (epa_support_line != nullptr)
				{
					delete epa_support_line;
					epa_support_line = nullptr;
				}

				if (epa_support_pt != nullptr)
				{
					delete epa_support_pt;
					epa_support_pt = nullptr;
				}

				epa_stat = EPA_POST_COMPLETE;
			}
				break;
			case EPA_POST_COMPLETE:
			{

			}
				break;
			default:
				break;
			}
		}
	}
}

static Mesh* obb3_obb3_pt_pos = nullptr;
static Mesh* obb3_obb3_face_center = nullptr;
void ControlGroup::RenderCore(Renderer* renderer)
{
	for (std::vector<GameObject*>::size_type idx = 0; idx < m_gos.size(); ++idx)
	{
		if (!m_gos[idx]->m_isInForwardPath)
			m_gos[idx]->Render(renderer);
	}

	if (m_id == CONTROL_HULL_HULL)
	{
		if (fake_hull != nullptr)
			fake_hull->RenderHull(renderer);

		// gjk
		if (gjk_supp_mesh != nullptr)
			DrawPoint(gjk_supp_mesh);

		if (gjk_supp_dir_mesh != nullptr)
			DrawLine(gjk_supp_dir_mesh);

		if (gjk_simplex_mesh != nullptr)
			GJK_DrawSimplex(gjk_simplex_mesh, gjk_simplex_stat);

		if (gjk_normal_base_mesh != nullptr)
			DrawPoint(gjk_normal_base_mesh, 20.f);

		// epa
		if (epa_simplex != nullptr)
			epa_simplex->Draw(renderer);

		if (epa_support_anchor != nullptr)
			DrawPoint(epa_support_anchor);

		if (epa_support_line != nullptr)
			DrawLine(epa_support_line);

		if (epa_support_pt != nullptr)
			DrawPoint(epa_support_pt);

		if (epa_face_centroid != nullptr)
			DrawPoint(epa_face_centroid);
	}
}

void ControlGroup::RenderUI()
{
	DrawTexts(m_view);
}

void ControlGroup::Update(float deltaTime)
{
	// clear all contacts at beginning of frame
	m_contacts.clear();

	ProcessInput();

	for (GameObject* go : m_gos)
		go->Update(deltaTime);

	// update collision
	switch (m_id)
	{
	case CONTROL_SPHERE_SPHERE:
	{
		Contact3 contact;

		Sphere* s1 = static_cast<Sphere*>(m_gos[0]);
		Sphere* s2 = static_cast<Sphere*>(m_gos[1]);

		SphereRB3* rb1 = static_cast<SphereRB3*>(s1->GetEntity());
		SphereRB3* rb2 = static_cast<SphereRB3*>(s2->GetEntity());

		const Sphere3& sph1 = rb1->m_primitive;
		const Sphere3& sph2 = rb2->m_primitive;

		bool intersected = CollisionDetector::Sphere3VsSphere3Core(sph1, sph2, contact);
		if (intersected)
			m_contacts.push_back(contact);
	}
		break;
	case CONTROL_SPHERE_PLANE:
	{
		Contact3 contact;

		Sphere* s = static_cast<Sphere*>(m_gos[0]);
		Quad* q = static_cast<Quad*>(m_gos[1]);

		SphereRB3* rbs = static_cast<SphereRB3*>(s->GetEntity());
		QuadRB3* rbq = static_cast<QuadRB3*>(q->GetEntity());

		const Sphere3& sph = rbs->m_primitive;
		const Plane& pl = rbq->m_primitive;

		bool intersected = CollisionDetector::Sphere3VsPlane3Core(sph, pl, contact);
		if (intersected)
			m_contacts.push_back(contact);
	}
		break;
	case CONTROL_BOX_PLANE:
	{
		Contact3 contact;
		
		Box* b = static_cast<Box*>(m_gos[0]);
		Quad* q = static_cast<Quad*>(m_gos[1]);

		BoxRB3* rbb = static_cast<BoxRB3*>(b->GetEntity());
		QuadRB3* rbq = static_cast<QuadRB3*>(q->GetEntity());

		const OBB3& obb = rbb->m_primitive;
		const Plane& pl = rbq->m_primitive;

		bool intersected = CollisionDetector::OBB3VsPlane3Core(obb, pl, contact);
		if (intersected)
			m_contacts.push_back(contact);
	}
		break;
	case CONTROL_BOX_SPHERE:
	{
		Contact3 contact;
		
		Box* b = static_cast<Box*>(m_gos[0]);
		Sphere* s = static_cast<Sphere*>(m_gos[1]);

		BoxRB3* rbb = static_cast<BoxRB3*>(b->GetEntity());
		SphereRB3* rbs = static_cast<SphereRB3*>(s->GetEntity());

		const OBB3& obb = rbb->m_primitive;
		const Sphere3& sph = rbs->m_primitive;

		bool intersected = CollisionDetector::OBB3VsSphere3Core(obb, sph, contact);
		if (intersected)
			m_contacts.push_back(contact);
	}
		break;
	case CONTROL_BOX_BOX:
	{
		Contact3 contact;

		Box* b_0 = static_cast<Box*>(m_gos[0]);
		Box* b_1 = static_cast<Box*>(m_gos[1]);

		BoxRB3* rbb_0 = static_cast<BoxRB3*>(b_0->GetEntity());
		BoxRB3* rbb_1 = static_cast<BoxRB3*>(b_1->GetEntity());

		const OBB3& obb_0 = rbb_0->m_primitive;
		const OBB3& obb_1 = rbb_1->m_primitive;

		//TODO("contact point is not correct - it is set to center of an entity, see Core for detail");
		//bool intersected = CollisionDetector::OBB3VsOBB3Core(obb_0, obb_1, contact);
		//if (intersected)
		//	m_contacts.push_back(contact);

		//// debug
		//Vector3 vert_pos;
		//Vector3 face_center;
		//CollisionDetector::OBB3VsOBB3CoreBreakdownPtVsFace(obb_0, obb_1, vert_pos, face_center);
		//if (obb3_obb3_pt_pos != nullptr)
		//{
		//	delete obb3_obb3_pt_pos;
		//	obb3_obb3_pt_pos = nullptr;
		//}
		//obb3_obb3_pt_pos = Mesh::CreatePointImmediate(VERT_PCU, vert_pos, Rgba::MEGENTA);
		//if (obb3_obb3_face_center != nullptr)
		//{
		//	delete obb3_obb3_face_center;
		//	obb3_obb3_face_center = nullptr;
		//}
		//obb3_obb3_face_center = Mesh::CreatePointImmediate(VERT_PCU, face_center, Rgba::BLUE);

		CollisionDetector::OBB3VsOBB3StepOne(obb_0, obb_1);
		CollisionDetector::OBB3VsOBB3StepTwo(obb_0, obb_1);
		CollisionDetector::OBB3VsOBB3StepThree(obb_0, obb_1);
	}
		break;
	case CONTROL_LINE_LINE:
	{

	}
		break;
	case CONTROL_HULL_HULL:
	{

	}
		break;
	default:
		break;
	}

	// fake hull
	if (fake_hull != nullptr)
	{
		// we need to update transform here for the fake_hull because it does not have a gameobject container
		fake_hull->m_ref_pos += net_disp;

		// update vertices
		for (std::vector<Vector3>::size_type idx = 0; idx < fake_hull->m_vertices.size(); ++idx)
			fake_hull->m_vertices[idx] += net_disp;
		
		net_disp = Vector3::ZERO;
	}

	UpdateUI();

	UpdateDebugDraw();
}

void ControlGroup::UpdateDebugDraw()
{
	for (const Contact3& contact : m_contacts)
	{
		Vector3 point = contact.m_point;
		Vector3 end = point + contact.m_normal * contact.m_penetration;
		DebugRenderLine(0.1f, point, end, 5.f, Rgba::BLUE, Rgba::BLUE, DEBUG_RENDER_USE_DEPTH);
	}
}

void ControlGroup::UpdateUI()
{
	Renderer* renderer = Renderer::GetInstance();
	BitmapFont* font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");

	DeleteVector(m_view);

	switch (m_id)
	{
	case CONTROL_SPHERE_SPHERE:
	{
		Vector2 min = m_startMin;

		// basic info
		std::string cp_title = "Sphere v.s sphere";
		Mesh* mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, cp_title, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		std::string contact_num = Stringf("Contact number: %i", m_contacts.size());
		mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, contact_num, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		// detailed contact info
		if (m_contacts.size() > 0U)
		{
			for (int i = 0; i < m_contacts.size(); ++i)
			{
				const Contact3& theContact = m_contacts[i];
				std::string contact_info = Stringf("Contact at (%f, %f, %f), has normal (%f, %f, %f), with penetration %f",
					theContact.m_point.x, theContact.m_point.y, theContact.m_point.z,
					theContact.m_normal.x, theContact.m_normal.y, theContact.m_normal.z, theContact.m_penetration);
				mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, contact_info, VERT_PCU);
				m_view.push_back(mesh);
				min -= Vector2(0.f, m_textHeight);
			}
		}
	}
		break;
	case CONTROL_SPHERE_PLANE:
	{
		Vector2 min = m_startMin;

		// basic info
		std::string cp_title = "Sphere v.s plane";
		Mesh* mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, cp_title, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		std::string contact_num = Stringf("Contact number: %i", m_contacts.size());
		mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, contact_num, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		if (m_contacts.size() > 0U)
		{
			for (int i = 0; i < m_contacts.size(); ++i)
			{
				const Contact3& theContact = m_contacts[i];
				std::string contact_info = Stringf("Contact at (%f, %f, %f), has normal (%f, %f, %f), with penetration %f",
					theContact.m_point.x, theContact.m_point.y, theContact.m_point.z,
					theContact.m_normal.x, theContact.m_normal.y, theContact.m_normal.z, theContact.m_penetration);
				mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, contact_info, VERT_PCU);
				m_view.push_back(mesh);
				min -= Vector2(0.f, m_textHeight);
			}
		}
	}
		break;
	case CONTROL_BOX_PLANE:
	{
		Vector2 min = m_startMin;

		std::string cp_title = "Box v.s plane";
		Mesh* mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, cp_title, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		std::string contact_num = Stringf("Contact number: %i", m_contacts.size());
		mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, contact_num, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		if (m_contacts.size() > 0U)
		{
			for (int i = 0; i < m_contacts.size(); ++i)
			{
				const Contact3& theContact = m_contacts[i];
				std::string contact_info = Stringf("Contact at (%f, %f, %f), has normal (%f, %f, %f), with penetration %f",
					theContact.m_point.x, theContact.m_point.y, theContact.m_point.z,
					theContact.m_normal.x, theContact.m_normal.y, theContact.m_normal.z, theContact.m_penetration);
				mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, contact_info, VERT_PCU);
				m_view.push_back(mesh);
				min -= Vector2(0.f, m_textHeight);
			}
		}
	}
		break;
	case CONTROL_BOX_SPHERE:
	{
		Vector2 min = m_startMin;

		std::string cp_title = "Box v.s sphere";
		Mesh* mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, cp_title, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		std::string contact_num = Stringf("Contact number: %i", m_contacts.size());
		mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, contact_num, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		if (m_contacts.size() > 0U)
		{
			for (int i = 0; i < m_contacts.size(); ++i)
			{
				const Contact3& theContact = m_contacts[i];
				std::string contact_info = Stringf("Contact at (%f, %f, %f), has normal (%f, %f, %f), with penetration %f",
					theContact.m_point.x, theContact.m_point.y, theContact.m_point.z,
					theContact.m_normal.x, theContact.m_normal.y, theContact.m_normal.z, theContact.m_penetration);
				mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, contact_info, VERT_PCU);
				m_view.push_back(mesh);
				min -= Vector2(0.f, m_textHeight);
			}
		}
	}
		break;
	case CONTROL_BOX_BOX:
	{
		Vector2 min = m_startMin;

		std::string cp_title = "Box v.s box";
		Mesh* mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, cp_title, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		std::string contact_num = Stringf("Contact number: %i", m_contacts.size());
		mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, contact_num, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		for (int i = 0; i < m_contacts.size(); ++i)
		{
			const Contact3& theContact = m_contacts[i];
			std::string contact_info = Stringf("Contact at (%f, %f, %f), has normal (%f, %f, %f), with penetration %f",
				theContact.m_point.x, theContact.m_point.y, theContact.m_point.z,
				theContact.m_normal.x, theContact.m_normal.y, theContact.m_normal.z, theContact.m_penetration);
			mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, contact_info, VERT_PCU);
			m_view.push_back(mesh);
			min -= Vector2(0.f, m_textHeight);
		}
	}
		break;
	case CONTROL_LINE_LINE:
	{
		Vector2 min = m_startMin;

		std::string cp_title = "Line v.s line";
		Mesh* mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, cp_title, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		// for convenience, update closest point here directly so that we do not need class variables for 
		Line* l_1 = static_cast<Line*>(m_gos[0]);
		const Vector3& start_1 = l_1->m_start;
		const Vector3& end_1 = l_1->m_end;
		LineSegment3 seg_1 = LineSegment3(start_1, end_1);

		Line* l_2 = static_cast<Line*>(m_gos[1]);
		const Vector3& start_2 = l_2->m_start;
		const Vector3& end_2 = l_2->m_end;
		LineSegment3 seg_2 = LineSegment3(start_2, end_2);

		Vector3 close_1, close_2;
		float t1, t2;
		float close_dist_sqr = LineSegment3::ClosestPointsSegments(seg_1, seg_2, t1, t2, close_1, close_2);
		DebugRenderLine(0.1f, close_1, close_2, 5.f, Rgba::BLUE, Rgba::BLUE, DEBUG_RENDER_USE_DEPTH);

		Vector3 normal = close_1 - close_2;
		float dot1 = DotProduct(seg_1.extent, normal);
		float dot2 = DotProduct(seg_2.extent, normal);

		std::string close_name1 = Stringf("Closest point on line 1: %f, %f, %f", close_1.x, close_1.y, close_1.z);
		mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, close_name1, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		std::string close_name2 = Stringf("Closest point on line 2: %f, %f, %f", close_2.x, close_2.y, close_2.z);
		mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, close_name2, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		std::string dot_str1 = Stringf("Close vector dot with line 1: %f", dot1);
		mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, dot_str1, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		std::string dot_str2 = Stringf("Close vector dot with line 2: %f", dot2);
		mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, dot_str2, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);
	}
		break;
	case CONTROL_HULL_HULL:
	{
		Vector2 min = m_startMin;

		std::string cp_title = "Hull v.s Hull";
		Mesh* mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, cp_title, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		// get closest distance and show it if gjk is complete
		if (gjk_stat == GJK_COMPLETE)
		{
			std::string dist_str = std::to_string(gjk_closest_dist);
			mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, dist_str, VERT_PCU);
			m_view.push_back(mesh);
			min -= Vector2(0.f, m_textHeight);
		}

		// epa debug
		if (epa_stat >= EPA_DELETE_VISIBLE)
		{
			std::string face_str = std::to_string(epa_simplex->m_unordered_faces.size());
			face_str += " - number of face of EPA simplex";
			mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, face_str, VERT_PCU);
			m_view.push_back(mesh);
			min -= Vector2(0.f, m_textHeight);
		}

		if (epa_stat == EPA_POST_COMPLETE)
		{
			std::string complete_str = "EPA Complete!";
			mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, complete_str, VERT_PCU);
			m_view.push_back(mesh);
			min -= Vector2(0.f, m_textHeight);

			// normal is the negativity of triangle normal here, think about the inducing hull
			Vector3 n = epa_close_face->m_normal;
			n.Normalize();
			std::string normal_str = Stringf("The normal is: (%f, %f, %f)", n.x, n.y, n.z);
			mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, normal_str, VERT_PCU);
			m_view.push_back(mesh);
			min -= Vector2(0.f, m_textHeight);

			std::string pen_str = Stringf("The penetration depth is: %f", epa_close_dist);
			mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, pen_str, VERT_PCU);
			m_view.push_back(mesh);
			min -= Vector2(0.f, m_textHeight);
		}
	}
		break;
	default:
		break;
	}
}
