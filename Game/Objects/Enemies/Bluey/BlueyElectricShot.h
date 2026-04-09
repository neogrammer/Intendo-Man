#pragma once

#include "../../GameObject.h"

namespace game
{
    class BlueyElectricShot : public GameObject
    {
    public:
        enum class Phase
        {
            Inactive,
            Falling,
            GroundDelay,
            GroundTravel
        };

        bool  Active{ false };
        Phase phase{ Phase::Inactive };

        float2 Velocity{ 0.0f, 0.0f };
        float Dir{ 1.0f };
        float Life{ 0.0f };

        float GroundDelayTimer{ 0.0f };

        // Player-center X captured when fired.
        // When the electric wave travels past this X (miss), it despawns.
        float TargetX{ 0.0f };

        // Tunables
        static constexpr float SpeedDown   = 900.0f;
        static constexpr float SpeedGround = 520.0f;

        static constexpr float LifeMax     = 30.00f;

        // *** This is the “small time lapse from hitting ground until it travels” ***
        static constexpr float GroundDelay = 0.14f;

        // Sprite source crop (still using small crop inside the weapons cell)
        static constexpr float FrameW = 32.0f;
        static constexpr float FrameH = 32.0f;

        // Collider (small)
        static constexpr float ColliderW = 9.0f;
        static constexpr float ColliderH = 9.0f;

        BlueyElectricShot();

        void Spawn(float2 worldPos, float dir, float targetX);
        void UpdateShot(float dt);

        // Called by PlayState when a falling shot intersects a solid tile.
        void LandOnGround(float groundTopY);

        bool IsFalling() const noexcept { return phase == Phase::Falling; }
        bool IsGroundDelay() const noexcept { return phase == Phase::GroundDelay; }
        bool IsGroundTravel() const noexcept { return phase == Phase::GroundTravel; }

        void Kill() noexcept { Active = false; phase = Phase::Inactive; }
    };
}