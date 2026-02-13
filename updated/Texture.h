#pragma once
#include <winrt/Microsoft.Graphics.Canvas.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Foundation.h>

namespace engine
{
    struct Texture final
    {
        Texture();

        winrt::Microsoft::Graphics::Canvas::CanvasBitmap Bitmap;

        // DIPs + pixels sizes are both useful (SpriteBatch uses DIPs by default).
        winrt::Windows::Foundation::Size SizeDips;
        winrt::Windows::Graphics::Imaging::BitmapSize SizePixels; // from CanvasBitmap.SizeInPixels :contentReference[oaicite:5]{index=5}
    };
}
