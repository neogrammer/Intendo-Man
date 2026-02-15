#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <algorithm>

namespace game::anim
{
    struct AnimContext
    {
        float moveX{ 0.0f };
        bool grounded{ true };

        // events (computed by PlayState)
        bool justLanded{ false };
        bool justJumped{ false };

        bool wantShoot{ false };
        bool wantCharge{ false };
        bool wantCrouch{ false };
        bool wantDash{ false };

        bool gotHit{ false };
        bool dead{ false };

        float velY{ 0.0f };

        // wall interaction (computed by PlayState)
        bool touchWallLeft{ false };
        bool touchWallRight{ false };
        bool wallSliding{ false };
        bool justWallJumped{ false }; // 1-tick event
    };

    enum class Locomotion : uint8_t
    {
        Idle,
        Run,
        JumpRise,
        JumpPeak,
        Fall,

        WallGrab,
        WallSlide,
        WallKick,
        Land,
        Crouch,
        Dash
    };

    enum class Overlay : uint8_t { None, Shoot, Charge };

    struct AnimProfile
    {
        // base
        std::optional<std::wstring> idle;
        std::optional<std::wstring> run;
        std::optional<std::wstring> jumpRise;
        std::optional<std::wstring> jumpPeak;
        std::optional<std::wstring> fall;

        std::optional<std::wstring> wallGrab;
        std::optional<std::wstring> wallSlide;
        std::optional<std::wstring> wallKick;

        std::optional<std::wstring> land;
        std::optional<std::wstring> crouch;
        std::optional<std::wstring> dash;

        // shoot variants
        std::optional<std::wstring> idleShoot;
        std::optional<std::wstring> runShoot;
        std::optional<std::wstring> jumpRiseShoot;
        std::optional<std::wstring> jumpPeakShoot;
        std::optional<std::wstring> fallShoot;

        std::optional<std::wstring> wallGrabShoot;
        std::optional<std::wstring> wallSlideShoot;
        std::optional<std::wstring> wallKickShoot;


        std::optional<std::wstring> landShoot;

        // charge variants
        std::optional<std::wstring> idleCharge;
        std::optional<std::wstring> runCharge;
        std::optional<std::wstring> jumpRiseCharge;
        std::optional<std::wstring> jumpPeakCharge;
        std::optional<std::wstring> fallCharge;


        std::optional<std::wstring> wallGrabCharge;
        std::optional<std::wstring> wallSlideCharge;
        std::optional<std::wstring> wallKickCharge;

        std::optional<std::wstring> landCharge;

        // overrides
        std::optional<std::wstring> hit;
        std::optional<std::wstring> die;

        template<class Actor>
        void ValidateAgainst(Actor const& a)
        {
            auto check = [&](std::optional<std::wstring>& o)
                {
                    if (o && !a.hasClip(*o))
                        o.reset();
                };

            check(idle); check(run); check(jumpRise); check(jumpPeak); check(fall); check(land);
            check(wallGrab); check(wallSlide); check(wallKick);
            check(crouch); check(dash);

            check(idleShoot); check(runShoot); check(jumpRiseShoot); check(jumpPeakShoot);
            check(fallShoot); check(landShoot);
            check(wallGrabShoot); check(wallSlideShoot); check(wallKickShoot);

            check(idleCharge); check(runCharge); check(jumpRiseCharge); check(jumpPeakCharge);
            check(fallCharge); 
            check(wallGrabCharge); check(wallSlideCharge); check(wallKickCharge);
            check(landCharge);
            check(hit); check(die);
        }

