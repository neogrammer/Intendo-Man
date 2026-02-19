#include "pch.h"
#include "Bluey.h"
#include "BlueyElectricShot.h"
#include "BlueyMissileShot.h"

#include "../../../Resources/Cfg.h"

#include <algorithm>
#include <cmath>

namespace game
{
    using winrt::Windows::Foundation::Numerics::float2;

    Bluey::Bluey()
        : AnimObject{}
    {
        LoadFromAnmFile(L"Assets\\Anims\\Bluey.anm");

        setAffectedByGravity(false);
        SetFacingRight(true);
        SyncToBase();
    }

    Bluey::Bluey(float2 startPos)
        : Bluey{}
    {
        Reset(startPos);
    }

float2 Bluey::ElectricSpawnPos(float side) const noexcept
{
    auto const pos = GetWorldPosition();
    auto const sz = GetWorldSize();

    float cx = pos.x + (sz.x * 0.5f);
    float x = cx + (side * (sz.x * 0.22f));  // shoulders
    float y = pos.y + (sz.y * 0.16f);        // shoulder height

    return { x, y };
}


    void Bluey::Reset(float2 startPos)
    {
        m_hp = 25;
        m_dead = false;

        m_state = State::Idle;
        m_lastDir = 1.0f;

        m_electricFired = false;
        m_missilesRemaining = 0;
        m_missileTimer = 0.0f;
        m_cooldownTimer = 0.0f;

        m_hitFlashTimer = 0.0f;
        m_hitFlashAccum = 0.0f;
        m_flashVisible = true;

        SetWorldPosition(startPos);

        if (hasClip(L"idle"))
            Play(L"idle", true);
        else
            Play(CurrentClipKey(), true);

        SetTint({ 1.0f, 1.0f, 1.0f, 1.0f });
        SyncToBase();
    }

    float2 Bluey::SpriteTopLeft() const noexcept
    {
        return Sub(GetWorldPosition(), GetTextureOffset());
    }

    float2 Bluey::ElectricSpawnPos() const noexcept
    {
        auto const pos = GetWorldPosition();
        auto const sz = GetWorldSize();
        float const dir = IsFacingRight() ? 1.0f : -1.0f;

        float x = pos.x + (sz.x * 0.5f) + (dir * (sz.x * 0.20f));
        float y = pos.y + (sz.y * 0.42f);
        return { x, y };
    }

    float2 Bluey::MissileSpawnPos() const noexcept
    {
        auto const pos = GetWorldPosition();
        auto const sz = GetWorldSize();
        float const dir = IsFacingRight() ? 1.0f : -1.0f;

        float x = pos.x + (sz.x * 0.5f) + (dir * (sz.x * 0.25f));
        float y = pos.y + (sz.y * 0.22f);
        return { x, y };
    }

    void Bluey::UpdateHitFlash(float dt)
    {
        if (m_hitFlashTimer <= 0.0f)
        {
            auto t = GetTint();
            t.w = 1.0f;
            SetTint(t);
            return;
        }

        m_hitFlashTimer = std::max<float>(0.0f, m_hitFlashTimer - dt);

        constexpr float kBlink = 0.05f;
        m_hitFlashAccum += dt;
        while (m_hitFlashAccum >= kBlink)
        {
            m_hitFlashAccum -= kBlink;
            m_flashVisible = !m_flashVisible;
        }

        float alpha = m_flashVisible ? 1.0f : 0.25f;
        auto t = GetTint();
        t.w = alpha;
        SetTint(t);
    }

