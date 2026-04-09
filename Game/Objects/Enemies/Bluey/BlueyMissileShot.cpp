#include "pch.h"
#include "BlueyMissileShot.h"

namespace game
{
    using winrt::Windows::Foundation::Numerics::float2;

    // Weapons grid is 102x67.5. Missile-with-exhaust at (col=2,row=9).
    static constexpr float kCellX = 2.0f * 102.0f;
    static constexpr float kCellY = 9.0f * 67.5f;

    static constexpr float kSrcX = kCellX + 6.0f;
    static constexpr float kSrcY = kCellY + 17.5f;
    static constexpr float kSrcW = 64.5f;
    static constexpr float kSrcH = 21.5f;

    BlueyMissileShot::BlueyMissileShot()
        : GameObject{}
    {
        SetTexID(Cfg::Textures::BlueyAtlas);

        SetTexPosition({ kSrcX, kSrcY });
        SetFrameSize({ kSrcW, kSrcH });

        SetWorldSize({ ColliderW, ColliderH });

        float offX = (kSrcW - ColliderW) * 0.5f;
        float offY = (kSrcH - ColliderH) * 0.5f;
        SetTextureOffset({ offX, offY });

        SetScale({ 1.0f, 1.0f });
        SetRotationRad(0.0f);
        SetTint({ 1.0f, 1.0f, 1.0f, 1.0f });
        SetFlip(engine::CanvasSpriteFlip::None);

        setAffectedByGravity(false);
    }

    void BlueyMissileShot::Spawn(float2 worldPos, float dir)
    {
        Active = true;
        Launched = false;

        Dir = (dir >= 0.0f) ? 1.0f : -1.0f;
        Life = LifeMax;

        SetWorldPosition(worldPos);
        Velocity = { 0.0f, 0.0f };

        SetFlip(Dir >= 0.0f ? engine::CanvasSpriteFlip::None
            : engine::CanvasSpriteFlip::Horizontal);
    }

    void BlueyMissileShot::Launch()
    {
        if (!Active) return;
        if (Launched) return;

        Launched = true;
        Velocity = { Dir * Speed, 0.0f };
    }

    void BlueyMissileShot::UpdateShot(float dt)
    {
        if (!Active) return;

        Life -= dt;
        if (Life <= 0.0f)
        {
            Kill();
            return;
        }

        if (Launched)
        {
            Move({ Velocity.x * dt, Velocity.y * dt });
        }
    }
}