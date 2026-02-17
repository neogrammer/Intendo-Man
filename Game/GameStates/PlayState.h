#pragma once

#include "GameState.h"
#include "../Objects/AnimObject.h"

#include "../Resources/Cfg.h"
#include <memory>

#include <winrt/Windows.Foundation.Numerics.h>
#include "../Objects/BusterShot.h"
#include <array>

namespace engine
{
	class ActionMap;
	class Camera2D;
	struct Text;
	class Renderer2D;
	
}



namespace game
{

	template<typename To, typename From>
	std::unique_ptr<To> dynamic_unique_cast(std::unique_ptr<From>&& src) noexcept {
		if (auto casted = dynamic_cast<To*>(src.get())) {
			src.release(); // release ownership from src
			return std::unique_ptr<To>(casted);
		}
		return nullptr; // cast failed
	}

	class Tilemap;

	class PlayState : public game::GameState
	{
		std::wstring type() override final;

		std::unique_ptr<game::AnimObject> player{ nullptr };
		std::unique_ptr<game::Tilemap> tmap{ nullptr };
	
	public:

		std::array<game::BusterShot, 3> m_busterShots{};
		float m_busterCooldown{ 0.0f };

		void enter() override final;
		void exit() override final;

		void processInput(const engine::ActionMap& actMap_) override final;
		void update(float dt_) override final;

		// Called right before rendering (GameManager::SyncObjects)
		void syncObjects() override final;

		std::vector<engine::Text>& render(engine::Renderer2D& renderer_) override final;
		float getTmapTileHeight();

		PlayState();
		PlayState(const PlayState&) = delete;

		PlayState& operator=(const PlayState&) = delete;
		PlayState(PlayState&&) = default;
		PlayState& operator=(PlayState&&) = default;
		~PlayState() override final;
	};
}
