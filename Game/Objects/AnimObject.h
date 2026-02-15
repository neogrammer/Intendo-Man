#pragma once

// AnimObject: a GameObject with animation containers.
// - Holds per-animation containers (frames/offsets/delays + atlas info)
// - Update() advances the internal frame index (NO base writes).
// - SyncToBase() copies *render-related* values into the base GameObject's single-value fields.
//
// Design choice (your call): collision box size is constant.
// - worldPosition is moved by gameplay/collisions.
// - worldSize is NOT animated per-frame; it's set once (constructor or [object] world_size), and then kept.

#include "GameObject.h"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Numerics.h>


#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace game
{
    using winrt::Windows::Foundation::Rect;
    using winrt::Windows::Foundation::Numerics::float2;

    class AnimObject : public GameObject
    {
    public:
        enum class CollisionMode { Constant, Frame };
        enum class CollisionAnchor { TopLeft, BottomLeft, BottomCenter, Center };

        struct Clip
        {
            // Texture + atlas description
            Cfg::Textures texID{ Cfg::Textures::None };

            float2 frameSize{ 0.0f, 0.0f };
            float2 startPx{ 0.0f, 0.0f };
            uint32_t startCol{ 0 };
            uint32_t startRow{ 0 };
            uint32_t pitch{ 1 };          // frames per row in the atlas grid
            uint32_t framesPerDir{ 0 };   // number of frames for one direction (Right); total = uni ? N : 2N
            bool uniDirectional{ true };

            // Optional explicit left start (if omitted, left is assumed directly BELOW the right block)
            std::optional<float2> startPxLeft;
            std::optional<uint32_t> startColLeft;
            std::optional<uint32_t> startRowLeft;

            // Timing
            bool looping{ true };
            bool loopWait{ false };
            float loopDelay{ 0.0f };

            // End-of-clip behavior (data-driven via .anm "looping" value)
            enum class EndMode { Loop, Wait, Goto };

            EndMode endMode{ EndMode::Loop };

            // For EndMode::Loop: restart from this frame index (0 = normal loop)
            uint32_t loopFromFrame{ 0 };

            // For EndMode::Goto: switch to another clip at end
            std::wstring endToClip{};
            uint32_t endToFrame{ 0 };
            bool endOppositeDirection{ false };

            // Optional explicit frame cells (col,row) in the atlas grid.
            // If provided, these override start_col/start_row scanning.
            // Count must be uni?N : 2N.
            std::vector<float2> rects;

            // Per-frame values (parallel arrays) — length must be uni?N : 2N
            std::vector<float2> offsets; // textureOffset per frame
            std::vector<float2> sizes;   // legacy/compat: parsed from .anm, but worldSize is constant (only used for initial default)
            std::vector<float> delays;   // seconds per frame


            // New (parsed, but not yet applied until you wire it):
            float startDelay{ 0.0f };         // seconds to hold on first frame when clip begins
            std::vector<float2> velocities;   // optional per-frame velocity (x,y)

            // Computed source rects (parallel with the arrays above)
            std::vector<Rect> sourceRects;
        };

    private:
        std::unordered_map<std::wstring, Clip> m_clips{};

        std::wstring m_currentClip{};
        uint32_t m_currentIndex{ 0 };
        bool m_playing{ true };
        bool m_facingRight{ true };

        float m_animElapsed{ 0.0f };
        float m_loopElapsed{ 0.0f };
        bool m_waitingForLoop{ false };
        float m_startDelayRemaining{ 0.0f };


        // (parsed from [object], but not yet applied until you wire it):
        CollisionMode   m_collisionMode{ CollisionMode::Constant };
        CollisionAnchor m_collisionAnchor{ CollisionAnchor::TopLeft };

        std::unique_ptr<GameObject> under{ nullptr };

    public:
        AnimObject() = default;
        explicit AnimObject(std::wstring const& anmFilePath);

        // Clip management
        void clearClips();
        bool hasClip(std::wstring const& name) const;
        std::vector<std::wstring> clipNames() const;

        // Loading
        void LoadFromAnmFile(std::wstring const& path);
        void LoadFromAnmText(std::wstring const& text);


        // Animation control
        void Play(std::wstring const& name, bool restart = false, uint32_t startFrame = 0);

        void PlaySynced(std::wstring const& name);
        void Stop() noexcept { m_playing = false; }
        void Resume() noexcept { m_playing = true; }
        bool IsPlaying() const noexcept { return m_playing; }

        void SetFacingRight(bool right) noexcept;
        bool IsFacingRight() const noexcept { return m_facingRight; }

        // Advances internal animation timers + current frame index.
        // Per your design: does NOT touch the base GameObject values.
        void Update(float dt);

        GameObject* getUnder();

        // Copies current clip/frame data into the base GameObject values.
        void SyncToBase();

        // --- Introspection (needed by animation state machines)
        std::wstring const& CurrentClipKey() const noexcept { return m_currentClip; }
        uint32_t CurrentFrameIndex() const noexcept { return m_currentIndex; }

        // --- Parsed settings (not yet used until you wire logic)
        CollisionMode GetCollisionMode() const noexcept { return m_collisionMode; }
        CollisionAnchor GetCollisionAnchor() const noexcept { return m_collisionAnchor; }
        float2 CurrentFrameVelocity() const noexcept;

    private:
        // ---- Parsing helpers
        static std::wstring ResolvePath(std::wstring const& path);
        static std::wstring Trim(std::wstring s);
        static std::wstring ToLower(std::wstring s);
        static bool IEquals(std::wstring const& a, std::wstring const& b);

        static bool ParseBool(std::wstring const& s, bool defaultValue = false);
        static uint32_t ParseU32(std::wstring const& s, uint32_t defaultValue = 0);
        static float ParseF32(std::wstring const& s, float defaultValue = 0.0f);

        static float2 ParseFloat2Pair(std::wstring const& s, float2 defaultValue = { 0.0f, 0.0f });
        static std::vector<float2> ParseFloat2List(std::wstring const& s);
        static std::vector<float> ParseFloatList(std::wstring const& s);


        static uint32_t ParseU32Prefix(std::wstring const& s, uint32_t defaultValue = 0);
        static void ParseLoopingSpec(std::wstring const& s, Clip& clip);

        static void BuildSourceRects(Clip& clip);

        size_t currentFrameLinearIndex(Clip const& clip) const noexcept;

        Clip* currentClip();
        Clip const* currentClip() const;

        static Cfg::Textures FindTextureEnum(std::wstring const& token);
    };
}
