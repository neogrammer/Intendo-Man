#include "pch.h"
#include "Engine/InputState.h"

#include <algorithm>
#include <cmath>

using namespace winrt;

namespace engine
{
    void InputState::Initialize(winrt::Windows::UI::Core::CoreWindow const& window)
    {
        // Keyboard events come from CoreWindow when the app window has focus. :contentReference[oaicite:6]{index=6}
        m_keyDownRevoker = window.KeyDown(winrt::auto_revoke,
            [this](auto const&, winrt::Windows::UI::Core::KeyEventArgs const& args)
            {
                auto vk = static_cast<uint32_t>(args.VirtualKey());
                if (!IsValidKeyIndex(vk)) return;

                std::lock_guard lock(m_keyboardMutex);
                m_keyDownLive.set(vk, true);
            });

        m_keyUpRevoker = window.KeyUp(winrt::auto_revoke,
            [this](auto const&, winrt::Windows::UI::Core::KeyEventArgs const& args)
            {
                auto vk = static_cast<uint32_t>(args.VirtualKey());
                if (!IsValidKeyIndex(vk)) return;

                std::lock_guard lock(m_keyboardMutex);
                m_keyDownLive.set(vk, false);
            });

        // Gamepad: instances come from Gamepad::Gamepads or GamepadAdded. :contentReference[oaicite:7]{index=7}
        m_gamepadAddedRevoker = winrt::Windows::Gaming::Input::Gamepad::GamepadAdded(winrt::auto_revoke,
            [this](winrt::Windows::Foundation::IInspectable const&, winrt::Windows::Gaming::Input::Gamepad const& gp)
            {
                std::lock_guard lock(m_gamepadMutex);
                if (!m_gamepad) m_gamepad = gp;
            });

        m_gamepadRemovedRevoker = winrt::Windows::Gaming::Input::Gamepad::GamepadRemoved(winrt::auto_revoke,
            [this](winrt::Windows::Foundation::IInspectable const&, winrt::Windows::Gaming::Input::Gamepad const& gp)
            {
                std::lock_guard lock(m_gamepadMutex);
                if (m_gamepad && gp == m_gamepad)
                {
                    m_gamepad = nullptr;
                }
            });

        SelectDefaultGamepadIfNeeded();
    }

    void InputState::Update()
    {
        // Snapshot keyboard
        {
            std::lock_guard lock(m_keyboardMutex);
            m_keyDown = m_keyDownLive;
        }

        // Edge detection
        m_keyPressed = (m_keyDown & ~m_keyDownPrev);
        m_keyReleased = (~m_keyDown & m_keyDownPrev);
        m_keyDownPrev = m_keyDown;

        // Poll gamepad
        UpdateGamepad();
    }

    bool InputState::KeyDown(winrt::Windows::System::VirtualKey key) const noexcept
    {
        auto vk = static_cast<uint32_t>(key);
        if (!IsValidKeyIndex(vk)) return false;
        return m_keyDown.test(vk);
    }

    bool InputState::KeyPressed(winrt::Windows::System::VirtualKey key) const noexcept
    {
        auto vk = static_cast<uint32_t>(key);
        if (!IsValidKeyIndex(vk)) return false;
        return m_keyPressed.test(vk);
    }

    bool InputState::KeyReleased(winrt::Windows::System::VirtualKey key) const noexcept
    {
        auto vk = static_cast<uint32_t>(key);
        if (!IsValidKeyIndex(vk)) return false;
        return m_keyReleased.test(vk);
    }

    bool InputState::GamepadConnected() const noexcept
    {
        std::lock_guard lock(m_gamepadMutex);
        return m_gamepad != nullptr;
    }

    bool InputState::ButtonDown(winrt::Windows::Gaming::Input::GamepadButtons b) const noexcept
    {
        return (m_buttonsDown & b) == b;
    }

    bool InputState::ButtonPressed(winrt::Windows::Gaming::Input::GamepadButtons b) const noexcept
    {
        return (m_buttonsPressed & b) == b;
    }

    bool InputState::ButtonReleased(winrt::Windows::Gaming::Input::GamepadButtons b) const noexcept
    {
        return (m_buttonsReleased & b) == b;
    }

    float2 InputState::ApplyRadialDeadzone(float2 v, float deadzone) noexcept
    {
        float mag = std::sqrt(v.x * v.x + v.y * v.y);
        if (mag <= deadzone) return { 0,0 };

        float legal = (mag - deadzone) / (1.0f - deadzone);
        legal = std::clamp(legal, 0.0f, 1.0f);

        float invMag = 1.0f / mag;
        return { v.x * invMag * legal, v.y * invMag * legal };
    }

    void InputState::SelectDefaultGamepadIfNeeded()
    {
        std::lock_guard lock(m_gamepadMutex);
        if (m_gamepad) return;

        auto pads = winrt::Windows::Gaming::Input::Gamepad::Gamepads();
        if (pads.Size() > 0)
        {
            m_gamepad = pads.GetAt(0);
        }
    }

