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

            // Apex hang (reduce gravity near the top)
            constexpr float apexVelWindow = 140.0f;     // px/s (|velY| under this => hang)
            constexpr float apexGravityScale = 0.35f;   // 0..1 (lower = floatier)

            // Wall
            constexpr float wallSlideMaxFall = 350.0f;  // px/s downward clamp while sliding
            constexpr float wallJumpSpeedX = 650.0f;    // px/s horizontal kick-off
            constexpr float wallJumpLockMax = 0.15f;    // seconds of forced wall-jump horizontal velocity

            // Wall stick grace (prevents 1-frame flicker)
            constexpr float wallStickGrace = 0.06f;     // ~4 frames @ 60fps
            constexpr float wallProbeW = 1.0f;          // normal probe thickness
            constexpr float wallProbeWGrace = 3.0f;     // wider probe used only during grace
            constexpr float wallProbeInsetY = 2.0f;     // avoid floor/ceiling edges counting as wall

            // Jump timing
            constexpr float coyoteMax = 0.10f;
            constexpr float bufferMax = 0.10f;

            // --- Persistent state
            static float velY = 0.0f;
            static bool  jumpCutApplied = false;
            static float coyoteTimer = 0.0f;
            static float jumpBufferTimer = 0.0f;

            static float wallJumpLockTimer = 0.0f;
            static float wallJumpVelX = 0.0f;

            static float wallStickLeft = 0.0f;
            static float wallStickRight = 0.0f;

            // Input
            float2 move = actMap->MoveAxis();
            bool jumpPressed = actMap->Pressed(engine::Action::MoveUp);
            bool jumpHeld = actMap->Down(engine::Action::MoveUp);
            bool jumpReleased = actMap->Released(engine::Action::MoveUp);

            // Camera pan
            float2 pan = actMap->PanAxis();
            constexpr float camPanSpeed = 450.0f;
            cameraOffset.x += pan.x * camPanSpeed * dt_;
            cameraOffset.y += pan.y * camPanSpeed * dt_;

            // Snapshot grounded at start of frame
            bool wasGrounded = player->isGrounded();

            // Timers
            coyoteTimer = wasGrounded ? coyoteMax : std::max<float>(0.0f, coyoteTimer - dt_);
            jumpBufferTimer = std::max<float>(0.0f, jumpBufferTimer - dt_);
            wallJumpLockTimer = std::max<float>(0.0f, wallJumpLockTimer - dt_);

            if (wasGrounded)
            {
                velY = 0.0f;
                jumpCutApplied = false;
            }

            auto StartJump = [&](bool isHeldNow)
                {
                    velY = -jumpSpeed;
                    player->inAir();
                    coyoteTimer = 0.0f;
                    jumpBufferTimer = 0.0f;
                    jumpCutApplied = false;

                    if (!isHeldNow)
                    {
                        if (velY < -jumpCutSpeed) velY = -jumpCutSpeed;
                        jumpCutApplied = true;
                    }
                };

            // Strict overlap for probes (no edge-touch false positives)
            auto OverlapsStrict = [](winrt::Windows::Foundation::Rect const& a,
                winrt::Windows::Foundation::Rect const& b) noexcept
                {
                    return (a.X < b.X + b.Width) && (a.X + a.Width > b.X) &&
                        (a.Y < b.Y + b.Height) && (a.Y + a.Height > b.Y);
                };

            auto ProbeSolid = [&](winrt::Windows::Foundation::Rect const& probe) -> bool
                {
                    auto nearTiles = tmap->getSolidTilesInRect(probe, 0);
                    for (auto* t : nearTiles)
                    {
                        if (t && OverlapsStrict(probe, t->getWorldRect()))
                            return true;
                    }
                    return false;
                };

            bool pressLeft = (move.x < -0.20f);
            bool pressRight = (move.x > 0.20f);

            // --- Wall probes PRE (before movement) using grace timers
            auto const posPre = player->GetWorldPosition();
            auto const sizePre = player->GetWorldSize();
            float probeHPre = std::max<float>(0.0f, sizePre.y - (wallProbeInsetY * 2.0f));

            winrt::Windows::Foundation::Rect leftProbePre{
                posPre.x - wallProbeW, posPre.y + wallProbeInsetY, wallProbeW, probeHPre
            };
            winrt::Windows::Foundation::Rect rightProbePre{
                posPre.x + sizePre.x, posPre.y + wallProbeInsetY, wallProbeW, probeHPre
            };

            winrt::Windows::Foundation::Rect leftProbePreG{
                posPre.x - wallProbeWGrace, posPre.y + wallProbeInsetY, wallProbeWGrace, probeHPre
            };
            winrt::Windows::Foundation::Rect rightProbePreG{
                posPre.x + sizePre.x, posPre.y + wallProbeInsetY, wallProbeWGrace, probeHPre
            };

            bool rawLeftPre = (!wasGrounded) && ProbeSolid(leftProbePre);
            bool rawRightPre = (!wasGrounded) && ProbeSolid(rightProbePre);

            bool touchWallLeftPre = rawLeftPre || (wallStickLeft > 0.0f && ProbeSolid(leftProbePreG));
            bool touchWallRightPre = rawRightPre || (wallStickRight > 0.0f && ProbeSolid(rightProbePreG));

            bool pressingIntoWallPre =
                (touchWallLeftPre && pressLeft) ||
                (touchWallRightPre && pressRight);

            bool justWallJumped = false;

            // Jump press routing
            if (jumpPressed)
            {
                if (pressingIntoWallPre)
                {
                    // Kick away from wall
                    const float pushDir = (touchWallLeftPre && pressLeft) ? 1.0f : -1.0f;

                    wallJumpVelX = pushDir * wallJumpSpeedX;
                    wallJumpLockTimer = wallJumpLockMax;

                    velY = -jumpSpeed;
                    player->inAir();
                    coyoteTimer = 0.0f;
                    jumpBufferTimer = 0.0f;
                    jumpCutApplied = false;

                    player->SetFacingRight(pushDir > 0.0f);
                    justWallJumped = true;
                }
                else if (wasGrounded || coyoteTimer > 0.0f)
                {
                    StartJump(jumpHeld);
                }
                else if (!wasGrounded && velY > 0.0f)
                {
                    // Buffer ONLY while falling
                    jumpBufferTimer = bufferMax;
                }
            }

            // Variable jump height
            if (jumpReleased && velY < 0.0f && !jumpCutApplied)
            {
                if (velY < -jumpCutSpeed) velY = -jumpCutSpeed;
                jumpCutApplied = true;
            }

            // Gravity (apply ONCE, with apex hang)
            if (!wasGrounded)
            {
                float g = gravity;
                if (std::abs(velY) < apexVelWindow)
                    g *= apexGravityScale;

                velY += g * dt_;
            }

            // Pre-slide clamp
            bool wallSlidingPre = (!wasGrounded) && pressingIntoWallPre && (velY > 0.0f) && (wallJumpLockTimer <= 0.0f);
            if (wallSlidingPre && velY > wallSlideMaxFall)
                velY = wallSlideMaxFall;

            // X velocity (with wall jump lock / slide)
            float xVel = move.x * playerSpeed;
            if (wallJumpLockTimer > 0.0f)
            {
                xVel = wallJumpVelX;
            }
            else if (wallSlidingPre)
            {
                xVel = 0.0f;
            }

            // *** THIS is the key fix: delta MUST include Y ***
            float2 delta{ xVel * dt_, velY * dt_ };

            // Sweep tiles BEFORE moving
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

            // Move + collision resolve
            player->Move(delta);
            phys::handleCollisions(*player, tiles);

            // trustFall (only decides when to un-ground)
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

            // Head bonk detection
            auto const endPos = player->GetWorldPosition();
            if (velY < 0.0f && endPos.y > expectedNewY + 0.01f)
            {
                velY = 0.0f;
            }

            // Kill downward velocity if grounded
            if (nowGrounded && velY > 0.0f)
            {
                velY = 0.0f;
            }

            // Buffered jump on landing (buffer was only set while falling)
            if (nowGrounded && jumpBufferTimer > 0.0f)
            {
                StartJump(jumpHeld);
                nowGrounded = false;
                landedThisFrame = false;
            }

            // --- Wall probes POST (after collision), update grace timers
            auto const posPost = player->GetWorldPosition();
            auto const sizePost = player->GetWorldSize();
            float probeHPost = std::max<float>(0.0f, sizePost.y - (wallProbeInsetY * 2.0f));

            winrt::Windows::Foundation::Rect leftProbePost{
                posPost.x - wallProbeW, posPost.y + wallProbeInsetY, wallProbeW, probeHPost
            };
            winrt::Windows::Foundation::Rect rightProbePost{
                posPost.x + sizePost.x, posPost.y + wallProbeInsetY, wallProbeW, probeHPost
            };

            winrt::Windows::Foundation::Rect leftProbePostG{
                posPost.x - wallProbeWGrace, posPost.y + wallProbeInsetY, wallProbeWGrace, probeHPost
            };
            winrt::Windows::Foundation::Rect rightProbePostG{
                posPost.x + sizePost.x, posPost.y + wallProbeInsetY, wallProbeWGrace, probeHPost
            };

            bool rawLeftPost = (!nowGrounded) && ProbeSolid(leftProbePost);
            bool rawRightPost = (!nowGrounded) && ProbeSolid(rightProbePost);

            if (nowGrounded)
            {
                wallStickLeft = 0.0f;
                wallStickRight = 0.0f;
            }
            else
            {
                wallStickLeft = rawLeftPost ? wallStickGrace : std::max<float>(0.0f, wallStickLeft - dt_);
                wallStickRight = rawRightPost ? wallStickGrace : std::max<float>(0.0f, wallStickRight - dt_);
            }

            bool touchWallLeft = rawLeftPost || (wallStickLeft > 0.0f && ProbeSolid(leftProbePostG));
            bool touchWallRight = rawRightPost || (wallStickRight > 0.0f && ProbeSolid(rightProbePostG));

            bool pressingIntoWall =
                (touchWallLeft && pressLeft) ||
                (touchWallRight && pressRight);

            bool wallSliding = (!nowGrounded) && (velY > 0.0f) && pressingIntoWall && (wallJumpLockTimer <= 0.0f);
            if (wallSliding && velY > wallSlideMaxFall)
                velY = wallSlideMaxFall;

            // Animation context
            if (auto* p = dynamic_cast<Player*>(player.get()))
            {
                Player::AnimContext animCtx{};
                animCtx.moveX = move.x;
                animCtx.grounded = nowGrounded;
                animCtx.justLanded = landedThisFrame;
                animCtx.wantShoot = actMap->Down(engine::Action::Fire);
                animCtx.velY = velY;

                animCtx.touchWallLeft = touchWallLeft;
                animCtx.touchWallRight = touchWallRight;
                animCtx.wallSliding = wallSliding;
                animCtx.justWallJumped = justWallJumped;

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

            if (playerCenterX < leftBound)
            {
                newCamX -= (leftBound - playerCenterX);
            }
            else if (playerCenterX > rightBound)
            {
                newCamX += (playerCenterX - rightBound);
            }

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
