#pragma once

#include "Engine/Font.h"
#include "Engine/Math.h"
#include "Engine/Matrix2D.h"

#include <winrt/Microsoft.Graphics.Canvas.h>
#include <winrt/Microsoft.Graphics.Canvas.Text.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Text.h>

#include <memory>
#include <optional>
#include <string>

namespace engine
{
    enum class TextBoundsMode
    {
        LayoutBounds,
        DrawBounds
    };

    struct Text final
    {
        // Outline (SFML-like)
        winrt::Windows::UI::Color OutlineColor = winrt::Windows::UI::Colors::Black();
        float OutlineThickness = 0.0f; // in DIPs

        enum class OutlineMode { None, GeometryStroke, OffsetCopies };
        OutlineMode Outline = OutlineMode::GeometryStroke;

        // cache geometry when using GeometryStroke
        mutable winrt::Microsoft::Graphics::Canvas::Geometry::CanvasGeometry m_geometry{ nullptr };

        // Resource
        std::shared_ptr<Font> FontRef;

        // Content
        std::wstring String;
        float FontSize = 24.0f;
        winrt::Windows::UI::Color Color = winrt::Windows::UI::Colors::White();

        // Transform
        float2 Position{ 0,0 };
        float2 Scale{ 1,1 };
        float  Rotation = 0.0f; // radians
        float2 Origin{ 0,0 };

        // Layout options
        // If unset -> no wrap (requested width/height = FLT_MAX)
        std::optional<float2> LayoutBoxSize;

        // Formatting
        winrt::Windows::UI::Text::FontWeight Weight = winrt::Windows::UI::Text::FontWeights::Normal();
        winrt::Windows::UI::Text::FontStyle  Style = winrt::Windows::UI::Text::FontStyle::Normal;

        winrt::Microsoft::Graphics::Canvas::Text::CanvasWordWrapping WordWrapping =
            winrt::Microsoft::Graphics::Canvas::Text::CanvasWordWrapping::NoWrap;

        winrt::Microsoft::Graphics::Canvas::Text::CanvasHorizontalAlignment HorizontalAlignment =
            winrt::Microsoft::Graphics::Canvas::Text::CanvasHorizontalAlignment::Left;

        // Call when you change String/FontRef/FontSize/Weight/Style/Wrap/Alignment/LayoutBoxSize
        void Invalidate() const noexcept { m_dirty = true; }

        winrt::Windows::Foundation::Rect LocalBounds(
            winrt::Microsoft::Graphics::Canvas::ICanvasResourceCreator const& creator,
            TextBoundsMode mode = TextBoundsMode::LayoutBounds) const;

        void SetOriginTopLeft(
            winrt::Microsoft::Graphics::Canvas::ICanvasResourceCreator const& creator,
            TextBoundsMode mode = TextBoundsMode::LayoutBounds);

        void SetOriginCenter(
            winrt::Microsoft::Graphics::Canvas::ICanvasResourceCreator const& creator,
            TextBoundsMode mode = TextBoundsMode::LayoutBounds);

        void Draw(
            winrt::Microsoft::Graphics::Canvas::CanvasDrawingSession const& ds,
            winrt::Microsoft::Graphics::Canvas::ICanvasResourceCreator const& creator) const;

    private:
        void EnsureLayout(winrt::Microsoft::Graphics::Canvas::ICanvasResourceCreator const& creator) const;

    private:
        mutable bool m_dirty = true;

        // Rebuild if device changes (device lost/recreate)
        mutable winrt::Microsoft::Graphics::Canvas::CanvasDevice m_device{ nullptr };

        mutable winrt::Microsoft::Graphics::Canvas::Text::CanvasTextFormat m_format{ nullptr };
        mutable winrt::Microsoft::Graphics::Canvas::Text::CanvasTextLayout m_layout{ nullptr };

        mutable winrt::Windows::Foundation::Rect m_layoutBounds{};
        mutable winrt::Windows::Foundation::Rect m_drawBounds{};
    };
}