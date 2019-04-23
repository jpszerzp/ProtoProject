#pragma once

#include "Game/Physics3State.hpp"
#include <vector>

class Renderer;
class InputSystem;
class TheGame;
class AudioSystem;
class SpriteSheet;

extern Renderer* g_renderer;
extern InputSystem* g_input;
extern TheGame* g_theGame;
extern Physics3State* g_physState;
extern AudioSystem* g_audio;
extern SpriteSheet* g_atlas;
extern SpriteSheet* g_archer;
extern SpriteSheet* g_mage;
extern SpriteSheet* g_warrior;

enum eTeam
{
	TEAM_PLAYER,
	TEAM_ENEMY,
	TEAM_NEUTRAL,
	NUM_OF_TEAM
};
