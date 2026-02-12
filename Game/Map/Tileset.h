#pragma once
#include <vector>
#include <utility>
#include <memory>
#include "Tile.h"



namespace game
{
	class Tileset
	{
		std::vector<std::unique_ptr<game::Tile>> tiles{};

		Cfg::Textures texID{ Cfg::Textures::None };

		float tw{ 0.f };
		float th{ 0.f };
		int pitch{ 0 };
		int numTiles{ 0 };

	public:
		Tileset();
		explicit Tileset(Cfg::Textures texID_, winrt::Windows::Foundation::Numerics::float2 size_, int pitch_, int numTiles_);
		~Tileset();
		

		Tileset(const Tileset& o) = delete;
		Tileset(Tileset&& o) = default;
		Tileset& operator=(const Tileset& o) = delete;
		Tileset& operator=(Tileset&& o) = default;


		void addTiles(const std::wstring& filename_);

		void addTile(bool solid_, winrt::Windows::Foundation::Numerics::float2 texPosition_);
		std::unique_ptr<Tile> copyTile(int index_, winrt::Windows::Foundation::Numerics::float2 worldPos_);
		inline bool hasTiles() { return (tiles.size() > 0); }
		inline float tileW() { return tw; }
		inline float tileH() { return th; }
	};
}