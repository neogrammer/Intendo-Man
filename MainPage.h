#pragma once

#include "MainPage.g.h"
#include "Engine/Game.h"

#include <winrt/Microsoft.Graphics.Canvas.UI.Xaml.h>
#include <winrt/Microsoft.Graphics.Canvas.UI.h>

namespace winrt::Intendo_Man::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
        MainPage();

        void Canvas_CreateResources(
            winrt::Microsoft::Graphics::Canvas::UI::Xaml::CanvasAnimatedControl const& sender,
            winrt::Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesEventArgs const& args);

        void Canvas_Update(
            winrt::Microsoft::Graphics::Canvas::UI::Xaml::ICanvasAnimatedControl const& sender,
            winrt::Microsoft::Graphics::Canvas::UI::Xaml::CanvasAnimatedUpdateEventArgs const& args);

        void Canvas_Draw(
            winrt::Microsoft::Graphics::Canvas::UI::Xaml::ICanvasAnimatedControl const& sender,
            winrt::Microsoft::Graphics::Canvas::UI::Xaml::CanvasAnimatedDrawEventArgs const& args);

    private:
        engine::Game m_game{};
    };
}

namespace winrt::Intendo_Man::factory_implementation
{
    struct MainPage : MainPageT<MainPage, winrt::Intendo_Man::implementation::MainPage> {};
}