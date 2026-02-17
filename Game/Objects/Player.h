#pragma once

#include "AnimObject.h"

// NOTE: adjust include path if needed for your folder layout.
#include "../Animation/AnimatorSM.h" // game::anim::AnimContext / AnimProfile / AnimatorSM

namespace game
{
    class Player : public AnimObject
    {
    public:
        enum class AnimName
        {
            Teleport_Start,
            Teleport_Land,
            Idle,
            Idle_Blink,
            Run,
            Jump_Rise,
            Jump_Peak,
            Fall,
            Land,
            Shoot_Start,
            Shoot,
            Dash,
            AirDash,
            Crouch,
            Charge,
            RunShoot,
            JumpShoot_Rise,
            JumpShoot_Peak,
            FallShoot,
            LandShoot,
            CrouchShoot,
            RunCharge,
            JumpCharge_Rise,
            JumpCharge_Peak,
            FallCharge,
            LandCharge,
            CrouchCharge,
            ChargeShot_Full,
            ChargeShot_Partial,
            WallGrab,
            WallSlide,
            WallKick,
            WallGrabShoot,
            WallSlideShoot,
            WallKickShoot,
            WallGrabCharge,
            WallSlideCharge,
            WallKickCharge,
            Blank,
            Hit,
            Die,
        };

        // Alias so existing call sites can keep using Player::AnimContext.
        using AnimContext = game::anim::AnimContext;

        Player();
        ~Player() override;
        explicit Player(float2 startPos);

        AnimName GetAnimName();

        void SetAnim(AnimName anim, bool restart = false, uint32_t startFrame = 0);

        void UpdateAnimation(float dt, AnimContext const& ctx)
        {
            m_animSM.Tick(*this, dt, ctx, m_animProfile);
        }

        static constexpr wchar_t const* AnimKey(AnimName anim) noexcept
        {
            switch (anim)
            {
            case AnimName::Teleport_Start:return L"teleport_start";
            case AnimName::Teleport_Land: return L"teleport_land";
            case AnimName::Idle:  return L"idle";
            case AnimName::Idle_Blink:return L"idle_blink";
            case AnimName::Run:   return L"run";
            case AnimName::Jump_Rise: return L"jump_rise";
            case AnimName::Jump_Peak: return L"jump_peak";
            case AnimName::Fall:  return L"fall";
            case AnimName::Land:  return L"land";
            case AnimName::Shoot_Start:   return L"shoot_start";
            case AnimName::Shoot: return L"shoot";
            case AnimName::Dash:  return L"dash";
            case AnimName::AirDash: return L"airdash";
            case AnimName::Crouch:return L"crouch";
            case AnimName::Charge:return L"charge";
            case AnimName::RunShoot:  return L"runshoot";
            case AnimName::JumpShoot_Rise:return L"jumpshoot_rise";
            case AnimName::JumpShoot_Peak:return L"jumpshoot_peak";
            case AnimName::FallShoot: return L"fallshoot";
            case AnimName::LandShoot: return L"landshoot";
            case AnimName::CrouchShoot:   return L"crouchshoot";
            case AnimName::RunCharge: return L"runcharge";
            case AnimName::JumpCharge_Rise:   return L"jumpcharge_rise";
            case AnimName::JumpCharge_Peak:   return L"jumpcharge_peak";
            case AnimName::FallCharge:return L"fallcharge";
            case AnimName::LandCharge:return L"landcharge";
            case AnimName::CrouchCharge:  return L"crouchcharge";
            case AnimName::ChargeShot_Full:   return L"chargeshot_full";
            case AnimName::ChargeShot_Partial:return L"chargeshot_partial";
            case AnimName::WallGrab:  return L"wallgrab";
            case AnimName::WallSlide: return L"wallslide";
            case AnimName::WallKick:  return L"wallkick";
            case AnimName::WallGrabShoot: return L"wallgrabshoot";
            case AnimName::WallSlideShoot:return L"wallslideshoot";
            case AnimName::WallKickShoot: return L"wallkickshoot";
            case AnimName::WallGrabCharge:return L"wallgrabcharge";
            case AnimName::WallSlideCharge:   return L"wallslidecharge";
            case AnimName::WallKickCharge:return L"wallkickcharge";
            case AnimName::Blank: return L"blank";
            case AnimName::Hit:   return L"hit";
            case AnimName::Die:   return L"die";
            default:  return L"idle";
            }
        }

