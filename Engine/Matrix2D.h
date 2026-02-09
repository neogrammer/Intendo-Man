#pragma once

#include <winrt/Windows.Foundation.Numerics.h>
#include <cmath>

namespace engine
{
    using winrt::Windows::Foundation::Numerics::float2;
    using winrt::Windows::Foundation::Numerics::float3x2;

    inline float3x2 Identity2D() noexcept
    {
        return { 1,0,0,1,0,0 };
    }

    inline float3x2 Translation(float2 t) noexcept
    {
        return { 1,0,0,1, t.x, t.y };
    }

    inline float3x2 Scale(float s) noexcept
    {
        return { s,0,0,s, 0,0 };
    }

    inline float3x2 Scale(float2 s) noexcept
    {
        return { s.x,0,0,s.y, 0,0 };
    }

    // Row-vector layout rotation (counterclockwise for positive radians).
    inline float3x2 Rotation(float radians) noexcept
    {
        float c = std::cos(radians);
        float s = std::sin(radians);

        // x' = x*c + y*(-s)
        // y' = x*s + y*( c )
        return { c, s, -s, c, 0, 0 };
    }

    // Standard affine multiply for row-vector layout:
    // Treat each as a 3x3 with last column [0,0,1], then compute A*B.
    inline float3x2 Multiply(float3x2 const& a, float3x2 const& b) noexcept
    {
        return
        {
            a.m11 * b.m11 + a.m12 * b.m21,
            a.m11 * b.m12 + a.m12 * b.m22,

            a.m21 * b.m11 + a.m22 * b.m21,
            a.m21 * b.m12 + a.m22 * b.m22,

            a.m31 * b.m11 + a.m32 * b.m21 + b.m31,
            a.m31 * b.m12 + a.m32 * b.m22 + b.m32
        };
    }

    inline float3x2 Invert(float3x2 const& m) noexcept
    {
        float det = m.m11 * m.m22 - m.m12 * m.m21;
        if (det == 0.0f)
        {
            return Identity2D();
        }

        float invDet = 1.0f / det;

        float3x2 inv
        {
            m.m22 * invDet,
            -m.m12 * invDet,

            -m.m21 * invDet,
            m.m11 * invDet,

            0, 0
        };

        // translation inverse = -t * invLinear
        float tx = m.m31 * inv.m11 + m.m32 * inv.m21;
        float ty = m.m31 * inv.m12 + m.m32 * inv.m22;

        inv.m31 = -tx;
        inv.m32 = -ty;

        return inv;
    }
}