        std::wstring const* Resolve(Locomotion loco, Overlay ov) const noexcept
        {
            auto ptr = [](std::optional<std::wstring> const& o) -> std::wstring const*
                {
                    return o ? &(*o) : nullptr;
                };

            // --- overlay first
            if (ov == Overlay::Shoot)
            {
                if (loco == Locomotion::Run)      if (auto p = ptr(runShoot))      return p;
                if (loco == Locomotion::JumpRise) if (auto p = ptr(jumpRiseShoot)) return p;
                if (loco == Locomotion::JumpPeak) if (auto p = ptr(jumpPeakShoot)) return p;
                if (loco == Locomotion::Fall)     if (auto p = ptr(fallShoot))     return p;
                if (loco == Locomotion::WallGrab)  if (auto p = ptr(wallGrabShoot))  return p;
                if (loco == Locomotion::WallSlide) if (auto p = ptr(wallSlideShoot)) return p;
                if (loco == Locomotion::WallKick)  if (auto p = ptr(wallKickShoot))  return p;
                if (loco == Locomotion::Land)     if (auto p = ptr(landShoot))     return p;
                if (loco == Locomotion::Idle)     if (auto p = ptr(idleShoot))     return p;
            }
            else if (ov == Overlay::Charge)
            {
                if (loco == Locomotion::Run)      if (auto p = ptr(runCharge))      return p;
                if (loco == Locomotion::JumpRise) if (auto p = ptr(jumpRiseCharge)) return p;
                if (loco == Locomotion::JumpPeak) if (auto p = ptr(jumpPeakCharge)) return p;
                if (loco == Locomotion::Fall)     if (auto p = ptr(fallCharge))     return p;

                if (loco == Locomotion::WallGrab)  if (auto p = ptr(wallGrabCharge))  return p;
                if (loco == Locomotion::WallSlide) if (auto p = ptr(wallSlideCharge)) return p;
                if (loco == Locomotion::WallKick)  if (auto p = ptr(wallKickCharge))  return p;
                if (loco == Locomotion::Land)     if (auto p = ptr(landCharge))     return p;
                if (loco == Locomotion::Idle)     if (auto p = ptr(idleCharge))     return p;
            }

            // --- base fallback
            if (loco == Locomotion::Run)      if (auto p = ptr(run))      return p;
            if (loco == Locomotion::JumpRise) if (auto p = ptr(jumpRise)) return p;
            if (loco == Locomotion::JumpPeak) if (auto p = ptr(jumpPeak)) return p;
            if (loco == Locomotion::Fall)     if (auto p = ptr(fall))     return p;

            if (loco == Locomotion::WallGrab)  if (auto p = ptr(wallGrab))  return p;
            if (loco == Locomotion::WallSlide) if (auto p = ptr(wallSlide)) return p;
            if (loco == Locomotion::WallKick)  if (auto p = ptr(wallKick))  return p;
            if (loco == Locomotion::Land)     if (auto p = ptr(land))     return p;
            if (loco == Locomotion::Crouch)   if (auto p = ptr(crouch))   return p;
            if (loco == Locomotion::Dash)     if (auto p = ptr(dash))     return p;
            if (loco == Locomotion::Idle)     if (auto p = ptr(idle))     return p;

            // --- final fallback
            if (auto p = ptr(idle)) return p;
            return nullptr;
        }
    };

