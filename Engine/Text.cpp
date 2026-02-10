#include "pch.h"
#include "Engine/Text.h"

#include <cfloat>

namespace engine
{
    void Text::EnsureLayout(winrt::Microsoft::Graphics::Canvas::ICanvasResourceCreator const& creator) const
    {
        auto dev = creator.Device();

        if (!m_dirty && m_layout && m_device && dev == m_device)
            return;

        m_device = dev;

        if (!FontRef || FontRef->FamilySpec.empty())
        {
            m_layout = nullptr;
            m_format = nullptr;
            m_layoutBounds = {};
            m_drawBounds = {};
            m_dirty = false;
            return;
        }

        // Format
        m_format = winrt::Microsoft::Graphics::Canvas::Text::CanvasTextFormat();
        m_format.FontFamily(FontRef->FamilySpec);
        m_format.FontSize(FontSize);
        m_format.FontWeight(Weight);
        m_format.FontStyle(Style);
        m_format.WordWrapping(WordWrapping);
        m_format.HorizontalAlignment(HorizontalAlignment);

        float w = FLT_MAX;
        float h = FLT_MAX;

        if (LayoutBoxSize)
        {
            if (LayoutBoxSize->x > 0.0f) w = LayoutBoxSize->x;
            if (LayoutBoxSize->y > 0.0f) h = LayoutBoxSize->y;
        }

        // Layout
        m_layout = winrt::Microsoft::Graphics::Canvas::Text::CanvasTextLayout(
            creator,
            winrt::hstring(String),
            m_format,
            w,
            h);

        m_layoutBounds = m_layout.LayoutBounds();
        m_drawBounds = m_layout.DrawBounds();

        m_dirty = false;
        m_geometry = nullptr;
    }

    winrt::Windows::Foundation::Rect Text::LocalBounds(
        winrt::Microsoft::Graphics::Canvas::ICanvasResourceCreator const& creator,
        TextBoundsMode mode) const
    {
        EnsureLayout(creator);
        return (mode == TextBoundsMode::LayoutBounds) ? m_layoutBounds : m_drawBounds;
    }

    void Text::SetOriginTopLeft(
        winrt::Microsoft::Graphics::Canvas::ICanvasResourceCreator const& creator,
        TextBoundsMode mode)
    {
        auto b = LocalBounds(creator, mode);
        Origin = { b.X, b.Y };
    }

    void Text::SetOriginCenter(
        winrt::Microsoft::Graphics::Canvas::ICanvasResourceCreator const& creator,
        TextBoundsMode mode)
    {
        auto b = LocalBounds(creator, mode);
        Origin = { b.X + b.Width * 0.5f, b.Y + b.Height * 0.5f };
    }

    void Text::Draw(
        winrt::Microsoft::Graphics::Canvas::CanvasDrawingSession const& ds,
        winrt::Microsoft::Graphics::Canvas::ICanvasResourceCreator const& creator) const
    {
        EnsureLayout(creator);
        if (!m_layout) return;

        auto old = ds.Transform();

        auto local =
            Multiply(
                Multiply(
                    Multiply(
                        Translation({ -Origin.x, -Origin.y }),
                        engine::Scale(Scale)),
                    engine::Rotation(Rotation)),
                engine::Translation(Position));

        ds.Transform(Multiply(local, old));

        bool doOutline = (OutlineThickness > 0.0f) && (OutlineColor.A != 0);

        if (doOutline && Outline == OutlineMode::GeometryStroke)
        {
            if (!m_geometry)
            {
                // Create geometry from the text layout 
                m_geometry = winrt::Microsoft::Graphics::Canvas::Geometry::CanvasGeometry::CreateText(m_layout);
            }

            // If you want SFML-ish "mostly outside" outline:
            // draw stroke FIRST with width = 2*t, then fill; the fill covers the inner half.
            float strokeW = OutlineThickness * 2.0f;

            ds.DrawGeometry(m_geometry, OutlineColor, strokeW); // 
            ds.FillGeometry(m_geometry, Color);                 // 
        }
        else if (doOutline && Outline == OutlineMode::OffsetCopies)
        {
            // Cheap, preserves DrawTextLayout rasterization; cost = many draws
            int r = std::max<int>(1, (int)std::lround(OutlineThickness));

            for (int y = -r; y <= r; ++y)
            {
                for (int x = -r; x <= r; ++x)
                {
                    if (x == 0 && y == 0) continue;
                    ds.DrawTextLayout(m_layout, float2{ (float)x, (float)y }, OutlineColor);
                }
            }

            ds.DrawTextLayout(m_layout, float2{ 0,0 }, Color);
        }
        else
        {
            ds.DrawTextLayout(m_layout, float2{ 0,0 }, Color);
        }

        ds.Transform(old);
    }
}