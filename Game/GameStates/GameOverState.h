#pragma once
#include "GameState.h"

class GameOverState : public GameState
{
	std::wstring type();

public:
	std::wstring getType() override final;

	GameOverState();
	GameOverState(const GameOverState&);
	GameOverState(GameOverState&&);
	GameOverState& operator=(const GameOverState&);
	GameOverState& operator=(GameOverState&&);
	~GameOverState() override final;
};