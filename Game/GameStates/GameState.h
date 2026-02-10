#pragma once
#include "GameStateBase.h"
#include "../../Engine/Matrix2D.h"

#include <string>

namespace engine
{
	class ActionMap;
	class Camera2D;
	class Renderer2D;
	struct Text;

}

namespace game
{
	using winrt::Windows::Foundation::Numerics::float2;
	using winrt::Windows::Foundation::Numerics::float3x2;

	class GameState : public GameStateBase
	{
	protected:
		virtual std::wstring type();
		std::shared_ptr<engine::Camera2D> camera;
		float2 cameraOffset;
		std::vector<engine::Text> uiStrings;
		engine::ActionMap const * actMap{ nullptr };

	public:

		virtual void enter() = 0;
		virtual void exit() = 0;

		virtual void processInput(const engine::ActionMap& actMap_) = 0;
		virtual void update(float dt_) = 0;
		virtual std::vector<engine::Text>& render(engine::Renderer2D& renderer_) = 0;

		bool isType(const std::wstring& type_);
		std::wstring getType();


		GameState();
		GameState(const GameState&);

		GameState& operator=(const GameState&);

		virtual ~GameState();

		std::shared_ptr<engine::Camera2D> getCamera();
		float2 getCamOffset();
	};
}