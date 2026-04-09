#include "pch.h"
#include "BusterShot.h"
#include <cmath>
namespace game
{
    using winrt::Windows::Foundation::Numerics::float2;

    // We’re using the LAST 4 frames of your 11-frame strip (the pellet travel frames).
    // Your sheet is 244x26, but frames are effectively 21x26 cells.
    // Frame X positions if cells are 21 wide:
    // 0:0, 1:21, 2:42, 3:63, 4:84, 5:105, 6:126, 7:147, 8:168, 9:189 10::210
    static constexpr float kPelletX[4] = { 147.0f, 168.0f, 189.0f, 210.0f };
    static constexpr float kPelletY = 0.0f;
    static constexpr float kFrameW = 21.0f;
    static constexpr float kFrameH = 26.0f;

    BusterShot::BusterShot()
        : GameObject{}
    {
        SetTexID(Cfg::Textures::BusterShot);

        // Collider is smaller than sprite
        SetWorldSize({ ColliderW, ColliderH });

        // Center collider inside 24x26 sprite: (24-14)/2=5, (26-10)/2=8
        SetTextureOffset({ 5.0f, 8.0f });

        SetScale({ 1.0f, 1.0f });
        SetRotationRad(0.0f);
        SetTint({ 1.0f, 1.0f, 1.0f, 1.0f });
        SetFlip(engine::CanvasSpriteFlip::None);

        setAffectedByGravity(false);

        ApplyFrame(0);
    }

    void BusterShot::ApplyFrame(int idx)
    {
        idx = idx % 4;
        SetTexPosition({ kPelletX[idx], kPelletY });
        SetFrameSize({ kFrameW, kFrameH });
    }

    void BusterShot::Spawn(float2 worldPos, float dir)
    {
        Active = true;
        Life = LifeMax;

        AnimTimer = 0.0f;
        Frame = 0;

        SetWorldPosition(worldPos);
        Velocity = { dir * Speed, 0.0f };

        SetFlip(dir >= 0.0f ? engine::CanvasSpriteFlip::None
            : engine::CanvasSpriteFlip::Horizontal);
        Reflected = false;

        ApplyFrame(0);
    }

    void BusterShot::UpdateShot(float dt)
    {
        if (!Active) return;

        Life -= dt;
        if (Life <= 0.0f)
        {
            Active = false;
            return;
        }

        // Animate
        AnimTimer += dt;
        while (AnimTimer >= FrameTime)
        {
            AnimTimer -= FrameTime;
            Frame = (Frame + 1) % 4;
            ApplyFrame(Frame);
        }

        // Move
        Move({ Velocity.x * dt, Velocity.y * dt });
    }


    void BusterShot::Reflect45Up()
    {
        constexpr float kInvSqrt2 = 0.70710678f;

        float newDirX = (Velocity.x >= 0.0f) ? -1.0f : 1.0f;

        Velocity =
        {
            newDirX * Speed * kInvSqrt2,
            -Speed * kInvSqrt2
        };

        Reflected = true;

        SetFlip(Velocity.x >= 0.0f
            ? engine::CanvasSpriteFlip::None
            : engine::CanvasSpriteFlip::Horizontal);
    }
}