        static game::Player::AnimName AnimNameLUT(const wchar_t* animIn) noexcept
        {
            std::wstring anim{ animIn };

            if (anim == L"teleport_start") { return game::Player::AnimName::Teleport_Start; }
            else if (anim == L"teleport_land") { return game::Player::AnimName::Teleport_Land; }
            else if (anim == L"idle") { return game::Player::AnimName::Idle; }
            else if (anim == L"idle_blink") { return game::Player::AnimName::Idle_Blink; }
            else if (anim == L"run") { return game::Player::AnimName::Run; }
            else if (anim == L"jump_rise") { return game::Player::AnimName::Jump_Rise; }
            else if (anim == L"jump_peak") { return game::Player::AnimName::Jump_Peak; }
            else if (anim == L"fall") { return game::Player::AnimName::Fall; }
            else if (anim == L"land") { return game::Player::AnimName::Land; }
            else if (anim == L"shoot_start") { return game::Player::AnimName::Shoot_Start; }
            else if (anim == L"shoot") { return game::Player::AnimName::Shoot; }
            else if (anim == L"dash") { return game::Player::AnimName::Dash; }
            else if (anim == L"airdash") { return game::Player::AnimName::AirDash; }
            else if (anim == L"crouch") { return game::Player::AnimName::Crouch; }
            else if (anim == L"charge") { return game::Player::AnimName::Charge; }
            else if (anim == L"runshoot") { return game::Player::AnimName::RunShoot; }
            else if (anim == L"jumpshoot_rise") { return game::Player::AnimName::JumpShoot_Rise; }
            else if (anim == L"jumpshoot_peak") { return game::Player::AnimName::JumpShoot_Peak; }
            else if (anim == L"fallshoot") { return game::Player::AnimName::FallShoot; }
            else if (anim == L"landshoot") { return game::Player::AnimName::LandShoot; }
            else if (anim == L"crouchshoot") { return game::Player::AnimName::CrouchShoot; }
            else if (anim == L"runcharge") { return game::Player::AnimName::RunCharge; }
            else if (anim == L"jumpcharge_rise") { return game::Player::AnimName::JumpCharge_Rise; }
            else if (anim == L"jumpcharge_peak") { return game::Player::AnimName::JumpCharge_Peak; }
            else if (anim == L"fallcharge") { return game::Player::AnimName::FallCharge; }
            else if (anim == L"landcharge") { return game::Player::AnimName::LandCharge; }
            else if (anim == L"crouchcharge") { return game::Player::AnimName::CrouchCharge; }
            else if (anim == L"chargeshot_full") { return game::Player::AnimName::ChargeShot_Full; }
            else if (anim == L"chargeshot_partial") { return game::Player::AnimName::ChargeShot_Partial; }
            else if (anim == L"wallgrab") { return game::Player::AnimName::WallGrab; }
            else if (anim == L"wallslide") { return game::Player::AnimName::WallSlide; }
            else if (anim == L"wallkick") { return game::Player::AnimName::WallKick; }
            else if (anim == L"wallgrabshoot") { return game::Player::AnimName::WallGrabShoot; }
            else if (anim == L"wallslideshoot") { return game::Player::AnimName::WallSlideShoot; }
            else if (anim == L"wallkickshoot") { return game::Player::AnimName::WallKickShoot; }
            else if (anim == L"wallgrabcharge") { return game::Player::AnimName::WallGrabCharge; }
            else if (anim == L"wallslidecharge") { return game::Player::AnimName::WallSlideCharge; }
            else if (anim == L"wallkickcharge") { return game::Player::AnimName::WallKickCharge; }
            else if (anim == L"blank") { return game::Player::AnimName::Blank; }
            else if (anim == L"hit") { return game::Player::AnimName::Hit; }
            else if (anim == L"die") { return game::Player::AnimName::Die; }
            else { return game::Player::AnimName::Idle; }
        }

    private:
        game::anim::AnimatorSM   m_animSM{};
        game::anim::AnimProfile  m_animProfile{};
    };
}