    class AnimatorSM
    {
    public:
        template<class Actor>
        void Tick(Actor& a, float dt, AnimContext const& ctx, AnimProfile const& profile)
        {

            // Facing: prefer input; if no input, face the wall you're touching (when airborne).
            const bool touchingWall = (!ctx.grounded) && (ctx.touchWallLeft || ctx.touchWallRight);
            float dir = DeadZone(ctx.moveX);

            if (dir < 0.0f) 
            { 
                a.SetFacingRight(false);
            }
            else if (dir > 0.0f)
            {
                a.SetFacingRight(true);
            }
            else if (touchingWall)
               {
                if (ctx.touchWallLeft && !ctx.touchWallRight)
                {
                    a.SetFacingRight(false);
                }
                else if (ctx.touchWallRight && !ctx.touchWallLeft) 
                {
                    a.SetFacingRight(true);
                }
            }

            // --- hard overrides (dead/hit)
            if (ctx.dead)
            {
                EnterMode(Mode::Dead);
                TryPlay(a, profile.die, /*restart*/m_modeChanged);
                m_prevVelY = ctx.velY;
                return;
            }
            if (ctx.gotHit)
            {
                EnterMode(Mode::Hit);
                TryPlay(a, profile.hit, /*restart*/m_modeChanged);
                m_prevVelY = ctx.velY;
                return;
            }
            EnterMode(Mode::Normal);

            // Overlay
            Overlay ov = Overlay::None;
            if (ctx.wantCharge) ov = Overlay::Charge;
            else if (ctx.wantShoot) ov = Overlay::Shoot;


                        // If we just wall-jumped, lock into wallkick briefly so it actually shows.
               if (ctx.justWallJumped)
                {
               m_wallKickLock = true;
               m_wallKickTime = 0.0f;
               }
             if (ctx.grounded)
                 {
                m_wallKickLock = false;
                m_wallKickTime = 0.0f;
                }
            
                if (m_wallKickLock && !ctx.grounded)
                 {
                constexpr float kWallKickMinTime = 0.10f; // tweak to taste
                m_wallKickTime += dt;
                
                    ApplyDesired(a, profile, Locomotion::WallKick, ov);
                
                    if (Locomotion::WallKick != m_loco || ov != m_overlay)
                     {
                    m_timeInState = 0.0f;
                    m_loco = Locomotion::WallKick;
                    m_overlay = ov;
                    }
                 else
                    {
                   m_timeInState += dt;
                   }
                   if (m_wallKickTime >= kWallKickMinTime)
                    {
                   m_wallKickLock = false;
                   }
                
                    m_prevVelY = ctx.velY;
                return;
                }
            

            // Landing trigger: force land for at least 1 frame (prevents “restart every frame”)
            if (ctx.justLanded)
            {
                m_loco = Locomotion::Land;
                m_overlay = ov;
                m_timeInState = 0.0f;

                // reset air tracking
                m_inAir = false;
                m_sawRising = false;
                m_peakWindow = false;
                m_jumpHeight = 0.0f;

                if (auto* landKey = profile.Resolve(Locomotion::Land, ov))
                {
                    if (a.hasClip(*landKey))
                    {
                        if (a.CurrentClipKey() != *landKey)
                            a.Play(*landKey, /*restart*/true);
                        else
                            a.Play(*landKey, /*restart*/false);
                    }
                }

                m_prevVelY = ctx.velY;
                return;
            }

            // If we are in land, keep it unless interrupted (movement / overlay / leaving ground)
            if (IsLandVariant(a.CurrentClipKey(), profile))
            {
                if (ctx.grounded && DeadZone(ctx.moveX) == 0.0f && !ctx.wantShoot && !ctx.wantCharge)
                {
                    // allow swapping land <-> landshoot <-> landcharge without jitter
                    if (auto* desired = profile.Resolve(Locomotion::Land, ov))
                    {
                        if (a.hasClip(*desired) && a.CurrentClipKey() != *desired)
                            a.PlaySynced(*desired);
                    }

                    m_timeInState += dt;
                    m_loco = Locomotion::Land;
                    m_overlay = ov;
                    m_prevVelY = ctx.velY;
                    return;
                }
                // else: interrupted -> fall through to normal selection
            }

            // --- Air logic (JumpRise / JumpPeak / Fall)
            if (!ctx.grounded)
            {

                const bool onWall = (ctx.touchWallLeft || ctx.touchWallRight);
                const float dz = DeadZone(ctx.moveX);
                
                    const bool pressingIntoWall =
                    (ctx.touchWallLeft && dz < 0.0f) ||
                    (ctx.touchWallRight && dz > 0.0f);
                
                                    // Prefer wall slide if falling and pressing into a wall (or if PlayState already flagged it)
                    const bool sliding = ctx.wallSliding || (onWall && pressingIntoWall && ctx.velY > 0.0f);
                
                    if (sliding)
                    {
                    ApplyDesired(a, profile, Locomotion::WallSlide, ov);
                    
                        if (Locomotion::WallSlide != m_loco || ov != m_overlay)
                         {
                        m_timeInState = 0.0f;
                        m_loco = Locomotion::WallSlide;
                        m_overlay = ov;
                        }
                     else
                         {
                        m_timeInState += dt;
                        }
                    
                        m_prevVelY = ctx.velY;
                    return;
                    }
                
                                    // Optional wall grab (if you have the clip). Shows while rising / at apex if you’re clinging.
                    if (onWall && pressingIntoWall && ctx.velY <= 0.0f)
                     {
                    ApplyDesired(a, profile, Locomotion::WallGrab, ov);
                    
                        if (Locomotion::WallGrab != m_loco || ov != m_overlay)
                         {
                        m_timeInState = 0.0f;
                        m_loco = Locomotion::WallGrab;
                        m_overlay = ov;
                        }
                     else
                         {
                        m_timeInState += dt;
                        }
                    
                        m_prevVelY = ctx.velY;
                    return;
                    }
                

                const float feetY = a.GetWorldPosition().y + a.GetWorldSize().y;

                if (!m_inAir)
                {
                    m_inAir = true;
                    m_sawRising = false;
                    m_peakWindow = false;
                    m_jumpStartFeetY = feetY;
                    m_apexFeetY = feetY;
                    m_jumpHeight = 0.0f;
                }

                if (ctx.velY < -0.01f)
                    m_sawRising = true;

                // “95% up” approximation: enter peak when upward speed is small
                // Tune this number to taste.
                constexpr float peakEnterVel = 150.0f; // px/s
                if (m_sawRising && !m_peakWindow && ctx.velY < 0.0f && ctx.velY > -peakEnterVel)
                    m_peakWindow = true;

                // Apex detection (crossing upward -> downward)
                if (m_sawRising && m_prevVelY < 0.0f && ctx.velY >= 0.0f)
                {
                    m_apexFeetY = feetY;
                    m_jumpHeight = (m_jumpStartFeetY - m_apexFeetY);
                    if (m_jumpHeight < 0.0f) m_jumpHeight = 0.0f;
                    m_peakWindow = true;
                }

                // “5% down” before switching to fall
                if (m_peakWindow && ctx.velY > 0.0f && m_jumpHeight > 0.0f)
                {
                    float down = feetY - m_apexFeetY;
                    float trigger = std::max<float>(2.0f, m_jumpHeight * 0.05f);
                    if (down >= trigger)
                        m_peakWindow = false;
                }

                Locomotion loco = Locomotion::Fall;
                if (m_sawRising && m_peakWindow) loco = Locomotion::JumpPeak;
                else if (ctx.velY < 0.0f) loco = Locomotion::JumpRise;
                else loco = Locomotion::Fall;

                ApplyDesired(a, profile, loco, ov);

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

                m_prevVelY = ctx.velY;
                return;
            }

            // grounded (not landing)
            m_inAir = false;
            m_sawRising = false;
            m_peakWindow = false;
            m_jumpHeight = 0.0f;

            Locomotion loco = Locomotion::Idle;
            if (ctx.wantDash) loco = Locomotion::Dash;
            else if (ctx.wantCrouch) loco = Locomotion::Crouch;
            else if (DeadZone(ctx.moveX) != 0.0f) loco = Locomotion::Run;

            ApplyDesired(a, profile, loco, ov);

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

            m_prevVelY = ctx.velY;
        }

