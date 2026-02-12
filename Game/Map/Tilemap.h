#pragma once
#include <vector>
#include <utility>
#include <memory>
#include <string>
#include "Tileset.h"


namespace engine
{
	class Renderer2D;
	class Camera2D;
}

namespace game
{
	class Tilemap
	{
		std::unique_ptr<game::Tileset> tileset;
		std::vector<std::unique_ptr<game::Tile>> tiles;

		std::vector<Tile*> getTilesOnScreen(engine::Camera2D cam_);
		std::vector<Tile*> getSolidTilesOnScreen(engine::Camera2D cam_);
	public:
		Tilemap();
		explicit Tilemap(Cfg::Textures texID_, winrt::Windows::Foundation::Numerics::float2 sizeTile_, int pitchSheet_, int numTilesSheet_);
		~Tilemap();


		Tilemap(const Tilemap& o) = delete;
		Tilemap(Tilemap&& o) = default;
		Tilemap& operator=(const Tilemap& o) = delete;
		Tilemap& operator=(Tilemap&& o) = default;

		void loadTileset(const std::wstring& filename_);
		void loadTilemap(const std::wstring& filename_);

		void addTiles(const std::wstring& filename_);
		void addTile(int index_, float2 worldPos_);

		void render(engine::Renderer2D& renderer_, engine::Camera2D camera_);
	};
}