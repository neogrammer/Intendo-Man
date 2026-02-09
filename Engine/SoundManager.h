#pragma once

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Audio.h>
#include <winrt/Windows.Media.Render.h>
#include <winrt/Windows.Storage.h>

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

namespace engine
{
    class SoundManager final
    {
    public:
        static SoundManager& Instance() noexcept;

        // Call once at startup (CreateResourcesAsync is fine).
        winrt::Windows::Foundation::IAsyncAction InitializeAsync(
            winrt::Windows::Media::Render::AudioRenderCategory category =
            winrt::Windows::Media::Render::AudioRenderCategory::GameEffects);

        void Shutdown();

        bool IsInitialized() const noexcept;

        void RegisterSfx(std::wstring key, winrt::Windows::Foundation::Uri uri);
        void RegisterMusic(std::wstring key, winrt::Windows::Foundation::Uri uri);

        // Fire-and-forget (creates a node, plays it, auto-cleans on FileCompleted).
        winrt::fire_and_forget PlaySfx(std::wstring const& key, float volume = 1.0f);

        // Music = one persistent node (optionally looped).
        winrt::Windows::Foundation::IAsyncAction PlayMusicAsync(
            std::wstring const& key, bool loop = true, float volume = 0.5f);

        void StopMusic();

        void SetMasterVolume(float v);
        void SetSfxVolume(float v);
        void SetMusicVolume(float v);

    private:
        SoundManager() = default;

        void ApplyBusGains();
        void RemoveSfxNode(winrt::Windows::Media::Audio::AudioFileInputNode const& node);

        float Clamp01(float v) const noexcept
        {
            return (v < 0.0f) ? 0.0f : (v > 1.0f) ? 1.0f : v;
        }

    private:
        // Graph + routing
        winrt::Windows::Media::Audio::AudioGraph m_graph{ nullptr };
        winrt::Windows::Media::Audio::AudioDeviceOutputNode m_output{ nullptr };
        winrt::Windows::Media::Audio::AudioSubmixNode m_sfxBus{ nullptr };
        winrt::Windows::Media::Audio::AudioSubmixNode m_musicBus{ nullptr };

        float m_masterVol{ 1.0f };
        float m_sfxVol{ 1.0f };
        float m_musicVol{ 1.0f };

        // Registered assets
        std::unordered_map<std::wstring, winrt::Windows::Foundation::Uri> m_sfxSources;
        std::unordered_map<std::wstring, winrt::Windows::Foundation::Uri> m_musicSources;

        // Lifetime: keep nodes alive while playing
        mutable std::mutex m_mutex{};
        std::vector<winrt::Windows::Media::Audio::AudioFileInputNode> m_activeSfx;
        winrt::Windows::Media::Audio::AudioFileInputNode m_musicNode{ nullptr };
    };
}