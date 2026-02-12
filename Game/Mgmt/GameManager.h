#pragma once
#include "../../Engine/InputState.h"
#include "../../Engine/ActionMap.h"
#include "../../Engine/Camera2D.h"
#include "../../Engine/Renderer2D.h"
#include "../../Engine/Text.h"
#include "../../Engine/Matrix2D.h"
#include <winrt/Windows.Foundation.Numerics.h>
#include <cmath>
#include <memory>



namespace game
{
	class GameStateManager;

	class GameManager
	{
		std::unique_ptr<GameStateManager> gStateMgr{ nullptr };

	public:
		GameManager();
		GameManager(const GameManager&) = delete;
		GameManager(GameManager&&) = default;
		GameManager& operator=(const GameManager&) = delete;
		GameManager& operator=(GameManager&&) = default;
		~GameManager();

		void processInput(const engine::ActionMap& actMap_);
		std::shared_ptr<engine::Camera2D> getCamera();
		void update(float dt_);
		void SyncObjects();
		std::vector<engine::Text>& render(engine::Renderer2D& renderer_);

		bool hasTmap();
		float getTmapTileHeight();
	};
}