    void Bluey::UpdateBluey(
        float dt,
        float2 playerPos,
        std::function<BlueyElectricShot* (float2, float, float)> const& spawnElectric,
        std::function<BlueyMissileShot* (float2, float)> const& spawnMissile)
    {
        if (m_dead)
            return;

        Update(dt);
        UpdateHitFlash(dt);


        bool inRange{ false };
        {
            auto const myPos = GetWorldPosition();
            auto const mySz = GetWorldSize();
            float2 myCenter{ myPos.x + mySz.x * 0.5f, myPos.y + mySz.y * 0.5f };

        float dx = playerPos.x - myCenter.x;
        float dy = playerPos.y - myCenter.y;
        float distSq = (dx * dx) + (dy * dy);
        float rSq = m_triggerRadius * m_triggerRadius;
        inRange = (distSq <= rSq);

        if (m_state == State::Idle && inRange)
        {
            if (dx < 0.0f) SetFacingRight(false);
            if (dx > 0.0f) SetFacingRight(true);
            m_lastDir = IsFacingRight() ? 1.0f : -1.0f;

            if (hasClip(L"top"))
                Play(L"top", true);

            m_state = State::Animating;
        }
    }
       

        if (m_state == State::Idle)
        {
            if (hasClip(L"idle") && CurrentClipKey() != L"idle")
                Play(L"idle", true);
            return;
        }

        if (m_state == State::Animating)
        {
            if (CurrentClipKey() == L"lower" && !IsPlaying())
            {
                m_state = State::Attacking;
               // Lock attack direction + target X at the moment we start the electric phase
    auto const myPos = GetWorldPosition();
    auto const mySz = GetWorldSize();
    float2 myCenter{ myPos.x + mySz.x * 0.5f, myPos.y + mySz.y * 0.5f };

    float dx = playerPos.x - myCenter.x;
    m_attackDir = (dx < 0.0f) ? -1.0f : 1.0f;
    SetFacingRight(m_attackDir > 0.0f);

    m_electricTargetX = playerPos.x;

    // Clear pointers
    m_eShots[0] = nullptr;
    m_eShots[1] = nullptr;
    m_missile1 = nullptr;
    m_missile2 = nullptr;

    // Spawn BOTH shoulder shots
    if (spawnElectric)
    {
        m_eShots[0] = spawnElectric(ElectricSpawnPos(-1.0f), m_attackDir, m_electricTargetX);
        m_eShots[1] = spawnElectric(ElectricSpawnPos(+1.0f), m_attackDir, m_electricTargetX);
    }

    m_attackPhase = AttackPhase::Electric;
    m_phaseTimer = 0.0f;

    return;
            }
            return;
        }


if (m_state == State::Attacking)
{
    auto const myPos = GetWorldPosition();
    auto const mySz = GetWorldSize();
    float2 myCenter{ myPos.x + mySz.x * 0.5f, myPos.y + mySz.y * 0.5f };

    // Attack dir is locked for the whole cycle
    // (This matches “direction Bluey is facing” + “beyond his position if missed” cleanly.)
    auto IsAlive = [](BlueyElectricShot* s) { return (s && s->Active); };

    static bool ranonce = false;

    switch (m_attackPhase)
    {
    case AttackPhase::Electric:
    {
        if (!ranonce)
        {
            Cfg::PlaySfx(L"electric_shot", 0.65f);
            ranonce = true;
        }

        // Wait for BOTH electric shots to finish:
        // - either they hit MegaMan (PlayState kills them),
        // - or they pass TargetX (they kill themselves),
        // - or they time out.
        bool anyAlive = IsAlive(m_eShots[0]) || IsAlive(m_eShots[1]);
        if (anyAlive)
            return;

        m_attackPhase = AttackPhase::AfterElectricDelay;
        m_phaseTimer = m_afterElectricDelay;
        return;
    }

    case AttackPhase::AfterElectricDelay:
    {
        m_phaseTimer = std::max<float>(0.0f, m_phaseTimer - dt);
        if (m_phaseTimer > 0.0f)
            return;

        // Missile #1: begins moving now
        if (spawnMissile)
        {
            m_missile1 = spawnMissile(MissileSpawnPos(), m_attackDir);
            if (m_missile1)
            {
                m_missile1->Launch();
                Cfg::PlaySfx(L"missile_launch", 0.65f);
            }
        }

        // Missile #2: appears now (while #1 is moving), but waits before moving
        if (spawnMissile)
        {
            m_missile2 = spawnMissile(MissileSpawnPos(), m_attackDir);
        }

        m_attackPhase = AttackPhase::Missile2Delay;
        m_phaseTimer = m_missile2Warmup;
        return;
    }

    case AttackPhase::Missile2Delay:
    {
        m_phaseTimer = std::max<float>(-10.0f, m_phaseTimer - dt);
        if (m_phaseTimer > -1.0f)
            return;

        if (m_missile2)
        {
            m_missile2->Launch();
            Cfg::PlaySfx(L"missile_launch", 0.65f);
        }


        m_attackPhase = AttackPhase::Cooldown;
        m_phaseTimer = m_postAttackCooldown;
        return;
    }

    case AttackPhase::Cooldown:
    {
        m_phaseTimer = std::max<float>(0.0f, m_phaseTimer - dt);
        if (m_phaseTimer > 0.0f)
            return;

        m_attackPhase = AttackPhase::None;

        // Restart the loop (top->lower->hold->attack) if still in range
        if (inRange)
        {
            if (hasClip(L"top"))
                Play(L"top", true, 0);
            m_state = State::Animating;
            ranonce = false;
        }
        else
        {
            if (hasClip(L"idle"))
                Play(L"idle", true, 0);
            m_state = State::Idle;
        }
        return;
    }

    default:
        break;
    }

    return;
}

    }

    void Bluey::TakeDamage(int dmg)
    {
        if (m_dead) return;

        m_hp = std::max<int>(0, m_hp - std::max<int>(0, dmg));

        m_hitFlashTimer = 0.20f;
        m_hitFlashAccum = 0.0f;
        m_flashVisible = false;

        Cfg::PlaySfx(Cfg::Sounds::EnemyHit, 0.65f);

        if (m_hp <= 0)
            m_dead = true;
    }
}