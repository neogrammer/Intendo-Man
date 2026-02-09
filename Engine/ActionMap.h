#pragma once

#include "Engine/InputState.h"

#include <array>
#include <algorithm>
#include <vector>
#include <cstdint>
#include <cmath>

namespace engine
{
    enum class Action : uint32_t
    {
        MoveUp,
        MoveDown,
        MoveLeft,
        MoveRight,

        CamUp,
        CamDown,
        CamLeft,
        CamRight,

        ZoomIn,
        ZoomOut,

        RotCW,
        RotCCW,

        ResetView,
        Fire,
        Pause,

        COUNT
    };

    class ActionMap final
    {
    public:
        void SetDefaultBindings();
        void ClearAllBindings();
        void ClearBindings(Action a);

        void Bind(Action a, winrt::Windows::System::VirtualKey key);
        void Bind(Action a, winrt::Windows::Gaming::Input::GamepadButtons button);

        void Unbind(Action a, winrt::Windows::System::VirtualKey key);
        void Unbind(Action a, winrt::Windows::Gaming::Input::GamepadButtons button);

        // Call once per frame after InputState::Update()
        void Update(InputState const& input);

        bool Down(Action a) const noexcept;
        bool Pressed(Action a) const noexcept;
        bool Released(Action a) const noexcept;

        // Convenience axes derived from bound actions + optional analog sources
        float2 MoveAxis() const noexcept { return m_move; }     // WASD + left stick
        float2 PanAxis() const noexcept { return m_pan; }       // Arrows + right stick
        float  ZoomAxis() const noexcept { return m_zoom; }     // Q/E + triggers (RT-LT)
        float  RotateAxis() const noexcept { return m_rotate; } // Z/C + bumpers (digital)

        // Optional: turn off analog contributions if you want “digital only”
        void UseMoveStick(bool enabled) noexcept { m_useMoveStick = enabled; }
        void UsePanStick(bool enabled) noexcept { m_usePanStick = enabled; }
        void UseTriggerZoom(bool enabled) noexcept { m_useTriggerZoom = enabled; }

    private:
        using VirtualKey = winrt::Windows::System::VirtualKey;
        using GamepadButtons = winrt::Windows::Gaming::Input::GamepadButtons;

        struct Bindings
        {
            std::vector<VirtualKey> keys;
            std::vector<GamepadButtons> buttons;
        };

        struct State
        {
            bool down{};
            bool prev{};
            bool pressed{};
            bool released{};
        };

        static constexpr size_t N = static_cast<size_t>(Action::COUNT);

        std::array<Bindings, N> m_bindings{};
        std::array<State, N> m_states{};

        bool m_useMoveStick{ true };
        bool m_usePanStick{ true };
        bool m_useTriggerZoom{ true };

        float2 m_move{ 0,0 };
        float2 m_pan{ 0,0 };
        float  m_zoom{ 0 };
        float  m_rotate{ 0 };

    private:
        static void NormalizeIfNeeded(float2& v) noexcept;
        static size_t Idx(Action a) noexcept { return static_cast<size_t>(a); }
    };
}