//#pragma once
//
//#include "AnimObject.h"
//
//// NOTE: adjust include path if needed for your folder layout.
//#include "../Animation/AnimatorSM.h" // game::anim::AnimContext / AnimProfile / AnimatorSM
//
//namespace game
//{
//    class Player : public AnimObject
//    {
//    public:
//        enum class AnimName
//        {
//            Teleport_Start,
//            Teleport_Land,
//            Idle,
//            Idle_Blink,
//            Run,
//            Jump_Rise,
//            Jump_Peak,
//            Fall,
//            Land,
//            Shoot_Start,
//            Shoot,
//            Dash,
//            Crouch,
//            Charge,
//            RunShoot,
//            JumpShoot_Rise,
//            JumpShoot_Peak,
//            FallShoot,
//            LandShoot,
//            CrouchShoot,
//            RunCharge,
//            JumpCharge_Rise,
//            JumpCharge_Peak,
//            FallCharge,
//            LandCharge,
//            CrouchCharge,
//            ChargeShot_Full,
//            ChargeShot_Partial,
//            WallGrab,
//            WallSlide,
//            WallKick,
//            WallGrabShoot,
//            WallSlideShoot,
//            WallKickShoot,
//            WallGrabCharge,
//            WallSlideCharge,
//            WallKickCharge,
//            Blank,
//            Hit,
//            Die,
//        };
//
//        // Alias so existing call sites can keep using Player::AnimContext.
//        using AnimContext = game::anim::AnimContext;
//
//        Player();
//        ~Player() override;
//        explicit Player(float2 startPos);
//
//        AnimName GetAnimName();
//
//        void SetAnim(AnimName anim, bool restart = false, uint32_t startFrame = 0);
//
//        void UpdateAnimation(float dt, AnimContext const& ctx)
//        {
//            m_animSM.Tick(*this, dt, ctx, m_animProfile);
//        }
//
//        static constexpr wchar_t const* AnimKey(AnimName anim) noexcept
//        {
//            switch (anim)
//            {
//            case AnimName::Teleport_Start:return L"teleport_start";
//            case AnimName::Teleport_Land: return L"teleport_land";
//            case AnimName::Idle:  return L"idle";
//            case AnimName::Idle_Blink:return L"idle_blink";
//            case AnimName::Run:   return L"run";
//            case AnimName::Jump_Rise: return L"jump_rise";
//            case AnimName::Jump_Peak: return L"jump_peak";
//            case AnimName::Fall:  return L"fall";
//            case AnimName::Land:  return L"land";
//            case AnimName::Shoot_Start:   return L"shoot_start";
//            case AnimName::Shoot: return L"shoot";
//            case AnimName::Dash:  return L"dash";
//            case AnimName::Crouch:return L"crouch";
//            case AnimName::Charge:return L"charge";
//            case AnimName::RunShoot:  return L"runshoot";
//            case AnimName::JumpShoot_Rise:return L"jumpshoot_rise";
//            case AnimName::JumpShoot_Peak:return L"jumpshoot_peak";
//            case AnimName::FallShoot: return L"fallshoot";
//            case AnimName::LandShoot: return L"landshoot";
//            case AnimName::CrouchShoot:   return L"crouchshoot";
//            case AnimName::RunCharge: return L"runcharge";
//            case AnimName::JumpCharge_Rise:   return L"jumpcharge_rise";
//            case AnimName::JumpCharge_Peak:   return L"jumpcharge_peak";
//            case AnimName::FallCharge:return L"fallcharge";
//            case AnimName::LandCharge:return L"landcharge";
//            case AnimName::CrouchCharge:  return L"crouchcharge";
//            case AnimName::ChargeShot_Full:   return L"chargeshot_full";
//            case AnimName::ChargeShot_Partial:return L"chargeshot_partial";
//            case AnimName::WallGrab:  return L"wallgrab";
//            case AnimName::WallSlide: return L"wallslide";
//            case AnimName::WallKick:  return L"wallkick";
//            case AnimName::WallGrabShoot: return L"wallgrabshoot";
//            case AnimName::WallSlideShoot:return L"wallslideshoot";
//            case AnimName::WallKickShoot: return L"wallkickshoot";
//            case AnimName::WallGrabCharge:return L"wallgrabcharge";
//            case AnimName::WallSlideCharge:   return L"wallslidecharge";
//            case AnimName::WallKickCharge:return L"wallkickcharge";
//            case AnimName::Blank: return L"blank";
//            case AnimName::Hit:   return L"hit";
//            case AnimName::Die:   return L"die";
//            default:  return L"idle";
//            }
//        }
//
//        static game::Player::AnimName AnimNameLUT(const wchar_t* animIn) noexcept
//        {
//            std::wstring anim{ animIn };
//
//            if (anim == L"teleport_start") { return game::Player::AnimName::Teleport_Start; }
//            else if (anim == L"teleport_land") { return game::Player::AnimName::Teleport_Land; }
//            else if (anim == L"idle") { return game::Player::AnimName::Idle; }
//            else if (anim == L"idle_blink") { return game::Player::AnimName::Idle_Blink; }
//            else if (anim == L"run") { return game::Player::AnimName::Run; }
//            else if (anim == L"jump_rise") { return game::Player::AnimName::Jump_Rise; }
//            else if (anim == L"jump_peak") { return game::Player::AnimName::Jump_Peak; }
//            else if (anim == L"fall") { return game::Player::AnimName::Fall; }
//            else if (anim == L"land") { return game::Player::AnimName::Land; }
//            else if (anim == L"shoot_start") { return game::Player::AnimName::Shoot_Start; }
//            else if (anim == L"shoot") { return game::Player::AnimName::Shoot; }
//            else if (anim == L"dash") { return game::Player::AnimName::Dash; }
//            else if (anim == L"crouch") { return game::Player::AnimName::Crouch; }
//            else if (anim == L"charge") { return game::Player::AnimName::Charge; }
//            else if (anim == L"runshoot") { return game::Player::AnimName::RunShoot; }
//            else if (anim == L"jumpshoot_rise") { return game::Player::AnimName::JumpShoot_Rise; }
//            else if (anim == L"jumpshoot_peak") { return game::Player::AnimName::JumpShoot_Peak; }
//            else if (anim == L"fallshoot") { return game::Player::AnimName::FallShoot; }
//            else if (anim == L"landshoot") { return game::Player::AnimName::LandShoot; }
//            else if (anim == L"crouchshoot") { return game::Player::AnimName::CrouchShoot; }
//            else if (anim == L"runcharge") { return game::Player::AnimName::RunCharge; }
//            else if (anim == L"jumpcharge_rise") { return game::Player::AnimName::JumpCharge_Rise; }
//            else if (anim == L"jumpcharge_peak") { return game::Player::AnimName::JumpCharge_Peak; }
//            else if (anim == L"fallcharge") { return game::Player::AnimName::FallCharge; }
//            else if (anim == L"landcharge") { return game::Player::AnimName::LandCharge; }
//            else if (anim == L"crouchcharge") { return game::Player::AnimName::CrouchCharge; }
//            else if (anim == L"chargeshot_full") { return game::Player::AnimName::ChargeShot_Full; }
//            else if (anim == L"chargeshot_partial") { return game::Player::AnimName::ChargeShot_Partial; }
//            else if (anim == L"wallgrab") { return game::Player::AnimName::WallGrab; }
//            else if (anim == L"wallslide") { return game::Player::AnimName::WallSlide; }
//            else if (anim == L"wallkick") { return game::Player::AnimName::WallKick; }
//            else if (anim == L"wallgrabshoot") { return game::Player::AnimName::WallGrabShoot; }
//            else if (anim == L"wallslideshoot") { return game::Player::AnimName::WallSlideShoot; }
//            else if (anim == L"wallkickshoot") { return game::Player::AnimName::WallKickShoot; }
//            else if (anim == L"wallgrabcharge") { return game::Player::AnimName::WallGrabCharge; }
//            else if (anim == L"wallslidecharge") { return game::Player::AnimName::WallSlideCharge; }
//            else if (anim == L"wallkickcharge") { return game::Player::AnimName::WallKickCharge; }
//            else if (anim == L"blank") { return game::Player::AnimName::Blank; }
//            else if (anim == L"hit") { return game::Player::AnimName::Hit; }
//            else if (anim == L"die") { return game::Player::AnimName::Die; }
//            else { return game::Player::AnimName::Idle; }
//        }
//
//    private:
//        game::anim::AnimatorSM   m_animSM{};
//        game::anim::AnimProfile  m_animProfile{};
//    };
//}