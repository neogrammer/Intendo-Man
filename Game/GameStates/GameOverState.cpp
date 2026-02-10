#include "pch.h"
#include "GameOverState.h"

std::wstring GameOverState::type()
{
	// TODO: insert return statement here
	return L"GameOverState";
}

std::wstring GameOverState::getType()
{
	// TODO: insert return statement here
	return  L"GameOverState";
}

GameOverState::GameOverState()
	: GameState{}
{
}

GameOverState::GameOverState(const GameOverState&)
{
}

GameOverState::GameOverState(GameOverState&&)
{
}

GameOverState& GameOverState::operator=(const GameOverState&)
{
	// TODO: insert return statement here
	return *this;
}

GameOverState& GameOverState::operator=(GameOverState&&)
{
	// TODO: insert return statement here
	return *this;
}

GameOverState::~GameOverState()
{
}
