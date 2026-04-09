#pragma once
#include "../../GameObject.h"

namespace game
{
    class ShellyShot : public GameObject
    {
    public:
        bool  Active{ false };
        float2 Velocity{ 0.0f, 0.0f };
        float Life{ 0.0f };
        float Dir{ 1.0f };

        static constexpr float Speed = 340.0f;
        static constexpr float LifeMax = 2.0f;

        static constexpr float ColliderW = 10.0f;
        static constexpr float ColliderH = 10.0f;

        ShellyShot();

        void Spawn(float2 worldPos, float2 targetPos);
        void UpdateShot(float dt);

        void Kill() noexcept { Active = false; }
    };
}