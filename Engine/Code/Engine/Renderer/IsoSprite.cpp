#include "Engine/Renderer/IsoSprite.hpp"
#include "Engine/Renderer/Renderer.hpp"


IsoSprite::IsoSprite()
{

}


IsoSprite::~IsoSprite()
{

}


void IsoSprite::PopulateDataWithElement(const XMLElement& ele)
{
	m_id = ParseXmlAttribute(ele, "id", "");

	for(const XMLElement* theEle = ele.FirstChildElement(); theEle != 0; theEle = theEle->NextSiblingElement())
	{
		std::string val = theEle->Value();
		if (val.compare("facing") == 0)
		{
			IntVector2 faceDir = ParseXmlAttribute(*theEle, "dir", IntVector2(0, 0));
			IntVector2 scale = ParseXmlAttribute(*theEle, "scale", IntVector2(1, 1));
			std::string id = ParseXmlAttribute(*theEle, "src", "");

			FaceInfo fInfo = FaceInfo();
			fInfo.m_dir = faceDir;
			fInfo.m_scale = scale;
			fInfo.m_src = id;

			m_info.push_back(fInfo);
		}
	}
}


std::string IsoSprite::GetSpriteSrc(IntVector2 faceDir)
{
	std::string res;

	for (std::vector<FaceInfo>::size_type faceInfoCount = 0; faceInfoCount < m_info.size(); ++faceInfoCount)
	{
		FaceInfo finfo = m_info[faceInfoCount];

		if (finfo.m_dir == faceDir)
		{
			res = finfo.m_src;
			break;
		}
	}

	return res;
}


IntVector2 IsoSprite::GetScale(IntVector2 faceDir)
{
	for (std::vector<FaceInfo>::size_type faceInfoCount = 0; faceInfoCount < m_info.size(); ++faceInfoCount)
	{
		FaceInfo finfo = m_info[faceInfoCount];

		if (finfo.m_dir == faceDir)
		{
			return finfo.m_scale;
		}
	}

	return IntVector2(0, 0);
}