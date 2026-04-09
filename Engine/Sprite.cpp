#include "pch.h"
#include "Engine/Sprite.h"

using namespace winrt;

namespace engine
{
    ICanvasResourceCreator Sprite::s_creator{ nullptr };
    std::shared_mutex Sprite::s_mutex{};
    std::unordered_map<std::wstring, Uri> Sprite::s_sources{};
    std::unordered_map<std::wstring, std::shared_ptr<Sprite::SharedTextureData>> Sprite::s_cache{};

    Sprite::Sprite(std::wstring const& key)
        : m_key(key)
    {
        std::shared_lock lock(s_mutex);

        auto it = s_cache.find(key);
        if (it != s_cache.end())
        {
            m_texture = it->second;
        }
    }

    void Sprite::SetResourceCreator(ICanvasResourceCreator const& creator)
    {
        std::unique_lock lock(s_mutex);
        s_creator = creator;
    }

    void Sprite::Register(std::wstring key, Uri uri)
    {
        std::unique_lock lock(s_mutex);
        s_sources.insert_or_assign(std::move(key), std::move(uri));
    }

    IAsyncAction Sprite::RegisterAndLoadAsync(std::wstring key, Uri uri)
    {
        Register(key, std::move(uri));
        co_await LoadSharedAsync(key);
    }

    IAsyncAction Sprite::LoadSharedAsync(std::wstring const& key)
    {
        ICanvasResourceCreator creator{ nullptr };
        Uri uri{ nullptr };

        {
            std::shared_lock lock(s_mutex);

            auto found = s_cache.find(key);
            if (found != s_cache.end())
            {
                co_return;
            }

            auto src = s_sources.find(key);
            if (src == s_sources.end())
            {
                throw hresult_invalid_argument(
                    hstring(L"Sprite::LoadSharedAsync: key not registered: ") + hstring(key));
            }

            creator = s_creator;
            uri = src->second;
        }

        if (!creator)
        {
            throw hresult_error(E_FAIL, L"Sprite::LoadSharedAsync: resource creator not set.");
        }

        auto bitmap = co_await CanvasBitmap::LoadAsync(creator, uri);

        auto data = std::make_shared<SharedTextureData>();
        data->Bitmap = bitmap;
        data->SizeDips = bitmap.Size();
        data->SizePixels = bitmap.SizeInPixels();

        {
            std::unique_lock lock(s_mutex);

            auto found = s_cache.find(key);
            if (found == s_cache.end())
            {
                s_cache.emplace(key, std::move(data));
            }
        }
    }

    IAsyncAction Sprite::LoadAsync(std::wstring const& key)
    {
        co_await LoadSharedAsync(key);

        {
            std::shared_lock lock(s_mutex);

            auto found = s_cache.find(key);
            if (found == s_cache.end())
            {
                throw hresult_error(E_FAIL, L"Sprite::LoadAsync: shared texture missing after load.");
            }

            m_key = key;
            m_texture = found->second;
        }
    }

    void Sprite::Unload(std::wstring const& key)
    {
        std::unique_lock lock(s_mutex);
        s_cache.erase(key);
    }

    void Sprite::ClearCache()
    {
        std::unique_lock lock(s_mutex);
        s_cache.clear();
    }

    bool Sprite::IsValid() const noexcept
    {
        return m_texture && m_texture->Bitmap;
    }

    void Sprite::SetOriginTopLeft() noexcept
    {
        Origin = { 0.0f, 0.0f };
    }

    void Sprite::SetOriginCenter()
    {
        if (!IsValid())
        {
            Origin = { 0.0f, 0.0f };
            return;
        }

        if (SourceRect)
        {
            Origin = { SourceRect->Width * 0.5f, SourceRect->Height * 0.5f };
        }
        else
        {
            Origin =
            {
                m_texture->SizeDips.Width * 0.5f,
                m_texture->SizeDips.Height * 0.5f
            };
        }
    }
    void Sprite::Draw(CanvasSpriteBatch const& batch) const
    {
        if (!IsValid())
        {
            return;
        }

        auto const bmp = m_texture->Bitmap;
        auto const originInBitmap = ComputeBitmapSpaceOrigin();

        if (SourceRect)
        {
            batch.DrawFromSpriteSheet(
                bmp,
                Position,
                *SourceRect,
                Tint,
                originInBitmap,
                Rotation,
                Scale,
                Flip);
        }
        else
        {
            batch.Draw(
                bmp,
                Position,
                Tint,
                originInBitmap,
                Rotation,
                Scale,
                Flip);
        }
    }

    float2 Sprite::ComputeBitmapSpaceOrigin() const noexcept
    {
        return Origin;
    }

    void Sprite::Draw(SpriteBatchScope const& batchScope) const
    {
        if (!IsValid())
        {
            return;
        }

        if (batchScope.IsBatching())
        {
            Draw(batchScope.Batch());
            return;
        }

        auto const bmp = m_texture->Bitmap;

        if (SourceRect)
        {
            batchScope.DrawingSession().DrawImage(
                bmp,
                Position,
                *SourceRect);
        }
        else
        {
            batchScope.DrawingSession().DrawImage(
                bmp,
                Position);
        }
    }
}