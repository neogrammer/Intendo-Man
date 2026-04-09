#include "pch.h"

#include "Engine/Game.h"
#include "Engine/SpriteBatchScope.h"
#include "Engine/Matrix2D.h"
#include "Engine/SoundManager.h"

#include <chrono>
#include <algorithm>
#include "../Game/Resources/Cfg.h"
#include <iostream>


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
        try
        {
            OutputDebugStringW(L"[DBG] Game::CreateResourcesAsync enter\n");

            co_await Cfg::InitializeAsync(sender);

            OutputDebugStringW(L"[DBG] Cfg::InitializeAsync finished\n");

            gameMgr = std::make_unique<game::GameManager>();

            OutputDebugStringW(L"[DBG] GameManager created\n");

            m_time = 0.0f;

            co_return;
        }
        catch (winrt::hresult_error const& e)
        {
            std::wstring msg = L"[ERR] CreateResourcesAsync hresult_error: ";
            msg += e.message().c_str();
            msg += L"\n";
            OutputDebugStringW(msg.c_str());
            throw;
        }
        catch (std::exception const& e)
        {
            std::wstring msg = L"[ERR] CreateResourcesAsync std::exception: ";
            msg += winrt::to_hstring(e.what()).c_str();
            msg += L"\n";
            OutputDebugStringW(msg.c_str());
            throw;
        }
        catch (...)
        {
            OutputDebugStringW(L"[ERR] CreateResourcesAsync unknown exception\n");
            throw;
        }
    }
    void Game::Update(
        winrt::Microsoft::Graphics::Canvas::UI::Xaml::ICanvasAnimatedControl const& sender,
        winrt::Microsoft::Graphics::Canvas::UI::Xaml::CanvasAnimatedUpdateEventArgs const& args)
    {
        if (!gameMgr)
        {
            return;
        }

        auto s = sender.Size();
        auto cam = gameMgr->getCamera();
        constexpr float VIRTUAL_W = 960.0f;
        constexpr float VIRTUAL_H = 540.0f;

        cam->SetViewportSize({ VIRTUAL_W,VIRTUAL_H });    //static_cast<float>(s.Width), static_cast<float>(s.Height) });
        float dt = std::chrono::duration<float>(args.Timing().ElapsedTime).count();
        m_time += dt;
        m_input.Update();
        m_actions.Update(m_input);
        gameMgr->processInput(m_actions);
        gameMgr->update(dt);
    }

    //static void DrawGrid(winrt::Microsoft::Graphics::Canvas::CanvasDrawingSession const& ds)
    //{
    //    for (int i = -20; i <= 20; ++i)
    //    {
    //        float x = i * 100.0f;
    //        ds.DrawLine({ x, -2000.0f }, { x, 2000.0f }, Colors::DimGray());
    //        float y = i * 100.0f;
    //        ds.DrawLine({ -2000.0f, y }, { 2000.0f, y }, Colors::DimGray());
    //    }
    //    ds.DrawLine({ -3000.0f, 0.0f }, { 3000.0f, 0.0f }, Colors::DarkRed());
    //    ds.DrawLine({ 0.0f, -3000.0f }, { 0.0f, 3000.0f }, Colors::DarkGreen());
    //}

    void Game::Draw(
        winrt::Microsoft::Graphics::Canvas::UI::Xaml::ICanvasAnimatedControl const& sender,
        winrt::Microsoft::Graphics::Canvas::UI::Xaml::CanvasAnimatedDrawEventArgs const& args)
    {

        if (!gameMgr)
        {
            return;
        }

        auto cam = gameMgr->getCamera();
        auto ds = args.DrawingSession();
        ds.Clear(Colors::Black());
        {
            {
                constexpr float VIRTUAL_W = 960.0f;
                constexpr float VIRTUAL_H = 540.0f;

                auto s = sender.Size();
                float actualW = (float)s.Width;
                float actualH = (float)s.Height;
                


                float scale = std::min<float>(actualW / VIRTUAL_W, actualH / VIRTUAL_H);
                       

                engine::float2 offset;
                
               
                offset = { (actualW - VIRTUAL_W * scale) * 0.5f,(actualH - VIRTUAL_H * scale) * 0.5f };


                ds.Transform(engine::Multiply(
                    cam->WorldToScreen(),
                    engine::Multiply(engine::Scale(scale), engine::Translation(offset))
                ));
            }

            //ds.Transform(cam->WorldToScreen());
            //DrawGrid(ds);



            std::vector<engine::Text>* uiStrings{};
            {
                engine::SpriteBatchScope batch(ds);
                uiStrings = &gameMgr->render(batch);
            }

            

            // render this in state, and after it draws it returns the UI strings that need to be displayed
            // HUD strings to draw = renderer.Draw(m_player);
            // for each (HUD string).Draw(ds, sender);
            ds.Transform(engine::Identity2D());
            for (auto& str : *uiStrings)
            {
                str.Draw(ds, sender);
            }
            //ds.DrawText(L"Space/X: SFX | WASD/LS: Move | Arrows/RS: Pan | Q/E+Triggers: Zoom | Z/C+LB/RB: Rotate | R/A: Reset",
            //    200.0f, 10.0f, Colors::White());
        }
    }
}