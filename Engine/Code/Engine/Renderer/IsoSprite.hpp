#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Core/Util/XmlUtilities.hpp"
#include "Engine/Renderer/Sprite.hpp"

#include <vector>

struct FaceInfo
{
	IntVector2 m_dir;
	IntVector2 m_scale;
	std::string m_src;
};

class IsoSprite
{
private:
	std::string m_id;
	std::vector<FaceInfo> m_info;

public:
	IsoSprite();
	~IsoSprite();

	void PopulateDataWithElement(const XMLElement& ele);

	std::string GetID() const { return m_id; }
	std::string GetSpriteSrc(IntVector2 faceDir);
	IntVector2 GetScale(IntVector2 faceDir);
};