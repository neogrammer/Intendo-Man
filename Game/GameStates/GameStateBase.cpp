#include "pch.h"
#include "GameStateBase.h"

GameStateBase::GameStateBase()
{
}

GameStateBase::GameStateBase(const GameStateBase&)
{
}

GameStateBase::GameStateBase(GameStateBase&&)
{
}

GameStateBase& GameStateBase::operator=(const GameStateBase&)
{
	return *this;
}

GameStateBase& GameStateBase::operator=(GameStateBase&&)
{
	return *this;
}

GameStateBase::~GameStateBase()
{
}
