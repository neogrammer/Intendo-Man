#pragma once

#include "Engine/Matrix2D.h"

namespace engine
{
    class Camera2D final
    {
    public:
        float2 Position{ 0,0 }; // world position that maps to screen center
        float  Zoom{ 1.0f };    // 1 = normal
        float  RotationRad{ 0.0f };

        void SetViewportSize(float2 size) noexcept { m_viewportSize = size; }
        float2 ViewportSize() const noexcept { return m_viewportSize; }

        void Reset() noexcept
        {
            Position = { 0,0 };
            Zoom = 1.0f;
            RotationRad = 0.0f;
        }

        // World -> Screen transform for CanvasDrawingSession.Transform :contentReference[oaicite:10]{index=10}
        winrt::Windows::Foundation::Numerics::float3x2 WorldToScreen() const noexcept;

        winrt::Windows::Foundation::Numerics::float3x2 ScreenToWorld() const noexcept
        {
            return Invert(WorldToScreen());
        }

        float getWidth();
        float getHeight();


    private:
        float2 m_viewportSize{ 0,0 };
    };
}