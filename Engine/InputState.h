#pragma once

#include "Engine/Math.h"

#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Gaming.Input.h>

#include <bitset>
#include <mutex>
#include <optional>

namespace engine
{
    class InputState final
    {
    public:
        // Call once from UI thread (MainPage ctor is fine).
        void Initialize(winrt::Windows::UI::Core::CoreWindow const& window);

        // Call once per frame from Game::Update (game loop thread).
        void Update();

        // ---- Keyboard queries (per-frame edge detection included) ----
        bool KeyDown(winrt::Windows::System::VirtualKey key) const noexcept;
        bool KeyPressed(winrt::Windows::System::VirtualKey key) const noexcept;
        bool KeyReleased(winrt::Windows::System::VirtualKey key) const noexcept;

        // ---- Gamepad queries ----
        bool GamepadConnected() const noexcept;

        bool ButtonDown(winrt::Windows::Gaming::Input::GamepadButtons b) const noexcept;
        bool ButtonPressed(winrt::Windows::Gaming::Input::GamepadButtons b) const noexcept;
        bool ButtonReleased(winrt::Windows::Gaming::Input::GamepadButtons b) const noexcept;

        // Normalized [-1,1] axes, with deadzone and Y mapped to screen coordinates (up is -Y).
        float2 LeftStick() const noexcept { return m_leftStick; }
        float2 RightStick() const noexcept { return m_rightStick; }
        float  LeftTrigger() const noexcept { return m_leftTrigger; }   // [0,1]
        float  RightTrigger() const noexcept { return m_rightTrigger; } // [0,1]

        // Convenience “game-like” controls (keyboard OR gamepad)
        float2 MoveAxis() const noexcept;      // WASD + left stick
        float2 CameraPanAxis() const noexcept; // Arrows + right stick
        float  ZoomAxis() const noexcept;      // Q/E + triggers (positive zoom in)
        float  RotateAxis() const noexcept;    // Z/C + bumpers (positive = rotate CW)
        bool   ResetViewPressed() const noexcept; // R or A

    private:
        // ---- keyboard state ----
        mutable std::mutex m_keyboardMutex{};
        std::bitset<256> m_keyDownLive{};       // written by UI thread events
        std::bitset<256> m_keyDown{};           // snapshot for this frame (game thread)
        std::bitset<256> m_keyDownPrev{};
        std::bitset<256> m_keyPressed{};
        std::bitset<256> m_keyReleased{};

        winrt::Windows::UI::Core::CoreWindow::KeyDown_revoker m_keyDownRevoker{};
        winrt::Windows::UI::Core::CoreWindow::KeyUp_revoker   m_keyUpRevoker{};

        // ---- gamepad state ----
        mutable std::mutex m_gamepadMutex{};
        winrt::Windows::Gaming::Input::Gamepad m_gamepad{ nullptr };

        winrt::Windows::Gaming::Input::GamepadButtons m_buttonsDown{};
        winrt::Windows::Gaming::Input::GamepadButtons m_buttonsPrev{};
        winrt::Windows::Gaming::Input::GamepadButtons m_buttonsPressed{};
        winrt::Windows::Gaming::Input::GamepadButtons m_buttonsReleased{};

        float2 m_leftStick{ 0,0 };
        float2 m_rightStick{ 0,0 };
        float  m_leftTrigger{ 0.0f };
        float  m_rightTrigger{ 0.0f };

        winrt::Windows::Gaming::Input::Gamepad::GamepadAdded_revoker   m_gamepadAddedRevoker{};
        winrt::Windows::Gaming::Input::Gamepad::GamepadRemoved_revoker m_gamepadRemovedRevoker{};

    private:
        static bool IsValidKeyIndex(uint32_t vk) noexcept { return vk < 256; }
        static float2 ApplyRadialDeadzone(float2 v, float deadzone) noexcept;

        void SelectDefaultGamepadIfNeeded();
        void UpdateGamepad();
    };
}