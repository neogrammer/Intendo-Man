#pragma once

#include "../../GameObject.h"

namespace game
{
    class BlueyMissileShot : public GameObject
    {
    public:
        bool  Active{ false };
        bool  Launched{ false };

        float Dir{ 1.0f };
        float2 Velocity{ 0.0f, 0.0f };
        float Life{ 0.0f };

        static constexpr float Speed   = 520.0f;
        static constexpr float LifeMax = 30.25f;

        static constexpr float ColliderW = 28.0f;
        static constexpr float ColliderH = 12.0f;

        BlueyMissileShot();

        // Spawn it (appears) but does NOT move until Launch()
        void Spawn(float2 worldPos, float dir);

        // Starts movement using stored Dir
        void Launch();

        void UpdateShot(float dt);
        void Kill() noexcept { Active = false; Launched = false; Velocity = { 0.0f, 0.0f }; }
    };
}