#include "pch.h"
#include "Player.h"

namespace game
{
    Player::Player()
    {
        LoadFromAnmFile(L"Assets\\Anims\\Player.anm");

        // Base locomotion
        m_animProfile.idle     = AnimKey(AnimName::Idle);
        m_animProfile.run      = AnimKey(AnimName::Run);
        m_animProfile.jumpRise = AnimKey(AnimName::Jump_Rise);
        m_animProfile.jumpPeak = AnimKey(AnimName::Jump_Peak);
        m_animProfile.fall     = AnimKey(AnimName::Fall);
        m_animProfile.land     = AnimKey(AnimName::Land);
        m_animProfile.crouch   = AnimKey(AnimName::Crouch);
        m_animProfile.dash     = AnimKey(AnimName::Dash);
        m_animProfile.airDash  = AnimKey(AnimName::AirDash);

        // Shoot overlay
        m_animProfile.idleShoot     = AnimKey(AnimName::Shoot);
        m_animProfile.runShoot      = AnimKey(AnimName::RunShoot);
        m_animProfile.jumpRiseShoot = AnimKey(AnimName::JumpShoot_Rise);
        m_animProfile.jumpPeakShoot = AnimKey(AnimName::JumpShoot_Peak);
        m_animProfile.fallShoot     = AnimKey(AnimName::FallShoot);
        m_animProfile.landShoot     = AnimKey(AnimName::LandShoot);

        // Charge overlay
        m_animProfile.idleCharge     = AnimKey(AnimName::Charge);
        m_animProfile.runCharge      = AnimKey(AnimName::RunCharge);
        m_animProfile.jumpRiseCharge = AnimKey(AnimName::JumpCharge_Rise);
        m_animProfile.jumpPeakCharge = AnimKey(AnimName::JumpCharge_Peak);
        m_animProfile.fallCharge     = AnimKey(AnimName::FallCharge);
        m_animProfile.landCharge     = AnimKey(AnimName::LandCharge);

        // Wall base
        m_animProfile.wallGrab  = AnimKey(AnimName::WallGrab);
        m_animProfile.wallSlide = AnimKey(AnimName::WallSlide);
        m_animProfile.wallKick  = AnimKey(AnimName::WallKick);

        // Wall shoot
        m_animProfile.wallGrabShoot  = AnimKey(AnimName::WallGrabShoot);
        m_animProfile.wallSlideShoot = AnimKey(AnimName::WallSlideShoot);
        m_animProfile.wallKickShoot  = AnimKey(AnimName::WallKickShoot);

        // Wall charge
        m_animProfile.wallGrabCharge  = AnimKey(AnimName::WallGrabCharge);
        m_animProfile.wallSlideCharge = AnimKey(AnimName::WallSlideCharge);
        m_animProfile.wallKickCharge  = AnimKey(AnimName::WallKickCharge);

        // Overrides
        m_animProfile.hit = AnimKey(AnimName::Hit);
        m_animProfile.die = AnimKey(AnimName::Die);

        // Strip any keys that aren't present in the loaded .anm
        m_animProfile.ValidateAgainst(*this);
    }

    Player::~Player() {}

    Player::Player(float2 startPos)
        : Player()
    {
        SetWorldPosition(startPos);
    }

    void Player::SetAnim(AnimName anim, bool restart, uint32_t startFrame)
    {
        Play(AnimKey(anim), restart, startFrame);
    }

    game::Player::AnimName Player::GetAnimName()
    {
        return AnimNameLUT(CurrentClipKey().c_str());
    }
}
