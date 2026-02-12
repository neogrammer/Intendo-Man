#include "pch.h"
#include "Tileset.h"
#include <fstream>
#include <cassert>
#include "../Engine/Utils.h"

namespace game
{


	using winrt::Windows::Foundation::Numerics::float2;

	Tileset::Tileset()
	{
		tiles.clear();
	}

	Tileset::Tileset(Cfg::Textures texID_, float2 size_, int pitch_, int numTiles_)
		: texID{ texID_ }
		, tw {size_.x}
		, th{ size_.y }
		, pitch{ pitch_ }
		, numTiles{ numTiles_ }
	{
		tiles.clear();
	}

	Tileset::~Tileset()
	{
	}

	void Tileset::addTiles(const std::wstring& filename_)
	{
		std::wstring content;

		if (filename_.rfind(L"ms-appx:///", 0) == 0)
		{
			content = util::ReadAppxTextFileSync(filename_);
			if (content.empty())
				return;
		}
		else
		{
			std::wifstream file(filename_);
			if (!file.is_open())
				return;

			std::wstringstream ss;
			ss << file.rdbuf();
			content = ss.str();
		}

		std::wistringstream iFile(content);

		tiles.clear();
		tiles.reserve(numTiles);

		int numRows = static_cast<int>(std::ceil((float)numTiles / (float)pitch));

		for (int y = 0; y < numRows; y++)
		{
			for (int x = 0; x < pitch; x++)
			{
				int s = 0;
				iFile >> s;

				float2 pos = { x * tw, y * th };
				addTile((s == 1), pos);
			}
		}
	}

	void Tileset::addTile(bool solid_, float2 texPosition_)
	{
		std::unique_ptr<Tile> t = std::make_unique<Tile>(texID, float2{ tw, th }, float2{ tw, th }, solid_, texPosition_, float2{ 0.f,0.f }, float2{ 0.f,0.f });

		tiles.emplace_back(std::move(t));
	}

	std::unique_ptr<Tile> Tileset::copyTile(int index_, float2 worldPos_)
	{
		assert(numTiles > 0);

		int col = index_ % pitch;
		int row = index_ / pitch;

		auto& tmp = tiles.at(index_);
		std::unique_ptr<Tile> t = std::make_unique<Tile>(texID, float2{ tw,th }, float2{ tw,th }, tmp->isSolid(), float2{ (float)(col * tw), (float)(row * th) }, float2{ 0.f,0.f }, worldPos_);
		return std::move(t);
	}
}