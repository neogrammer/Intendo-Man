#pragma once

namespace game::anim
{
    template <typename TState>
    class StateMachine
    {
    public:
        explicit constexpr StateMachine(TState initial) noexcept
            : m_state(initial) {
        }

        void Step(float dt) noexcept { m_time += dt; }

        void Set(TState s) noexcept
        {
            if (s != m_state)
            {
                m_state = s;
                m_time = 0.0f;
            }
        }

        TState Get() const noexcept { return m_state; }
        float Time() const noexcept { return m_time; }

    private:
        TState m_state;
        float  m_time{ 0.0f };
    };
}