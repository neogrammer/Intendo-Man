#pragma once

// GameObject: base functionality for rendering + collisions.
// - Stores ONLY single values (no containers)
// - Values are protected (per your design)

#include "../Resources/Cfg.h"          // Cfg::Textures + Cfg::GetTex
#include "../../Engine/Sprite.h"       // engine::Sprite

#include <winrt/Windows.Foundation.h>            // Rect
#include <winrt/Windows.Foundation.Numerics.h>   // float2/float4

#include <memory>

namespace game
{
    using winrt::Windows::Foundation::Rect;
    using winrt::Windows::Foundation::Numerics::float2;
    using winrt::Windows::Foundation::Numerics::float4;

    class GameObject
    {
    protected:
        // --- Render data (single values)
        Cfg::Textures texID{ Cfg::Textures::None };

        // Where the source-rect starts inside the texture.
        float2 texPosition{ 0.0f, 0.0f };

        // Size of the sprite frame (source rect size, also the on-screen drawn size when Scale = {1,1}).
        float2 frameSize{ 0.0f, 0.0f };

        // World-space collider box (top-left + size)
        float2 worldPosition{ 0.0f, 0.0f };
        float2 worldSize{ 0.0f, 0.0f };

        // From frame top-left -> collider top-left (so draw at worldPosition - textureOffset)
        float2 textureOffset{ 0.0f, 0.0f };

        // Optional sprite params (still single values)
        float2 scale{ 1.0f, 1.0f };
        float rotationRad{ 0.0f };
        float4 tint{ 1.0f, 1.0f, 1.0f, 1.0f };
        engine::CanvasSpriteFlip flip{ engine::CanvasSpriteFlip::None };
        bool onGround{ false };

        winrt::Windows::Foundation::Rect rect{ 0L,0L,0L,0L };

        bool affectedByGravity{ true };


    public:

        inline void land() { onGround = true; }
        inline void inAir() { onGround = false; }

        inline bool isAffectedByGravity() { return affectedByGravity; }
        inline void setAffectedByGravity(bool cond_) { affectedByGravity = cond_; }


        inline bool isGrounded() { return onGround; }

        GameObject() = default;

        GameObject(
            Cfg::Textures texID_,
            float2 worldPosition_,
            float2 worldSize_,
            float2 frameSize_,
            float2 texPosition_ = { 0.0f, 0.0f },
            float2 textureOffset_ = { 0.0f, 0.0f });

        virtual ~GameObject() = default;

        // Create a sprite on-demand. Caller can immediately do:
        //    renderer.Draw(*obj.getSprite());
        std::unique_ptr<engine::Sprite> getSprite() const;

        Rect getWorldRect() const noexcept;
        Rect& getWorldRectRef() noexcept;
        bool intersects(GameObject const& other) const noexcept;

        // --- Basic accessors / mutators (still single values)
        void SetWorldPosition(float2 p) noexcept { worldPosition = p; }
        float2 GetWorldPosition() const noexcept { return worldPosition; }
        void Move(float2 delta) noexcept { worldPosition = Add(worldPosition, delta); }

        void SetWorldSize(float2 s) noexcept { worldSize = s; }
        float2 GetWorldSize() const noexcept { return worldSize; }

        void SetTextureOffset(float2 o) noexcept { textureOffset = o; }
        float2 GetTextureOffset() const noexcept { return textureOffset; }

        void SetTexPosition(float2 p) noexcept { texPosition = p; }
        float2 GetTexPosition() const noexcept { return texPosition; }

        void SetFrameSize(float2 s) noexcept { frameSize = s; }
        float2 GetFrameSize() const noexcept { return frameSize; }

        void SetTexID(Cfg::Textures id) noexcept { texID = id; }
        Cfg::Textures GetTexID() const noexcept { return texID; }

        void SetScale(float2 s) noexcept { scale = s; }
        float2 GetScale() const noexcept { return scale; }

        void SetRotationRad(float r) noexcept { rotationRad = r; }
        float GetRotationRad() const noexcept { return rotationRad; }

        void SetTint(float4 t) noexcept { tint = t; }
        float4 GetTint() const noexcept { return tint; }

        void SetFlip(engine::CanvasSpriteFlip f) noexcept { flip = f; }
        engine::CanvasSpriteFlip GetFlip() const noexcept { return flip; }

        // Helpers (float2 has no operators)
        static constexpr float2 Add(float2 a, float2 b) noexcept { return { a.x + b.x, a.y + b.y }; }
        static constexpr float2 Sub(float2 a, float2 b) noexcept { return { a.x - b.x, a.y - b.y }; }
        static constexpr float2 Mul(float2 a, float s) noexcept { return { a.x * s, a.y * s }; }
    };
}
