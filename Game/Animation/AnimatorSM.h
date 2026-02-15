#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace game::anim
{
    // Generic input to the shared animation state machine.
    struct AnimContext
    {
        float moveX{ 0.0f };
        bool grounded{ true };

        bool wantShoot{ false };
        bool wantCharge{ false };
        bool wantCrouch{ false };
        bool wantDash{ false };

        bool gotHit{ false };
        bool dead{ false };

        float velY{ 0.0f };
    };

    enum class Locomotion : uint8_t { Idle, Run, AirFall, Crouch, Dash };
    enum class Overlay : uint8_t { None, Shoot, Charge };

    // Optional clip-key mapping per actor.
    struct AnimProfile
    {
        std::optional<std::wstring> idle;
        std::optional<std::wstring> run;
        std::optional<std::wstring> fall;
        std::optional<std::wstring> crouch;
        std::optional<std::wstring> dash;

        std::optional<std::wstring> idleShoot;
        std::optional<std::wstring> runShoot;
        std::optional<std::wstring> fallShoot;

        std::optional<std::wstring> idleCharge;
        std::optional<std::wstring> runCharge;
        std::optional<std::wstring> fallCharge;

        std::optional<std::wstring> hit;
        std::optional<std::wstring> die;

        // Strip any keys that are not present on the actor (prevents debug spam).
        template<class Actor>
        void ValidateAgainst(Actor const& a)
        {
            auto check = [&](std::optional<std::wstring>& o)
                {
                    if (o && !a.hasClip(*o))
                        o.reset();
                };

            check(idle);   check(run);    check(fall);  check(crouch); check(dash);
            check(idleShoot); check(runShoot); check(fallShoot);
            check(idleCharge); check(runCharge); check(fallCharge);
            check(hit); check(die);
        }

        // Returns nullptr => "do nothing / keep whatever is playing"
        std::wstring const* Resolve(Locomotion loco, Overlay ov) const noexcept
        {
            auto ptr = [](std::optional<std::wstring> const& o) -> std::wstring const*
                {
                    return o ? &(*o) : nullptr;
                };

            // 1) Try overlay variant
            if (ov == Overlay::Shoot)
            {
                if (loco == Locomotion::Run)     if (auto p = ptr(runShoot))  return p;
                if (loco == Locomotion::AirFall) if (auto p = ptr(fallShoot)) return p;
                if (loco == Locomotion::Idle)    if (auto p = ptr(idleShoot)) return p;
            }
            else if (ov == Overlay::Charge)
            {
                if (loco == Locomotion::Run)     if (auto p = ptr(runCharge))  return p;
                if (loco == Locomotion::AirFall) if (auto p = ptr(fallCharge)) return p;
                if (loco == Locomotion::Idle)    if (auto p = ptr(idleCharge)) return p;
            }

            // 2) Fall back to base locomotion
            if (loco == Locomotion::Run)     if (auto p = ptr(run))    return p;
            if (loco == Locomotion::AirFall) if (auto p = ptr(fall))   return p;
            if (loco == Locomotion::Crouch)  if (auto p = ptr(crouch)) return p;
            if (loco == Locomotion::Dash)    if (auto p = ptr(dash))   return p;
            if (loco == Locomotion::Idle)    if (auto p = ptr(idle))   return p;

            // 3) Final fallback: idle
            if (auto p = ptr(idle)) return p;

            // 4) Nothing defined => no-op
            return nullptr;
        }
    };

    class AnimatorSM
    {
    public:
        static bool IsRunVariant(std::wstring const& key, AnimProfile const& profile) noexcept
        {
            auto eq = [&](std::optional<std::wstring> const& o) noexcept -> bool
                {
                    return o && *o == key;
                };

            return eq(profile.run) || eq(profile.runShoot) || eq(profile.runCharge);
        }

        bool m_holdRun{ false };
        uint32_t m_holdRunFrameIndex{ 0 };

        template<class Actor>
        void Tick(Actor& a, float dt, AnimContext const& ctx, AnimProfile const& profile)
        {
            // --- Facing (based on moveX sign)
            const auto& currentKey = a.CurrentClipKey();
            float dir = DeadZone(ctx.moveX);
            if (dir < 0.0f) a.SetFacingRight(false);
            else if (dir > 0.0f) a.SetFacingRight(true);



            // --- Override states (dead/hit)
            if (ctx.dead)
            {
                if (m_mode != Mode::Dead)
                {
                    m_mode = Mode::Dead;
                    m_timeInState = 0.0f;
                    TryPlay(a, profile.die, /*restart*/true);
                }
                else
                {
                    TryPlay(a, profile.die, /*restart*/false);
                }
                return;
            }

            if (ctx.gotHit)
            {
                if (m_mode != Mode::Hit)
                {
                    m_mode = Mode::Hit;
                    m_timeInState = 0.0f;
                    TryPlay(a, profile.hit, /*restart*/true);
                }
                else
                {
                    TryPlay(a, profile.hit, /*restart*/false);
                }
                return;
            }

            // Back to normal mode
            if (m_mode != Mode::Normal)
            {
                m_mode = Mode::Normal;
                m_timeInState = 0.0f;
            }

            // --- Overlay
            Overlay ov = Overlay::None;
            if (ctx.wantCharge) ov = Overlay::Charge;
            else if (ctx.wantShoot) ov = Overlay::Shoot;

            Locomotion loco = Locomotion::Idle;
            if (!ctx.grounded) loco = Locomotion::AirFall;
            else if (ctx.wantDash) loco = Locomotion::Dash;
            else if (ctx.wantCrouch) loco = Locomotion::Crouch;
            else
            {
                //if (dir != 0.0f)
                //{
                //    m_holdRun = false;
                //    loco = Locomotion::Run;
                //}
                //else
                //{
                //    const bool currentIsRunVariant = IsRunVariant(currentKey, profile);
                //    if (currentIsRunVariant)
                //    {
                //        if (!m_holdRun)
                //        {
                //            m_holdRun = true;
                //            m_holdRunFrameIndex = a.CurrentFrameIndex();
                //        }

                //        // Hold run until the frame index advances.
                //        if (a.CurrentFrameIndex() == m_holdRunFrameIndex)
                //            loco = Locomotion::Run;
                //        else
                //        {
                //            m_holdRun = false;
                //            loco = Locomotion::Idle;
                //        }
                //    }
                //    else
                //    {
                //        m_holdRun = false;
                //        loco = Locomotion::Idle;
                //    }
                //}
                dir = DeadZone(ctx.moveX);
                const bool curRun = IsRunVariant(a.CurrentClipKey(), profile);

                if (dir != 0.0f)
                {
                    // real movement -> real run
                    m_holdRun = false;
                    loco = Locomotion::Run;
                }
                else if (curRun)
                {
                    // transition moment: keep running until the frame index changes
                    if (!m_holdRun)
                    {
                        m_holdRun = true;
                        m_holdRunFrame = a.CurrentFrameIndex();
                    }

                    if (a.CurrentFrameIndex() == m_holdRunFrame)
                        loco = Locomotion::Run;
                    else
                    {
                        m_holdRun = false;
                        loco = Locomotion::Idle;
                    }
                }
                else
                {
                    m_holdRun = false;
                    loco = Locomotion::Idle;
                }
            }

            // --- Resolve -> clip key
            auto* desired = profile.Resolve(loco, ov);
            if (desired && a.hasClip(*desired))
            {
                const bool currentIsRunVariant = IsRunVariant(currentKey, profile);
                const bool desiredIsRunVariant = IsRunVariant(*desired, profile);

                if (currentIsRunVariant && desiredIsRunVariant && currentKey != *desired)
                    a.PlaySynced(*desired);
                else
                    a.Play(*desired, /*restart*/false);
            }

            // Track semantic state time
            if (loco != m_loco || ov != m_overlay)
            {
                m_timeInState = 0.0f;
                m_loco = loco;
                m_overlay = ov;
            }
            else
            {
                m_timeInState += dt;
            }
        }

        float TimeInState() const noexcept { return m_timeInState; }

    private:


        uint32_t m_holdRunFrame{ 0 };

        enum class Mode : uint8_t { Normal, Hit, Dead };

        static constexpr float DeadZone(float x) noexcept
        {
            return (x < -0.20f) ? -1.0f : (x > 0.20f) ? 1.0f : 0.0f;
        }

        template<class Actor>
        static void TryPlay(Actor& a, std::optional<std::wstring> const& key, bool restart)
        {
            if (!key || key->empty()) return;
            if (!a.hasClip(*key)) return;
            a.Play(*key, restart);
        }

        Mode m_mode{ Mode::Normal };
        float m_timeInState{ 0.0f };
        Locomotion m_loco{ Locomotion::Idle };
        Overlay m_overlay{ Overlay::None };
    };
}