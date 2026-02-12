#include "pch.h"
#include "Tileset.h"
#include <fstream>
#include <cassert>

namespace game
{


	using winrt::Windows::Foundation::Numerics::float2;

	game::Tileset::Tileset()
	{
		tiles.clear();
	}

	game::Tileset::Tileset(Cfg::Textures texID_, float2 size_, int pitch_, int numTiles_)
		: texID{ texID_ }
		, tw {size_.x}
		, th{ size_.y }
		, pitch{ pitch_ }
		, numTiles{ numTiles_ }
	{
		tiles.clear();
	}

	game::Tileset::~Tileset()
	{
	}

	void game::Tileset::addTiles(const std::wstring& filename_)
	{
		std::ifstream iFile{ filename_ };

		if (!iFile.is_open()) return;

		tiles.clear();

		tiles.reserve(numTiles);

		int numRows = (int)std::ceil((float)numTiles / (float)pitch);

		for (int y = 0; y < numRows; y++)
			for (int x = 0; x < pitch; x++)
			{

				int s;
				iFile >> s;
				
				float2 pos = { x * tw, y * th };

				addTile(((s == 1) ? true : false) , pos);
			}

		iFile.close();

	}

	void game::Tileset::addTile(bool solid_, float2 texPosition_)
	{
		tiles.emplace_back(std::make_unique<Tile>(texID, float2{ tw, th }, float2{ tw, th }, solid_, texPosition_, float2{ 0.f,0.f }, float2{ 0.f,0.f }));
	}

	std::unique_ptr<game::Tile> game::Tileset::copyTile(int index_, float2 worldPos_)
	{
		assert(numTiles > 0);

		int col = index_ % pitch;
		int row = index_ / pitch;

		auto& tmp = tiles.at(index_);
		std::unique_ptr<Tile> t = std::make_unique<Tile>(texID, float2{ tw,th }, float2{ tw,th }, tmp->isSolid(), float2{ (float)(col * tw), (float)(row * th) }, float2{ 0.f,0.f }, worldPos_);
		return std::move(t);
	}
}