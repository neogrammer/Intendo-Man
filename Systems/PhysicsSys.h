#pragma once
#include "../Game/Objects/AnimObject.h"

namespace phys
{
	extern winrt::Windows::Foundation::Rect IsColliding(game::GameObject& a, game::GameObject& b, int& xDir, int& yDir);
	extern void resolveCollision(game::GameObject& a, const winrt::Windows::Foundation::Rect& r, int xDir, int yDir);
	extern void handleCollisions(game::GameObject& a, std::vector<game::GameObject*>& bVec);
	extern void trustFall(game::AnimObject& a, std::vector<game::GameObject*>& bVec);
}