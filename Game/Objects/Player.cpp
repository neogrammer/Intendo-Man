#include "pch.h"
#include "Player.h"

namespace game
{
    Player::Player()
    {
        LoadFromAnmFile(L"Assets\\Anims\\Player.anm");

        // Default facing direction
        SetFacingRight(true);

        SyncToBase();
    }

    Player::~Player()
    {
    }

    Player::Player(float2 startPos)
        : Player()
    {
        SetWorldPosition(startPos);
    }

    void Player::SetAnim(AnimName anim, bool restart, uint32_t startFrame)
    {
        currentAnimName = anim;
        Play(AnimKey(anim), restart, startFrame);
    }

    game::Player::AnimName Player::GetAnimName()
    {
        return this->currentAnimName;
    }
}
