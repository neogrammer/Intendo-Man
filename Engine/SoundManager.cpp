#include "pch.h"
#include "Engine/SoundManager.h"


using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Media::Audio;
using namespace Windows::Media::Render;
using namespace Windows::Storage;

namespace engine
{
    SoundManager& SoundManager::Instance() noexcept
    {
        static SoundManager s{};
        return s;
    }

    bool SoundManager::IsInitialized() const noexcept
    {
        std::lock_guard lock(m_mutex);
        return m_graph != nullptr;
    }

    void SoundManager::RegisterSfx(std::wstring key, Uri uri)
    {
        std::lock_guard lock(m_mutex);
        m_sfxSources.emplace(std::move(key), std::move(uri));
    }

    void SoundManager::RegisterMusic(std::wstring key, Uri uri)
    {
        std::lock_guard lock(m_mutex);
        m_musicSources.emplace(std::move(key), std::move(uri));
    }

    void SoundManager::ApplyBusGains()
    {
        // OutgoingGain is a linear multiplier for audio leaving the node. :contentReference[oaicite:9]{index=9}
        if (m_sfxBus)   m_sfxBus.OutgoingGain(static_cast<double>(m_masterVol * m_sfxVol));
        if (m_musicBus) m_musicBus.OutgoingGain(static_cast<double>(m_masterVol * m_musicVol));
    }

    IAsyncAction SoundManager::InitializeAsync(AudioRenderCategory category)
    {
        // Idempotent: if already initialized, return.
        {
            std::lock_guard lock(m_mutex);
            if (m_graph) co_return;
        }

        // Settings optimized for chosen AudioRenderCategory. :contentReference[oaicite:10]{index=10}
        AudioGraphSettings settings(category);
        settings.QuantumSizeSelectionMode(QuantumSizeSelectionMode::LowestLatency); // :contentReference[oaicite:11]{index=11}

        auto graphResult = co_await AudioGraph::CreateAsync(settings);
        if (graphResult.Status() != AudioGraphCreationStatus::Success) // Status indicates creation success/failure. :contentReference[oaicite:12]{index=12}
        {
            throw hresult_error(E_FAIL, L"AudioGraph::CreateAsync failed.");
        }

        AudioGraph graph = graphResult.Graph();

        auto outResult = co_await graph.CreateDeviceOutputNodeAsync();
        if (outResult.Status() != AudioDeviceNodeCreationStatus::Success) // Check Status and read DeviceOutputNode on success. :contentReference[oaicite:13]{index=13}
        {
            throw hresult_error(E_FAIL, L"AudioGraph::CreateDeviceOutputNodeAsync failed.");
        }

        AudioDeviceOutputNode output = outResult.DeviceOutputNode();

        AudioSubmixNode sfxBus = graph.CreateSubmixNode();
        AudioSubmixNode musicBus = graph.CreateSubmixNode();

        // Route buses to output using outgoing connections. :contentReference[oaicite:14]{index=14}
        sfxBus.AddOutgoingConnection(output);
        musicBus.AddOutgoingConnection(output);

        graph.Start(); // Start graph processing. :contentReference[oaicite:15]{index=15}

        {
            std::lock_guard lock(m_mutex);
            m_graph = graph;
            m_output = output;
            m_sfxBus = sfxBus;
            m_musicBus = musicBus;
            ApplyBusGains();
        }
    }

    void SoundManager::Shutdown()
    {
        std::lock_guard lock(m_mutex);

        // Stop and close nodes (IClosable).
        if (m_musicNode)
        {
            m_musicNode.Stop();
            m_musicNode.Close();
            m_musicNode = nullptr;
        }

        for (auto& n : m_activeSfx)
        {
            if (n)
            {
                n.Stop();
                n.Close();
            }
        }
        m_activeSfx.clear();

        if (m_sfxBus) { m_sfxBus.Close();   m_sfxBus = nullptr; }
        if (m_musicBus) { m_musicBus.Close(); m_musicBus = nullptr; }

        if (m_graph)
        {
            m_graph.Stop();
            m_graph.Close();
            m_graph = nullptr;
        }

        m_output = nullptr;
    }

    void SoundManager::SetMasterVolume(float v)
    {
        std::lock_guard lock(m_mutex);
        m_masterVol = Clamp01(v);
        ApplyBusGains();
    }

    void SoundManager::SetSfxVolume(float v)
    {
        std::lock_guard lock(m_mutex);
        m_sfxVol = Clamp01(v);
        ApplyBusGains();
    }

