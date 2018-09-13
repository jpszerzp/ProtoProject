#pragma once


//-----------------------------------------------------------------------------------------------
#include "ThirdParty/fmod/fmod.hpp"
#include <string>
#include <vector>
#include <map>


//-----------------------------------------------------------------------------------------------
typedef size_t SoundID;
typedef size_t SoundGroupID;
typedef size_t SoundPlaybackID;
constexpr size_t MISSING_SOUND_ID = (size_t)(-1); // for bad SoundIDs and SoundPlaybackIDs
constexpr size_t MISSING_GROUP_ID = (size_t)(-1);

//-----------------------------------------------------------------------------------------------
class AudioSystem;

/////////////////////////////////////////////////////////////////////////////////////////////////
class AudioSystem
{
public:
	AudioSystem();
	virtual ~AudioSystem();

public:
	static AudioSystem*			GetInstance();
	static void					DestroyInstance();

	virtual void				BeginFrame();
	virtual void				EndFrame();
	
	//virtual void				SetSoundGroup(const std::string& path);
	virtual SoundID				CreateOrGetSound( const std::string& soundFilePath );
	virtual SoundGroupID		CreateOrGetSoundGroup(const std::string& path);
	virtual SoundPlaybackID		PlaySound( SoundID soundID, bool isLooped=false, float volume=1.f, float balance=0.0f, float speed=1.0f, bool isPaused=false );
	virtual void				PlaySoundGroup(SoundGroupID id);
	virtual void				StopSound( SoundPlaybackID soundPlaybackID );
	virtual void				SetSoundPlaybackVolume( SoundPlaybackID soundPlaybackID, float volume );	// volume is in [0,1]
	virtual void				SetSoundPlaybackBalance( SoundPlaybackID soundPlaybackID, float balance );	// balance is in [-1,1], where 0 is L/R centered
	virtual void				SetSoundPlaybackSpeed( SoundPlaybackID soundPlaybackID, float speed );		// speed is frequency multiplier (1.0 == normal)

	virtual void				ValidateResult( FMOD_RESULT result );

public:
	FMOD::System*						m_fmodSystem;
	std::map< std::string, SoundID >	m_registeredSoundIDs;
	std::vector< FMOD::Sound* >			m_registeredSounds;
	std::map<std::string, SoundGroupID> m_registeredGroupIDs;
	std::vector<FMOD::SoundGroup*>		m_registeredGroups;

	static AudioSystem*					m_audioInstance;
};

