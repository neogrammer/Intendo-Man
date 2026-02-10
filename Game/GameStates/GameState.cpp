#include "pch.h"
#include "GameState.h"
#include <iostream>

#include "../Game/Resources/Cfg.h"

std::wstring GameState::type()
{
	return L"GameState";
}

bool GameState::isType(const std::wstring& type_)
{
	Cfg::debugPrint(L"getType() " + getType() + L" : testType " + type_ );

	return (type_ == getType());
}

std::wstring GameState::getType()
{
	return L"GameState";
}

GameState::GameState()
	: GameStateBase{}
{
}

GameState::GameState(const GameState&)
{
}

GameState::GameState(GameState&&)
{
}

GameState& GameState::operator=(const GameState&)
{
	// TODO: insert return statement here
	return *this;
}

GameState& GameState::operator=(GameState&&)
{
	// TODO: insert return statement here
	return *this;
}

GameState::~GameState()
{
}
