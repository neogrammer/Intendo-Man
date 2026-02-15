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
            // --- Tunables
            constexpr float playerSpeed = 300.0f;

            constexpr float gravity = 1988.88f;     // px/s^2 (down)
            constexpr float jumpSpeed = 900.0f;     // px/s (up is negative)
            constexpr float jumpCutSpeed = 300.0f;  // release early clamps to this upward speed

            constexpr float coyoteMax = 0.10f;      // seconds
            constexpr float bufferMax = 0.10f;      // seconds

            // --- State
            static float velY = 0.0f;
            static bool  jumpCutApplied = false;
            static float coyoteTimer = 0.0f;
            static float jumpBufferTimer = 0.0f;

            // Input
            float2 move = actMap->MoveAxis();
            bool jumpPressed = actMap->Pressed(engine::Action::MoveUp);   // W / DPadUp
            bool jumpHeld = actMap->Down(engine::Action::MoveUp);
            bool jumpReleased = actMap->Released(engine::Action::MoveUp);

            // Camera pan (keep)
            float2 pan = actMap->PanAxis();
            constexpr float camPanSpeed = 450.0f;
            cameraOffset.x += pan.x * camPanSpeed * dt_;
            cameraOffset.y += pan.y * camPanSpeed * dt_;

            // Ground snapshot at start of frame
            bool wasGrounded = player->isGrounded();

            // Timers
            coyoteTimer = wasGrounded ? coyoteMax : std::max<float>(0.0f, coyoteTimer - dt_);
            jumpBufferTimer = jumpPressed ? bufferMax : std::max<float>(0.0f, jumpBufferTimer - dt_);

            if (wasGrounded)
            {
                velY = 0.0f;
                jumpCutApplied = false;
            }

            auto StartJump = [&](bool isHeldNow)
                {
                    velY = -jumpSpeed;
                    player->inAir();           // leave ground immediately
                    coyoteTimer = 0.0f;
                    jumpBufferTimer = 0.0f;
                    jumpCutApplied = false;

                    // If jump begins but button isn't held, apply a short-hop cut immediately
                    if (!isHeldNow)
                    {
                        if (velY < -jumpCutSpeed) velY = -jumpCutSpeed;
                        jumpCutApplied = true;
                    }
                };

            // Coyote jump (buffered or pressed this frame)
            if (jumpBufferTimer > 0.0f && (wasGrounded || coyoteTimer > 0.0f))
            {
                StartJump(jumpHeld);
            }

            // Variable height: release early cuts upward speed once
            if (jumpReleased && velY < 0.0f && !jumpCutApplied)
            {
                if (velY < -jumpCutSpeed) velY = -jumpCutSpeed;
                jumpCutApplied = true;
            }

            // Gravity
            if (!player->isGrounded())
            {
                velY += gravity * dt_;
            }

            // Final movement delta
            float2 delta
            {
                move.x * playerSpeed * dt_,
                velY * dt_
            };

            // Build sweep rect BEFORE moving (so tile query is correct)
            auto const startPos = player->GetWorldPosition();
            float expectedNewY = startPos.y + delta.y;

            auto const r0 = player->getWorldRect();
            float left = std::min<float>(r0.X, r0.X + delta.x);
            float top = std::min<float>(r0.Y, r0.Y + delta.y);
            float right = std::max<float>(r0.X + r0.Width, r0.X + r0.Width + delta.x);
            float bottom = std::max<float>(r0.Y + r0.Height, r0.Y + r0.Height + delta.y);

            winrt::Windows::Foundation::Rect sweepR{ left, top, right - left, bottom - top };
            auto sweepTiles = tmap->getSolidTilesInRect(sweepR, 1);

            std::vector<game::GameObject*> tiles;
            tiles.reserve(sweepTiles.size());
            for (auto* tile : sweepTiles)
                tiles.push_back(tile);

            // Move + collisions ONCE
            player->Move(delta);
            phys::handleCollisions(*player, tiles);

            // trustFall ONCE (only “do I still have support?”)
            if (player->isGrounded() && player->isAffectedByGravity())
            {
                std::vector<game::GameObject*> underVec;
                if (auto* underObj = player->getUnder())
                {
                    auto const underRect = underObj->getWorldRect();
                    auto underTiles = tmap->getSolidTilesInRect(underRect, 1);

                    underVec.reserve(underTiles.size());
                    for (auto* t : underTiles)
                        underVec.push_back(t);

                    phys::trustFall(*player, underVec);
                }
            }

            bool nowGrounded = player->isGrounded();
            bool landedThisFrame = (nowGrounded && !wasGrounded);

            // Head bonk: tried to go up but got pushed down by collision
            auto const endPos = player->GetWorldPosition();
            if (velY < 0.0f && endPos.y > expectedNewY + 0.01f)
            {
                velY = 0.0f;
            }

            // Kill falling velocity when grounded
            if (nowGrounded && velY > 0.0f)
            {
                velY = 0.0f;
            }

            // Jump buffer on landing (this completes “buffer time” feature)
            if (nowGrounded && jumpBufferTimer > 0.0f)
            {
                StartJump(jumpHeld);
                nowGrounded = false;
                landedThisFrame = false; // we jump immediately; don’t play land
            }

            // Animation context
            if (auto* p = dynamic_cast<Player*>(player.get()))
            {
                Player::AnimContext animCtx{};
                animCtx.moveX = move.x;
                animCtx.grounded = nowGrounded;
                animCtx.justLanded = landedThisFrame;
                animCtx.wantShoot = actMap->Down(engine::Action::Fire);
                animCtx.velY = velY;

                p->UpdateAnimation(dt_, animCtx);
            }

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
		
		uiStrings[0].String = L"Player AnimFrame = " + std::to_wstring(player->CurrentFrameIndex());
		uiStrings[0].Invalidate();

		if (player)
		{
			renderer_.Draw(player->getSprite());
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
