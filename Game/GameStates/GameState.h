#pragma once
#include "GameStateBase.h"
#include <string>
class GameState : public GameStateBase
{
	std::wstring type();


public:
	bool isType(const std::wstring& type_);
	virtual std::wstring getType();


	GameState();
	GameState(const GameState&);
	GameState(GameState&&);
	GameState& operator=(const GameState&);
	GameState& operator=(GameState&&);
	virtual ~GameState();
};