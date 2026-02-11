#pragma once

#include "AnimObject.h"

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

        Player();
        explicit Player(float2 startPos);

        void SetAnim(AnimName anim, bool restart = false, uint32_t startFrame = 0);

        static constexpr wchar_t const* AnimKey(AnimName anim) noexcept
        {
            switch (anim)
            {
            case AnimName::Teleport_Start:    return L"teleport_start";
            case AnimName::Teleport_Land:     return L"teleport_land";
            case AnimName::Idle:              return L"idle";
            case AnimName::Idle_Blink:        return L"idle_blink";
            case AnimName::Run:               return L"run";
            case AnimName::Jump_Rise:         return L"jump_rise";
            case AnimName::Jump_Peak:         return L"jump_peak";
            case AnimName::Fall:              return L"fall";
            case AnimName::Land:              return L"land";
            case AnimName::Shoot_Start:       return L"shoot_start";
            case AnimName::Shoot:             return L"shoot";
            case AnimName::Dash:              return L"dash";
            case AnimName::Crouch:            return L"crouch";
            case AnimName::Charge:            return L"charge";
            case AnimName::RunShoot:          return L"runshoot";
            case AnimName::JumpShoot_Rise:    return L"jumpshoot_rise";
            case AnimName::JumpShoot_Peak:    return L"jumpshoot_peak";
            case AnimName::FallShoot:         return L"fallshoot";
            case AnimName::LandShoot:         return L"landshoot";
            case AnimName::CrouchShoot:       return L"crouchshoot";
            case AnimName::RunCharge:         return L"runcharge";
            case AnimName::JumpCharge_Rise:   return L"jumpcharge_rise";
            case AnimName::JumpCharge_Peak:   return L"jumpcharge_peak";
            case AnimName::FallCharge:        return L"fallcharge";
            case AnimName::LandCharge:        return L"landcharge";
            case AnimName::CrouchCharge:      return L"crouchcharge";
            case AnimName::ChargeShot_Full:   return L"chargeshot_full";
            case AnimName::ChargeShot_Partial:return L"chargeshot_partial";
            case AnimName::WallGrab:          return L"wallgrab";
            case AnimName::WallSlide:         return L"wallslide";
            case AnimName::WallKick:          return L"wallkick";
            case AnimName::WallGrabShoot:     return L"wallgrabshoot";
            case AnimName::WallSlideShoot:    return L"wallslideshoot";
            case AnimName::WallKickShoot:     return L"wallkickshoot";
            case AnimName::WallGrabCharge:    return L"wallgrabcharge";
            case AnimName::WallSlideCharge:   return L"wallslidecharge";
            case AnimName::WallKickCharge:    return L"wallkickcharge";
            case AnimName::Blank:             return L"blank";
            case AnimName::Hit:               return L"hit";
            case AnimName::Die:               return L"die";
            default:                          return L"idle";
            }
        }
    };
}
