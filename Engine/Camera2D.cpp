#include "pch.h"
#include "Engine/Camera2D.h"

namespace engine
{
    winrt::Windows::Foundation::Numerics::float3x2 Camera2D::WorldToScreen() const noexcept
    {
        float2 center{ m_viewportSize.x * 0.5f, m_viewportSize.y * 0.5f };

        // With row-vector layout (vM), transforms apply left-to-right. :contentReference[oaicite:11]{index=11}
        // Want: translate world by -Position, rotate by -Rotation, scale by Zoom, then translate to screen center.
        auto t = Translation({ -Position.x, -Position.y });
        auto r = Rotation(-RotationRad);
        auto s = Scale(Zoom);
        auto c = Translation(center);

        return Multiply(Multiply(Multiply(t, r), s), c);
    }
}