#pragma once

#include "GameState.h"

class PlayState : public GameState
{
	std::wstring type();
public:
	std::wstring getType() override final;
	PlayState();
	PlayState(const PlayState&);
	PlayState(PlayState&&);
	PlayState& operator=(const PlayState&);
	PlayState& operator=(PlayState&&);
	~PlayState() override final;
};