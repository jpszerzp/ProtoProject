/*
#pragma once

#include "Engine/Audio/AudioSystem.hpp"
#include <vector>

class AudioGroup
{
public:
	AudioGroup(){}
	AudioGroup(char* idx)
		: m_id(idx){}
	~AudioGroup(){}

	void AddSound(const std::string path);
	void PlayGroupOneOffInOrder();

	// call back
	static FMOD_RESULT F_CALLBACK IndicateClipStopTimeCB(FMOD_CHANNELCONTROL*channelControl,
		FMOD_CHANNELCONTROL_TYPE controlType,
		FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType,
		void* commanData1, void* commanData2);

public:
	char* m_id;
	static bool  m_channelBusy;

	std::vector<SoundID> m_soundIDs;
};
*/