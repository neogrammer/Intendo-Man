#include "pch.h"
#include "GameObject.h"

namespace game
{
    GameObject::GameObject(
        Cfg::Textures texID_,
        float2 worldPosition_,
        float2 worldSize_,
        float2 frameSize_,
        float2 texPosition_,
        float2 textureOffset_)
        : texID{ texID_ }
        , texPosition{ texPosition_ }
        , frameSize{ frameSize_ }
        , worldPosition{ worldPosition_ }
        , worldSize{ worldSize_ }
        , textureOffset{ textureOffset_ }
    {
    }

    engine::Sprite GameObject::getSprite() const
    {
        // Note: Cfg::GetTex maps enum -> TextureStore key.
        engine::Sprite out = engine::Sprite{ Cfg::GetTexKey(texID) };

        // Even if the texture is missing (nullptr), Renderer2D will skip invalid sprites.
        out.Position = Sub(worldPosition, textureOffset);
        out.Scale = scale;
        out.Rotation = rotationRad;
        out.Tint = tint;
        out.Flip = flip;

        // Top-left origin (so Position is the top-left of the drawn frame)
        out.Origin = { 0.0f, 0.0f };

        // Source rect (sprite sheet frame)
        if (frameSize.x > 0.0f && frameSize.y > 0.0f)
        {
            out.SourceRect = Rect{ texPosition.x, texPosition.y, frameSize.x, frameSize.y };
        }
        else
        {
            out.SourceRect.reset();
        }

        return std::move(static_cast<engine::Sprite&&>(out));
    }

    Rect GameObject::getWorldRect() const noexcept
    {
        return Rect{ worldPosition.x, worldPosition.y, worldSize.x, worldSize.y };
    }

    Rect& GameObject::getWorldRectRef() noexcept
    {
        rect = Rect{ worldPosition.x, worldPosition.y, worldSize.x, worldSize.y };
        return rect;
    }

    bool GameObject::intersects(GameObject const& other) const noexcept
    {
        auto const a = getWorldRect();
        auto const b = other.getWorldRect();

        // AABB overlap test
        if (a.X + a.Width <= b.X) return false;
        if (b.X + b.Width <= a.X) return false;
        if (a.Y + a.Height <= b.Y) return false;
        if (b.Y + b.Height <= a.Y) return false;

        return true;
    }
}
