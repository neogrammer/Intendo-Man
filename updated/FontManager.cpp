#include "pch.h"
#include "Engine/FontManager.h"

using namespace winrt;

namespace engine
{
    FontManager& FontManager::Instance() noexcept
    {
        static FontManager s{};
        return s;
    }

    void FontManager::RegisterSystem(std::wstring key, std::wstring familyName)
    {
        auto f = std::make_shared<Font>();
        f->FamilySpec = winrt::hstring(familyName);

        std::unique_lock lock(m_mutex);
        m_fonts[std::move(key)] = std::move(f);
    }

    void FontManager::RegisterAppFont(std::wstring key,
        winrt::Windows::Foundation::Uri const& fontFileUri,
        std::wstring familyName)
    {
        auto f = std::make_shared<Font>();

        // Validate/load the font file set (optional but useful).
        f->FontSet = winrt::Microsoft::Graphics::Canvas::Text::CanvasFontSet(fontFileUri);

        // Construct "uri#family" spec used by CanvasTextFormat.FontFamily.
        f->FamilySpec = fontFileUri.AbsoluteUri() + winrt::hstring(L"#") + winrt::hstring(familyName);

        std::unique_lock lock(m_mutex);
        m_fonts[std::move(key)] = std::move(f);
    }

    std::shared_ptr<Font> FontManager::TryGet(std::wstring const& key) const noexcept
    {
        std::shared_lock lock(m_mutex);
        auto it = m_fonts.find(key);
        return (it != m_fonts.end()) ? it->second : nullptr;
    }

    std::shared_ptr<Font> FontManager::Get(std::wstring const& key) const
    {
        if (auto f = TryGet(key))
            return f;

        throw winrt::hresult_invalid_argument(
            winrt::hstring(L"FontManager::Get: missing font key: ") + winrt::hstring(key));
    }

    void FontManager::Unload(std::wstring const& key)
    {
        std::unique_lock lock(m_mutex);
        m_fonts.erase(key);
    }

    void FontManager::Clear()
    {
        std::unique_lock lock(m_mutex);
        m_fonts.clear();
    }
}