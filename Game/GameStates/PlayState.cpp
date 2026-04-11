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
#include <cmath>
#include <cstdint>
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

    using winrt::Windows::Foundation::Rect;
    using winrt::Windows::Foundation::Numerics::float2;
    using winrt::Windows::Foundation::Numerics::float4;

    enum class StagePickupType
    {
        EnergyCell,
        WeaponCore,
        HeartTank,
    };

    enum class StageEnemyType
    {
        Walker,
        Turret,
        Drone,
        MiniBoss,
        Boss,
    };

    struct StagePickup
    {
        StagePickupType type{ StagePickupType::EnergyCell };
        game::GameObject body{};
        float2 origin{ 0.0f, 0.0f };
        float bobTimer{ 0.0f };
        bool active{ true };
    };

    struct StageProjectile
    {
        game::GameObject body{};
        float2 velocity{ 0.0f, 0.0f };
        float life{ 0.0f };
        float dir{ 1.0f };
        bool active{ false };
    };

    struct StageEnemy
    {
        StageEnemyType type{ StageEnemyType::Walker };
        game::GameObject body{};
        float2 origin{ 0.0f, 0.0f };
        float patrolMinX{ 0.0f };
        float patrolMaxX{ 0.0f };
        float minY{ 0.0f };
        float maxY{ 0.0f };
        float baseY{ 0.0f };
        float velocityX{ 0.0f };
        float velocityY{ 0.0f };
        float direction{ 1.0f };
        float actionTimer{ 0.0f };
        float cooldown{ 0.0f };
        float flashTimer{ 0.0f };
        int hp{ 1 };
        int maxHp{ 1 };
        bool active{ true };
        bool grounded{ true };
        std::wstring displayName{};
    };

    struct StageRuntime
    {
        std::vector<game::GameObject> solids{};
        std::vector<game::GameObject> spikes{};
        std::vector<StagePickup> pickups{};
        std::vector<StageEnemy> enemies{};
        std::array<StageProjectile, 32> enemyProjectiles{};
        std::array<game::GameObject, 2> gates{};
        std::array<bool, 2> gateActive{ false, false };

        StageEnemy miniBoss{};
        StageEnemy boss{};

        float miniTriggerX{ 1840.0f };
        float miniArenaLeft{ 1780.0f };
        float miniArenaRight{ 2400.0f };
        float bossTriggerX{ 3180.0f };
        float bossArenaLeft{ 3140.0f };
        float bossArenaRight{ 3880.0f };

        bool miniIntroActive{ false };
        bool miniTriggered{ false };
        bool miniCleared{ false };
        bool bossIntroActive{ false };
        bool bossTriggered{ false };
        bool bossCleared{ false };

        bool cameraLocked{ false };
        float cameraLockLeft{ 0.0f };
        float cameraLockRight{ 0.0f };

        float encounterTimer{ 0.0f };
        float bannerTimer{ 0.0f };
        std::wstring bannerText{};

        int energyCells{ 0 };
        int weaponCores{ 0 };
        int heartTanks{ 0 };
        std::array<bool, 4> weaponsOwned{ true, false, false, false };
    };

    static StageRuntime s_stage{};

    static int s_playerHudHp{ 8 };
    static int s_playerHudHpMax{ 8 };
    static bool s_showBossBar{ false };
    static int s_bossHudHp{ 0 };
    static int s_bossHudHpMax{ 0 };

    constexpr float kGroundTopY = 440.0f;
    constexpr float kGateTopY = 180.0f;
    constexpr float kGateHeight = 260.0f;

    constexpr float4 ColorU8(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255) noexcept
    {
        return
        {
            r / 255.0f,
            g / 255.0f,
            b / 255.0f,
            a / 255.0f
        };
    }

    bool RectsOverlap(Rect const& a, Rect const& b) noexcept
    {
        return
            (a.X <= b.X + b.Width) && (a.X + a.Width > b.X) &&
            (a.Y <= b.Y + b.Height) && (a.Y + a.Height > b.Y);
    }

    game::GameObject MakeRectObject(float x, float y, float w, float h, float4 tint)
    {
        game::GameObject obj{};
        obj.SetTexID(Cfg::Textures::WhitePixel);
        obj.SetTexPosition({ 0.0f, 0.0f });
        obj.SetFrameSize({ 1.0f, 1.0f });
        obj.SetTextureOffset({ 0.0f, 0.0f });
        obj.SetWorldPosition({ x, y });
        obj.SetWorldSize({ w, h });
        obj.SetScale({ w, h });
        obj.SetTint(tint);
        obj.SetRotationRad(0.0f);
        obj.SetFlip(engine::CanvasSpriteFlip::None);
        obj.setAffectedByGravity(false);
        return obj;
    }

    void SetRectObject(game::GameObject& obj, float x, float y, float w, float h)
    {
        obj.SetWorldPosition({ x, y });
        obj.SetWorldSize({ w, h });
        obj.SetScale({ w, h });
    }

    float CenterX(game::GameObject const& obj) noexcept
    {
        auto const rect = obj.getWorldRect();
        return rect.X + (rect.Width * 0.5f);
    }

    float CenterY(game::GameObject const& obj) noexcept
    {
        auto const rect = obj.getWorldRect();
        return rect.Y + (rect.Height * 0.5f);
    }

    float2 Center(game::GameObject const& obj) noexcept
    {
        auto const rect = obj.getWorldRect();
        return { rect.X + (rect.Width * 0.5f), rect.Y + (rect.Height * 0.5f) };
    }

    void DrawRect(engine::SpriteBatchScope const& batch_, Rect const& rect_, float4 tint_)
    {
        engine::Sprite spr{ Cfg::GetTexKey(Cfg::Textures::WhitePixel) };
        spr.Position = { rect_.X, rect_.Y };
        spr.Scale = { rect_.Width, rect_.Height };
        spr.SourceRect = Rect{ 0.0f, 0.0f, 1.0f, 1.0f };
        spr.Tint = tint_;
        spr.SetOriginTopLeft();
        spr.Draw(batch_);
    }

    void DrawPanel(engine::SpriteBatchScope const& batch_, Rect const& rect_, float4 fill_, float4 border_)
    {
        DrawRect(batch_, rect_, fill_);

        constexpr float border = 2.0f;
        DrawRect(batch_, Rect{ rect_.X, rect_.Y, rect_.Width, border }, border_);
        DrawRect(batch_, Rect{ rect_.X, rect_.Y + rect_.Height - border, rect_.Width, border }, border_);
        DrawRect(batch_, Rect{ rect_.X, rect_.Y, border, rect_.Height }, border_);
        DrawRect(batch_, Rect{ rect_.X + rect_.Width - border, rect_.Y, border, rect_.Height }, border_);
    }

    StageEnemy MakeStageEnemy(
        StageEnemyType type_,
        float x_,
        float y_,
        float w_,
        float h_,
        float4 tint_,
        int hp_,
        float patrolMinX_,
        float patrolMaxX_,
        std::wstring name_)
    {
        StageEnemy out{};
        out.type = type_;
        out.body = MakeRectObject(x_, y_, w_, h_, tint_);
        out.origin = { x_, y_ };
        out.baseY = y_;
        out.patrolMinX = patrolMinX_;
        out.patrolMaxX = patrolMaxX_;
        out.hp = hp_;
        out.maxHp = hp_;
        out.displayName = std::move(name_);
        return out;
    }

    StagePickup MakePickup(StagePickupType type_, float x_, float y_, float tintAlpha_, float4 tint_)
    {
        StagePickup out{};
        out.type = type_;
        out.origin = { x_, y_ };
        out.body = MakeRectObject(x_, y_, 20.0f, 20.0f, { tint_.x, tint_.y, tint_.z, tintAlpha_ });
        return out;
    }

    void DisableGates()
    {
        s_stage.gateActive = { false, false };
        s_stage.cameraLocked = false;
    }

    void EnableGates(float leftX_, float rightX_, float4 tint_)
    {
        s_stage.gates[0] = MakeRectObject(leftX_, kGateTopY, 24.0f, kGateHeight, tint_);
        s_stage.gates[1] = MakeRectObject(rightX_, kGateTopY, 24.0f, kGateHeight, tint_);
        s_stage.gateActive = { true, true };
    }

    StageProjectile* SpawnEnemyProjectile(
        float x_,
        float y_,
        float w_,
        float h_,
        float2 velocity_,
        float life_,
        float4 tint_)
    {
        for (auto& proj : s_stage.enemyProjectiles)
        {
            if (proj.active)
                continue;

            proj.active = true;
            proj.body = MakeRectObject(x_, y_, w_, h_, tint_);
            proj.velocity = velocity_;
            proj.life = life_;
            proj.dir = (velocity_.x >= 0.0f) ? 1.0f : -1.0f;
            return &proj;
        }
        return nullptr;
    }

    void ResetStageRuntime()
    {
        s_stage = StageRuntime{};

        s_stage.solids.push_back(MakeRectObject(540.0f, 340.0f, 180.0f, 22.0f, ColorU8(84, 148, 188, 255)));
        s_stage.solids.push_back(MakeRectObject(860.0f, 260.0f, 180.0f, 22.0f, ColorU8(96, 170, 208, 255)));
        s_stage.solids.push_back(MakeRectObject(1180.0f, 360.0f, 130.0f, 20.0f, ColorU8(104, 176, 214, 255)));
        s_stage.solids.push_back(MakeRectObject(1480.0f, 240.0f, 220.0f, 22.0f, ColorU8(114, 186, 222, 255)));
        s_stage.solids.push_back(MakeRectObject(1660.0f, 320.0f, 150.0f, 20.0f, ColorU8(88, 160, 198, 255)));
        s_stage.solids.push_back(MakeRectObject(1940.0f, 292.0f, 120.0f, 18.0f, ColorU8(214, 156, 72, 255)));
        s_stage.solids.push_back(MakeRectObject(2220.0f, 292.0f, 120.0f, 18.0f, ColorU8(214, 156, 72, 255)));
        s_stage.solids.push_back(MakeRectObject(2520.0f, 360.0f, 170.0f, 20.0f, ColorU8(98, 164, 204, 255)));
        s_stage.solids.push_back(MakeRectObject(2800.0f, 300.0f, 140.0f, 20.0f, ColorU8(104, 172, 214, 255)));
        s_stage.solids.push_back(MakeRectObject(3360.0f, 300.0f, 130.0f, 18.0f, ColorU8(126, 188, 222, 255)));
        s_stage.solids.push_back(MakeRectObject(3580.0f, 240.0f, 160.0f, 18.0f, ColorU8(126, 188, 222, 255)));

        s_stage.spikes.push_back(MakeRectObject(1090.0f, 418.0f, 150.0f, 22.0f, ColorU8(240, 92, 80, 220)));
        s_stage.spikes.push_back(MakeRectObject(1715.0f, 418.0f, 115.0f, 22.0f, ColorU8(240, 92, 80, 220)));
        s_stage.spikes.push_back(MakeRectObject(2860.0f, 418.0f, 155.0f, 22.0f, ColorU8(240, 92, 80, 220)));

        s_stage.pickups.push_back(MakePickup(StagePickupType::EnergyCell, 615.0f, 306.0f, 0.95f, ColorU8(108, 242, 162, 255)));
        s_stage.pickups.push_back(MakePickup(StagePickupType::HeartTank, 1560.0f, 206.0f, 0.95f, ColorU8(254, 120, 128, 255)));
        s_stage.pickups.push_back(MakePickup(StagePickupType::WeaponCore, 2088.0f, 254.0f, 0.95f, ColorU8(246, 210, 96, 255)));
        s_stage.pickups.push_back(MakePickup(StagePickupType::EnergyCell, 2850.0f, 266.0f, 0.95f, ColorU8(108, 242, 162, 255)));

        auto walkerA = MakeStageEnemy(StageEnemyType::Walker, 780.0f, kGroundTopY - 40.0f, 56.0f, 40.0f, ColorU8(248, 184, 82, 255), 3, 760.0f, 980.0f, L"Strider");
        walkerA.velocityX = 78.0f;
        s_stage.enemies.push_back(walkerA);

        auto turretA = MakeStageEnemy(StageEnemyType::Turret, 1235.0f, 316.0f, 42.0f, 44.0f, ColorU8(248, 216, 94, 255), 4, 1235.0f, 1235.0f, L"Needler");
        turretA.cooldown = 0.85f;
        s_stage.enemies.push_back(turretA);

        auto droneA = MakeStageEnemy(StageEnemyType::Drone, 1530.0f, 220.0f, 46.0f, 30.0f, ColorU8(92, 224, 232, 255), 3, 1510.0f, 1720.0f, L"Skimmer");
        droneA.velocityX = 62.0f;
        droneA.cooldown = 1.2f;
        droneA.minY = 208.0f;
        droneA.maxY = 284.0f;
        s_stage.enemies.push_back(droneA);

        auto walkerB = MakeStageEnemy(StageEnemyType::Walker, 2620.0f, kGroundTopY - 40.0f, 56.0f, 40.0f, ColorU8(248, 184, 82, 255), 3, 2580.0f, 2790.0f, L"Strider");
        walkerB.velocityX = 88.0f;
        walkerB.direction = -1.0f;
        s_stage.enemies.push_back(walkerB);

        auto turretB = MakeStageEnemy(StageEnemyType::Turret, 2910.0f, 256.0f, 42.0f, 44.0f, ColorU8(248, 216, 94, 255), 5, 2910.0f, 2910.0f, L"Needler");
        turretB.cooldown = 0.35f;
        s_stage.enemies.push_back(turretB);

        s_stage.miniBoss = MakeStageEnemy(StageEnemyType::MiniBoss, 2025.0f, kGroundTopY - 96.0f, 148.0f, 96.0f, ColorU8(232, 116, 84, 255), 28, 1860.0f, 2240.0f, L"Crusher Unit");
        s_stage.miniBoss.active = false;
        s_stage.miniBoss.actionTimer = 1.15f;

        s_stage.boss = MakeStageEnemy(StageEnemyType::Boss, 3440.0f, 172.0f, 160.0f, 110.0f, ColorU8(98, 214, 238, 255), 42, 3300.0f, 3700.0f, L"Volt Warden");
        s_stage.boss.active = false;
        s_stage.boss.velocityX = 170.0f;
        s_stage.boss.minY = 132.0f;
        s_stage.boss.maxY = 268.0f;
        s_stage.boss.cooldown = 1.0f;

        DisableGates();

        s_playerHudHp = 8;
        s_playerHudHpMax = 8;
        s_showBossBar = false;
        s_bossHudHp = 0;
        s_bossHudHpMax = 0;
    }

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
        ResetStageRuntime();

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

        // --- HUD text
        auto MakeHudText = [&](float x_, float y_, float size_, winrt::Windows::UI::Color color_) -> engine::Text
            {
                engine::Text out{};
                out.FontRef = Cfg::GetFont(L"bubbly");
                out.FontSize = size_;
                out.OutlineThickness = 2.0f;
                out.OutlineColor = winrt::Windows::UI::Colors::Black();
                out.Color = color_;
                out.Position = { x_, y_ };
                out.Invalidate();
                return out;
            };

        uiStrings.push_back(MakeHudText(18.0f, 10.0f, 18.0f, winrt::Windows::UI::Colors::White()));
        uiStrings.push_back(MakeHudText(18.0f, 40.0f, 18.0f, winrt::Windows::UI::Colors::Green()));
        uiStrings.push_back(MakeHudText(18.0f, 68.0f, 18.0f, winrt::Windows::UI::Colors::DeepSkyBlue()));
        uiStrings.push_back(MakeHudText(0.0f, 30.0f, 32.0f, winrt::Windows::UI::Colors::Orange()));
        uiStrings.back().LayoutBoxSize = { 960.0f, 44.0f };
        uiStrings.back().HorizontalAlignment = winrt::Microsoft::Graphics::Canvas::Text::CanvasHorizontalAlignment::Center;
        uiStrings.back().Invalidate();
        uiStrings.push_back(MakeHudText(0.0f, 86.0f, 18.0f, winrt::Windows::UI::Colors::White()));
        uiStrings.back().LayoutBoxSize = { 960.0f, 30.0f };
        uiStrings.back().HorizontalAlignment = winrt::Microsoft::Graphics::Canvas::Text::CanvasHorizontalAlignment::Center;
        uiStrings.back().Invalidate();


       
        for (auto& s : m_busterShots) s.Kill();
        m_busterCooldown = 0.0f;


        // --- Blueys (enemies)
        for (auto& b : s_blueys) b.reset();

        s_blueys[0] = std::make_unique<game::Bluey>(float2{ 650.0f, 316.0f });
        s_blueys[1] = std::make_unique<game::Bluey>(float2{ 1400.0f, 316.0f });
        for (auto& e : s_blueyElectric) e.Kill();
        for (auto& m : s_blueyMissiles) m.Kill();

        s_shelly = std::make_unique<game::Shelly>(float2{ 1000.0f, 387.0f });
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
        s_stage = StageRuntime{};
        s_playerHudHp = 8;
        s_playerHudHpMax = 8;
        s_showBossBar = false;
        s_bossHudHp = 0;
        s_bossHudHpMax = 0;

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
            constexpr int   baseHpMax = 8;
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

            static int   hp = baseHpMax;
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
            s_stage.bannerTimer = std::max<float>(0.0f, s_stage.bannerTimer - dt_);

            int hpCap = baseHpMax + (s_stage.heartTanks * 2);
            hp = (std::min)(hp, hpCap);


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

            auto CollectSolids = [&](winrt::Windows::Foundation::Rect const& query_, int padTiles_) -> std::vector<game::GameObject*>
                {
                    std::vector<game::GameObject*> out;

                    auto tileHits = tmap->getSolidTilesInRect(query_, padTiles_);
                    out.reserve(tileHits.size() + s_stage.solids.size() + s_stage.gates.size());

                    for (auto* tile : tileHits)
                    {
                        out.push_back(tile);
                    }

                    for (auto& solid : s_stage.solids)
                    {
                        if (Overlaps(query_, solid.getWorldRect()))
                            out.push_back(&solid);
                    }

                    for (size_t i = 0; i < s_stage.gates.size(); ++i)
                    {
                        if (s_stage.gateActive[i] && Overlaps(query_, s_stage.gates[i].getWorldRect()))
                            out.push_back(&s_stage.gates[i]);
                    }

                    return out;
                };

            auto ProbeSolid = [&](winrt::Windows::Foundation::Rect const& probe) -> bool
                {
                    auto nearby = CollectSolids(probe, 0);
                    for (auto* t : nearby)
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
                auto tiles = CollectSolids(sweepR, 1);

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
                    underVec = CollectSolids(underRect, 1);

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

                    auto nearTiles = CollectSolids(sweep, 1);
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

                    hp = std::max<int>(0, hp - hitDamage);
                    
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

                    if (hp <= 0)
                    {
                        dead = true;
                        hitStunTimer = 0.0f;
                    }
                    
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

                        auto nearTiles = CollectSolids(sweep, 1);
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

                        auto nearTiles = CollectSolids(sweep, 1);
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
                
                    auto SetBanner = [&](std::wstring text_, float time_)
                        {
                            s_stage.bannerText = std::move(text_);
                            s_stage.bannerTimer = time_;
                        };

                    auto ApplyStageHit = [&](float fromDir) -> bool
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

                            Cfg::PlaySfx(L"player_hit", 0.65f);
                            return true;
                        };

                    auto UpdateEnemyFlash = [&](StageEnemy& enemy_)
                        {
                            if (!enemy_.active)
                                return;

                            enemy_.flashTimer = std::max<float>(0.0f, enemy_.flashTimer - dt_);
                            auto tint = enemy_.body.GetTint();
                            tint.w = (enemy_.flashTimer > 0.0f) ? 0.35f : 1.0f;
                            enemy_.body.SetTint(tint);
                        };

                    auto DamageStageEnemy = [&](StageEnemy& enemy_, int damage_) -> bool
                        {
                            if (!enemy_.active)
                                return false;

                            enemy_.hp = std::max<int>(0, enemy_.hp - damage_);
                            enemy_.flashTimer = 0.18f;

                            if (enemy_.hp <= 0)
                            {
                                enemy_.active = false;
                                Cfg::PlaySfx(Cfg::Sounds::EnemyDie, 0.70f);
                            }
                            else
                            {
                                Cfg::PlaySfx(Cfg::Sounds::EnemyHit, 0.65f);
                            }

                            return true;
                        };

                    auto FireAtPlayer = [&](StageEnemy const& enemy_, float speed_, float width_, float height_, float4 tint_)
                        {
                            float2 const enemyCenter = Center(enemy_.body);
                            float2 velocity
                            {
                                pCenter.x - enemyCenter.x,
                                pCenter.y - enemyCenter.y
                            };

                            float len = std::sqrt((velocity.x * velocity.x) + (velocity.y * velocity.y));
                            if (len < 0.0001f)
                                len = 1.0f;

                            velocity.x = (velocity.x / len) * speed_;
                            velocity.y = (velocity.y / len) * speed_;

                            SpawnEnemyProjectile(
                                enemyCenter.x - (width_ * 0.5f),
                                enemyCenter.y - (height_ * 0.5f),
                                width_,
                                height_,
                                velocity,
                                3.0f,
                                tint_);
                        };

                    if (!s_stage.miniTriggered && pCenter.x >= s_stage.miniTriggerX)
                    {
                        s_stage.miniTriggered = true;
                        s_stage.miniIntroActive = true;
                        s_stage.encounterTimer = 1.05f;
                        s_stage.cameraLocked = true;
                        s_stage.cameraLockLeft = s_stage.miniArenaLeft;
                        s_stage.cameraLockRight = s_stage.miniArenaRight;
                        EnableGates(s_stage.miniArenaLeft, s_stage.miniArenaRight, ColorU8(238, 150, 82, 220));
                        SetBanner(L"WARNING  MID BOSS BLOCKADE", 1.20f);
                    }

                    if (s_stage.miniIntroActive)
                    {
                        s_stage.encounterTimer = std::max<float>(0.0f, s_stage.encounterTimer - dt_);
                        if (s_stage.encounterTimer <= 0.0f)
                        {
                            s_stage.miniIntroActive = false;
                            s_stage.miniBoss.active = true;
                            SetBanner(s_stage.miniBoss.displayName, 1.25f);
                        }
                    }

                    if (s_stage.miniCleared && !s_stage.bossTriggered && pCenter.x >= s_stage.bossTriggerX)
                    {
                        s_stage.bossTriggered = true;
                        s_stage.bossIntroActive = true;
                        s_stage.encounterTimer = 1.20f;
                        s_stage.cameraLocked = true;
                        s_stage.cameraLockLeft = s_stage.bossArenaLeft;
                        s_stage.cameraLockRight = s_stage.bossArenaRight;
                        EnableGates(s_stage.bossArenaLeft, s_stage.bossArenaRight, ColorU8(84, 210, 236, 220));
                        SetBanner(L"WARNING  BOSS DOORS SEALED", 1.25f);
                    }

                    if (s_stage.bossIntroActive)
                    {
                        s_stage.encounterTimer = std::max<float>(0.0f, s_stage.encounterTimer - dt_);
                        if (s_stage.encounterTimer <= 0.0f)
                        {
                            s_stage.bossIntroActive = false;
                            s_stage.boss.active = true;
                            SetBanner(s_stage.boss.displayName, 1.35f);
                        }
                    }

                    auto const playerRectStage = player->getWorldRect();

                    for (auto& pickup : s_stage.pickups)
                    {
                        if (!pickup.active)
                            continue;

                        pickup.bobTimer += dt_ * 2.6f;

                        auto pos = pickup.origin;
                        pos.y += std::sin(pickup.bobTimer) * 5.0f;
                        pickup.body.SetWorldPosition(pos);

                        if (!RectsOverlap(playerRectStage, pickup.body.getWorldRect()))
                            continue;

                        pickup.active = false;

                        switch (pickup.type)
                        {
                        case StagePickupType::EnergyCell:
                            ++s_stage.energyCells;
                            hp = (std::min)(hp + 2, hpCap);
                            SetBanner(L"ENERGY CELL RECOVERED", 1.05f);
                            break;

                        case StagePickupType::WeaponCore:
                            ++s_stage.weaponCores;
                            s_stage.weaponsOwned[1] = true;
                            SetBanner(L"ARC CHIP ACQUIRED", 1.30f);
                            break;

                        case StagePickupType::HeartTank:
                            ++s_stage.heartTanks;
                            hpCap = baseHpMax + (s_stage.heartTanks * 2);
                            hp = (std::min)(hp + 2, hpCap);
                            SetBanner(L"HEART TANK INSTALLED", 1.30f);
                            break;
                        }
                    }

                    for (auto& enemy : s_stage.enemies)
                    {
                        if (!enemy.active)
                            continue;

                        UpdateEnemyFlash(enemy);

                        switch (enemy.type)
                        {
                        case StageEnemyType::Walker:
                        {
                            auto pos = enemy.body.GetWorldPosition();
                            pos.x += enemy.direction * enemy.velocityX * dt_;

                            if (pos.x <= enemy.patrolMinX)
                            {
                                pos.x = enemy.patrolMinX;
                                enemy.direction = 1.0f;
                            }
                            else if (pos.x >= enemy.patrolMaxX)
                            {
                                pos.x = enemy.patrolMaxX;
                                enemy.direction = -1.0f;
                            }

                            enemy.body.SetWorldPosition(pos);
                            break;
                        }

                        case StageEnemyType::Turret:
                        {
                            enemy.cooldown = std::max<float>(0.0f, enemy.cooldown - dt_);
                            if (enemy.cooldown <= 0.0f && std::abs(CenterX(enemy.body) - pCenter.x) < 620.0f)
                            {
                                FireAtPlayer(enemy, 330.0f, 12.0f, 12.0f, ColorU8(255, 216, 108, 255));
                                enemy.cooldown = 1.30f;
                            }
                            break;
                        }

                        case StageEnemyType::Drone:
                        {
                            enemy.actionTimer += dt_;
                            auto pos = enemy.body.GetWorldPosition();
                            pos.x += enemy.direction * enemy.velocityX * dt_;

                            if (pos.x <= enemy.patrolMinX)
                            {
                                pos.x = enemy.patrolMinX;
                                enemy.direction = 1.0f;
                            }
                            else if (pos.x >= enemy.patrolMaxX)
                            {
                                pos.x = enemy.patrolMaxX;
                                enemy.direction = -1.0f;
                            }

                            pos.y = enemy.origin.y + (std::sin(enemy.actionTimer * 2.0f) * 26.0f);
                            pos.y = std::clamp(pos.y, enemy.minY, enemy.maxY);
                            enemy.body.SetWorldPosition(pos);

                            enemy.cooldown = std::max<float>(0.0f, enemy.cooldown - dt_);
                            if (enemy.cooldown <= 0.0f)
                            {
                                FireAtPlayer(enemy, 280.0f, 10.0f, 16.0f, ColorU8(126, 236, 248, 255));
                                enemy.cooldown = 1.60f;
                            }
                            break;
                        }

                        default:
                            break;
                        }

                        if (enemy.active && RectsOverlap(enemy.body.getWorldRect(), playerRectStage))
                        {
                            ApplyStageHit((CenterX(enemy.body) >= pCenter.x) ? 1.0f : -1.0f);
                        }
                    }

                    if (s_stage.miniBoss.active)
                    {
                        auto& enemy = s_stage.miniBoss;
                        UpdateEnemyFlash(enemy);

                        auto pos = enemy.body.GetWorldPosition();

                        if (enemy.grounded)
                        {
                            pos.x += enemy.direction * 92.0f * dt_;

                            if (pos.x <= enemy.patrolMinX)
                            {
                                pos.x = enemy.patrolMinX;
                                enemy.direction = 1.0f;
                            }
                            else if (pos.x >= enemy.patrolMaxX)
                            {
                                pos.x = enemy.patrolMaxX;
                                enemy.direction = -1.0f;
                            }

                            enemy.actionTimer = std::max<float>(0.0f, enemy.actionTimer - dt_);
                            if (enemy.actionTimer <= 0.0f)
                            {
                                enemy.grounded = false;
                                enemy.velocityY = -780.0f;
                                enemy.velocityX = (pCenter.x < CenterX(enemy.body)) ? -230.0f : 230.0f;
                                enemy.direction = (enemy.velocityX >= 0.0f) ? 1.0f : -1.0f;
                            }
                        }
                        else
                        {
                            enemy.velocityY += 1940.0f * dt_;
                            pos.x += enemy.velocityX * dt_;
                            pos.y += enemy.velocityY * dt_;
                            pos.x = std::clamp(pos.x, enemy.patrolMinX, enemy.patrolMaxX);

                            if (pos.y >= enemy.baseY)
                            {
                                pos.y = enemy.baseY;
                                enemy.grounded = true;
                                enemy.velocityY = 0.0f;
                                enemy.actionTimer = 1.00f;

                                auto const rect = enemy.body.getWorldRect();
                                SpawnEnemyProjectile(rect.X + 8.0f, kGroundTopY - 10.0f, 28.0f, 10.0f, { -290.0f, 0.0f }, 1.60f, ColorU8(255, 134, 92, 255));
                                SpawnEnemyProjectile(rect.X + rect.Width - 36.0f, kGroundTopY - 10.0f, 28.0f, 10.0f, { 290.0f, 0.0f }, 1.60f, ColorU8(255, 134, 92, 255));
                            }
                        }

                        enemy.body.SetWorldPosition(pos);

                        if (RectsOverlap(enemy.body.getWorldRect(), playerRectStage))
                        {
                            ApplyStageHit((CenterX(enemy.body) >= pCenter.x) ? 1.0f : -1.0f);
                        }
                    }

                    if (s_stage.boss.active)
                    {
                        auto& enemy = s_stage.boss;
                        UpdateEnemyFlash(enemy);

                        enemy.actionTimer += dt_;
                        enemy.cooldown = std::max<float>(0.0f, enemy.cooldown - dt_);

                        auto pos = enemy.body.GetWorldPosition();
                        pos.x += enemy.velocityX * dt_;

                        if (pos.x <= enemy.patrolMinX)
                        {
                            pos.x = enemy.patrolMinX;
                            enemy.velocityX = std::abs(enemy.velocityX);
                        }
                        else if (pos.x >= enemy.patrolMaxX)
                        {
                            pos.x = enemy.patrolMaxX;
                            enemy.velocityX = -std::abs(enemy.velocityX);
                        }

                        float yRange = std::max<float>(10.0f, enemy.maxY - enemy.minY);
                        pos.y = enemy.minY + (((std::sin(enemy.actionTimer * 1.7f) * 0.5f) + 0.5f) * yRange);
                        enemy.body.SetWorldPosition(pos);

                        if (enemy.cooldown <= 0.0f)
                        {
                            float2 enemyCenter = Center(enemy.body);
                            float2 toPlayer
                            {
                                pCenter.x - enemyCenter.x,
                                pCenter.y - enemyCenter.y
                            };

                            float baseLen = std::sqrt((toPlayer.x * toPlayer.x) + (toPlayer.y * toPlayer.y));
                            if (baseLen < 0.0001f)
                                baseLen = 1.0f;

                            toPlayer.x /= baseLen;
                            toPlayer.y /= baseLen;

                            for (float spread : { -0.35f, 0.0f, 0.35f })
                            {
                                float2 vel
                                {
                                    (toPlayer.x * 320.0f) + (spread * 120.0f),
                                    (toPlayer.y * 320.0f) + (spread * 45.0f)
                                };

                                SpawnEnemyProjectile(enemyCenter.x - 7.0f, enemyCenter.y - 7.0f, 14.0f, 14.0f, vel, 3.00f, ColorU8(120, 244, 255, 255));
                            }

                            enemy.velocityX = (pCenter.x < enemyCenter.x) ? -260.0f : 260.0f;
                            enemy.cooldown = (enemy.hp <= (enemy.maxHp / 2)) ? 0.95f : 1.35f;
                        }

                        if (RectsOverlap(enemy.body.getWorldRect(), playerRectStage))
                        {
                            ApplyStageHit((CenterX(enemy.body) >= pCenter.x) ? 1.0f : -1.0f);
                        }
                    }

                    for (auto& shot : m_busterShots)
                    {
                        if (!shot.Active)
                            continue;

                        for (auto& enemy : s_stage.enemies)
                        {
                            if (!enemy.active)
                                continue;

                            if (!RectsOverlap(shot.getWorldRect(), enemy.body.getWorldRect()))
                                continue;

                            shot.Kill();
                            DamageStageEnemy(enemy, 1);
                            break;
                        }

                        if (!shot.Active)
                            continue;

                        if (s_stage.miniBoss.active && RectsOverlap(shot.getWorldRect(), s_stage.miniBoss.body.getWorldRect()))
                        {
                            shot.Kill();
                            DamageStageEnemy(s_stage.miniBoss, 1);
                        }

                        if (!shot.Active)
                            continue;

                        if (s_stage.boss.active && RectsOverlap(shot.getWorldRect(), s_stage.boss.body.getWorldRect()))
                        {
                            shot.Kill();
                            DamageStageEnemy(s_stage.boss, 1);
                        }
                    }

                    if (!s_stage.miniCleared && s_stage.miniBoss.hp <= 0)
                    {
                        s_stage.miniBoss.active = false;
                        s_stage.miniCleared = true;
                        s_stage.weaponsOwned[2] = true;
                        DisableGates();
                        SetBanner(L"PULSE BREAKER ONLINE", 1.45f);
                    }

                    if (!s_stage.bossCleared && s_stage.boss.hp <= 0)
                    {
                        s_stage.boss.active = false;
                        s_stage.bossCleared = true;
                        s_stage.weaponsOwned[3] = true;
                        DisableGates();
                        SetBanner(L"VOLT EDGE ACQUIRED", 1.60f);
                    }

                    for (auto& spike : s_stage.spikes)
                    {
                        if (RectsOverlap(playerRectStage, spike.getWorldRect()))
                        {
                            ApplyStageHit((CenterX(spike) >= pCenter.x) ? 1.0f : -1.0f);
                            break;
                        }
                    }

                    for (auto& proj : s_stage.enemyProjectiles)
                    {
                        if (!proj.active)
                            continue;

                        proj.life -= dt_;
                        if (proj.life <= 0.0f)
                        {
                            proj.active = false;
                            continue;
                        }

                        auto before = proj.body.GetWorldPosition();
                        proj.body.Move({ proj.velocity.x * dt_, proj.velocity.y * dt_ });
                        auto after = proj.body.GetWorldPosition();
                        auto size = proj.body.GetWorldSize();

                        Rect r0{ before.x, before.y, size.x, size.y };
                        Rect r1{ after.x, after.y, size.x, size.y };

                        float left = std::min<float>(r0.X, r1.X);
                        float top = std::min<float>(r0.Y, r1.Y);
                        float right = std::max<float>(r0.X + r0.Width, r1.X + r1.Width);
                        float bottom = std::max<float>(r0.Y + r0.Height, r1.Y + r1.Height);

                        Rect sweep{ left, top, right - left, bottom - top };
                        auto solids = CollectSolids(sweep, 1);

                        bool hitSolid = false;
                        for (auto* solid : solids)
                        {
                            if (solid && RectsOverlap(sweep, solid->getWorldRect()))
                            {
                                hitSolid = true;
                                break;
                            }
                        }

                        if (hitSolid)
                        {
                            proj.active = false;
                            continue;
                        }

                        if (RectsOverlap(proj.body.getWorldRect(), playerRectStage))
                        {
                            ApplyStageHit(proj.dir);
                            proj.active = false;
                        }
                    }

                    s_showBossBar = s_stage.boss.active;
                    s_bossHudHp = s_stage.boss.active ? s_stage.boss.hp : 0;
                    s_bossHudHpMax = s_stage.boss.active ? s_stage.boss.maxHp : 0;
                    s_playerHudHp = hp;
                    s_playerHudHpMax = hpCap;

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

                auto nearTiles = CollectSolids(sweep, 1);
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

            if (s_stage.cameraLocked)
            {
                float lockMin = s_stage.cameraLockLeft + camHalfW;
                float lockMax = s_stage.cameraLockRight - camHalfW;

                if (lockMax < lockMin)
                {
                    newCamX = (s_stage.cameraLockLeft + s_stage.cameraLockRight) * 0.5f;
                }
                else
                {
                    newCamX = std::clamp(newCamX, lockMin, lockMax);
                }
            }

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
        float const viewLeft = camera->Position.x - (camera->getWidth() * 0.5f);
        float const viewTop = camera->Position.y - (camera->getHeight() * 0.5f);
        float const viewWidth = camera->getWidth();
        float const viewHeight = camera->getHeight();

        DrawRect(batch_, Rect{ viewLeft, viewTop, viewWidth, viewHeight }, ColorU8(14, 24, 38, 255));
        DrawRect(batch_, Rect{ viewLeft, viewTop + 92.0f, viewWidth, viewHeight - 92.0f }, ColorU8(24, 42, 58, 188));

        auto DrawParallaxLayer = [&](float factor_, float baseY_, float spanW_, std::array<float, 6> const& heights_, float4 color_)
            {
                float startX = std::floor(((camera->Position.x * (1.0f - factor_)) - spanW_) / spanW_) * spanW_;
                for (int i = -1; i < 6; ++i)
                {
                    float x = startX + (i * spanW_);
                    float height = heights_[static_cast<size_t>((i + 6) % 6)];
                    DrawRect(batch_, Rect{ x, viewTop + baseY_ + (200.0f - height), spanW_ - 28.0f, height }, color_);
                }
            };

        static constexpr std::array<float, 6> layerAHeights{ 90.0f, 160.0f, 120.0f, 190.0f, 110.0f, 150.0f };
        static constexpr std::array<float, 6> layerBHeights{ 120.0f, 210.0f, 160.0f, 180.0f, 130.0f, 220.0f };
        static constexpr std::array<float, 6> layerCHeights{ 150.0f, 240.0f, 200.0f, 170.0f, 220.0f, 195.0f };

        DrawParallaxLayer(0.15f, 280.0f, 240.0f, layerAHeights, ColorU8(42, 78, 96, 88));
        DrawParallaxLayer(0.30f, 320.0f, 220.0f, layerBHeights, ColorU8(62, 106, 124, 116));
        DrawParallaxLayer(0.52f, 350.0f, 200.0f, layerCHeights, ColorU8(92, 148, 164, 144));

        if (s_stage.boss.active || s_stage.bossIntroActive)
        {
            DrawRect(batch_, Rect{ viewLeft, viewTop, viewWidth, viewHeight }, ColorU8(16, 36, 48, 84));
        }

        tmap->render(batch_, *camera);

        for (auto const& solid : s_stage.solids)
        {
            DrawPanel(batch_, solid.getWorldRect(), solid.GetTint(), ColorU8(18, 32, 48, 220));
        }

        for (auto const& spike : s_stage.spikes)
        {
            auto const rect = spike.getWorldRect();
            DrawPanel(batch_, rect, spike.GetTint(), ColorU8(88, 18, 18, 220));

            constexpr float toothW = 18.0f;
            for (int i = 0; i < 8; ++i)
            {
                DrawRect(
                    batch_,
                    Rect{ rect.X + (i * toothW), rect.Y - 10.0f, toothW - 4.0f, 10.0f },
                    ColorU8(255, 154, 138, 220));
            }
        }

        for (auto const& pickup : s_stage.pickups)
        {
            if (!pickup.active)
                continue;

            DrawPanel(batch_, pickup.body.getWorldRect(), pickup.body.GetTint(), ColorU8(255, 255, 255, 180));
        }

        for (auto const& enemy : s_stage.enemies)
        {
            if (!enemy.active)
                continue;

            DrawPanel(batch_, enemy.body.getWorldRect(), enemy.body.GetTint(), ColorU8(22, 26, 30, 200));
        }

        if (s_stage.miniBoss.active)
        {
            DrawPanel(batch_, s_stage.miniBoss.body.getWorldRect(), s_stage.miniBoss.body.GetTint(), ColorU8(64, 18, 12, 220));
        }

        if (s_stage.boss.active)
        {
            DrawPanel(batch_, s_stage.boss.body.getWorldRect(), s_stage.boss.body.GetTint(), ColorU8(12, 42, 52, 220));
        }

        for (size_t i = 0; i < s_stage.gates.size(); ++i)
        {
            if (!s_stage.gateActive[i])
                continue;

            auto const gateRect = s_stage.gates[i].getWorldRect();
            DrawPanel(batch_, gateRect, s_stage.gates[i].GetTint(), ColorU8(255, 255, 255, 90));

            for (int bar = 0; bar < 4; ++bar)
            {
                DrawRect(
                    batch_,
                    Rect{ gateRect.X + 4.0f + (bar * 5.0f), gateRect.Y + 8.0f, 2.0f, gateRect.Height - 16.0f },
                    ColorU8(255, 255, 255, 90));
            }
        }

        for (auto& b : s_blueys)
        {
            if (b) b->getSprite().Draw(batch_);
        }

        if (s_shelly)
        {
            s_shelly->getSprite().Draw(batch_);
        }

        for (auto& s : s_shellyShots)
        {
            if (s.Active)
                s.getSprite().Draw(batch_);
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

        for (auto const& proj : s_stage.enemyProjectiles)
        {
            if (proj.active)
            {
                DrawPanel(batch_, proj.body.getWorldRect(), proj.body.GetTint(), ColorU8(255, 255, 255, 60));
            }
        }

        if (player)
        {
            player->getSprite().Draw(batch_);
        }

        for (auto& shot : m_busterShots)
        {
            if (shot.Active)
                shot.getSprite().Draw(batch_);
        }

        float hudLeft = camera->Position.x - (camera->getWidth() * 0.5f) + 12.0f;
        float hudTop = camera->Position.y - (camera->getHeight() * 0.5f) + 12.0f;

        DrawPanel(batch_, Rect{ hudLeft, hudTop, 332.0f, 78.0f }, ColorU8(12, 20, 28, 190), ColorU8(98, 192, 224, 210));

        for (int i = 0; i < s_playerHudHpMax; ++i)
        {
            int row = i / 8;
            int col = i % 8;
            float x = hudLeft + 12.0f + (col * 19.0f);
            float y = hudTop + 12.0f + (row * 24.0f);

            DrawPanel(
                batch_,
                Rect{ x, y, 15.0f, 18.0f },
                (i < s_playerHudHp) ? ColorU8(255, 108, 92, 255) : ColorU8(62, 70, 78, 255),
                ColorU8(12, 12, 12, 220));
        }

        float pickupPanelX = hudLeft + 184.0f;
        DrawPanel(batch_, Rect{ pickupPanelX, hudTop + 10.0f, 136.0f, 54.0f }, ColorU8(18, 32, 42, 180), ColorU8(88, 160, 198, 210));
        DrawPanel(batch_, Rect{ pickupPanelX + 8.0f, hudTop + 18.0f, 18.0f, 18.0f }, ColorU8(108, 242, 162, 255), ColorU8(16, 28, 22, 220));
        DrawPanel(batch_, Rect{ pickupPanelX + 8.0f, hudTop + 40.0f, 18.0f, 18.0f }, ColorU8(246, 210, 96, 255), ColorU8(36, 30, 12, 220));
        DrawPanel(batch_, Rect{ pickupPanelX + 72.0f, hudTop + 18.0f, 18.0f, 18.0f }, ColorU8(254, 120, 128, 255), ColorU8(42, 14, 18, 220));

        float weaponPanelX = camera->Position.x - (camera->getWidth() * 0.5f) + 356.0f;
        DrawPanel(batch_, Rect{ weaponPanelX, hudTop, 592.0f, 52.0f }, ColorU8(12, 20, 28, 170), ColorU8(84, 170, 204, 210));

        std::array<float4, 4> weaponColors
        {
            ColorU8(84, 196, 246, 255),
            ColorU8(246, 208, 108, 255),
            ColorU8(248, 152, 92, 255),
            ColorU8(118, 244, 255, 255)
        };

        for (size_t i = 0; i < s_stage.weaponsOwned.size(); ++i)
        {
            float x = weaponPanelX + 12.0f + (static_cast<float>(i) * 142.0f);
            DrawPanel(
                batch_,
                Rect{ x, hudTop + 10.0f, 126.0f, 30.0f },
                s_stage.weaponsOwned[i] ? weaponColors[i] : ColorU8(42, 52, 60, 255),
                ColorU8(10, 12, 16, 220));
        }

        if (s_showBossBar && s_bossHudHpMax > 0)
        {
            float bossLeft = camera->Position.x - 170.0f;
            float bossTop = camera->Position.y - (camera->getHeight() * 0.5f) + 22.0f;
            float ratio = std::clamp(static_cast<float>(s_bossHudHp) / static_cast<float>(s_bossHudHpMax), 0.0f, 1.0f);

            DrawPanel(batch_, Rect{ bossLeft, bossTop, 340.0f, 22.0f }, ColorU8(16, 24, 30, 220), ColorU8(112, 240, 255, 210));
            DrawRect(batch_, Rect{ bossLeft + 4.0f, bossTop + 4.0f, 332.0f * ratio, 14.0f }, ColorU8(112, 240, 255, 255));
        }

        uiStrings[0].String = L"STAGE 01   FORGE LINE";
        uiStrings[0].Invalidate();

        uiStrings[1].String =
            L"Cells " + std::to_wstring(s_stage.energyCells) +
            L"   Cores " + std::to_wstring(s_stage.weaponCores) +
            L"   Hearts " + std::to_wstring(s_stage.heartTanks);
        uiStrings[1].Invalidate();

        uiStrings[2].String =
            std::wstring(L"BSTR ") + (s_stage.weaponsOwned[0] ? L"ON" : L"LOCK") +
            L"   ARC " + (s_stage.weaponsOwned[1] ? L"ON" : L"LOCK") +
            L"   PLS " + (s_stage.weaponsOwned[2] ? L"ON" : L"LOCK") +
            L"   VLT " + (s_stage.weaponsOwned[3] ? L"ON" : L"LOCK");
        uiStrings[2].Invalidate();

        uiStrings[3].String = (s_stage.bannerTimer > 0.0f) ? s_stage.bannerText : L"";
        uiStrings[3].Invalidate();

        uiStrings[4].String = s_showBossBar ? s_stage.boss.displayName : L"";
        uiStrings[4].Invalidate();

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
