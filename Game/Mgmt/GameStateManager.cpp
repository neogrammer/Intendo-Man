#include "pch.h"
#include "GameStateManager.h"
#include "../Resources/Cfg.h"
#include "../../Engine/ActionMap.h"
#include "../../Engine/Camera2D.h"
#include "../../Engine/SpriteBatchScope.h"
#include "../../Engine/Text.h"
#include "game_states.h"

namespace game
{

	GameStateManager::GameStateManager()
		: gameStates{}
		, currState{ nullptr }
	{
		gameStates.emplace(std::pair<std::wstring, std::unique_ptr<GameState>>{ L"StartState", std::make_unique<game::StartState>() });
		gameStates.emplace(std::pair<std::wstring, std::unique_ptr<GameState>>{L"TitleState", std::make_unique<game::TitleState>()});
		gameStates.emplace(std::pair<std::wstring, std::unique_ptr<GameState>>{L"PlayState", std::make_unique<game::PlayState>()});
		gameStates.emplace(std::pair<std::wstring, std::unique_ptr<GameState>>{L"GameOverState", std::make_unique<game::GameOverState>()});
		gameStates.emplace(std::pair<std::wstring, std::unique_ptr<GameState>>{L"StageSelectState", std::make_unique<game::StageSelectState>()});
		gameStates.emplace(std::pair<std::wstring, std::unique_ptr<GameState>>{L"MenuState", std::make_unique<game::MenuState>()});

		currState = gameStates.at(L"PlayState").get();
		currState->enter();

	}

	GameStateManager::~GameStateManager()
	{
		currState->exit();
	}

	void GameStateManager::processInput(const engine::ActionMap& actMap_)
	{
		currState->processInput(actMap_);




	}

	std::shared_ptr<engine::Camera2D> GameStateManager::getCurrentCamera()
	{
		return currState->getCamera();
	}

	void GameStateManager::update(float dt_)
	{
		currState->update(dt_);
	}


	void GameStateManager::SyncObjects()
	{
		if (currState)
		{
			currState->syncObjects();
		}
	}

	std::vector<engine::Text>& GameStateManager::render(engine::SpriteBatchScope const& batch_)
	{
		return currState->render(batch_);
	}
	float2 GameStateManager::getCamOffset()
	{
		return currState->getCamOffset();
	}
	bool GameStateManager::hasTmap()
	{
		return (currState->getType() == L"PlayState");
	}
	float GameStateManager::getTmapTileHeight()
	{
		auto* tmp = dynamic_cast<PlayState*>(currState);
		if (!tmp) { return 1.f; }

		return tmp->getTmapTileHeight();
	}
}