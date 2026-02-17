#pragma once
#include "GameObject.h"

namespace game
{
    class BusterShot : public GameObject
    {
    public:
        bool  Active{ false };
        float2 Velocity{ 0.0f, 0.0f };
        float Life{ 0.0f };

        float AnimTimer{ 0.0f };
        int   Frame{ 0 };

        // Tunables
        static constexpr float Speed = 900.0f;
        static constexpr float LifeMax = 0.90f;
        static constexpr float FrameTime = 0.05f; // 20 FPS for the pellet

        // Collider (keep small)
        static constexpr float ColliderW = 10.0f;
        static constexpr float ColliderH = 10.0f;

        BusterShot();

        void Spawn(float2 worldPos, float dir /* -1 or +1 */);
        void UpdateShot(float dt);

        void Kill() noexcept { Active = false; }

    private:
        void ApplyFrame(int idx);
    };
}