    void SoundManager::SetMusicVolume(float v)
    {
        std::lock_guard lock(m_mutex);
        m_musicVol = Clamp01(v);
        ApplyBusGains();
    }

    void SoundManager::RemoveSfxNode(AudioFileInputNode const& node)
    {
        std::lock_guard lock(m_mutex);

        auto it = std::find(m_activeSfx.begin(), m_activeSfx.end(), node);
        if (it != m_activeSfx.end())
        {
            // FileCompleted fires when playback reaches end. :contentReference[oaicite:16]{index=16}
            (*it).Stop();
            (*it).Close();
            m_activeSfx.erase(it);
        }
    }

    fire_and_forget SoundManager::PlaySfx(std::wstring const& key, float volume)
    {
        Uri uri{ nullptr };
        AudioGraph graph{ nullptr };
        AudioSubmixNode sfxBus{ nullptr };

        {
            std::lock_guard lock(m_mutex);
            if (!m_graph || !m_sfxBus) co_return;

            auto it = m_sfxSources.find(key);
            if (it == m_sfxSources.end()) co_return;

            uri = it->second;
            graph = m_graph;
            sfxBus = m_sfxBus;
        }

        // Access packaged assets via ms-appx:/// using StorageFile. :contentReference[oaicite:17]{index=17}
        StorageFile file = co_await StorageFile::GetFileFromApplicationUriAsync(uri);

        // Create a file input node (Status indicates success/failure). :contentReference[oaicite:18]{index=18}
        auto nodeResult = co_await graph.CreateFileInputNodeAsync(file);
        if (nodeResult.Status() != AudioFileNodeCreationStatus::Success) co_return;

        AudioFileInputNode node = nodeResult.FileInputNode();

        // Volume as outgoing gain multiplier. :contentReference[oaicite:19]{index=19}
        node.OutgoingGain(static_cast<double>((volume < 0.0f) ? 0.0f : volume));

        // Route node into SFX bus. :contentReference[oaicite:20]{index=20}
        node.AddOutgoingConnection(sfxBus);

        // Auto-cleanup when finished. :contentReference[oaicite:21]{index=21}
        node.FileCompleted([this](AudioFileInputNode const& sender, IInspectable const&)
            {
                RemoveSfxNode(sender);
            });

        // Nodes are created started by default; Start restarts if stopped. :contentReference[oaicite:22]{index=22}
        node.Start();

        {
            std::lock_guard lock(m_mutex);
            m_activeSfx.push_back(node);
        }
    }

    IAsyncAction SoundManager::PlayMusicAsync(std::wstring const& key, bool loop, float volume)
    {
        Uri uri{ nullptr };
        AudioGraph graph{ nullptr };
        AudioSubmixNode musicBus{ nullptr };

        {
            std::lock_guard lock(m_mutex);
            if (!m_graph || !m_musicBus)
                co_return;

            auto it = m_musicSources.find(key);
            if (it == m_musicSources.end())
                co_return;

            uri = it->second;
            graph = m_graph;
            musicBus = m_musicBus;

            // Stop prior music if any
            if (m_musicNode)
            {
                m_musicNode.Stop();
                m_musicNode.Close();
                m_musicNode = nullptr;
            }
        }

        StorageFile file = co_await StorageFile::GetFileFromApplicationUriAsync(uri); // :contentReference[oaicite:23]{index=23}
        auto nodeResult = co_await graph.CreateFileInputNodeAsync(file);              // :contentReference[oaicite:24]{index=24}
        if (nodeResult.Status() != AudioFileNodeCreationStatus::Success) co_return;

        AudioFileInputNode node = nodeResult.FileInputNode();
        node.OutgoingGain(static_cast<double>((volume < 0.0f) ? 0.0f : volume)); // :contentReference[oaicite:25]{index=25}
        node.AddOutgoingConnection(musicBus);                                     // :contentReference[oaicite:26]{index=26}

        if (loop)
        {
            // Setting LoopCount to null loops indefinitely. :contentReference[oaicite:27]{index=27}
            node.LoopCount(nullptr);
        }

        node.Start(); // :contentReference[oaicite:28]{index=28}

        {
            std::lock_guard lock(m_mutex);
            m_musicNode = node;
        }
    }

    void SoundManager::StopMusic()
    {
        std::lock_guard lock(m_mutex);
        if (!m_musicNode) return;

        m_musicNode.Stop();
        m_musicNode.Close();
        m_musicNode = nullptr;
    }
}