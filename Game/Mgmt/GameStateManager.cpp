#include "pch.h"
#include "GameStateManager.h"
#include "../Resources/Cfg.h"
#include "../../Engine/ActionMap.h"
#include "../../Engine/Camera2D.h"
#include "../../Engine/Renderer2D.h"
#include "../../Engine/Text.h"
#include "game_states.h"

namespace game
{

	GameStateManager::GameStateManager()
	{
		pState = new game::PlayState{};
		pState->enter();
		currState = pState;

	}

	GameStateManager::GameStateManager(const GameStateManager&)
	{
	}

	GameStateManager::GameStateManager(GameStateManager&&)
	{
	}

	GameStateManager& GameStateManager::operator=(const GameStateManager&)
	{
		// TODO: insert return statement here
		return *this;
	}

	GameStateManager& GameStateManager::operator=(GameStateManager&&)
	{
		// TODO: insert return statement here
		return *this;
	}

	GameStateManager::~GameStateManager()
	{
		currState->exit();

		if (pState)
			delete pState;
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
		if (currState->isType(L"PlayState"))
		{
			Cfg::debugPrint(L"Winner");
			//std::wcout << L"Winner!" << std::endl;
		}
		else
		{
			Cfg::debugPrint(L"No Dice!");
		}

		currState->update(dt_);
	}

	std::vector<engine::Text>& GameStateManager::render(engine::Renderer2D& renderer_)
	{
		return currState->render(renderer_);
	}
	float2 GameStateManager::getCamOffset()
	{
		return currState->getCamOffset();
	}
}