        float TimeInState() const noexcept { return m_timeInState; }

    private:
        enum class Mode : uint8_t { Normal, Hit, Dead };

        static constexpr float DeadZone(float x) noexcept
        {
            return (x < -0.20f) ? -1.0f : (x > 0.20f) ? 1.0f : 0.0f;
        }

        void EnterMode(Mode m) noexcept
        {
            m_modeChanged = (m != m_mode);
            m_mode = m;
            if (m_modeChanged)
                m_timeInState = 0.0f;
        }

        template<class Actor>
        static void TryPlay(Actor& a, std::optional<std::wstring> const& key, bool restart)
        {
            if (!key || key->empty()) return;
            if (!a.hasClip(*key)) return;
            a.Play(*key, restart);
        }

        static bool Eq(std::wstring const& k, std::optional<std::wstring> const& o) noexcept
        {
            return o && *o == k;
        }

        static bool IsRunVariant(std::wstring const& k, AnimProfile const& p) noexcept
        {
            return Eq(k, p.run) || Eq(k, p.runShoot) || Eq(k, p.runCharge);
        }

        static bool IsJumpRiseVariant(std::wstring const& k, AnimProfile const& p) noexcept
        {
            return Eq(k, p.jumpRise) || Eq(k, p.jumpRiseShoot) || Eq(k, p.jumpRiseCharge);
        }

