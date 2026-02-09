#pragma once
#include "Texture.h"
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace engine
{
    class TextureStore final
    {
    public:
        static TextureStore& Instance() noexcept;

        // Call from Win2D CreateResources (the resource creator owns the device).
        void SetResourceCreator(winrt::Microsoft::Graphics::Canvas::ICanvasResourceCreator const& creator);

        // Logical name -> URI (ms-appx:///..., ms-appdata:///..., etc)
        void Register(std::wstring key, winrt::Windows::Foundation::Uri uri);

        // Load a registered texture (no-op if already loaded).
        winrt::Windows::Foundation::IAsyncAction LoadAsync(std::wstring const& key);

        // Convenience: Register + Load
        winrt::Windows::Foundation::IAsyncAction RegisterAndLoadAsync(std::wstring key, winrt::Windows::Foundation::Uri uri);

        std::shared_ptr<Texture> Get(std::wstring const& key) const;
        std::shared_ptr<Texture> TryGet(std::wstring const& key) const noexcept;

        void Unload(std::wstring const& key);
        void Clear();

    private:
        TextureStore() = default;

        winrt::Microsoft::Graphics::Canvas::ICanvasResourceCreator m_creator{ nullptr };

        mutable std::shared_mutex m_mutex{};
        std::unordered_map<std::wstring, winrt::Windows::Foundation::Uri> m_sources;
        std::unordered_map<std::wstring, std::shared_ptr<Texture>> m_textures;
    };
}