#include "pch.h"
#include "TextureStore.h"

using namespace winrt;

namespace engine
{
    TextureStore& TextureStore::Instance() noexcept
    {
        static TextureStore s{};
        return s;
    }

    void TextureStore::SetResourceCreator(winrt::Microsoft::Graphics::Canvas::ICanvasResourceCreator const& creator)
    {
        std::unique_lock lock(m_mutex);
        m_creator = creator;
    }

    void TextureStore::Register(std::wstring key, winrt::Windows::Foundation::Uri uri)
    {
        std::unique_lock lock(m_mutex);
        m_sources.emplace(std::move(key), std::move(uri));
    }

    winrt::Windows::Foundation::IAsyncAction TextureStore::RegisterAndLoadAsync(std::wstring key, winrt::Windows::Foundation::Uri uri)
    {
        Register(key, uri);
        co_await LoadAsync(key);
    }

    winrt::Windows::Foundation::IAsyncAction TextureStore::LoadAsync(std::wstring const& key)
    {
        winrt::Microsoft::Graphics::Canvas::ICanvasResourceCreator creator{ nullptr };
        winrt::Windows::Foundation::Uri uri{ nullptr };

        {
            std::shared_lock lock(m_mutex);

            if (m_textures.find(key) != m_textures.end())
            {
                co_return; // already loaded
            }

            auto it = m_sources.find(key);
            if (it == m_sources.end())
            {
                throw winrt::hresult_invalid_argument(
                    winrt::hstring(L"TextureStore::LoadAsync: key not registered: ") + winrt::hstring(key));
            }

            creator = m_creator;
            uri = it->second;
        }

        if (!creator)
        {
            throw winrt::hresult_error(E_FAIL, L"TextureStore::LoadAsync: ResourceCreator not set. Call SetResourceCreator from CreateResources.");
        }

        // Async load (Win2D CanvasBitmap.LoadAsync) :contentReference[oaicite:6]{index=6}
        auto bitmap = co_await winrt::Microsoft::Graphics::Canvas::CanvasBitmap::LoadAsync(creator, uri);

        auto tex = std::make_shared<Texture>();
        tex->Bitmap = bitmap;
        tex->SizeDips = bitmap.Size();           // :contentReference[oaicite:7]{index=7}
        tex->SizePixels = bitmap.SizeInPixels(); // :contentReference[oaicite:8]{index=8}

        {
            std::unique_lock lock(m_mutex);
            m_textures[key] = std::move(tex);
        }
    }

    std::shared_ptr<Texture> TextureStore::TryGet(std::wstring const& key) const noexcept
    {
        std::shared_lock lock(m_mutex);
        auto it = m_textures.find(key);
        return (it != m_textures.end()) ? it->second : nullptr;
    }

    std::shared_ptr<Texture> TextureStore::Get(std::wstring const& key) const
    {
        if (auto t = TryGet(key))
        {
            return t;
        }

        throw winrt::hresult_invalid_argument(
            winrt::hstring(L"TextureStore::Get: texture not loaded: ") + winrt::hstring(key));
    }

    void TextureStore::Unload(std::wstring const& key)
    {
        std::unique_lock lock(m_mutex);
        m_textures.erase(key);
    }

    void TextureStore::Clear()
    {
        std::unique_lock lock(m_mutex);
        m_textures.clear();
    }
}