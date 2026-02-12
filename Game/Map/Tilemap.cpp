#include "pch.h"
#include "Tilemap.h"
#include "../../Engine/Camera2D.h"
#include "../../Engine/Renderer2D.h"
#include <fstream>
#include <cassert>
#include <Engine/Utils.h>


namespace game
{



	using winrt::Windows::Foundation::Numerics::float2;

	std::vector<game::Tile*> Tilemap::getTilesOnScreen(engine::Camera2D cam_)
	{

		std::vector<Tile*> tmp{};
		tmp.reserve(tiles.size());
		for (auto& t : tiles)
		{
			tmp.push_back(t.get());
		}
		tmp.shrink_to_fit();

		return tmp;
	}

	std::vector<game::Tile*> game::Tilemap::getSolidTilesOnScreen(engine::Camera2D cam_)
	{
		std::vector<Tile*> tmp{};
		tmp.reserve(tiles.size());
		for (auto& t : tiles)
		{
			if (t->isSolid())
				tmp.push_back(t.get());
		}
		tmp.shrink_to_fit();

		return tmp;
	}

	Tilemap::Tilemap()
	{
	}

	Tilemap::Tilemap(Cfg::Textures texID_, float2 sizeTile_, int pitchSheet_, int numTilesSheet_)
		: tileset{ std::make_unique<Tileset>(texID_, sizeTile_, pitchSheet_, numTilesSheet_) }
	{
	}

	Tilemap::~Tilemap()
	{
	}


	void Tilemap::loadTileset(const std::wstring& filename_)
	{
		tileset->addTiles(filename_);
	}

	void Tilemap::loadTilemap(const std::wstring& filename_)
	{
		addTiles(filename_);
	}

	void Tilemap::addTiles(const std::wstring& filename_)
	{
		bool h = tileset->hasTiles();
		assert(h);

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

		int cols = 0;
		int rows = 0;

		iFile >> cols >> rows;

		tiles.reserve(cols * rows);

		for (int y = 0; y < rows; y++)
		{
			for (int x = 0; x < cols; x++)
			{
				int idx = 0;
				iFile >> idx;

				float2 pos =
				{
					x * tileset->tileW(),
					y * tileset->tileH()
				};

				addTile(idx, pos);
			}
		}
	}

	void Tilemap::addTile(int index_, float2 worldPos_)
	{
		tiles.emplace_back(std::move(tileset->copyTile(index_, worldPos_)));
	}

	void Tilemap::render(engine::Renderer2D& renderer_, engine::Camera2D camera_)
	{
		for (auto& t : getTilesOnScreen(camera_))
			renderer_.Draw(*t->getSprite());	
	}

}
