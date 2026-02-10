#pragma once

class GameManager
{
public:
	GameManager();
	GameManager(const GameManager&);
	GameManager(GameManager&&);
	GameManager& operator=(const GameManager&);
	GameManager& operator=(GameManager&&);
	~GameManager();
};