    void InputState::UpdateGamepad()
    {
        SelectDefaultGamepadIfNeeded();

        winrt::Windows::Gaming::Input::Gamepad gp{ nullptr };
        {
            std::lock_guard lock(m_gamepadMutex);
            gp = m_gamepad;
        }

        if (!gp)
        {
            m_buttonsPrev = m_buttonsDown = m_buttonsPressed = m_buttonsReleased = {};
            m_leftStick = m_rightStick = { 0,0 };
            m_leftTrigger = m_rightTrigger = 0.0f;
            return;
        }

        // GetCurrentReading returns a snapshot of state at the moment you call it. :contentReference[oaicite:8]{index=8}
        auto r = gp.GetCurrentReading();

        m_buttonsDown = r.Buttons;
        m_buttonsPressed = (m_buttonsDown & ~m_buttonsPrev);
        m_buttonsReleased = (~m_buttonsDown & m_buttonsPrev);
        m_buttonsPrev = m_buttonsDown;

        // Map Y to screen coordinates (Win2D screen Y grows downward):
        // - Stick up should move/aim upward => negative Y.
        float2 ls{ static_cast<float>(r.LeftThumbstickX),  static_cast<float>(-r.LeftThumbstickY) };
        float2 rs{ static_cast<float>(r.RightThumbstickX), static_cast<float>(-r.RightThumbstickY) };

        m_leftStick = ApplyRadialDeadzone(ls, 0.20f);
        m_rightStick = ApplyRadialDeadzone(rs, 0.20f);

        m_leftTrigger = static_cast<float>(r.LeftTrigger);
        m_rightTrigger = static_cast<float>(r.RightTrigger);
    }

    static float Sign(bool positive) noexcept { return positive ? 1.0f : 0.0f; }

    float2 InputState::MoveAxis() const noexcept
    {
        using winrt::Windows::System::VirtualKey;

        float x = 0.0f;
        float y = 0.0f;

        x += KeyDown(VirtualKey::D) ? 1.0f : 0.0f;
        x -= KeyDown(VirtualKey::A) ? 1.0f : 0.0f;
        y += KeyDown(VirtualKey::S) ? 1.0f : 0.0f;
        y -= KeyDown(VirtualKey::W) ? 1.0f : 0.0f;

        float2 v{ x, y };
        v.x += m_leftStick.x;
        v.y += m_leftStick.y;

        float mag = std::sqrt(v.x * v.x + v.y * v.y);
        if (mag > 1.0f)
        {
            v.x /= mag;
            v.y /= mag;
        }

        return v;
    }

    float2 InputState::CameraPanAxis() const noexcept
    {
        using winrt::Windows::System::VirtualKey;

        float x = 0.0f;
        float y = 0.0f;

        x += KeyDown(VirtualKey::Right) ? 1.0f : 0.0f;
        x -= KeyDown(VirtualKey::Left) ? 1.0f : 0.0f;
        y += KeyDown(VirtualKey::Down) ? 1.0f : 0.0f;
        y -= KeyDown(VirtualKey::Up) ? 1.0f : 0.0f;

        float2 v{ x, y };
        v.x += m_rightStick.x;
        v.y += m_rightStick.y;

        float mag = std::sqrt(v.x * v.x + v.y * v.y);
        if (mag > 1.0f)
        {
            v.x /= mag;
            v.y /= mag;
        }

        return v;
    }

    float InputState::ZoomAxis() const noexcept
    {
        using winrt::Windows::System::VirtualKey;

        float z = 0.0f;
        z += KeyDown(VirtualKey::E) ? 1.0f : 0.0f; // zoom in
        z -= KeyDown(VirtualKey::Q) ? 1.0f : 0.0f; // zoom out

        // Triggers: RT zoom in, LT zoom out
        z += (m_rightTrigger - m_leftTrigger);

        return std::clamp(z, -1.0f, 1.0f);
    }

    float InputState::RotateAxis() const noexcept
    {
        using winrt::Windows::System::VirtualKey;
        using winrt::Windows::Gaming::Input::GamepadButtons;

        float r = 0.0f;
        r += KeyDown(VirtualKey::C) ? 1.0f : 0.0f;
        r -= KeyDown(VirtualKey::Z) ? 1.0f : 0.0f;

        // RB rotates CW, LB rotates CCW
        r += ButtonDown(GamepadButtons::RightShoulder) ? 1.0f : 0.0f;
        r -= ButtonDown(GamepadButtons::LeftShoulder) ? 1.0f : 0.0f;

        return std::clamp(r, -1.0f, 1.0f);
    }

    bool InputState::ResetViewPressed() const noexcept
    {
        using winrt::Windows::System::VirtualKey;
        using winrt::Windows::Gaming::Input::GamepadButtons;

        return KeyPressed(VirtualKey::R) || ButtonPressed(GamepadButtons::A);
    }
}