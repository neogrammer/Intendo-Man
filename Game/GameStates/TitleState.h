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

	class TitleState : public GameState
	{
		std::wstring type() override final;
	public:
		void enter() override final;
		void exit() override final;

		void processInput(const engine::ActionMap& actMap_) override final;
		void update(float dt_) override final;
		std::vector<engine::Text>& render(engine::Renderer2D& renderer_) override final;
		TitleState();
		TitleState(const TitleState&) = delete;

		TitleState& operator=(const TitleState&) = delete;
		TitleState(TitleState&&) = default;
		TitleState& operator=(TitleState&&) = default;
		~TitleState() override final;
	};
}