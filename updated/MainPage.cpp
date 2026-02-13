#include "pch.h"
#include "MainPage.h"
#if __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#endif

using namespace winrt::Microsoft::Graphics::Canvas::UI;
using namespace winrt;
using namespace Microsoft::Graphics::Canvas::UI::Xaml;

namespace winrt::Intendo_Man::implementation 
{
        MainPage::MainPage()
        {
            InitializeComponent();


            // Hook keyboard + gamepad from the app window.
            m_game.InitializeInput(winrt::Windows::UI::Xaml::Window::Current().CoreWindow());

        }

        void MainPage::Canvas_CreateResources(CanvasAnimatedControl const& sender, CanvasCreateResourcesEventArgs const& args)
        {
            // Track async loads so Update/Draw won't start early. :contentReference[oaicite:3]{index=3}
            args.TrackAsyncAction(m_game.CreateResourcesAsync(sender));
        }

        void MainPage::Canvas_Update(ICanvasAnimatedControl const& sender, CanvasAnimatedUpdateEventArgs const& args)
        {
            m_game.Update(sender, args);
        }

        void MainPage::Canvas_Draw(ICanvasAnimatedControl const& sender, CanvasAnimatedDrawEventArgs const& args)
        {
            m_game.Draw(sender, args);
        }   
}