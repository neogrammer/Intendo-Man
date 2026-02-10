#include "pch.h"

#include "Engine/Game.h"
#include "Engine/TextureStore.h"
#include "Engine/Renderer2D.h"
#include "Engine/Matrix2D.h"
#include "Engine/SoundManager.h"

#include <winrt/Windows.UI.h>
#include <chrono>
#include <algorithm>
#include "../Game/Resources/Cfg.h"
#include <iostream>

#include <windows.h>
#include <string>

namespace engine
{
    using winrt::Windows::Foundation::Uri;
    using winrt::Windows::UI::Colors;
    void Game::InitializeInput(winrt::Windows::UI::Core::CoreWindow const& window)
    {
        m_input.Initialize(window);
        m_actions.SetDefaultBindings();
    }

    winrt::Windows::Foundation::IAsyncAction Game::CreateResourcesAsync(
        winrt::Microsoft::Graphics::Canvas::UI::Xaml::CanvasAnimatedControl const& sender)
    {


        co_await Cfg::InitializeAsync(sender);
        
        

        gameMgr = std::make_unique<game::GameManager>();


        m_time = 0.0f;
  
  
    }

    void Game::Update(
        winrt::Microsoft::Graphics::Canvas::UI::Xaml::ICanvasAnimatedControl const& sender,
        winrt::Microsoft::Graphics::Canvas::UI::Xaml::CanvasAnimatedUpdateEventArgs const& args)
    {


        auto s = sender.Size();
        auto cam = gameMgr->getCamera();
        cam->SetViewportSize({ static_cast<float>(s.Width), static_cast<float>(s.Height) });
        float dt = std::chrono::duration<float>(args.Timing().ElapsedTime).count();
        m_time += dt;
        m_input.Update();
        m_actions.Update(m_input);
        gameMgr->processInput(m_actions);
        gameMgr->update(dt);
    }

    static void DrawGrid(winrt::Microsoft::Graphics::Canvas::CanvasDrawingSession const& ds)
    {
        for (int i = -20; i <= 20; ++i)
        {
            float x = i * 100.0f;
            ds.DrawLine({ x, -2000.0f }, { x, 2000.0f }, Colors::DimGray());
            float y = i * 100.0f;
            ds.DrawLine({ -2000.0f, y }, { 2000.0f, y }, Colors::DimGray());
        }
        ds.DrawLine({ -3000.0f, 0.0f }, { 3000.0f, 0.0f }, Colors::DarkRed());
        ds.DrawLine({ 0.0f, -3000.0f }, { 0.0f, 3000.0f }, Colors::DarkGreen());
    }

    void Game::Draw(
        winrt::Microsoft::Graphics::Canvas::UI::Xaml::ICanvasAnimatedControl const& sender,
        winrt::Microsoft::Graphics::Canvas::UI::Xaml::CanvasAnimatedDrawEventArgs const& args)
    {
        auto cam = gameMgr->getCamera();
        auto ds = args.DrawingSession();
        ds.Clear(Colors::Black());
        {
            ds.Transform(cam->WorldToScreen());
            DrawGrid(ds);

            std::vector<engine::Text> uiStrings{};
            {
                engine::Renderer2D renderer(ds);
                // pass renderer down to the state
                uiStrings = gameMgr->render(renderer);
            }

            engine::Renderer2D renderer(ds);

            // render this in state, and after it draws it returns the UI strings that need to be displayed
            // HUD strings to draw = renderer.Draw(m_player);
            // for each (HUD string).Draw(ds, sender);
            ds.Transform(engine::Identity2D());
            for (auto& str : uiStrings)
            {
                str.Draw(ds, sender);
            }
            ds.DrawText(L"Space/X: SFX | WASD/LS: Move | Arrows/RS: Pan | Q/E+Triggers: Zoom | Z/C+LB/RB: Rotate | R/A: Reset",
                200.0f, 10.0f, Colors::White());
        }
    }
}