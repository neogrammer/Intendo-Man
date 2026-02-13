#include "pch.h"
#include "PlayState.h"

#include "../Resources/Cfg.h"
#include "../../Engine/ActionMap.h"
#include "../../Engine/Camera2D.h"
#include "../../Engine/Renderer2D.h"
#include "../../Engine/Text.h"
#include "../Objects/Player.h"
#include "../Map/Tilemap.h"
#include "../Systems/PhysicsSys.h"

namespace game
{
	using winrt::Windows::Foundation::Numerics::float2;

	std::wstring PlayState::type()
	{
		return L"PlayState";
	}

	void game::PlayState::enter()
	{
		Cfg::PlayMusicAsync(L"theme", true, 0.25f);
		uiStrings.clear();

		// --- Player (AnimObject)
		// You can load from a file:
		// player = std::make_unique<game::AnimObject>(L"Assets\\Anims\\Player.anm");
		// ...or load from text (handy while iterating):
		player = std::make_unique<game::Player>();

		const std::wstring shipTestAnm = LR"(
# Minimal test anim that points at the existing Ship texture.
[object]
position = 450 450
start_anim = idle

[anim idle]
texture        = ship
frame_size     = 481 611
start_col      = 0
start_row      = 0
start_px       = 0 0
pitch          = 1
frames         = 1
uni_directional= true

offsets = (0,0)
sizes   = (481,611)
delays  = 0.10

looping    = true
loop_wait  = false
loop_delay = 0
)";

		tmap = std::make_unique<game::Tilemap>(Cfg::Textures::Tileset1, winrt::Windows::Foundation::Numerics::float2{ 40.f,40.f }, 16, 256);
		tmap->loadTileset(L"ms-appx:///Assets/Datas/Tilesets/tileset2.tst");
		tmap->loadTilemap(L"ms-appx:///Assets/Datas/Tilemaps/tilemap1.map");

		//player->LoadFromAnmText(shipTestAnm);

		// --- HUD
		engine::Text m_hud{};

		m_hud.FontRef = Cfg::GetFont(L"bubbly");
		m_hud.String = L"Cool Text Bitches!";
		m_hud.FontSize = 22.0f;
		m_hud.OutlineThickness = 2;
		m_hud.OutlineColor = winrt::Windows::UI::Colors::White();
		m_hud.Color = winrt::Windows::UI::Colors::Green();
		m_hud.Position = { 10.0f, 10.0f };
		m_hud.Invalidate();

		uiStrings.push_back(m_hud);




	}

	void PlayState::exit()
	{
		tmap.reset();
		tmap = nullptr;

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
		if (player)
		{
			player->Update(dt_);
		}

		if (actMap && player)
		{
			float playerPrevYVel{ 0.f };
			float playerAccumGrav{ 0.f };
			static bool movedDownYet{ false };
			static float amt = 0.f;
			amt = 0.f;
			// Player movement (MoveAxis = WASD + left stick)
			float2 move = actMap->MoveAxis();
			
			float playerSpeed = 300.f;

			auto v = tmap->getSolidTilesOnScreen(*camera);
			std::vector<game::GameObject*> tiles{};
			for (auto& tile : v)
			{
				tiles.push_back(tile);
			}
	
			phys::trustFall(*player, tiles);

			if (player->isGrounded())
			{
				playerAccumGrav = 0;
			}
			else
			{
				if (playerAccumGrav > 0.0001f)
					playerAccumGrav += ((playerAccumGrav + 988.88f) * dt_ * dt_);
				else
					playerAccumGrav = 11.8f;
			}

			


			auto& ptr = *dynamic_cast<Player*>(player.get());

			if (move.x == -1 && player->IsFacingRight())
			{
				

					if (ptr.GetAnimName() == game::Player::AnimName::Idle || ptr.GetAnimName() == game::Player::AnimName::Idle_Blink)
					{
						ptr.SetAnim(game::Player::AnimName::Run);
					}
				
				player->SetFacingRight(false);

			}
			else if (move.x == 1 && !player->IsFacingRight())
			{


				if (ptr.GetAnimName() == game::Player::AnimName::Idle || ptr.GetAnimName() == game::Player::AnimName::Idle_Blink)
				{
					ptr.SetAnim(game::Player::AnimName::Run);
				}

				player->SetFacingRight(true);

			}
			else if (move.x == 1 && player->IsFacingRight())
			{


				if (ptr.GetAnimName() != game::Player::AnimName::Run)
				{
					ptr.SetAnim(game::Player::AnimName::Run);
				}

				player->SetFacingRight(true);

			}
			else if (move.x == -1 && !player->IsFacingRight())
			{


				if (ptr.GetAnimName() != game::Player::AnimName::Run)
				{
					ptr.SetAnim(game::Player::AnimName::Run);
				}

				player->SetFacingRight(false);

			}
			else if (move.x == 0 && (ptr.GetAnimName() != game::Player::AnimName::Idle && ptr.GetAnimName() != game::Player::AnimName::Idle_Blink))
			{
				ptr.SetAnim(game::Player::AnimName::Idle);
			}



			player->Move(float2{ move.x * playerSpeed * dt_, (move.y * playerSpeed * dt_) + playerAccumGrav });

			
			
			
			phys::handleCollisions(*player, tiles);
			
		
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

		float camHalfW = camera->getWidth() * 0.5f;
		float camHalfH = camera->getHeight() * 0.5f;

		float leftBound = camera->Position.x - camHalfW + (camera->getWidth() / 3.f);
		float rightBound = camera->Position.x + camHalfW - (camera->getWidth() / 3.f);

		// Follow player with user pan offset (use center of collider box)
		if (player)
		{
			auto const pos = player->GetWorldPosition();
			auto const size = player->GetWorldSize();

			float playerCenterX = pos.x + (size.x * 0.5f);

			float camHalfW = camera->getWidth() * 0.5f;
			float camHalfH = camera->getHeight() * 0.5f;

			float leftBound = camera->Position.x - camHalfW + (camera->getWidth() / 3.f);
			float rightBound = camera->Position.x + camHalfW - (camera->getWidth() / 3.f);

			float newCamX = camera->Position.x;

			// Only move if outside middle third
			if (playerCenterX < leftBound)
			{
				newCamX -= (leftBound - playerCenterX);
			}
			else if (playerCenterX > rightBound)
			{
				newCamX += (playerCenterX - rightBound);
			}

			// Clamp to tilemap
			float worldWidth = tmap->getPitch() * tmap->getTileSize().x;

			newCamX = std::min<float>(
				std::max<float>(newCamX, camHalfW),
				worldWidth - camHalfW
			);

			camera->Position = { newCamX + cameraOffset.x, camHalfH + cameraOffset.y };
		}
		else
		{
			camera->Position = { camHalfW, camHalfH };
		}
	}

	void PlayState::syncObjects()
	{
		if (player)
		{
			player->SyncToBase();
		}
	}

	std::vector<engine::Text>& PlayState::render(engine::Renderer2D& renderer_)
	{
		tmap->render(renderer_, *camera);

		//uiStrings[0].String = L"Player X = " + std::to_wstring(player->GetWorldPosition().x)

		if (player)
		{
			renderer_.Draw(*player->getSprite());
		}

		return uiStrings;
	}

	float PlayState::getTmapTileHeight()
	{
		return tmap->getTileSize().y;
	}

	PlayState::PlayState()
		: GameState{}
		, player{ nullptr }
	{
	}

	PlayState::~PlayState()
	{
	}
}
