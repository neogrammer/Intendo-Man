#pragma once

class GameStateManager
{
public:
	GameStateManager();
	GameStateManager(const GameStateManager&);
	GameStateManager(GameStateManager&&);
	GameStateManager& operator=(const GameStateManager&);
	GameStateManager& operator=(GameStateManager&&);
	~GameStateManager();
};