#pragma once
#include <winrt/Microsoft.Graphics.Canvas.h>
#include <winrt/Windows.Foundation.h>

namespace engine
{
    class SpriteBatchScope final
    {
    public:
        explicit SpriteBatchScope(winrt::Microsoft::Graphics::Canvas::CanvasDrawingSession const& ds)
            : m_ds(ds)
        {
            // CanvasSpriteBatch is available on supported OS versions and is more efficient for many sprites. :contentReference[oaicite:9]{index=9}
            if (winrt::Microsoft::Graphics::Canvas::CanvasSpriteBatch::IsSupported(ds.Device()))
            {
                m_batch = ds.CreateSpriteBatch();
            }
        }

        SpriteBatchScope(SpriteBatchScope const&) = delete;
        SpriteBatchScope& operator=(SpriteBatchScope const&) = delete;

        ~SpriteBatchScope()
        {
            // IMPORTANT: Close() flushes the batch to the drawing session (SFML-like "end"). :contentReference[oaicite:10]{index=10}
            if (m_batch)
            {
                m_batch.Close();
            }
        }

        bool IsBatching() const noexcept { return m_batch != nullptr; }
        winrt::Microsoft::Graphics::Canvas::CanvasSpriteBatch const& Batch() const noexcept { return m_batch; }
        winrt::Microsoft::Graphics::Canvas::CanvasDrawingSession const& DrawingSession() const noexcept { return m_ds; }

    private:
        winrt::Microsoft::Graphics::Canvas::CanvasDrawingSession m_ds{ nullptr };
        winrt::Microsoft::Graphics::Canvas::CanvasSpriteBatch m_batch{ nullptr };
    };
}