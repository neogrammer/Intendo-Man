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

    Player::Player(float2 startPos)
        : Player()
    {
        SetWorldPosition(startPos);
    }

    void Player::SetAnim(AnimName anim, bool restart, uint32_t startFrame)
    {
        Play(AnimKey(anim), restart, startFrame);
    }
}
