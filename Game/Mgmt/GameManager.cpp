#include "pch.h"
#include "GameManager.h"
#include "GameStateManager.h"

using namespace engine;

namespace game 
{

	GameManager::GameManager()
	{
        gStateMgr = std::make_unique<GameStateManager>();
	}

	GameManager::GameManager(const GameManager&)
	{
	}

	GameManager::GameManager(GameManager&&)
	{
	}

	GameManager& GameManager::operator=(const GameManager&)
	{
		// TODO: insert return statement here
		return *this;
	}

	GameManager& GameManager::operator=(GameManager&&)
	{
		// TODO: insert return statement here
		return *this;
	}

	GameManager::~GameManager()
	{
	}
	void GameManager::processInput(const engine::ActionMap& actMap_)
	{




        // pass actions down
        gStateMgr->processInput(actMap_);


	}
	std::shared_ptr<engine::Camera2D> GameManager::getCamera()
	{
		return gStateMgr->getCurrentCamera();
	}
	void GameManager::update(float dt_)
	{
		gStateMgr->update(dt_);
	}
	std::vector<engine::Text>& GameManager::render(engine::Renderer2D& renderer_)
	{
		return gStateMgr->render(renderer_);
	}
}