#pragma once
#include <winrt/Windows.Foundation.Numerics.h>
#include <winrt/Windows.UI.h>

namespace engine
{
    using winrt::Windows::Foundation::Numerics::float2;
    using winrt::Windows::Foundation::Numerics::float4;
    using winrt::Windows::UI::Color;

    constexpr float Pi = 3.14159265358979323846f;

    constexpr float DegToRad(float degrees) noexcept { return degrees * Pi / 180.0f; }
    constexpr float RadToDeg(float radians) noexcept { return radians * 180.0f / Pi; }

    // Win2D CanvasSpriteBatch tint is a non-premultiplied Vector4 multiplier. :contentReference[oaicite:4]{index=4}
    inline float4 Tint(Color const& c) noexcept
    {
        return { c.R / 255.0f, c.G / 255.0f, c.B / 255.0f, c.A / 255.0f };
    }
}