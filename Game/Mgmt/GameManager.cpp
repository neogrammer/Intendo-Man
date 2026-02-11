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

	void GameManager::SyncObjects()
	{
		gStateMgr->SyncObjects();
	}

	std::vector<engine::Text>& GameManager::render(engine::Renderer2D& renderer_)
	{
		// Make sure any AnimObjects have pushed their current frame into the base GameObject values
		// before the state renders.
		SyncObjects();
		return gStateMgr->render(renderer_);
	}
}
