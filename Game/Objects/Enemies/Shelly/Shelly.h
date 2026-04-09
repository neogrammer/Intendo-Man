#pragma once

#include "../../AnimObject.h"

#include <functional>

namespace game
{
    class ShellyShot;

    class Shelly : public AnimObject
    {
    public:
        enum class State
        {
            Patrol,
            Alert,
            Idle
        };

        Shelly();
        explicit Shelly(float2 startPos);

        void Reset(float2 startPos);

        void UpdateShelly(
            float dt,
            float2 playerCenter,
            bool playerFacingRight,
            std::function<ShellyShot* (float2, float2)> const& spawnShot);

        bool CanReflectBuster() const noexcept
        {
            return m_state == State::Alert || m_state == State::Idle;
        }

    private:
        bool MutualFacing(float2 playerCenter, bool playerFacingRight) const noexcept;
        float2 ShotSpawnPos() const noexcept;

    private:
        State m_state{ State::Patrol };

        float2 m_spawnPos{ 0.0f, 0.0f };

        float m_triggerRadius{ 300.0f };
        float m_patrolRange{ 150.0f };
        float m_patrolSpeed{ 65.0f };

        float m_fireDelay{ 0.50f };
        float m_lostSightTimeout{ 3.0f };

        float m_stateTimer{ 0.0f };
        float m_lostSightTimer{ 0.0f };

        float m_patrolDir{ 1.0f };

        bool  m_firedThisAlert{ false };
    };
}