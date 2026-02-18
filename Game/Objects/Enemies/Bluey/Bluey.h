#pragma once

#include "../../AnimObject.h"

#include <functional>

namespace game
{
    class BlueyElectricShot;
    class BlueyMissileShot;

    // Bluey: simple stationary enemy with a small scripted attack loop.
    class Bluey : public AnimObject
    {
    public:
        enum class State
        {
            Idle,
            Animating,
            Attacking,
        };

        Bluey();
        explicit Bluey(float2 startPos);

        void Reset(float2 startPos);

        // Spawners are provided by the caller (PlayState) so Bluey doesn't own projectile containers.
        void UpdateBluey(
    float dt,
    float2 playerPos,
    std::function<BlueyElectricShot*(float2, float, float)> const& spawnElectric,
    std::function<BlueyMissileShot*(float2, float)> const& spawnMissile);

        void TakeDamage(int dmg);
        bool IsDead() const noexcept { return m_dead; }
        int  HP() const noexcept { return m_hp; }

        void SetTriggerRadius(float r) noexcept { m_triggerRadius = r; }
        float TriggerRadius() const noexcept { return m_triggerRadius; }

    private:
        float2 SpriteTopLeft() const noexcept;

        float2 ElectricSpawnPos() const noexcept;
        float2 MissileSpawnPos() const noexcept;

        void UpdateHitFlash(float dt);

float2 ElectricSpawnPos(float side) const noexcept;

    private:
        State m_state{ State::Idle };

        int   m_hp{ 25 };
        bool  m_dead{ false };

        float m_triggerRadius{ 420.0f };
        float m_lastDir{ 1.0f };

        bool  m_electricFired{ false };
        int   m_missilesRemaining{ 0 };
        float m_missileTimer{ 0.0f };
        float m_betweenMissiles{ 0.18f };
        float m_postAttackCooldown{ 0.25f };
        float m_cooldownTimer{ 0.0f };

        float m_hitFlashTimer{ 0.0f };
        float m_hitFlashAccum{ 0.0f };
        bool  m_flashVisible{ true };

enum class AttackPhase
{
    None,
    Electric,           // two electrics active (or waiting to finish)
    AfterElectricDelay, // wait before missile #1 launches
    Missile2Delay,      // missile #2 visible, waiting to launch
    Cooldown
};

AttackPhase m_attackPhase{ AttackPhase::None };
float m_phaseTimer{ 0.0f };

float m_attackDir{ 1.0f };
float m_electricTargetX{ 0.0f };

BlueyElectricShot* m_eShots[2]{ nullptr, nullptr };
BlueyMissileShot*  m_missile1{ nullptr };
BlueyMissileShot*  m_missile2{ nullptr };

// Tunables (feel free to tweak)
float m_afterElectricDelay{ 0.18f };
float m_missile2Warmup{ 0.14f };

    };
}