#pragma once
#include "Engine/SpriteBatchScope.h"
#include "Engine/Sprite.h"

namespace engine
{
    class Renderer2D final
    {
    public:
        explicit Renderer2D(winrt::Microsoft::Graphics::Canvas::CanvasDrawingSession const& ds)
            : m_batch(ds) {
        }

        void Draw(Sprite const& sprite)
        {
            if (!sprite.IsValid())
            {
                return;
            }

            if (m_batch.IsBatching())
            {
                sprite.Draw(m_batch.Batch());
            }
            else
            {
                // Fallback if CanvasSpriteBatch unsupported:
                m_batch.DrawingSession().DrawImage(sprite.TextureRef->Bitmap, sprite.Position);
            }
        }

    private:
        SpriteBatchScope m_batch;
    };
}