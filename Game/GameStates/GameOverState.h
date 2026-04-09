#pragma once
#include "GameState.h"
#include "../../Engine/Sprite.h"


namespace engine
{
	class ActionMap;
	class Camera2D;
	struct Text;
	class SpriteBatchScope;
}
namespace game
{
	class ActionMap;
	class GameOverState : public GameState
	{
		std::wstring type() override final;


	public:

		void enter() override final;
		void exit() override final;

		void processInput(const engine::ActionMap& actMap_) override final;
		void update(float dt_) override final;
		std::vector<engine::Text>& render(engine::SpriteBatchScope const& batch_) override final;


		GameOverState();
		GameOverState(const GameOverState&) = delete;

		GameOverState& operator=(const GameOverState&) = delete;
		GameOverState(GameOverState&&) = default;
		GameOverState& operator=(GameOverState&&) = default;
		~GameOverState() override final;
	};
}