#include "pch.h"
#include "PlayState.h"
#include "../Resources/Cfg.h"
#include "../../Engine/ActionMap.h"
#include "../../Engine/Camera2D.h"
#include "../../Engine/Renderer2D.h"
#include "../../Engine/Text.h"
#include "../../Engine/Sprite.h"



namespace game
{

	std::wstring  PlayState::type()
	{
		// TODO: insert return statement here
		return L"PlayState";
	}

	void PlayState::enter()
	{
		Cfg::PlayMusicAsync(L"theme", true, 0.25f);
		uiStrings.clear();
		player = std::make_unique<engine::Sprite>(Cfg::GetTex(Cfg::Textures::Ship));
		player->Position = float2{ 450.0f, 450.0f };
		player->SetOriginCenter();

		
	}

	void PlayState::exit()
	{
		engine::SoundManager::Instance().StopMusic();
		uiStrings.clear();
		player.reset();
		player = nullptr;
	}

	void PlayState::processInput(const engine::ActionMap& actMap_)
	{
		   // do this in the playstate
		   if (actMap_.Pressed(engine::Action::ResetView))
		   {
			  camera->Reset();
			  cameraOffset = { 0,0 };
		   }

		   if (actMap_.Pressed(engine::Action::Fire))
		   {
				  Cfg::PlaySfx(L"blip");
		   }

		   actMap = &actMap_;
	}

	void PlayState::update(float dt_)
	{
		

		if (actMap)
		{
			//   // Player movement (MoveAxis = WASD + left stick)
			float2 move = actMap->MoveAxis();
			float playerSpeed = 300.0f;
			player->Position.x += move.x * playerSpeed * dt_;
			player->Position.y += move.y * playerSpeed * dt_;

			// Camera pan offset (PanAxis = arrows + right stick)
			float2 pan = actMap->PanAxis();
			float camPanSpeed = 450.0f;
			cameraOffset.x += pan.x * camPanSpeed * dt_;
			cameraOffset.y += pan.y * camPanSpeed * dt_;

			// Zoom (Q/E + triggers)
			float zoomAxis = actMap->ZoomAxis();
			float zoomRate = 1.25f;
			camera->Zoom *= (1.0f + zoomAxis * zoomRate * dt_);
			camera->Zoom = std::clamp<float>(camera->Zoom, 0.25f, 4.0f);

			// Rotate (Z/C + bumpers)
			float rotAxis = actMap->RotateAxis();
			float rotSpeed = 1.5f;
			camera->RotationRad += rotAxis * rotSpeed * dt_;

			actMap = nullptr;
		}

		// Follow player with user pan offset
		   camera->Position = { player->Position.x + cameraOffset.x, player->Position.y + cameraOffset.y };
		   player->Rotation = dt_ * 0.5f;

  
}

	std::vector<engine::Text>& PlayState::render(engine::Renderer2D& renderer_)
	{

		renderer_.Draw(*player);


		engine::Text m_hud{};

		m_hud.FontRef = Cfg::GetFont(L"bubbly");
		m_hud.String = L"Space/X: SFX";
		m_hud.FontSize = 22.0f;
		m_hud.OutlineThickness = 2;
		m_hud.OutlineColor = winrt::Windows::UI::Colors::White();
		m_hud.Color = winrt::Windows::UI::Colors::Green();
		m_hud.Position = { 10.0f, 10.0f };
		m_hud.Invalidate();

		uiStrings.push_back(m_hud);
		
		return uiStrings;
	}

	PlayState::PlayState()
		: GameState{}
		, player{nullptr}
	{
	}

	PlayState::PlayState(const PlayState&)
	{
	}

	PlayState::PlayState(PlayState&&)
	{
	}

	PlayState& PlayState::operator=(const PlayState&)
	{
		// TODO: insert return statement here
		return *this;
	}

	PlayState& PlayState::operator=(PlayState&&)
	{
		// TODO: insert return statement here
		return *this;
	}

	PlayState::~PlayState()
	{
	}
}
