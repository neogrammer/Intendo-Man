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

        m_player = Sprite(Cfg::GetTex(Cfg::Textures::Ship));
        m_player.Position = { 0.0f, 0.0f };
        m_player.SetOriginCenter();

        m_camera.Reset();
        m_cameraOffset = { 0,0 };
        m_time = 0.0f;

        Cfg::PlayMusicAsync(L"theme", true, 0.25f);

        if (m_state.isType(L"PlayState"))
        {
            Cfg::debugPrint(L"Winner");
            //std::wcout << L"Winner!" << std::endl;
        }
        else
        {
            Cfg::debugPrint(L"No Dice!");
        }

    }

    void Game::Update(
        winrt::Microsoft::Graphics::Canvas::UI::Xaml::ICanvasAnimatedControl const& sender,
        winrt::Microsoft::Graphics::Canvas::UI::Xaml::CanvasAnimatedUpdateEventArgs const& args)
    {
        auto s = sender.Size();
        m_camera.SetViewportSize({ static_cast<float>(s.Width), static_cast<float>(s.Height) });

        float dt = std::chrono::duration<float>(args.Timing().ElapsedTime).count();
        m_time += dt;

        m_input.Update();
        m_actions.Update(m_input);

        if (m_actions.Pressed(Action::ResetView))
        {
            m_camera.Reset();
            m_cameraOffset = { 0,0 };
        }

        if (m_actions.Pressed(Action::Fire))
        {
            Cfg::PlaySfx(L"blip");
        }

        // Player movement (MoveAxis = WASD + left stick)
        float2 move = m_actions.MoveAxis();
        float playerSpeed = 300.0f;
        m_player.Position.x += move.x * playerSpeed * dt;
        m_player.Position.y += move.y * playerSpeed * dt;

        // Camera pan offset (PanAxis = arrows + right stick)
        float2 pan = m_actions.PanAxis();
        float camPanSpeed = 450.0f;
        m_cameraOffset.x += pan.x * camPanSpeed * dt;
        m_cameraOffset.y += pan.y * camPanSpeed * dt;

        // Zoom (Q/E + triggers)
        float zoomAxis = m_actions.ZoomAxis();
        float zoomRate = 1.25f;
        m_camera.Zoom *= (1.0f + zoomAxis * zoomRate * dt);
        m_camera.Zoom = std::clamp(m_camera.Zoom, 0.25f, 4.0f);

        // Rotate (Z/C + bumpers)
        float rotAxis = m_actions.RotateAxis();
        float rotSpeed = 1.5f;
        m_camera.RotationRad += rotAxis * rotSpeed * dt;

        // Follow player with user pan offset
        m_camera.Position = { m_player.Position.x + m_cameraOffset.x, m_player.Position.y + m_cameraOffset.y };
        m_player.Rotation = m_time * 0.5f;
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
        auto ds = args.DrawingSession();
        ds.Clear(Colors::Black());

        {
            ds.Transform(m_camera.WorldToScreen());

            DrawGrid(ds);

            Renderer2D renderer(ds);
            renderer.Draw(m_player);
        }

        engine::Text m_hud{};

        m_hud.FontRef = Cfg::GetFont(L"bubbly");
        m_hud.String = L"Space/X: SFX";
        m_hud.FontSize = 22.0f;
        m_hud.OutlineThickness = 2;
        m_hud.OutlineColor = winrt::Windows::UI::Colors::White();
        m_hud.Color = winrt::Windows::UI::Colors::Green();
        m_hud.Position = { 10.0f, 10.0f };
        m_hud.Invalidate();

        ds.Transform(engine::Identity2D());
        m_hud.Draw(ds, sender);

        ds.Transform(engine::Identity2D());
        ds.DrawText(L"Space/X: SFX | WASD/LS: Move | Arrows/RS: Pan | Q/E+Triggers: Zoom | Z/C+LB/RB: Rotate | R/A: Reset",
            200.0f, 10.0f, Colors::White());


    }
}