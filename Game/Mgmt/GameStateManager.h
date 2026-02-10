#pragma once
#include "GameManager.h"

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
		game::GameState* currState;
	public:
		GameStateManager();
		GameStateManager(const GameStateManager&);
		GameStateManager(GameStateManager&&);
		GameStateManager& operator=(const GameStateManager&);
		GameStateManager& operator=(GameStateManager&&);
		~GameStateManager();

		void processInput(const engine::ActionMap& actMap_);
		std::shared_ptr<engine::Camera2D> getCurrentCamera();


		void update(float dt_);
		std::vector<engine::Text>& render(engine::Renderer2D& renderer_);

		game::PlayState* pState;
		float2 getCamOffset();
	};
}