#include "Engine/Math/HalfEdge.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Mesh.hpp"

#include <assert.h>

void HalfEdge::Draw(Renderer* renderer)
{
	// set up 
	Shader* shader = renderer->CreateOrGetShader("wireframe_color");
	renderer->UseShader(shader);
	Texture* texture = renderer->CreateOrGetTexture("Data/Images/white.png");
	renderer->SetTexture2D(0, texture);
	renderer->SetSampler2D(0, texture->GetSampler());
	glLineWidth(1.f);
	renderer->m_objectData.model = Matrix44::IDENTITY;
	renderer->m_currentShader->m_state.m_depthCompare = COMPARE_LESS;
	renderer->m_currentShader->m_state.m_cullMode = CULLMODE_BACK;
	renderer->m_currentShader->m_state.m_windOrder = WIND_COUNTER_CLOCKWISE;

	renderer->DrawMesh(m_body_mesh);
	
	for (int i = 0; i < 3; ++i)
		renderer->DrawMesh(m_arrow_mesh[i]);
}

Vector3 HalfEdge::GetHeadPos() const
{
	assert(m_next != nullptr);

	return m_next->m_tail;
}


void HalfEdge::FillBodyMesh()
{
	const Vector3& head = GetHeadPos();
	m_body_mesh = Mesh::CreateLineImmediate(VERT_PCU, m_tail, head, Rgba::RED);
}

void HalfEdge::FillArrowMeshes(Vector3 arrow_end[3])
{
	const Vector3& head = GetHeadPos();
	for (int i = 0; i < 3; ++i)
		m_arrow_mesh[i] = Mesh::CreateLineImmediate(VERT_PCU, head, arrow_end[i], Rgba::RED);
}

void HalfEdge::FillArrowMeshesOffset(Vector3 arrow_end[3], const Vector3& head)
{
	for (int i = 0; i < 3; ++i)
		m_arrow_mesh[i] = Mesh::CreateLineImmediate(VERT_PCU, head, arrow_end[i], Rgba::RED);
}

void HalfEdge::FillBodyMeshOffset(float percentage, const Vector3& centroid)
{
	Vector3 offset_head;
	Vector3 offset_tail;

	GetOffsetTailHead(percentage, centroid, offset_tail, offset_head);

	m_body_mesh = Mesh::CreateLineImmediate(VERT_PCU, offset_tail, offset_head, Rgba::RED);
}

void HalfEdge::CreateArrowMeshes()
{
	assert(m_next != nullptr);

	// draw a arrow
	const Vector3& tail = m_tail;
	const Vector3& head = m_next->m_tail;

	const Vector3& dir = (tail - head).GetNormalized();

	Vector3 used_aixs;
	Vector3 axis[3] = {Vector3(1.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f), Vector3(0.f, 0.f, 1.f)};
	float dot_0 = abs( DotProduct(axis[0], dir) );
	float dot_1 = abs( DotProduct(axis[1], dir) );
	float dot_2 = abs( DotProduct(axis[2], dir) );
	float dot_min = min(dot_0, min(dot_1, dot_2));

	if (dot_min == dot_0)
		used_aixs = axis[0];
	else if (dot_min == dot_1)
		used_aixs = axis[1];
	else if (dot_min == dot_2)
		used_aixs = axis[2];

	float ext = (tail - head).GetLength() * 0.1f;
	Vector3 ext_end = head + dir * ext;

	Vector3 arrow_end[3];
	Vector3 perpendicular_0 = dir.Cross(used_aixs);
	arrow_end[0] = ext_end + perpendicular_0;

	Vector3 local_dir = arrow_end[0] - head;
	local_dir = local_dir.RotateAboutAxisWithAngle(120.f, dir);
	arrow_end[1] = local_dir + head;
	local_dir = local_dir.RotateAboutAxisWithAngle(120.f, dir);
	arrow_end[2] = local_dir + head;

	FillArrowMeshes(arrow_end);
}

void HalfEdge::CreateArrowMeshesOffset(float percentage, const Vector3& centroid)
{
	Vector3 head;
	Vector3 tail;

	GetOffsetTailHead(percentage, centroid, tail, head);

	const Vector3& dir = (tail - head).GetNormalized();
	
	Vector3 used_aixs;
	Vector3 axis[3] = {Vector3(1.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f), Vector3(0.f, 0.f, 1.f)};
	float dot_0 = abs( DotProduct(axis[0], dir) );
	float dot_1 = abs( DotProduct(axis[1], dir) );
	float dot_2 = abs( DotProduct(axis[2], dir) );
	float dot_min = min(dot_0, min(dot_1, dot_2));

	if (dot_min == dot_0)
		used_aixs = axis[0];
	else if (dot_min == dot_1)
		used_aixs = axis[1];
	else if (dot_min == dot_2)
		used_aixs = axis[2];

	float ext = (tail - head).GetLength() * 0.1f;
	Vector3 ext_end = head + dir * ext;

	Vector3 arrow_end[3];
	Vector3 perpendicular_0 = dir.Cross(used_aixs);
	arrow_end[0] = ext_end + perpendicular_0;

	Vector3 local_dir = arrow_end[0] - head;
	local_dir = local_dir.RotateAboutAxisWithAngle(120.f, dir);
	arrow_end[1] = local_dir + head;
	local_dir = local_dir.RotateAboutAxisWithAngle(120.f, dir);
	arrow_end[2] = local_dir + head;

	FillArrowMeshesOffset(arrow_end, head);
}

void HalfEdge::GetOffsetTailHead(float percentage, const Vector3& centroid, Vector3& tail, Vector3& head)
{
	head = GetHeadPos();
	const Vector3& head_to_centroid = centroid - head;
	const Vector3& head_to_centroid_norm = head_to_centroid.GetNormalized();

	tail = m_tail;
	const Vector3& tail_to_centroid = centroid - tail;
	const Vector3& tail_to_centroid_norm = tail_to_centroid.GetNormalized();

	float head_to_centroid_length = head_to_centroid.GetLength() * percentage;
	float tail_to_centroid_length = tail_to_centroid.GetLength() * percentage;

	head = head + head_to_centroid_norm * head_to_centroid_length;
	tail = tail + tail_to_centroid_norm * tail_to_centroid_length;
}
