#include "pch.h"
#include "BlueyElectricShot.h"

namespace game
{
    using winrt::Windows::Foundation::Numerics::float2;

    // Weapons grid is 204x135. First weapons row starts at row 8 => y = 8*135 = 1080.
    // Small electric orb crop near the start of that cell.
    static constexpr float kTexX = 0.0f;
    static constexpr float kTexY = 1080.0f;

    BlueyElectricShot::BlueyElectricShot()
        : GameObject{}
    {
        SetTexID(Cfg::Textures::BlueyAtlas);

        SetFrameSize({ FrameW, FrameH });
        SetTexPosition({ kTexX, kTexY });

        SetWorldSize({ ColliderW, ColliderH });
        SetTextureOffset({ (FrameW - ColliderW) * 0.5f, (FrameH - ColliderH) * 0.5f });

        SetScale({ 1.0f, 1.0f });
        SetRotationRad(0.0f);
        SetTint({ 1.0f, 1.0f, 1.0f, 1.0f });
        SetFlip(engine::CanvasSpriteFlip::None);

        setAffectedByGravity(false);
    }

    void BlueyElectricShot::Spawn(float2 worldPos, float dir, float targetX)
    {
        Active = true;
        phase = Phase::Falling;

        Dir = (dir >= 0.0f) ? 1.0f : -1.0f;
        TargetX = targetX;

        Life = LifeMax;
        GroundDelayTimer = 0.0f;

        SetWorldPosition(worldPos);
        Velocity = { 0.0f, SpeedDown };

        // Flip not super important for the orb, but keep consistent
        SetFlip(Dir >= 0.0f ? engine::CanvasSpriteFlip::None
            : engine::CanvasSpriteFlip::Horizontal);
    }

    void BlueyElectricShot::LandOnGround(float groundTopY)
    {
        if (!Active) return;
        if (phase != Phase::Falling) return;

        // Snap to ground, stop, then wait a beat before moving horizontally
        auto p = GetWorldPosition();
        p.y = groundTopY - GetWorldSize().y;
        SetWorldPosition(p);

        Velocity = { 0.0f, 0.0f };
        phase = Phase::GroundDelay;
        GroundDelayTimer = GroundDelay;
    }

    void BlueyElectricShot::UpdateShot(float dt)
    {
        if (!Active) return;

        Life -= dt;
        if (Life <= 0.0f)
        {
            Kill();
            return;
        }

        if (phase == Phase::GroundDelay)
        {
            GroundDelayTimer -= dt;
            if (GroundDelayTimer > 0.0f)
                return;

            // Begin horizontal travel
            phase = Phase::GroundTravel;
            Velocity = { Dir * SpeedGround, 0.0f };
        }

        if (phase == Phase::Falling || phase == Phase::GroundTravel)
        {
            Move({ Velocity.x * dt, Velocity.y * dt });
        }

        // Miss logic: once traveling on ground, despawn after passing the player's X captured at fire-time
        if (phase == Phase::GroundTravel)
        {
            float cx = GetWorldPosition().x + (GetWorldSize().x * 0.5f);
            if ((Dir > 0.0f && cx > TargetX) || (Dir < 0.0f && cx < TargetX))
            {
                Kill();
                return;
            }
        }
    }
}