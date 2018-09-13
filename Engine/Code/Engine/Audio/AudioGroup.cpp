/*
#include "Engine/Audio/AudioGroup.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

void AudioGroup::AddSound(const std::string path)
{
	// create sound
	AudioSystem* audio = AudioSystem::GetInstance();

	SoundID sound = audio->CreateOrGetSound(path);

	m_soundIDs.push_back(sound);
}

void AudioGroup::PlayGroupOneOffInOrder()
{
	AudioSystem* audio = AudioSystem::GetInstance();

	for each (SoundID id in m_soundIDs)
	{
		SoundPlaybackID playbackID;
		if (!m_channelBusy)
		{
			playbackID = audio->PlaySound(id);	
			m_channelBusy = true;
		}

		FMOD::Channel* channel = (FMOD::Channel*) playbackID;
		channel->setCallback(AudioGroup::IndicateClipStopTimeCB);
	}
}


FMOD_RESULT F_CALLBACK AudioGroup::IndicateClipStopTimeCB(FMOD_CHANNELCONTROL* channelControl,
	FMOD_CHANNELCONTROL_TYPE controlType,
	FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType,
	void* commanData1, void* commanData2)
{
	if (controlType == FMOD_CHANNELCONTROL_CHANNEL &&
		callbackType == FMOD_CHANNELCONTROL_CALLBACK_END){    

		FMOD::Channel* channel = (FMOD::Channel*)channelControl;

		DebuggerPrintf("Song ends\n");

		m_channelBusy = false;
	}

	return FMOD_OK;
}
*/