#include "pch.h"
#include "PlayState.h"

#include "../Resources/Cfg.h"
#include "../../Engine/ActionMap.h"
#include "../../Engine/Camera2D.h"
#include "../../Engine/SpriteBatchScope.h"
#include "../../Engine/Text.h"
#include "../Objects/Player.h"
#include "../Map/Tilemap.h"
#include "../Systems/PhysicsSys.h"

#include "../Objects/Enemies/Bluey/Bluey.h"
#include "../Objects/Enemies/Bluey/BlueyElectricShot.h"
#include "../Objects/Enemies/Bluey/BlueyMissileShot.h"
#include "../Objects/Enemies/Shelly/Shelly.h"
#include <array>
#include <algorithm>
#include <unordered_map>


namespace
{
    /// <summary>
    ///  &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
    /// </summary>
    //std::unique_ptr<game::Bluey> s_bluey{ nullptr };
    std::array<std::unique_ptr<game::Bluey>, 2> s_blueys{};

    std::array<game::BlueyElectricShot, 4> s_blueyElectric{};
    std::array<game::BlueyMissileShot, 4> s_blueyMissiles{};

    static std::unordered_map<std::string, std::vector<game::AnimObject*>> s_entityMap = {};

}



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


       
        for (auto& s : m_busterShots) s.Kill();
        m_busterCooldown = 0.0f;


        // --- Blueys (enemies)
        for (auto& b : s_blueys) b.reset();

        s_blueys[0] = std::make_unique<game::Bluey>(float2{ 650.0f, 316.0f });
        s_blueys[1] = std::make_unique<game::Bluey>(float2{ 1400.0f, 316.0f });
        for (auto& e : s_blueyElectric) e.Kill();
        for (auto& m : s_blueyMissiles) m.Kill();

        s_shelly = std::make_unique<game::Shelly>(float2{ 1000.0f, 316.0f });
        for (auto& s : s_shellyShots)
            s.Kill();

        s_entityMap["bluey"] = std::vector<game::AnimObject*>{};
        s_entityMap["bluey"].clear();
        s_entityMap["bluey"].reserve(2);
        s_entityMap["bluey"].emplace_back(dynamic_cast<game::AnimObject*>(s_blueys[0].get()));
        s_entityMap["bluey"].emplace_back(dynamic_cast<game::AnimObject*>(s_blueys[1].get()));

        s_entityMap["shelly"] = std::vector<game::AnimObject*>{};
        s_entityMap["shelly"].clear();
        s_entityMap["shelly"].reserve(1);
        s_entityMap["shelly"].emplace_back(dynamic_cast<game::AnimObject*>(s_shelly.get()));


        // &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
        //        // --- Bluey (enemy)
        //if (s_bluey) { s_bluey.reset(); }
        //s_bluey = std::make_unique<game::Bluey>(float2{ 1000.0f, 186.0f });
        //for (auto& e : s_blueyElectric) e.Kill();
        //for (auto& m : s_blueyMissiles) m.Kill();

  

             // --- Shelly

        

    }

    void PlayState::exit()
    {

        for (auto& s : m_busterShots) s.Kill();
        m_busterCooldown = 0.0f;


        for (auto& e : s_blueyElectric) e.Kill();
        for (auto& m : s_blueyMissiles) m.Kill();
        for (auto& b : s_blueys) b.reset();
        // &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
        //for (auto& e : s_blueyElectric) e.Kill();
        //for (auto& m : s_blueyMissiles) m.Kill();
        //s_bluey.reset();

        tmap.reset();
        tmap = nullptr;

        engine::SoundManager::Instance().StopMusic();
        uiStrings.clear();
        player.reset();
        player = nullptr;

        
        for (auto& s : s_shellyShots)
            s.Kill();

        s_shelly.reset();
        

    }

    void PlayState::processInput(const engine::ActionMap& actMap_)
    {
        // do this in the playstate
        if (actMap_.Pressed(engine::Action::ResetView))
        {
            camera->Reset();
            cameraOffset = { 0,0 };
        }

        //if (actMap_.Pressed(engine::Action::Fire))
        //{
        //    Cfg::PlaySfx(L"blip");
        //}

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

            constexpr float gravity = 1988.88f;       // px/s^2 (down)
            constexpr float jumpSpeed = 900.0f;       // px/s (up is negative)
            constexpr float jumpCutSpeed = 300.0f;    // release early clamps to this upward speed

            // Apex hang (floatier near the top)
            constexpr float apexVelWindow = 140.0f;   // px/s (|velY| under this => hang)
            constexpr float apexGravityScale = 0.35f; // 0..1

            // Wall
            constexpr float wallSlideMaxFall = 350.0f; // px/s downward clamp while sliding
            constexpr float wallJumpSpeedX = 650.0f;   // px/s horizontal kick-off
            constexpr float wallJumpLockMax = 0.15f;   // seconds of forced wall-jump horizontal velocity

            // Dash
            constexpr float dashSpeed = 650.0f;        // px/s
            constexpr float dashDuration = 0.18f;      // seconds
            constexpr float dashCooldown = 0.10f;      // seconds (prevents re-trigger spam)

            // Air dash (single use per airtime)
            constexpr float airDashSpeed = 900.0f;      // px/s (tweak to taste)
            constexpr float airDashDuration = 0.16f;    // seconds

            // Hit / Death
            constexpr int   hpMax = 8;
            constexpr int   hitDamage = 1;
            constexpr float invulnTime = 1.00f;        // seconds of i-frames
            constexpr float hitStunTime = 0.30f;       // seconds of movement lock / knockback
            constexpr float hitKnockX = 420.0f;        // px/s
            constexpr float hitKnockY = 520.0f;        // px/s upward pop

            // Jump grace
            constexpr float coyoteMax = 0.10f;         // seconds
            constexpr float bufferMax = 0.10f;         // seconds


            // Shoot pose latch (prevents idle<->shoot jitter when tapping fire)
            constexpr float shootPoseHold = 0.20f;     // seconds to keep "shoot" overlay after a tap
            

            // --- State (static for now; you can move to Player later)
            static float velY = 0.0f;
            static bool  jumpCutApplied = false;
            static float coyoteTimer = 0.0f;
            static float jumpBufferTimer = 0.0f;

            static float wallJumpLockTimer = 0.0f;
            static float wallJumpVelX = 0.0f;

            static float dashTimer = 0.0f;
            static float dashCooldownTimer = 0.0f;
            static float dashDir = 1.0f;

            static float airDashTimer = 0.0f;
            static bool  airDashUsed = false;
            static float airDashDir = 1.0f;
            static bool  dashJumpCarry = false;   // true if we jumped during a ground dash and are carrying dash speed in-air
            static float dashJumpDir = 1.0f;      // direction to carry (+1 or -1)

            static int   hp = hpMax;
            static bool  dead = false;
            static float invulnTimer = 0.0f;
            static float hitStunTimer = 0.0f;
            static float hitVelX = 0.0f;
            static float shootPoseTimer = 0.0f; // seconds remaining to force shoot overlay

            // Input
            float2 move = actMap->MoveAxis();  // X only for platformer
            bool jumpPressed = actMap->Pressed(engine::Action::MoveUp);
            bool jumpHeld = actMap->Down(engine::Action::MoveUp);
            bool jumpReleased = actMap->Released(engine::Action::MoveUp);


            // TEMP bindings (no engine Action additions needed)
            bool dashPressed = actMap->Pressed(engine::Action::RotCW);   // C / Right shoulder
            bool debugHitPressed = actMap->Pressed(engine::Action::RotCCW);  // Z / Left shoulder
            bool debugDiePressed = actMap->Pressed(engine::Action::ZoomIn);  // E / RT (debug)

            bool shootPressed = actMap->Pressed(engine::Action::Fire);
            bool wantShoot = actMap->Down(engine::Action::Fire);

            // fire rate (tweak to taste)

            m_busterCooldown = std::max<float>(0.0f, m_busterCooldown - dt_);

            // Camera pan offset (keep your existing cameraOffset logic)
            float2 pan = actMap->PanAxis();
            constexpr float camPanSpeed = 450.0f;
            cameraOffset.x += pan.x * camPanSpeed * dt_;
            cameraOffset.y += pan.y * camPanSpeed * dt_;

            // Snapshot ground at start of frame
            bool wasGrounded = player->isGrounded();

            // --- Tick timers
            invulnTimer = std::max<float>(0.0f, invulnTimer - dt_);
            hitStunTimer = std::max<float>(0.0f, hitStunTimer - dt_);
            dashTimer = std::max<float>(0.0f, dashTimer - dt_);
            dashCooldownTimer = std::max<float>(0.0f, dashCooldownTimer - dt_);
            airDashTimer = std::max<float>(0.0f, airDashTimer - dt_);
            wallJumpLockTimer = std::max<float>(0.0f, wallJumpLockTimer - dt_);

            shootPoseTimer = std::max<float>(0.0f, shootPoseTimer - dt_);

            coyoteTimer = wasGrounded ? coyoteMax : std::max<float>(0.0f, coyoteTimer - dt_);
            jumpBufferTimer = std::max<float>(0.0f, jumpBufferTimer - dt_);


            // When grounded, reset vertical speed
            if (wasGrounded)
            {
                velY = 0.0f;
                jumpCutApplied = false;

                // Reset air-dash each time we are grounded
                airDashUsed = false;
                airDashTimer = 0.0f;

                dashJumpCarry = false;
            }

            auto StartJump = [&](bool isHeldNow)
                {
                    velY = -jumpSpeed;
                    player->inAir();          // immediately leave ground
                    coyoteTimer = 0.0f;
                    jumpBufferTimer = 0.0f;
                    jumpCutApplied = false;

                    // Short-hop if not held
                    if (!isHeldNow)
                    {
                        if (velY < -jumpCutSpeed) velY = -jumpCutSpeed;
                        jumpCutApplied = true;
                    }
                };

            // --- 1px wall probes (same pattern as under-probe)
            constexpr float kWallProbeW = 1.0f;
            constexpr float kWallProbeInsetY = 2.0f;

            auto Overlaps = [](winrt::Windows::Foundation::Rect const& a,
                winrt::Windows::Foundation::Rect const& b) noexcept
                {
                    return (a.X <= b.X + b.Width) && (a.X + a.Width > b.X) &&
                        (a.Y <= b.Y + b.Height) && (a.Y + a.Height > b.Y);
                };

            auto ProbeSolid = [&](winrt::Windows::Foundation::Rect const& probe) -> bool
                {
                    auto nearTiles = tmap->getSolidTilesInRect(probe, 0);
                    for (auto* t : nearTiles)
                    {
                        if (t && Overlaps(probe, t->getWorldRect()))
                            return true;
                    }
                    return false;
                };

            // Probe at the start of frame (pre-move)
            auto const posPre = player->GetWorldPosition();
            auto const sizePre = player->GetWorldSize();
            float probeHPre = std::max<float>(0.0f, sizePre.y - (kWallProbeInsetY * 2.0f));

            winrt::Windows::Foundation::Rect leftProbePre
            {
                posPre.x - kWallProbeW,
                posPre.y + kWallProbeInsetY,
                kWallProbeW,
                probeHPre
            };

            winrt::Windows::Foundation::Rect rightProbePre
            {
                posPre.x + sizePre.x,
                posPre.y + kWallProbeInsetY,
                kWallProbeW,
                probeHPre
            };

            bool touchWallLeftPre = (!wasGrounded) && ProbeSolid(leftProbePre);
            bool touchWallRightPre = (!wasGrounded) && ProbeSolid(rightProbePre);


            bool pressLeft = (move.x < -0.20f);
            bool pressRight = (move.x > 0.20f);

            bool pressingIntoWallPre =
                (touchWallLeftPre && pressLeft) ||
                (touchWallRightPre && pressRight);



            // --- DAMAGE / HIT (debug trigger for now)
            if (debugDiePressed && !dead)
            {
                hp = 0;
                dead = true;
                hitStunTimer = 0.0f;
                invulnTimer = 0.0f;

                // Cancel movement states
                dashTimer = 0.0f;
                dashCooldownTimer = 0.0f;
                airDashTimer = 0.0f;
                wallJumpLockTimer = 0.0f;
                hitVelX = 0.0f;
                velY = 0.0f;
                dashJumpCarry = false;

                if (player->hasClip(L"die"))
                    player->Play(L"die", /*restart*/true);
            }

            if (debugHitPressed && !dead && invulnTimer <= 0.0f)
            {
                hp = std::max<int>(0, hp - hitDamage);

                invulnTimer = invulnTime;
                hitStunTimer = hitStunTime;

                // Cancel movement states
                dashTimer = 0.0f;
                dashCooldownTimer = dashCooldown;
                airDashTimer = 0.0f;
                wallJumpLockTimer = 0.0f;
                dashJumpCarry = false;

                // Knock back opposite of facing (or opposite of input if you’re holding a direction)
                float kbDir = player->IsFacingRight() ? -1.0f : 1.0f;
                if (pressLeft)  kbDir = 1.0f;
                if (pressRight) kbDir = -1.0f;

                hitVelX = kbDir * hitKnockX;
                velY = -hitKnockY;
                player->inAir();

                if (hp <= 0)
                {
                    dead = true;
                    hitStunTimer = 0.0f;
                }
            }

            // Control lock during hitstun / dead
            bool controlLocked = dead || (hitStunTimer > 0.0f);
            if (controlLocked)
            {
                move = float2{ 0.0f, 0.0f };
                jumpPressed = false;
                jumpReleased = false;
                dashPressed = false;
                wantShoot = false;
                shootPressed = false;
                shootPoseTimer = 0.0f;
                // Don’t allow “buffered jump after hit”
                jumpBufferTimer = 0.0f;
                coyoteTimer = 0.0f;
                dashJumpCarry = false;
            }

            // Keep shoot pose briefly after tapping fire (even if button is released)
            if (!controlLocked && shootPressed)
                 {
                shootPoseTimer = shootPoseHold;
                }
            

            // --- DASH (ground-only start)
            if (!controlLocked && wasGrounded && dashPressed && dashTimer <= 0.0f && dashCooldownTimer <= 0.0f)
            {
                if (pressLeft)      dashDir = -1.0f;
                else if (pressRight) dashDir = 1.0f;
                else                dashDir = player->IsFacingRight() ? 1.0f : -1.0f;

                dashTimer = dashDuration;
                dashCooldownTimer = dashCooldown;

                dashJumpCarry = false;

                // Consume the press so we don't also start an air-dash later this frame.
                dashPressed = false;
            }

            bool justWallJumped = false;

            // Jump press routing:
            // - wall jump if touching wall and pressing into it
            // - else normal jump if grounded/coyote
            // - else buffer ONLY while falling
            if (!controlLocked && jumpPressed)
            {
                if (pressingIntoWallPre)
                {
                    dashJumpCarry = false;
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
                    // If we're currently in a ground dash AND the player is still holding the dash direction,
                    // carry dash speed into the air until landing or wall hit.
                    if (dashTimer > 0.0f)
                    {
                        bool holdingDashDir = (dashDir < 0.0f) ? pressLeft : pressRight;

                        if (holdingDashDir)
                        {
                            dashJumpCarry = true;
                            dashJumpDir = dashDir;
                        }
                        else
                        {
                            dashJumpCarry = false;
                        }
                    }
                    else
                    {
                        dashJumpCarry = false;
                    }

                    StartJump(jumpHeld);
                }
                else if (!wasGrounded && velY > 0.0f)
                {
                    // Buffer ONLY while falling
                    jumpBufferTimer = bufferMax;
                }
            }

            // Variable height: release early cuts upward speed once
            if (!controlLocked && jumpReleased && velY < 0.0f && !jumpCutApplied)
            {
                if (velY < -jumpCutSpeed) velY = -jumpCutSpeed;
                jumpCutApplied = true;
            }

            // --- AIR DASH (single use per airtime)
            // Uses the same Dash button (C). Only allowed if you were already airborne at the
            // start of this frame (prevents "jump + dash" on the exact same frame).
            if (!controlLocked && dashPressed && !wasGrounded && !airDashUsed && airDashTimer <= 0.0f)
            {
                dashJumpCarry = false;

                // Direction: prefer input; otherwise dash in facing direction.
                if (pressLeft)       airDashDir = -1.0f;
                else if (pressRight) airDashDir = 1.0f;
                else                 airDashDir = player->IsFacingRight() ? 1.0f : -1.0f;

                airDashUsed = true;
                airDashTimer = airDashDuration;

                // Cancel other horizontal locks so air-dash feels consistent.
                dashTimer = 0.0f;
                wallJumpLockTimer = 0.0f;

                player->SetFacingRight(airDashDir > 0.0f);

                // consume press
                dashPressed = false;
            }

            // Gravity (with apex hang)
            if (!player->isGrounded())
            {
                float g = gravity;
                if (std::abs(velY) < apexVelWindow)
                    g *= apexGravityScale;

                velY += g * dt_;
            }

            // Wall slide clamp (pre-move)
            bool wallSlidingPre = (!wasGrounded) && pressingIntoWallPre && (velY > 0.0f);
            if (wallSlidingPre && velY > wallSlideMaxFall)
            {
                velY = wallSlideMaxFall;
            }

            // Horizontal velocity priority:
            // Hitstun > air-dash > wall-jump lock > dash > wall slide (stops shove) > input
            float xVel = move.x * playerSpeed;

            if (airDashTimer > 0.0f)
            {
                xVel = airDashDir * airDashSpeed;
            }
            else if (wallJumpLockTimer > 0.0f)
            {
                xVel = wallJumpVelX;
            }
            else if (!wasGrounded && dashJumpCarry)
            {
                // Keep dash speed only while you continue holding that direction.
                bool holdingDashDir = (dashJumpDir < 0.0f) ? pressLeft : pressRight;

                if (holdingDashDir)
                {
                    xVel = dashJumpDir * dashSpeed;
                }
                else
                {
                    dashJumpCarry = false; // let normal air control take over
                }
            }
            else if (dashTimer > 0.0f)
            {
                xVel = dashDir * dashSpeed;
            }
            else if (wallSlidingPre)
            {
                xVel = 0.0f;
            }

            if (hitStunTimer > 0.0f)
            {
                xVel = hitVelX;
            }
            if (dead)
            {
                xVel = 0.0f;
            }

            float2 delta
            {
                xVel * dt_,
                velY * dt_
            };

            // Build sweep rect BEFORE moving (so tile query is correct)
            auto const startPos = player->GetWorldPosition();
            float expectedNewY = startPos.y + delta.y;
            {
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

                // Move + collide ONCE
                player->Move(delta);
                phys::handleCollisions(*player, tiles);
            }
            // Stop dash if we slammed into something horizontally (collision pushed us back)
            auto const afterPos = player->GetWorldPosition();
            if (dashTimer > 0.0f || (!wasGrounded && dashJumpCarry))
            {
                float expectedX = startPos.x + delta.x;
                if (std::abs(afterPos.x - expectedX) > 0.01f)
                {
                    dashTimer = 0.0f;
                    dashJumpCarry = false;
                }
            }

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
            if (nowGrounded)
            {
                dashJumpCarry = false;
            }
            bool landedThisFrame = (nowGrounded && !wasGrounded);

            // Head bonk: tried to go up but collision pushed us down
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

            // Jump buffer on landing (only if not locked)
            if (!controlLocked && nowGrounded && jumpBufferTimer > 0.0f)
            {
                dashJumpCarry = false;
                StartJump(jumpHeld);
                nowGrounded = false;
                landedThisFrame = false;
            }

            // Wall contact POST (for animation)
            auto const posPost = player->GetWorldPosition();
            auto const sizePost = player->GetWorldSize();
            float probeHPost = std::max<float>(0.0f, sizePost.y - (kWallProbeInsetY * 2.0f));

            winrt::Windows::Foundation::Rect leftProbe
            {
                posPost.x - kWallProbeW,
                posPost.y + kWallProbeInsetY,
                kWallProbeW,
                probeHPost
            };

            winrt::Windows::Foundation::Rect rightProbe
            {
                posPost.x + sizePost.x,
                posPost.y + kWallProbeInsetY,
                kWallProbeW,
                probeHPost
            };

            bool touchWallLeft = (!nowGrounded) && ProbeSolid(leftProbe);
            bool touchWallRight = (!nowGrounded) && ProbeSolid(rightProbe);

            // --- X-BUSTER (standard pellet) -----------------------------------------
            constexpr float kBusterInterval = 0.14f; // seconds between shots
         

            auto GetShotDir = [&]() -> float
                {
                    // If we’re touching a wall in-air, always shoot AWAY from the wall.
                    if (!nowGrounded)
                    {
                        if (touchWallLeft && !touchWallRight)  return +1.0f;
                        if (touchWallRight && !touchWallLeft)  return -1.0f;
                    }

                    // Otherwise, use input if held, else facing
                    if (pressLeft)  return -1.0f;
                    if (pressRight) return +1.0f;

                    return player->IsFacingRight() ? +1.0f : -1.0f;
                };

            auto GetMuzzlePos = [&](float dir) -> float2
                {
                    // IMPORTANT:
                    // AnimObject updates its internal frame index, but the base GameObject
                    // (FrameSize/TextureOffset/etc) is only updated by SyncToBase().
                    // We need correct sprite-space info *right now* for muzzle placement.
                    player->SyncToBase();

                    // Player collider top-left (physics)
                    const auto pWorld = player->GetWorldPosition();

                    // Sprite offset + size for CURRENT rendered frame
                    const auto pTexOff = player->GetTextureOffset();
                    const auto pFrame = player->GetFrameSize();

                    // Sprite top-left in world space (matches how you draw)
                    const float2 pSpriteTL{ pWorld.x - pTexOff.x, pWorld.y - pTexOff.y };

                    // ---- Defaults (use sensible shoot defaults as fallback)
                    float muzzleFromEdgeX;// = 43.0f; // px from sprite edge
                    float muzzleYFrac; //= 0.42f; // 0..1 down the sprite
                    bool assigned = false;
                    const auto& clip = player->CurrentClipKey();
                    const uint32_t frameIdx = player->CurrentFrameIndex();


                    if (clip == L"runshoot")
                    {
                        // You said X was the same for all runshoot frames
                        muzzleFromEdgeX = 29.0f;

                        static constexpr std::array<float, 12> runShootYFrac =
                        {
                            0.42f,
                            0.41f,
                            0.40f,
                            0.40f,
                            0.40f,
                            0.41f,
                            0.39f,
                            0.38f,
                            0.42f,
                            0.43f,
                            0.42f,
                            0.41f
                        };

                        muzzleYFrac = runShootYFrac[frameIdx % runShootYFrac.size()];
                        assigned = true;
                    }
                    else if (clip == L"shoot_start")
                    {
                        // You said X was the same for all runshoot frames
                        muzzleFromEdgeX = 29.0f;

                        static constexpr std::array<float, 3> runShootYFrac =
                        {
                            0.42f,
                            0.42f,
                            0.42f
                        };

                        muzzleYFrac = runShootYFrac[frameIdx % runShootYFrac.size()];
                        assigned = true;
                    }
                    else if (clip == L"jumpshoot_peak")
                    {
                        // You said X was the same for all runshoot frames
                        muzzleFromEdgeX = 40.0f;

                        static constexpr std::array<float, 1> runShootYFrac =
                        {
                            0.3285f                
                        };

                        muzzleYFrac = runShootYFrac[frameIdx % runShootYFrac.size()];
                        assigned = true;
                    }
                    else if (clip == L"fallshoot")
                    {
                        // You said X was the same for all runshoot frames
                        muzzleFromEdgeX = 40.0f;

                        static constexpr std::array<float, 3> runShootYFrac =
                        {
                            0.393f,
                            0.393f,
                            0.393f
                        };

                        muzzleYFrac = runShootYFrac[frameIdx % runShootYFrac.size()];
                        assigned = true;
                    }
                    else if (clip == L"landshoot")
                    {
                        // You said X was the same for all runshoot frames
                        muzzleFromEdgeX = 40.0f;

                        static constexpr std::array<float, 3> runShootYFrac =
                        {
                            0.385f,
                            0.385f,
                            0.385f 
                        };
                    

                        muzzleYFrac = runShootYFrac[frameIdx % runShootYFrac.size()];
                        assigned = true;
                    }
                    else if (clip == L"crouchshoot")
                    {
                        // You said X was the same for all runshoot frames
                        muzzleFromEdgeX = 29.0f;

                        static constexpr std::array<float, 1> runShootYFrac =
                        {
                            0.42f                       
                        };

                        muzzleYFrac = runShootYFrac[frameIdx % runShootYFrac.size()];
                        assigned = true;
                    }
                    else if (clip == L"jumpshoot_rise")
                    {
                        static bool passt = false;
                        // You said X was the same for all runshoot frames
                        muzzleFromEdgeX = 40.f;

                        static constexpr std::array<float, 4> runShootYFrac =
                        {
                            0.30f,
                            0.30f,
                            0.30f,
                            0.30f                        
                        };
            
                        muzzleYFrac = runShootYFrac[frameIdx % runShootYFrac.size()];
                        assigned = true;
                    }
                    else if (clip == L"shoot")
                    {
                        // keep defaults (or set explicitly if you want)
                        muzzleFromEdgeX = 43.0f;
                        muzzleYFrac = 0.42f;
                        assigned = true;
                    }
                    else
                    {
                        if (player->isGrounded())
                        {
                            muzzleFromEdgeX = 40.0f;
                            muzzleYFrac = 0.42f;
                        }
                        else
                        {
                            muzzleFromEdgeX = 40.0f;
                            muzzleYFrac = 0.34f;
                        }
                        // Fallback while firing during clip transitions:
                        // DON'T use wild values like 44 here; it will spawn bullets wrong for 1 frame.
                        // Keep defaults.
                    }

                    // Safety clamp so a bad tuning value can’t explode positions
                    muzzleYFrac = std::clamp(muzzleYFrac, 0.0f, 1.0f);

                    // Build muzzle point in sprite-space
                    const float muzzleX = (dir > 0.0f)
                        ? (pSpriteTL.x + pFrame.x - muzzleFromEdgeX)
                        : (pSpriteTL.x + muzzleFromEdgeX);

                    const float muzzleY = pSpriteTL.y + (pFrame.y * muzzleYFrac);

                    // Convert muzzle point -> bullet collider top-left
                    constexpr float kLead = 2.0f;

                    const float bulletX = (dir > 0.0f)
                        ? (muzzleX + kLead)
                        : (muzzleX - kLead - game::BusterShot::ColliderW);

                    const float bulletY = muzzleY - (game::BusterShot::ColliderH * 0.5f);

                    return { bulletX, bulletY };
                };


            {
                // =========================================================
         // SHELLY
         // =========================================================
                auto ApplyPlayerHit = [&](float fromDir) -> bool
                    {
                        if (dead) return false;
                        if (invulnTimer > 0.0f) return false;

                        hp = std::max<int>(0, hp - hitDamage);

                        invulnTimer = invulnTime;
                        hitStunTimer = hitStunTime;

                        dashTimer = 0.0f;
                        dashCooldownTimer = dashCooldown;
                        airDashTimer = 0.0f;
                        wallJumpLockTimer = 0.0f;
                        dashJumpCarry = false;

                        jumpBufferTimer = 0.0f;
                        coyoteTimer = 0.0f;

                        float kbDir = (fromDir >= 0.0f) ? 1.0f : -1.0f;
                        hitVelX = kbDir * hitKnockX;
                        velY = -hitKnockY;
                        player->inAir();

                        if (hp <= 0)
                        {
                            dead = true;
                            hitStunTimer = 0.0f;
                        }

                        return true;
                    };

                if (s_shelly && player)
                {
                    auto const pPos = player->GetWorldPosition();
                    auto const pSz = player->GetWorldSize();

                    float2 const pCenter
                    {
                        pPos.x + (pSz.x * 0.5f),
                        pPos.y + (pSz.y * 0.5f)
                    };

                    auto SpawnShellyShot = [&](float2 pos, float2 target) -> game::ShellyShot*
                        {
                            for (auto& s : s_shellyShots)
                            {
                                if (!s.Active)
                                {
                                    s.Spawn(pos, target);
                                    return &s;
                                }
                            }
                            return nullptr;
                        };

                    s_shelly->UpdateShelly(dt_, pCenter, player->IsFacingRight(), SpawnShellyShot);
                    s_shelly->SyncToBase();

                    // ---------------------------------------------
                    // Player buster vs Shelly
                    // ---------------------------------------------
                    auto shellyRect = s_shelly->getWorldRect();

                    for (auto& shot : m_busterShots)
                    {
                        if (!shot.Active) continue;

                        if (!Overlaps(shot.getWorldRect(), shellyRect))
                            continue;

                        if (s_shelly->CanReflectBuster() && !shot.Reflected)
                        {
                            shot.Reflect45Up();

                            // Nudge it outside the shell so it doesn't instantly collide again
                            auto sp = shot.GetWorldPosition();
                            auto ss = shot.GetWorldSize();

                            if (shot.Velocity.x < 0.0f)
                                sp.x = shellyRect.X - ss.x - 2.0f;
                            else
                                sp.x = shellyRect.X + shellyRect.Width + 2.0f;

                            sp.y = shellyRect.Y - ss.y - 2.0f;
                            shot.SetWorldPosition(sp);
                        }
                        else
                        {
                            // For now, Shelly just blocks the shot in non-shell state too.
                            // If you want him vulnerable while walking later, this is the place.
                            shot.Kill();
                        }
                    }
                }

                // ---------------------------------------------
                // Shelly shots
                // ---------------------------------------------
                auto playerRectShelly = player->getWorldRect();

                for (auto& s : s_shellyShots)
                {
                    if (!s.Active) continue;

                    auto before = s.GetWorldPosition();
                    s.UpdateShot(dt_);
                    auto after = s.GetWorldPosition();

                    auto sz = s.GetWorldSize();

                    winrt::Windows::Foundation::Rect r0{ before.x, before.y, sz.x, sz.y };
                    winrt::Windows::Foundation::Rect r1{ after.x,  after.y,  sz.x, sz.y };

                    float l = std::min<float>(r0.X, r1.X);
                    float t = std::min<float>(r0.Y, r1.Y);
                    float r = std::max<float>(r0.X + r0.Width, r1.X + r1.Width);
                    float b = std::max<float>(r0.Y + r0.Height, r1.Y + r1.Height);

                    winrt::Windows::Foundation::Rect sweep{ l, t, r - l, b - t };

                    auto nearTiles = tmap->getSolidTilesInRect(sweep, 1);
                    for (auto* tile : nearTiles)
                    {
                        if (!tile) continue;

                        if (Overlaps(sweep, tile->getWorldRect()))
                        {
                            s.Kill();
                            break;
                        }
                    }

                    if (s.Active && Overlaps(s.getWorldRect(), playerRectShelly))
                    {
                        ApplyPlayerHit(s.Dir);
                        s.Kill();
                    }
                }

            }

                            // --- BLUEY (enemy)  enemy projectiles ------------------------------
                    
                    auto ApplyPlayerHit = [&](float fromDir) -> bool
                     {
                    if (dead) return false;
                    if (invulnTimer > 0.0f) return false;
                    
                        invulnTimer = invulnTime;
                    hitStunTimer = hitStunTime;
                    
                                            // Cancel movement states
                        dashTimer = 0.0f;
                    dashCooldownTimer = dashCooldown;
                    airDashTimer = 0.0f;
                    wallJumpLockTimer = 0.0f;
                    dashJumpCarry = false;
                    
                                            // Don’t allow buffered jump after hit
                        jumpBufferTimer = 0.0f;
                    coyoteTimer = 0.0f;
                    
                                            // Knockback: push in the direction the projectile is travelling
                        float kbDir = (fromDir >= 0.0f) ? 1.0f : -1.0f;
                    hitVelX = kbDir * hitKnockX;
                    velY = -hitKnockY;
                    player->inAir();
                    
                        Cfg::PlaySfx(L"player_hit", 0.65f);
                    return true;
                    };
                

                    // &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
                    for (auto& b : s_blueys)
                    {
                        if (!b) continue;

                        auto const blueyRect = b->getWorldRect();

                        for (auto& shot : m_busterShots)
                        {
                            if (!shot.Active) continue;

                            if (Overlaps(shot.getWorldRect(), blueyRect))
                            {
                                shot.Kill();
                                b->TakeDamage(1);

                                if (b->IsDead())
                                {
                                    Cfg::PlaySfx(Cfg::Sounds::EnemyDie, 0.65f);
                                    b.reset();
                                }
                                break;
                            }
                        }
                    }
                                // Player buster -> Bluey damage (1 dmg per hit, Bluey has 25hp)
                  
                                
                         //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
                                
                                //if (s_bluey)
                    //{
                    //auto const blueyRect = s_bluey->getWorldRect();
                    //
                    //    for (auto& shot : m_busterShots)
                    //     {
                    //    if (!shot.Active) continue;
                    //    
                    //        if (Overlaps(shot.getWorldRect(), blueyRect))
                    //         {
                    //        shot.Kill();
                    //        s_bluey->TakeDamage(1);
                    //        
                    //            if (s_bluey->IsDead())
                    //             {
                    //            s_bluey.reset();
                    //            break;
                    //            }
                    //         }
                    //     }
                    // }
                
                       
                    
                    
                    
                    auto const pPos = player->GetWorldPosition();
                    auto const pSz = player->GetWorldSize();
                    float2 const pCenter{ pPos.x + (pSz.x * 0.5f), pPos.y + (pSz.y * 0.5f) };

                    auto SpawnElectric = [&](float2 pos, float dir, float targetX) -> game::BlueyElectricShot*
                        {
                            for (auto& e : s_blueyElectric)
                            {
                                
                                if (!e.Active)
                                {
                                    e.Spawn(pos, dir, targetX);
                                    return &e;
                                }
                            }
                            return nullptr;
                        };

                    auto SpawnMissile = [&](float2 pos, float dir) -> game::BlueyMissileShot*
                        {
                            for (auto& m : s_blueyMissiles)
                            {
                                if (!m.Active)
                                {
                                    m.Spawn(pos, dir);
                                    return &m;
                                }
                            }
                            return nullptr;
                        };

                    for (auto& b : s_blueys)
                    {
                        if (!b) continue;

                        b->UpdateBluey(dt_, pCenter, SpawnElectric, SpawnMissile);
                        b->SyncToBase();
                    }

                    //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
                    // Bluey AI update (spawns electric  missiles)
                    //if (s_bluey)
                    //{
                    //auto const pPos = player->GetWorldPosition();
                    //auto const pSz = player->GetWorldSize();
                    //float2 const pCenter{ pPos.x + (pSz.x * 0.5f), pPos.y + (pSz.y * 0.5f) };
                    //auto SpawnElectric = [&](float2 pos, float dir, float targetX) -> game::BlueyElectricShot*
                    //    {
                    //        for (auto& e : s_blueyElectric)
                    //        {
                    //            if (!e.Active)
                    //            {
                    //                e.Spawn(pos, dir, targetX);
                    //                return &e;
                    //            }
                    //        }
                    //        return nullptr;
                    //    };

                    //auto SpawnMissile = [&](float2 pos, float dir) -> game::BlueyMissileShot*
                    //    {
                    //        for (auto& m : s_blueyMissiles)
                    //        {
                    //            if (!m.Active)
                    //            {
                    //                m.Spawn(pos, dir);
                    //                return &m;
                    //            }
                    //        }
                    //        return nullptr;
                    //    };
                    //s_bluey->UpdateBluey(dt_, pCenter, SpawnElectric, SpawnMissile);
                    //s_bluey->SyncToBase();
                    //}
                
                                // Enemy projectiles: update  collide w/ tiles  hit player
                    auto const playerRect = player->getWorldRect();
                
                                // Electric (falls down, then travels along the ground)
                    for (auto& e : s_blueyElectric)
                    {
                        if (!e.Active) continue;

                        auto before = e.GetWorldPosition();
                        e.UpdateShot(dt_);
                        if (!e.Active) continue;

                        auto after = e.GetWorldPosition();

                        auto sz = e.GetWorldSize();
                        winrt::Windows::Foundation::Rect r0{ before.x, before.y, sz.x, sz.y };
                        winrt::Windows::Foundation::Rect r1{ after.x,  after.y,  sz.x, sz.y };

                        float l = std::min<float>(r0.X, r1.X);
                        float t = std::min<float>(r0.Y, r1.Y);
                        float r = std::max<float>(r0.X + r0.Width, r1.X + r1.Width);
                        float b = std::max<float>(r0.Y + r0.Height, r1.Y + r1.Height);

                        winrt::Windows::Foundation::Rect sweep{ l, t, r - l, b - t };

                        auto nearTiles = tmap->getSolidTilesInRect(sweep, 1);
                        for (auto* tile : nearTiles)
                        {
                            if (!tile) continue;
                            auto const tr = tile->getWorldRect();

                            if (!Overlaps(sweep, tr))
                                continue;

                            if (e.IsFalling())
                            {
                                // Snap + pause on ground (then it will auto-start travel after GroundDelay)
                                e.LandOnGround(tr.Y);
                            }
                            else if (e.IsGroundTravel())
                            {
                                // Hit a wall while ground-traveling
                                e.Kill();
                            }
                            // If GroundDelay, ignore (it’s sitting on ground intentionally)
                            break;
                        }

                        // Hit player: kill projectile always; apply damage only if not invuln
                        if (e.Active && Overlaps(e.getWorldRect(), playerRect))
                        {
                            ApplyPlayerHit(e.Dir);
                            e.Kill();
                        }
                    }
                
                                // Missiles (simple horizontal travel)
                    for (auto& m : s_blueyMissiles)
                    {
                        if (!m.Active) continue;

                        auto before = m.GetWorldPosition();
                        m.UpdateShot(dt_);
                        if (!m.Active) continue;

                        auto after = m.GetWorldPosition();

                        auto sz = m.GetWorldSize();
                        winrt::Windows::Foundation::Rect r0{ before.x, before.y, sz.x, sz.y };
                        winrt::Windows::Foundation::Rect r1{ after.x,  after.y,  sz.x, sz.y };

                        float l = std::min<float>(r0.X, r1.X);
                        float t = std::min<float>(r0.Y, r1.Y);
                        float r = std::max<float>(r0.X + r0.Width, r1.X + r1.Width);
                        float b = std::max<float>(r0.Y + r0.Height, r1.Y + r1.Height);

                        winrt::Windows::Foundation::Rect sweep{ l, t, r - l, b - t };

                        auto nearTiles = tmap->getSolidTilesInRect(sweep, 1);
                        for (auto* tile : nearTiles)
                        {
                            if (!tile) continue;
                            if (Overlaps(sweep, tile->getWorldRect()))
                            {
                                m.Kill();
                                break;
                            }
                        }

                        if (m.Active && Overlaps(m.getWorldRect(), playerRect))
                        {
                            ApplyPlayerHit(m.Dir);
                            m.Kill();
                        }
                    }
                

                   


            // Spawn (hold-to-fire, 3 shots max)
            if (!controlLocked && wantShoot && m_busterCooldown <= 0.0f)
            {
                for (auto& shot : m_busterShots)
                {
                    if (!shot.Active)
                    {
                        float dir = GetShotDir();
                        shot.Spawn(GetMuzzlePos(dir), dir);
                   
                        Cfg::PlaySfx(L"blip", 0.55f);
                        m_busterCooldown = kBusterInterval;
                        break;
                    }
                }
            }

            // Update + collide with solid tiles
            for (auto& shot : m_busterShots)
            {
                if (!shot.Active) continue;

                auto before = shot.GetWorldPosition();
                shot.UpdateShot(dt_);
                auto after = shot.GetWorldPosition();

                // Build a simple sweep rect so we don’t miss tiles
                auto sz = shot.GetWorldSize();
                winrt::Windows::Foundation::Rect r3{ before.x, before.y, sz.x, sz.y };
                winrt::Windows::Foundation::Rect r2{ after.x,  after.y,  sz.x, sz.y };

                float l = std::min<float>(r3.X, r2.X);
                float t = std::min<float>(r3.Y, r2.Y);
                float r = std::max<float>(r3.X + r3.Width, r2.X + r2.Width);
                float b = std::max<float>(r3.Y + r3.Height, r2.Y + r2.Height);

                winrt::Windows::Foundation::Rect sweep{ l, t, r - l, b - t };

                auto nearTiles = tmap->getSolidTilesInRect(sweep, 1);
                for (auto* tile : nearTiles)
                {
                    if (tile && Overlaps(sweep, tile->getWorldRect()))
                    {
                        shot.Kill();
                        break;
                    }
                }
            }


            bool wallSliding =
                (!nowGrounded) &&
                (velY > 0.0f) &&
                ((touchWallLeft && pressLeft) || (touchWallRight && pressRight));

            if (wallSliding && velY > wallSlideMaxFall)
                velY = wallSlideMaxFall;

            // Animation context
            if (auto* p = dynamic_cast<Player*>(player.get()))
            {
                Player::AnimContext animCtx{};

                // For facing: dash uses dashDir; hit/dead don't change facing.
                if (dead || hitStunTimer > 0.0f)        animCtx.moveX = 0.0f;
                else if (airDashTimer > 0.0f)           animCtx.moveX = airDashDir;
                else if (dashTimer > 0.0f)             animCtx.moveX = dashDir;
                else                                   animCtx.moveX = move.x;

                animCtx.grounded = nowGrounded;
                animCtx.justLanded = landedThisFrame;

     
                animCtx.wantShoot = wantShoot || (shootPoseTimer > 0.0f);
                animCtx.wantDash = (!controlLocked) && (dashTimer > 0.0f) && nowGrounded;

                animCtx.airDashing = (!controlLocked) && (airDashTimer > 0.0f) && (!nowGrounded);

                animCtx.gotHit = (!dead) && (hitStunTimer > 0.0f);
                animCtx.dead = dead;

                animCtx.velY = velY;

                animCtx.touchWallLeft = touchWallLeft;
                animCtx.touchWallRight = touchWallRight;
                animCtx.wallSliding = wallSliding;
                animCtx.justWallJumped = justWallJumped;

                p->UpdateAnimation(dt_, animCtx);
                p->SyncToBase();

                // --- INVULNERABILITY FLASH (alpha blink)
                // Simple on/off blink while invulnTimer > 0.
                {
                    constexpr float blinkPeriod = 0.08f; // seconds
                    static float blinkAccum = 0.0f;
                    static bool blinkVisible = true;

                    if (!dead && invulnTimer > 0.0f)
                    {
                        blinkAccum += dt_;
                        while (blinkAccum >= blinkPeriod)
                        {
                            blinkAccum -= blinkPeriod;
                            blinkVisible = !blinkVisible;
                        }

                        float alpha = blinkVisible ? 1.0f : 0.25f;
                        auto t = player->GetTint();
                        t.w = alpha;
                        player->SetTint(t);
                    }
                    else
                    {
                        blinkAccum = 0.0f;
                        blinkVisible = true;

                        auto t = player->GetTint();
                        t.w = 1.0f;
                        player->SetTint(t);
                    }
                }
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
        for (auto& b : s_blueys)
        {
            if (b) b->SyncToBase();
        }

        if (s_shelly)
        {
            s_shelly->SyncToBase();
        }
    }

    std::vector<engine::Text>& PlayState::render(engine::SpriteBatchScope const& batch_)
    {
     
        tmap->render(batch_, *camera);

        uiStrings[0].String = L"Player AnimFrame = " + std::to_wstring(player->CurrentFrameIndex());
        uiStrings[0].Invalidate();

        //for (auto& b : s_blueys)
        //{
        //    if (b) renderer_.Draw(b->getSprite());
        //}

        //if (s_shelly)
        //{
        //    renderer_.Draw(s_shelly->getSprite());
        //}

        for (auto& ent : s_entityMap["bluey"])
        {
            if (ent) ent->getSprite().Draw(batch_);
        }

        for (auto& ent : s_entityMap["shelly"])
        {
            if (ent) ent->getSprite().Draw(batch_);
        }


        for (auto& s : s_shellyShots)
        {
            if (s.Active)
                s.getSprite().Draw(batch_);
        }

        //auto spr = player->getSprite();
        //uiStrings[0].String = spr.IsValid() ? L"sprite valid" : L"sprite invalid";
        //uiStrings[0].Invalidate();
        //spr.Draw(batch_);

        if (player)
        {
            player->getSprite().Draw(batch_);
        }

        for (auto& shot : m_busterShots)
        {
            if (shot.Active)
                shot.getSprite().Draw(batch_);
        }


         for (auto& e : s_blueyElectric)
             {
            if (e.Active)
                e.getSprite().Draw(batch_);
            }
        
            for (auto& m : s_blueyMissiles)
             {
            if (m.Active)
                m.getSprite().Draw(batch_);
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
