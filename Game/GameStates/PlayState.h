#pragma once

#include "GameState.h"

namespace engine
{
	class ActionMap;
	class Camera2D;
	struct Text;
	class Renderer2D;
	struct Sprite;
}

namespace game
{
	class ActionMap;


	class PlayState : public game::GameState
	{
		std::wstring type() override final;
		std::unique_ptr<engine::Sprite> player{nullptr};

	public:
		void enter() override final;
		void exit() override final;

		void processInput(const engine::ActionMap& actMap_) override final;
		void update(float dt_) override final;
		std::vector<engine::Text>& render(engine::Renderer2D& renderer_) override final;

		PlayState();
		PlayState(const PlayState&);
		PlayState(PlayState&&);
		PlayState& operator=(const PlayState&);
		PlayState& operator=(PlayState&&);
		~PlayState() override final;
	};
}