        static bool IsJumpPeakVariant(std::wstring const& k, AnimProfile const& p) noexcept
        {
            return Eq(k, p.jumpPeak) || Eq(k, p.jumpPeakShoot) || Eq(k, p.jumpPeakCharge);
        }

        static bool IsFallVariant(std::wstring const& k, AnimProfile const& p) noexcept
        {
            return Eq(k, p.fall) || Eq(k, p.fallShoot) || Eq(k, p.fallCharge);
        }



        static bool IsWallGrabVariant(std::wstring const& k, AnimProfile const& p) noexcept
             {
            return Eq(k, p.wallGrab) || Eq(k, p.wallGrabShoot) || Eq(k, p.wallGrabCharge);
            }
        
            static bool IsWallSlideVariant(std::wstring const& k, AnimProfile const& p) noexcept
             {
            return Eq(k, p.wallSlide) || Eq(k, p.wallSlideShoot) || Eq(k, p.wallSlideCharge);
            }
        
            static bool IsWallKickVariant(std::wstring const& k, AnimProfile const& p) noexcept
             {
            return Eq(k, p.wallKick) || Eq(k, p.wallKickShoot) || Eq(k, p.wallKickCharge);
            }
        

        static bool IsLandVariant(std::wstring const& k, AnimProfile const& p) noexcept
        {
            return Eq(k, p.land) || Eq(k, p.landShoot) || Eq(k, p.landCharge);
        }

        static bool SameSyncGroup(std::wstring const& a, std::wstring const& b, AnimProfile const& p) noexcept
        {
            if (IsRunVariant(a, p) && IsRunVariant(b, p)) return true;
            if (IsJumpRiseVariant(a, p) && IsJumpRiseVariant(b, p)) return true;
            if (IsJumpPeakVariant(a, p) && IsJumpPeakVariant(b, p)) return true;
            if (IsFallVariant(a, p) && IsFallVariant(b, p)) return true;
           if (IsWallGrabVariant(a, p) && IsWallGrabVariant(b, p)) return true;
           if (IsWallSlideVariant(a, p) && IsWallSlideVariant(b, p)) return true;
           if (IsWallKickVariant(a, p) && IsWallKickVariant(b, p)) return true;
            if (IsLandVariant(a, p) && IsLandVariant(b, p)) return true;
            return false;
        }

        template<class Actor>
        static void ApplyDesired(Actor& a, AnimProfile const& profile, Locomotion loco, Overlay ov)
        {
            auto* desired = profile.Resolve(loco, ov);
            if (!desired || !a.hasClip(*desired))
                return;

            auto const& cur = a.CurrentClipKey();
            if (cur == *desired)
            {
                // already playing; do not restart
                return;
            }

            if (SameSyncGroup(cur, *desired, profile))
                a.PlaySynced(*desired);
            else
                a.Play(*desired, /*restart*/false);
        }

        Mode m_mode{ Mode::Normal };
        bool m_modeChanged{ false };

        float m_timeInState{ 0.0f };
        Locomotion m_loco{ Locomotion::Idle };
        Overlay m_overlay{ Overlay::None };

        // Jump/air tracking (per-actor instance)
        bool  m_inAir{ false };
        bool  m_sawRising{ false };
        bool  m_peakWindow{ false };
        float m_jumpStartFeetY{ 0.0f };
        float m_apexFeetY{ 0.0f };
        float m_jumpHeight{ 0.0f };

        float m_prevVelY{ 0.0f };


        
                    // Wall kick lock
         bool  m_wallKickLock{ false };
        float m_wallKickTime{ 0.0f };
    };
}