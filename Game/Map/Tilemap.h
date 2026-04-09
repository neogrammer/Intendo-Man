#pragma once
#include <vector>
#include <utility>
#include <memory>
#include <string>
#include "Tileset.h"
#include "../../Engine/Sprite.h"




namespace engine
{
	class SpriteBatchScope;
	class Camera2D;
}

namespace game
{
	class Tilemap
	{
		enum class Transitioning
		{
			Up,
			Down,
			None
		};

		std::unique_ptr<game::Tileset> tileset;
		std::vector<std::unique_ptr<game::Tile>> tiles;

		

		int pitch{ 0 };

		int currentLevel{ 0 };
		float currentTopTile{ 0.f };
		float currentBottomTile{ 13.5f };

		int levels{ 1 };

		std::vector<float> topOfLevels{};
		std::vector<float> bottomOfLevels{};
		Transitioning transitioning{ Transitioning::None };

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
		std::vector<std::unique_ptr<game::Tile>>& getTiles();


		void render(engine::SpriteBatchScope const& batch_, engine::Camera2D camera_);
		inline int getPitch() { return pitch; }
		float2 getTileSize();

		std::vector<Tile*> getTilesOnScreen(engine::Camera2D cam_);
		std::vector<Tile*> getSolidTilesOnScreen(engine::Camera2D cam_);
		std::vector<Tile*> getSolidTilesInRect(winrt::Windows::Foundation::Rect const& worldRect_, int padTiles = 1);
	};
}