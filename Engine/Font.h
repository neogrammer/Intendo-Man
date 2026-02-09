#pragma once

#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.Graphics.Canvas.Text.h>

namespace engine
{
    // SFML-ish "Font" resource wrapper.
    // In Win2D you mostly reference fonts by "family spec" (system family or ms-appx uri + #family).
    struct Font final
    {
        // Examples:
        //   L"Segoe UI"
        //   L"ms-appx:///Assets/Fonts/PressStart2P.ttf#Press Start 2P"
        winrt::hstring FamilySpec{};

        // Optional: keep a font set alive when using packaged fonts.
        // This isn't required to render, but it helps validate the font file path and keeps it referenced.
        winrt::Microsoft::Graphics::Canvas::Text::CanvasFontSet FontSet{ nullptr };
    };
}