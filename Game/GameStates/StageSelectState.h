#pragma once

#include "GameState.h"

namespace engine
{
	class ActionMap;
	class Camera2D;
	struct Text;
	class Renderer2D;
}

namespace game
{
	class ActionMap;
	class StageSelectState : public GameState
	{
		std::wstring type() override final;

	public:
		void enter() override final;
		void exit() override final;

		void processInput(const engine::ActionMap& actMap_) override final;
		void update(float dt_) override final;
		std::vector<engine::Text>& render(engine::Renderer2D& renderer_) override final;

		StageSelectState();
		StageSelectState(const StageSelectState&);
		StageSelectState(StageSelectState&&);
		StageSelectState& operator=(const StageSelectState&);
		StageSelectState& operator=(StageSelectState&&);
		~StageSelectState() override final;
	};
}