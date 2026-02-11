#pragma once

#include "GameState.h"
#include "../Objects/AnimObject.h"

namespace engine
{
	class ActionMap;
	class Camera2D;
	struct Text;
	class Renderer2D;
}

namespace game
{
	class PlayState : public game::GameState
	{
		std::wstring type() override final;

		std::unique_ptr<game::AnimObject> player{ nullptr };

	public:
		void enter() override final;
		void exit() override final;

		void processInput(const engine::ActionMap& actMap_) override final;
		void update(float dt_) override final;

		// Called right before rendering (GameManager::SyncObjects)
		void syncObjects() override final;

		std::vector<engine::Text>& render(engine::Renderer2D& renderer_) override final;

		PlayState();
		PlayState(const PlayState&) = delete;

		PlayState& operator=(const PlayState&) = delete;
		PlayState(PlayState&&) = default;
		PlayState& operator=(PlayState&&) = default;
		~PlayState() override final;
	};
}
