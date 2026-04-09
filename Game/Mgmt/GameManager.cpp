#include "pch.h"
#include "GameManager.h"
#include "GameStateManager.h"
#include "../../Engine/SpriteBatchScope.h"
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
		if (!gStateMgr)
			gStateMgr = std::make_unique<GameStateManager>();



		// pass actions down
		gStateMgr->processInput(actMap_);


	}
	std::shared_ptr<engine::Camera2D> GameManager::getCamera()
	{
		if (!gStateMgr)
			gStateMgr = std::make_unique<GameStateManager>();

		return gStateMgr->getCurrentCamera();
	}
	void GameManager::update(float dt_)
	{
		if (!gStateMgr)
			gStateMgr = std::make_unique<GameStateManager>();

		gStateMgr->update(dt_);
	}

	void GameManager::SyncObjects()
	{

		if (!gStateMgr)
			gStateMgr = std::make_unique<GameStateManager>();

		gStateMgr->SyncObjects();
	}

	std::vector<engine::Text>& GameManager::render(engine::SpriteBatchScope const& batch_)
	{

		if (!gStateMgr)
			gStateMgr = std::make_unique<GameStateManager>();
		// Make sure any AnimObjects have pushed their current frame into the base GameObject values
		// before the state renders.
		SyncObjects();
		return gStateMgr->render(batch_);
	}

	bool GameManager::hasTmap()
	{

		if (!gStateMgr)
			gStateMgr = std::make_unique<GameStateManager>();
		return (gStateMgr->hasTmap());
	}

	float GameManager::getTmapTileHeight()
	{

		if (!gStateMgr)
			gStateMgr = std::make_unique<GameStateManager>();
		return gStateMgr->getTmapTileHeight();
	}
}

