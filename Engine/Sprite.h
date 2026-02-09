#pragma once
#include "Engine/Texture.h"

#include <winrt/Microsoft.Graphics.Canvas.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Numerics.h>

#include <memory>
#include <optional>

namespace engine
{
    using winrt::Windows::Foundation::Rect;
    using winrt::Windows::Foundation::Numerics::float2;
    using winrt::Windows::Foundation::Numerics::float4;

    using winrt::Microsoft::Graphics::Canvas::CanvasSpriteBatch;
    using winrt::Microsoft::Graphics::Canvas::CanvasSpriteFlip;

    struct Sprite final
    {
        std::shared_ptr<Texture> TextureRef;

        float2 Position{ 0.0f, 0.0f };
        float2 Scale{ 1.0f, 1.0f };
        float Rotation{ 0.0f }; // radians

        // SFML-like: origin relative to SourceRect if set; otherwise relative to whole texture.
        float2 Origin{ 0.0f, 0.0f };

        // Non-premultiplied RGBA multiplier (1,1,1,1 = no tint)
        float4 Tint{ 1.0f, 1.0f, 1.0f, 1.0f };

        CanvasSpriteFlip Flip{ CanvasSpriteFlip::None };

        // Sprite-sheet source rect (DIPs)
        std::optional<Rect> SourceRect;

        Sprite() = default;
        explicit Sprite(std::shared_ptr<Texture> texture) : TextureRef(std::move(texture)) {}

        bool IsValid() const noexcept { return TextureRef && TextureRef->Bitmap; }

        void SetOriginTopLeft() noexcept { Origin = { 0,0 }; }
        void SetOriginCenter();

        void Draw(CanvasSpriteBatch const& batch) const;

    private:
        float2 ComputeBitmapSpaceOrigin() const noexcept;
    };
}