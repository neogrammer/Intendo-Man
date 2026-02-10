#pragma once


class GameStateBase
{
public:
	GameStateBase();
	GameStateBase(const GameStateBase&);
	GameStateBase(GameStateBase&&);
	GameStateBase& operator=(const GameStateBase&);
	GameStateBase& operator=(GameStateBase&&);
	~GameStateBase();
};