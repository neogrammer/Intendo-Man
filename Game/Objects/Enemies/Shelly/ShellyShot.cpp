#include "pch.h"
#include "ShellyShot.h"

#include <cmath>

namespace game
{
    using winrt::Windows::Foundation::Numerics::float2;

    // Reuse the first pellet travel frame from the existing BusterShot texture.
    static constexpr float kSrcX = 147.0f;
    static constexpr float kSrcY = 0.0f;
    static constexpr float kFrameW = 21.0f;
    static constexpr float kFrameH = 26.0f;

    ShellyShot::ShellyShot()
        : GameObject{}
    {
        SetTexID(Cfg::Textures::BusterShot);

        SetTexPosition({ kSrcX, kSrcY });
        SetFrameSize({ kFrameW, kFrameH });

        SetWorldSize({ ColliderW, ColliderH });
        SetTextureOffset({ 5.0f, 8.0f });

        SetScale({ 1.0f, 1.0f });
        SetRotationRad(0.0f);
        SetTint({ 1.0f, 1.0f, 1.0f, 1.0f });
        SetFlip(engine::CanvasSpriteFlip::None);

        setAffectedByGravity(false);
    }

    void ShellyShot::Spawn(float2 worldPos, float2 targetPos)
    {
        Active = true;
        Life = LifeMax;
        SetWorldPosition(worldPos);

        float2 d{ targetPos.x - worldPos.x, targetPos.y - worldPos.y };
        float len = std::sqrt((d.x * d.x) + (d.y * d.y));

        if (len < 0.0001f)
        {
            d = { 1.0f, 0.0f };
            len = 1.0f;
        }

        d.x /= len;
        d.y /= len;

        Velocity = { d.x * Speed, d.y * Speed };
        Dir = (Velocity.x >= 0.0f) ? 1.0f : -1.0f;

        SetFlip(Dir >= 0.0f ? engine::CanvasSpriteFlip::None
            : engine::CanvasSpriteFlip::Horizontal);
    }

    void ShellyShot::UpdateShot(float dt)
    {
        if (!Active) return;

        Life -= dt;
        if (Life <= 0.0f)
        {
            Active = false;
            return;
        }

        Move({ Velocity.x * dt, Velocity.y * dt });
    }
}