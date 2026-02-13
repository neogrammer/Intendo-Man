#include "pch.h"
#include "Tilemap.h"
#include "../../Engine/Camera2D.h"
#include "../../Engine/Renderer2D.h"
#include <fstream>
#include <cassert>
#include <Engine/Utils.h>
#include <algorithm>
#include <cmath>

namespace game
{
    using winrt::Windows::Foundation::Rect;
    using winrt::Windows::Foundation::Numerics::float2;

    struct TileRange
    {
        int x0{ 0 }, x1{ -1 };
        int y0{ 0 }, y1{ -1 };
        bool empty() const noexcept { return x1 < x0 || y1 < y0; }
    };

    inline int ClampInt(int v, int lo, int hi) noexcept
    {
        return (v < lo) ? lo : (v > hi) ? hi : v;
    }


    Tilemap::Tilemap()
    {
        topOfLevels.push_back(0.f);
        bottomOfLevels.push_back(13.5f);
    }

    Tilemap::Tilemap(Cfg::Textures texID_, float2 sizeTile_, int pitchSheet_, int numTilesSheet_)
        : tileset{ std::make_unique<Tileset>(texID_, sizeTile_, pitchSheet_, numTilesSheet_) }
    {
        topOfLevels.push_back(0.f);
        bottomOfLevels.push_back(13.5f);
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


    float2 Tilemap::getTileSize()
    {
        return { tileset->tileW(), tileset->tileH() };
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
        pitch = cols;

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


    TileRange RangeFromWorldRect(Rect const& worldRect,
        float tileW, float tileH,
        int cols, int rows,
        int padTiles) noexcept
    {
        TileRange out{};
        if (cols <= 0 || rows <= 0 || tileW <= 0.0f || tileH <= 0.0f ||
            worldRect.Width <= 0.0f || worldRect.Height <= 0.0f)
        {
            return out;
        }

        float left = worldRect.X;
        float top = worldRect.Y;
        float right = worldRect.X + worldRect.Width;
        float bottom = worldRect.Y + worldRect.Height;

        // inclusive end = ceil - 1
        int x0 = (int)std::floor(left / tileW) - padTiles;
        int y0 = (int)std::floor(top / tileH) - padTiles;
        int x1 = (int)std::ceil(right / tileW) + padTiles - 1;
        int y1 = (int)std::ceil(bottom / tileH) + padTiles - 1;

        x0 = ClampInt(x0, 0, cols - 1);
        y0 = ClampInt(y0, 0, rows - 1);
        x1 = ClampInt(x1, 0, cols - 1);
        y1 = ClampInt(y1, 0, rows - 1);

        out.x0 = x0; out.y0 = y0;
        out.x1 = x1; out.y1 = y1;
        return out;
    }

    // Camera view → world AABB (handles zoom + rotation)
    Rect WorldViewAabb(engine::Camera2D cam) noexcept
    {
        float zoom = std::max<float>(0.0001f, cam.Zoom);

        float halfW = (cam.getWidth() * 0.5f) / zoom;
        float halfH = (cam.getHeight() * 0.5f) / zoom;

        float c = std::cos(cam.RotationRad);
        float s = std::sin(cam.RotationRad);

        float absC = std::abs(c);
        float absS = std::abs(s);

        float boundHalfW = absC * halfW + absS * halfH;
        float boundHalfH = absS * halfW + absC * halfH;

        return Rect{
            cam.Position.x - boundHalfW,
            cam.Position.y - boundHalfH,
            boundHalfW * 2.0f,
            boundHalfH * 2.0f
        };
    }


    std::vector<Tile*> Tilemap::getTilesOnScreen(engine::Camera2D cam_)
    {
        std::vector<Tile*> out{};
        if (!tileset || tiles.empty() || pitch <= 0) return out;

        int cols = pitch;
        int rows = (int)(tiles.size() / (size_t)pitch);
        if (rows <= 0) return out;

        float tileW = tileset->tileW();
        float tileH = tileset->tileH();

        constexpr int PAD_TILES = 1;
        auto viewRect = WorldViewAabb(cam_);
        auto range = RangeFromWorldRect(viewRect, tileW, tileH, cols, rows, PAD_TILES);
        if (range.empty()) return out;

        out.reserve((size_t)(range.x1 - range.x0 + 1) * (size_t)(range.y1 - range.y0 + 1));

        for (int y = range.y0; y <= range.y1; ++y)
        {
            size_t rowBase = (size_t)y * (size_t)cols;
            for (int x = range.x0; x <= range.x1; ++x)
            {
                out.push_back(tiles[rowBase + (size_t)x].get());
            }
        }
        return out;
    }

    std::vector<Tile*> Tilemap::getSolidTilesOnScreen(engine::Camera2D cam_)
    {
        std::vector<Tile*> out{};
        if (!tileset || tiles.empty() || pitch <= 0) return out;

        int cols = pitch;
        int rows = (int)(tiles.size() / (size_t)pitch);
        if (rows <= 0) return out;

        float tileW = tileset->tileW();
        float tileH = tileset->tileH();

        constexpr int PAD_TILES = 1;
        auto viewRect = WorldViewAabb(cam_);
        auto range = RangeFromWorldRect(viewRect, tileW, tileH, cols, rows, PAD_TILES);
        if (range.empty()) return out;

        out.reserve((size_t)(range.x1 - range.x0 + 1) * (size_t)(range.y1 - range.y0 + 1));

        for (int y = range.y0; y <= range.y1; ++y)
        {
            size_t rowBase = (size_t)y * (size_t)cols;
            for (int x = range.x0; x <= range.x1; ++x)
            {
                auto* t = tiles[rowBase + (size_t)x].get();
                if (t && t->isSolid())
                    out.push_back(t);
            }
        }
        return out;
    }

    std::vector<game::Tile*> Tilemap::getSolidTilesInRect(Rect const& worldRect_, int padTiles)
    {
        std::vector<Tile*> out{};
        if (!tileset || tiles.empty() || pitch <= 0) return out;

        int cols = pitch;
        int rows = (int)(tiles.size() / (size_t)pitch);
        if (rows <= 0) return out;

        float tileW = tileset->tileW();
        float tileH = tileset->tileH();

        auto range = RangeFromWorldRect(worldRect_, tileW, tileH, cols, rows, padTiles);
        if (range.empty()) return out;

        out.reserve((size_t)(range.x1 - range.x0 + 1) * (size_t)(range.y1 - range.y0 + 1));

        for (int y = range.y0; y <= range.y1; ++y)
        {
            size_t rowBase = (size_t)y * (size_t)cols;
            for (int x = range.x0; x <= range.x1; ++x)
            {
                auto* t = tiles[rowBase + (size_t)x].get();
                if (t && t->isSolid())
                    out.push_back(t);
            }
        }
        return out;
    }

    void Tilemap::render(engine::Renderer2D& renderer_, engine::Camera2D camera_)
    {
        // Draw directly from the range (avoids building a temp vector)
        if (!tileset || tiles.empty() || pitch <= 0) return;

        int cols = pitch;
        int rows = (int)(tiles.size() / (size_t)pitch);
        if (rows <= 0) return;

        float tileW = tileset->tileW();
        float tileH = tileset->tileH();

        constexpr int PAD_TILES = 1;
        auto viewRect = WorldViewAabb(camera_);
        auto range = RangeFromWorldRect(viewRect, tileW, tileH, cols, rows, PAD_TILES);
        if (range.empty()) return;

        for (int y = range.y0; y <= range.y1; ++y)
        {
            size_t rowBase = (size_t)y * (size_t)cols;
            for (int x = range.x0; x <= range.x1; ++x)
            {
                auto* t = tiles[rowBase + (size_t)x].get();
                if (!t) continue;
                renderer_.Draw(t->getSprite());
            }
        }
    }

}