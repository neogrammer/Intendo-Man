#include "pch.h"
#include "Shelly.h"
#include "ShellyShot.h"

#include <algorithm>
#include <cmath>

namespace game
{
    using winrt::Windows::Foundation::Numerics::float2;

    Shelly::Shelly()
        : AnimObject{}
    {
        LoadFromAnmFile(L"Assets\\Anims\\Shelly.anm");

        setAffectedByGravity(false);
        SetFacingRight(true);
        SyncToBase();
    }

    Shelly::Shelly(float2 startPos)
        : Shelly{}
    {
        Reset(startPos);
    }

    void Shelly::Reset(float2 startPos)
    {
        m_spawnPos = startPos;

        m_state = State::Patrol;
        m_stateTimer = 0.0f;
        m_lostSightTimer = 0.0f;
        m_patrolDir = 1.0f;
        m_firedThisAlert = false;

        SetWorldPosition(startPos);
        SetFacingRight(true);

        if (hasClip(L"walk"))
            Play(L"walk", true, 0);

        SetTint({ 1.0f, 1.0f, 1.0f, 1.0f });
        SyncToBase();
    }

    bool Shelly::MutualFacing(float2 playerCenter, bool playerFacingRight) const noexcept
    {
        auto const pos = GetWorldPosition();
        auto const sz = GetWorldSize();

        float2 shellyCenter
        {
            pos.x + (sz.x * 0.5f),
            pos.y + (sz.y * 0.5f)
        };

        float dx = playerCenter.x - shellyCenter.x;
        float dy = playerCenter.y - shellyCenter.y;

        float distSq = (dx * dx) + (dy * dy);
        float rangeSq = m_triggerRadius * m_triggerRadius;

        if (distSq > rangeSq)
            return false;

        bool playerFacingShelly = false;
        bool shellyFacingPlayer = false;

        if (playerCenter.x < shellyCenter.x)
        {
            // player is left of Shelly
            playerFacingShelly = playerFacingRight;   // player must face right
            shellyFacingPlayer = !IsFacingRight();    // Shelly must face left
        }
        else
        {
            // player is right of Shelly
            playerFacingShelly = !playerFacingRight;  // player must face left
            shellyFacingPlayer = IsFacingRight();     // Shelly must face right
        }

        return playerFacingShelly && shellyFacingPlayer;
    }

    float2 Shelly::ShotSpawnPos() const noexcept
    {
        auto const pos = GetWorldPosition();
        auto const sz = GetWorldSize();

        float x = IsFacingRight()
            ? (pos.x + sz.x + 2.0f)
            : (pos.x - 2.0f - ShellyShot::ColliderW);

        float y = pos.y + (sz.y * 0.38f) - (ShellyShot::ColliderH * 0.5f);

        return { x, y };
    }

    void Shelly::UpdateShelly(
        float dt,
        float2 playerCenter,
        bool playerFacingRight,
        std::function<ShellyShot* (float2, float2)> const& spawnShot)
    {
        Update(dt);

        bool mutualFacing = MutualFacing(playerCenter, playerFacingRight);

        auto const pos = GetWorldPosition();
        auto const sz = GetWorldSize();

        float shellyCenterX = pos.x + (sz.x * 0.5f);
        float spawnCenterX = m_spawnPos.x + (sz.x * 0.5f);

        switch (m_state)
        {
        case State::Patrol:
        {
            if (CurrentClipKey() != L"walk" && hasClip(L"walk"))
                Play(L"walk", true, 0);

            Move({ m_patrolDir * m_patrolSpeed * dt, 0.0f });
            SetFacingRight(m_patrolDir > 0.0f);

            float currentCenterX = GetWorldPosition().x + (GetWorldSize().x * 0.5f);

            if (currentCenterX >= spawnCenterX + m_patrolRange)
            {
                SetWorldPosition({ (spawnCenterX + m_patrolRange) - (GetWorldSize().x * 0.5f), GetWorldPosition().y });
                m_patrolDir = -1.0f;
                SetFacingRight(false);
            }
            else if (currentCenterX <= spawnCenterX - m_patrolRange)
            {
                SetWorldPosition({ (spawnCenterX - m_patrolRange) - (GetWorldSize().x * 0.5f), GetWorldPosition().y });
                m_patrolDir = 1.0f;
                SetFacingRight(true);
            }

            if (mutualFacing)
            {
                m_state = State::Alert;
                m_stateTimer = m_fireDelay;
                m_lostSightTimer = 0.0f;
                m_firedThisAlert = false;

                if (hasClip(L"idle"))
                    Play(L"idle", true, 0);
            }
            break;
        }

        case State::Alert:
        {
            // Face the player while shelled up
            SetFacingRight(playerCenter.x >= shellyCenterX);

            if (CurrentClipKey() != L"idle" && hasClip(L"idle"))
                Play(L"idle", true, 0);

            if (mutualFacing) m_lostSightTimer = 0.0f;
            else              m_lostSightTimer += dt;

            if (m_lostSightTimer >= m_lostSightTimeout)
            {
                m_state = State::Patrol;
                m_patrolDir = IsFacingRight() ? 1.0f : -1.0f;
                m_stateTimer = 0.0f;
                m_firedThisAlert = false;

                if (hasClip(L"walk"))
                    Play(L"walk", true, 0);

                break;
            }

            m_stateTimer = std::max<float>(0.0f, m_stateTimer - dt);
            if (m_stateTimer <= 0.0f && !m_firedThisAlert)
            {
                if (mutualFacing && spawnShot)
                    spawnShot(ShotSpawnPos(), playerCenter);

                m_firedThisAlert = true;
                m_state = State::Idle;
            }
            break;
        }

        case State::Idle:
        {
            SetFacingRight(playerCenter.x >= shellyCenterX);

            if (CurrentClipKey() != L"idle" && hasClip(L"idle"))
                Play(L"idle", true, 0);

            if (mutualFacing) m_lostSightTimer = 0.0f;
            else              m_lostSightTimer += dt;

            if (m_lostSightTimer >= m_lostSightTimeout)
            {
                m_state = State::Patrol;
                m_patrolDir = IsFacingRight() ? 1.0f : -1.0f;
                m_stateTimer = 0.0f;
                m_firedThisAlert = false;

                if (hasClip(L"walk"))
                    Play(L"walk", true, 0);
            }
            break;
        }
        }

        SyncToBase();
    }
}