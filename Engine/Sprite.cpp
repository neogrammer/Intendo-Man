#include "pch.h"
#include "Engine/Sprite.h"

namespace engine
{
    void Sprite::SetOriginCenter()
    {
        if (!IsValid())
        {
            Origin = { 0,0 };
            return;
        }

        if (SourceRect)
        {
            Origin = { SourceRect->Width * 0.5f, SourceRect->Height * 0.5f };
        }
        else
        {
            Origin = {
                static_cast<float>(TextureRef->SizeDips.Width) * 0.5f,
                static_cast<float>(TextureRef->SizeDips.Height) * 0.5f
            };
        }
    }

    float2 Sprite::ComputeBitmapSpaceOrigin() const noexcept
    {
        // Win2D expects origin relative to the *source bitmap*, even for DrawFromSpriteSheet. :contentReference[oaicite:11]{index=11}
        // We expose SFML-like origin relative to SourceRect, and translate here.
        if (SourceRect)
        {
            return { Origin.x + SourceRect->X, Origin.y + SourceRect->Y };
        }
        return Origin;
    }

    void Sprite::Draw(CanvasSpriteBatch const& batch) const
    {
        if (!IsValid())
        {
            return;
        }

        auto const bmp = TextureRef->Bitmap;
        auto const originInBitmap = ComputeBitmapSpaceOrigin();

        if (SourceRect)
        {
            // DrawFromSpriteSheet overload supports per-sprite rotation/scale/tint/flip. :contentReference[oaicite:12]{index=12}
            batch.DrawFromSpriteSheet(bmp, Position, *SourceRect, Tint, originInBitmap, Rotation, Scale, Flip);
        }
        else
        {
            // Draw overload supports per-sprite rotation/scale/tint/flip. :contentReference[oaicite:13]{index=13}
            batch.Draw(bmp, Position, Tint, originInBitmap, Rotation, Scale, Flip);
        }
    }
}