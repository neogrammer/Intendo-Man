#pragma once

#include <winrt/Microsoft.Graphics.Canvas.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Numerics.h>
#include <winrt/Windows.Graphics.Imaging.h>

#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>

#include "SpriteBatchScope.h"

namespace engine
{
    using winrt::Microsoft::Graphics::Canvas::CanvasBitmap;
    using winrt::Microsoft::Graphics::Canvas::CanvasSpriteBatch;
    using winrt::Microsoft::Graphics::Canvas::CanvasSpriteFlip;
    using winrt::Microsoft::Graphics::Canvas::ICanvasResourceCreator;

    using winrt::Windows::Foundation::IAsyncAction;
    using winrt::Windows::Foundation::Rect;
    using winrt::Windows::Foundation::Size;
    using winrt::Windows::Foundation::Uri;
    using winrt::Windows::Foundation::Numerics::float2;
    using winrt::Windows::Foundation::Numerics::float4;
    using winrt::Windows::Graphics::Imaging::BitmapSize;

    class Sprite final
    {
    public:
        Sprite() = delete;
        explicit Sprite(std::wstring const& key);

        static void SetResourceCreator(ICanvasResourceCreator const& creator);
        static void Register(std::wstring key, Uri uri);
        static IAsyncAction RegisterAndLoadAsync(std::wstring key, Uri uri);

        static IAsyncAction LoadSharedAsync(std::wstring const& key);

        IAsyncAction LoadAsync(std::wstring const& key);

        static void Unload(std::wstring const& key);
        static void ClearCache();

        bool IsValid() const noexcept;

        void SetOriginTopLeft() noexcept;
        void SetOriginCenter();
        void Draw(CanvasSpriteBatch const& batch) const; 
        void Draw(SpriteBatchScope const& batchScope) const;
    public:
        float2 Position{ 0.0f, 0.0f };
        float2 Scale{ 1.0f, 1.0f };
        float Rotation{ 0.0f };
        float2 Origin{ 0.0f, 0.0f };
        float4 Tint{ 1.0f, 1.0f, 1.0f, 1.0f };
        CanvasSpriteFlip Flip{ CanvasSpriteFlip::None };
        std::optional<Rect> SourceRect;
        std::wstring m_key;

    private:
        struct SharedTextureData
        {
            CanvasBitmap Bitmap{ nullptr };
            Size SizeDips{};
            BitmapSize SizePixels{};
        };

        float2 ComputeBitmapSpaceOrigin() const noexcept;

    private:
        std::shared_ptr<SharedTextureData> m_texture;

        static ICanvasResourceCreator s_creator;
        static std::shared_mutex s_mutex;
        static std::unordered_map<std::wstring, Uri> s_sources;
        static std::unordered_map<std::wstring, std::shared_ptr<SharedTextureData>> s_cache;
    };
}