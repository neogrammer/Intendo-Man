#include "pch.h"
#include "PhysicsSys.h"

namespace phys
{
	using namespace winrt::Windows::Foundation;

	Rect IsColliding(game::GameObject& a, game::GameObject& b, int& xDir, int& yDir)
	{
		auto& r1 = a.getWorldRectRef();
		auto r2 = b.getWorldRect();

		long tmpX{ 0 }, tmpY{ 0 };
		long posX{0}, posY{ 0 };
		if ((r1.X <= r2.X + r2.Width) && (r1.X + r1.Width > r2.X) && (r1.Y <= r2.Y + r2.Height) && (r1.Y + r1.Height > r2.Y))
		{
			if (r1.X <= r2.X)
			{
				tmpX = (long)r1.X + (long)r1.Width - (long)r2.X;

				if (r1.Y <= r2.Y)
				{
					tmpY =  (long)r1.Y + (long)r1.Height - (long)r2.Y;
					posX =  (long)r2.X;
					posY =  (long)r2.Y;
					yDir = -1;
				}
				else
				{
					tmpY =   (long)r2.Y +  (long)r2.Height - (long)r1.Y;
					posX  =  (long)r1.X + (long)r1.Width - (long)r2.X;
					posY =   (long)r2.Y + (long)r2.Height - (long)r1.Y;
					yDir = 1;
				}
				xDir = -1;
			}
			else
			{
				tmpX = (long)r2.X + (long)r2.Width - (long)r1.X;

				if (r1.Y <= r2.Y)
				{
					tmpY = (long)r1.Y + (long)r1.Height - (long)r2.Y;
					posX = (long)r2.X + (long)r2.Width - (long)r1.X;
					posY = (long)r1.Y + (long)r1.Height - (long)r2.Y;
					yDir = -1;
				}
				else
				{
					tmpY = (long)r2.Y + (long)r2.Height - (long)r1.Y;
					posX = (long)r1.X;
					posY = (long)r1.Y;
					yDir = 1;
				}
				xDir = 1;
			}
			if (tmpX > tmpY) { xDir = 0; }
			else if (tmpX < tmpY) { yDir = 0; }

			return {(float)posX, (float)posY, (float)tmpX, (float)tmpY};
		}
		else
		{
			return { 0L,0L,0L,0L };
		}
	}

	void resolveCollision(game::GameObject& a, const Rect& r, int xDir, int yDir)
	{
		a.Move({ (float)(r.Width * xDir), (float)(r.Height * yDir) });
		if (yDir == -1)
		{
			if (a.isAffectedByGravity())
			{
				a.land();
			}
			else
			{
				a.Move({ 0, (float)(r.Height * yDir) + 1.f }); // bounce up
			}
		}
	}

	void handleCollisions(game::GameObject& a, std::vector<game::GameObject*>& bVec)
	{
		for (auto& b : bVec)
		{
			auto& o = *b;
			int xDir{0}, yDir{0};
			auto r = IsColliding(a, o, xDir, yDir);
			if (r == Rect{ 0L, 0L, 0L, 0L })
			{
				continue;
			}
			else
			{
				//collided
				resolveCollision(a, r, xDir, yDir);
			}
		}
	}

	void trustFall(game::AnimObject& a, std::vector<game::GameObject*>& bVec)
	{
		if (a.getUnder() == nullptr) { return; }

		for (auto& b : bVec)
		{
			auto& tile = *b;

			auto& c = *a.getUnder();

			auto r1 = c.getWorldRect();
			auto r2 = tile.getWorldRect();

			if ((r1.X <= r2.X + r2.Width) && (r1.X + r1.Width > r2.X) && (r1.Y <= r2.Y + r2.Height) && (r1.Y + r1.Height > r2.Y))
			{
				// under is colliding
				//a.land();
				return;
			}
			
		}
    a.inAir();
	}



}