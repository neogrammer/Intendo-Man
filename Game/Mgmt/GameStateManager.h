#pragma once
#include "GameManager.h"
#include <unordered_map>
#include <string>
namespace engine
{
	class ActionMap;
	class Camera2D;
	struct Text;
	class Renderer2D;
}

namespace game
{
	using winrt::Windows::Foundation::Numerics::float2;
	using winrt::Windows::Foundation::Numerics::float3x2;

	class GameState;
	class PlayState;


	class GameStateManager
	{
		std::unordered_map<std::wstring, std::unique_ptr<game::GameState>> gameStates;
		game::GameState* currState;
	public:
		GameStateManager();
		GameStateManager(const GameStateManager&) = delete;
		GameStateManager(GameStateManager&&) = default;
		GameStateManager& operator=(const GameStateManager&) = delete;
		GameStateManager& operator=(GameStateManager&&) = default;
		~GameStateManager();

		void processInput(const engine::ActionMap& actMap_);
		std::shared_ptr<engine::Camera2D> getCurrentCamera();


		void update(float dt_);
		void SyncObjects();
		std::vector<engine::Text>& render(engine::Renderer2D& renderer_);
		float2 getCamOffset();

		bool hasTmap();
		float getTmapTileHeight();

	};
}