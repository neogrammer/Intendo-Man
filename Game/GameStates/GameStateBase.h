#pragma once


class GameStateBase
{
public:
	GameStateBase();
	GameStateBase(const GameStateBase&) = delete;

	GameStateBase& operator=(const GameStateBase&) = delete;
	GameStateBase(GameStateBase&&) = default;
	GameStateBase& operator=(GameStateBase&&) = default;
	~GameStateBase();
};