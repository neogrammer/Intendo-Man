#include "pch.h"
#include "Player.h"

namespace game
{
    Player::Player()
    {
        LoadFromAnmFile(L"Assets\\Anims\\Player.anm");

        // Bind the shared animator profile to this actor's clip keys.
        // Missing clips are stripped by ValidateAgainst() (safe no-op).
        m_animProfile.idle = AnimKey(AnimName::Idle);
        m_animProfile.run = AnimKey(AnimName::Run);
        m_animProfile.fall = AnimKey(AnimName::Fall);
        m_animProfile.crouch = AnimKey(AnimName::Crouch);
        m_animProfile.dash = AnimKey(AnimName::Dash);

        m_animProfile.idleShoot = AnimKey(AnimName::Shoot);
        m_animProfile.runShoot = AnimKey(AnimName::RunShoot);
        m_animProfile.fallShoot = AnimKey(AnimName::FallShoot);

        m_animProfile.idleCharge = AnimKey(AnimName::Charge);
        m_animProfile.runCharge = AnimKey(AnimName::RunCharge);
        m_animProfile.fallCharge = AnimKey(AnimName::FallCharge);

        m_animProfile.hit = AnimKey(AnimName::Hit);
        m_animProfile.die = AnimKey(AnimName::Die);

        m_animProfile.ValidateAgainst(*this);

        SetFacingRight(true);
        SyncToBase();
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