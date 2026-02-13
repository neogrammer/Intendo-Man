#include "pch.h"
#include "Engine/ActionMap.h"

#include <algorithm>

namespace engine
{
    static void AddUnique(auto& vec, auto v)
    {
        if (std::find(vec.begin(), vec.end(), v) == vec.end())
            vec.push_back(v);
    }

    static void RemoveIfPresent(auto& vec, auto v)
    {
        vec.erase(std::remove(vec.begin(), vec.end(), v), vec.end());
    }

    void ActionMap::NormalizeIfNeeded(float2& v) noexcept
    {
        float mag = std::sqrt(v.x * v.x + v.y * v.y);
        if (mag > 1.0f)
        {
            v.x /= mag;
            v.y /= mag;
        }
    }

    void ActionMap::ClearAllBindings()
    {
        for (auto& b : m_bindings)
        {
            b.keys.clear();
            b.buttons.clear();
        }
    }

    void ActionMap::ClearBindings(Action a)
    {
        auto& b = m_bindings[Idx(a)];
        b.keys.clear();
        b.buttons.clear();
    }

    void ActionMap::Bind(Action a, winrt::Windows::System::VirtualKey key)
    {
        AddUnique(m_bindings[Idx(a)].keys, key);
    }

    void ActionMap::Bind(Action a, winrt::Windows::Gaming::Input::GamepadButtons button)
    {
        AddUnique(m_bindings[Idx(a)].buttons, button);
    }

    void ActionMap::Unbind(Action a, winrt::Windows::System::VirtualKey key)
    {
        RemoveIfPresent(m_bindings[Idx(a)].keys, key);
    }

    void ActionMap::Unbind(Action a, winrt::Windows::Gaming::Input::GamepadButtons button)
    {
        RemoveIfPresent(m_bindings[Idx(a)].buttons, button);
    }

    bool ActionMap::Down(Action a) const noexcept { return m_states[Idx(a)].down; }
    bool ActionMap::Pressed(Action a) const noexcept { return m_states[Idx(a)].pressed; }
    bool ActionMap::Released(Action a) const noexcept { return m_states[Idx(a)].released; }

    void ActionMap::SetDefaultBindings()
    {
        using VK = winrt::Windows::System::VirtualKey;
        using GB = winrt::Windows::Gaming::Input::GamepadButtons;

        ClearAllBindings();

        // Move (WASD + DPad)
        Bind(Action::MoveUp, VK::W);
        Bind(Action::MoveDown, VK::S);
        Bind(Action::MoveLeft, VK::A);
        Bind(Action::MoveRight, VK::D);

        Bind(Action::MoveUp, GB::DPadUp);
        Bind(Action::MoveDown, GB::DPadDown);
        Bind(Action::MoveLeft, GB::DPadLeft);
        Bind(Action::MoveRight, GB::DPadRight);

        // Camera pan (arrows; right stick handles analog)
        Bind(Action::CamUp, VK::Up);
        Bind(Action::CamDown, VK::Down);
        Bind(Action::CamLeft, VK::Left);
        Bind(Action::CamRight, VK::Right);

        // Zoom (Q/E) + triggers (RT-LT)
        Bind(Action::ZoomOut, VK::Q);
        Bind(Action::ZoomIn, VK::E);

        // Rotate camera (Z/C) + bumpers
        Bind(Action::RotCCW, VK::Z);
        Bind(Action::RotCW, VK::C);
        Bind(Action::RotCCW, GB::LeftShoulder);
        Bind(Action::RotCW, GB::RightShoulder);

        // Reset view (R / A)
        Bind(Action::ResetView, VK::R);
        Bind(Action::ResetView, GB::A);

        // Fire test sound (Space / X)
        Bind(Action::Fire, VK::Space);
        Bind(Action::Fire, GB::X);

        // Pause (Esc / Menu)
        Bind(Action::Pause, VK::Escape);
        Bind(Action::Pause, GB::Menu);
    }

    void ActionMap::Update(InputState const& input)
    {
        // Digital actions
        for (size_t i = 0; i < N; ++i)
        {
            bool down = false;

            for (auto const k : m_bindings[i].keys)
            {
                if (input.KeyDown(k)) { down = true; break; }
            }

            if (!down)
            {
                for (auto const b : m_bindings[i].buttons)
                {
                    if (input.ButtonDown(b)) { down = true; break; }
                }
            }

            auto& st = m_states[i];
            st.prev = st.down;
            st.down = down;
            st.pressed = (st.down && !st.prev);
            st.released = (!st.down && st.prev);
        }

        // Derived axes (digital + analog)
        float2 moveDigital
        {
            (Down(Action::MoveRight) ? 1.0f : 0.0f) - (Down(Action::MoveLeft) ? 1.0f : 0.0f),
            (Down(Action::MoveDown) ? 0.0f : 0.0f) - (Down(Action::MoveUp) ? 0.0f : 0.0f)
        };

        float2 panDigital
        {
            (Down(Action::CamRight) ? 1.0f : 0.0f) - (Down(Action::CamLeft) ? 1.0f : 0.0f),
            (Down(Action::CamDown) ? 1.0f : 0.0f) - (Down(Action::CamUp) ? 1.0f : 0.0f)
        };

        float2 moveAnalog = m_useMoveStick ? input.LeftStick() : float2{ 0,0 };
        float2 panAnalog = m_usePanStick ? input.RightStick() : float2{ 0,0 };

        m_move = { moveDigital.x + moveAnalog.x, moveDigital.y + moveAnalog.y };
        m_pan = { panDigital.x + panAnalog.x,   panDigital.y + panAnalog.y };

        NormalizeIfNeeded(m_move);
        NormalizeIfNeeded(m_pan);

        float zoom = (Down(Action::ZoomIn) ? 1.0f : 0.0f) - (Down(Action::ZoomOut) ? 1.0f : 0.0f);
        if (m_useTriggerZoom)
        {
            zoom += (input.RightTrigger() - input.LeftTrigger());
        }
        m_zoom = std::clamp(zoom, -1.0f, 1.0f);

        float rot = (Down(Action::RotCCW) ? 1.0f : 0.0f) - (Down(Action::RotCW) ? 1.0f : 0.0f);
        m_rotate = std::clamp(rot, -1.0f, 1.0f);
    }
}