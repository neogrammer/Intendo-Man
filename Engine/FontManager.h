#pragma once

#include "Engine/Font.h"

#include <winrt/Windows.Foundation.h>

#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace engine
{
    class FontManager final
    {
    public:
        static FontManager& Instance() noexcept;

        // Register a system-installed font family (works on PC + Xbox, e.g. "Segoe UI")
        void RegisterSystem(std::wstring key, std::wstring familyName);

        // Register a packaged font file:
        //   uri = ms-appx:///Assets/Fonts/MyFont.ttf
        //   familyName = EXACT family name embedded in the font (the one Windows sees)
        void RegisterAppFont(std::wstring key,
            winrt::Windows::Foundation::Uri const& fontFileUri,
            std::wstring familyName);

        std::shared_ptr<Font> Get(std::wstring const& key) const;
        std::shared_ptr<Font> TryGet(std::wstring const& key) const noexcept;

        void Unload(std::wstring const& key);
        void Clear();

    private:
        FontManager() = default;

        mutable std::shared_mutex m_mutex{};
        std::unordered_map<std::wstring, std::shared_ptr<Font>> m_fonts;
    };
}