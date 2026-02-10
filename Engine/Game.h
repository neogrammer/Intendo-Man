#pragma once

#include "Engine/InputState.h"
#include "Engine/ActionMap.h"
#include "Engine/Camera2D.h"
#include "Engine/Sprite.h"
#include "Engine/Text.h"


#include <winrt/Microsoft.Graphics.Canvas.UI.Xaml.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.Foundation.h>

#include "../Game/GameStates/PlayState.h"
#include "../Game/Mgmt/GameManager.h"

#include <memory>

namespace engine
{
    class Game final
    {
    public:
        void InitializeInput(winrt::Windows::UI::Core::CoreWindow const& window);

        winrt::Windows::Foundation::IAsyncAction CreateResourcesAsync(
            winrt::Microsoft::Graphics::Canvas::UI::Xaml::CanvasAnimatedControl const& sender);

        void Update(
            winrt::Microsoft::Graphics::Canvas::UI::Xaml::ICanvasAnimatedControl const& sender,
            winrt::Microsoft::Graphics::Canvas::UI::Xaml::CanvasAnimatedUpdateEventArgs const& args);

        void Draw(
            winrt::Microsoft::Graphics::Canvas::UI::Xaml::ICanvasAnimatedControl const& sender,
            winrt::Microsoft::Graphics::Canvas::UI::Xaml::CanvasAnimatedDrawEventArgs const& args);

    private:
        InputState m_input{};
        ActionMap  m_actions{};

        std::unique_ptr<game::GameManager> gameMgr{ nullptr };

        float m_time = 0.0f;
    };
}