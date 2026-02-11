#include "pch.h"
#include "AnimObject.h"

#include "../Resources/Cfg.h"

#include <winrt/base.h>
#include <winrt/Windows.ApplicationModel.h>

#include <algorithm>
#include <cwctype>
#include <cmath>
#include <fstream>
#include <locale>
#include <sstream>

namespace game
{
    using winrt::Windows::ApplicationModel::Package;

    AnimObject::AnimObject(std::wstring const& anmFilePath)
        : AnimObject{}
    {
        LoadFromAnmFile(anmFilePath);
    }

    void AnimObject::clearClips()
    {
        m_clips.clear();
        m_currentClip.clear();
        m_currentIndex = 0;
        m_animElapsed = 0.0f;
        m_loopElapsed = 0.0f;
        m_playing = true;
        m_facingRight = true;
        m_waitingForLoop = false;
    }

    bool AnimObject::hasClip(std::wstring const& name) const
    {
        auto key = ToLower(Trim(name));
        return m_clips.find(key) != m_clips.end();
    }

    std::vector<std::wstring> AnimObject::clipNames() const
    {
        std::vector<std::wstring> out;
        out.reserve(m_clips.size());
        for (auto const& kv : m_clips)
        {
            out.push_back(kv.first);
        }
        return out;
    }

    std::wstring AnimObject::ResolvePath(std::wstring const& path)
    {
        // If you pass an absolute path, we use it directly.
        // If you pass a relative path (e.g. L"Assets\\Anims\\Player.anm"), we resolve it
        // against the app's installed location.
        if (path.empty())
        {
            return path;
        }

        // crude absolute checks
        if (path.size() >= 2 && std::iswalpha(path[0]) && path[1] == L':')
        {
            return path;
        }
        if (path.rfind(L"\\\\", 0) == 0)
        {
            return path;
        }

        auto base = std::wstring(Package::Current().InstalledLocation().Path());
        if (!base.empty() && base.back() != L'\\' && base.back() != L'/')
        {
            base.push_back(L'\\');
        }

        // Normalize slashes a bit (optional)
        std::wstring rel = path;
        std::replace(rel.begin(), rel.end(), L'/', L'\\');

        return base + rel;
    }

    // ---- tiny string helpers

    std::wstring AnimObject::Trim(std::wstring s)
    {
        auto const isSpace = [](wchar_t c) { return std::iswspace(c) != 0; };

        while (!s.empty() && isSpace(s.front())) s.erase(s.begin());
        while (!s.empty() && isSpace(s.back())) s.pop_back();
        return s;
    }

    std::wstring AnimObject::ToLower(std::wstring s)
    {
        std::transform(s.begin(), s.end(), s.begin(), [](wchar_t c) {
            return static_cast<wchar_t>(std::towlower(c));
            });
        return s;
    }

    bool AnimObject::IEquals(std::wstring const& a, std::wstring const& b)
    {
        return ToLower(a) == ToLower(b);
    }

    bool AnimObject::ParseBool(std::wstring const& s, bool defaultValue)
    {
        auto v = ToLower(Trim(s));
        if (v == L"true" || v == L"1" || v == L"yes" || v == L"on") return true;
        if (v == L"false" || v == L"0" || v == L"no" || v == L"off") return false;
        return defaultValue;
    }

    uint32_t AnimObject::ParseU32(std::wstring const& s, uint32_t defaultValue)
    {
        try
        {
            auto v = Trim(s);
            if (v.empty()) return defaultValue;
            auto n = std::stoul(v);
            return static_cast<uint32_t>(n);
        }
        catch (...)
        {
            return defaultValue;
        }
    }

    float AnimObject::ParseF32(std::wstring const& s, float defaultValue)
    {
        try
        {
            auto v = Trim(s);
            if (v.empty()) return defaultValue;
            return std::stof(v);
        }
        catch (...)
        {
            return defaultValue;
        }
    }

    float2 AnimObject::ParseFloat2Pair(std::wstring const& s, float2 defaultValue)
    {
        std::wistringstream iss(s);
        float x = 0.0f;
        float y = 0.0f;
        if (!(iss >> x >> y))
        {
            return defaultValue;
        }
        return float2{ x, y };
    }

    std::vector<float2> AnimObject::ParseFloat2List(std::wstring const& s)
    {
        // Accepts: (x,y) (x,y) ... across whitespace/newlines.
        std::vector<float2> out;
        size_t i = 0;
        while (i < s.size())
        {
            if (s[i] != L'(')
            {
                ++i;
                continue;
            }

            // parse (x,y)
            ++i; // '(' 

            // x
            size_t start = i;
            while (i < s.size() && s[i] != L',' && s[i] != L')') ++i;
            if (i >= s.size() || s[i] != L',') break;
            auto xs = Trim(s.substr(start, i - start));
            ++i; // ','

            // y
            start = i;
            while (i < s.size() && s[i] != L')') ++i;
            if (i >= s.size() || s[i] != L')') break;
            auto ys = Trim(s.substr(start, i - start));
            ++i; // ')'

            float x = ParseF32(xs, 0.0f);
            float y = ParseF32(ys, 0.0f);
            out.push_back(float2{ x, y });
        }
        return out;
    }

    std::vector<float> AnimObject::ParseFloatList(std::wstring const& s)
    {
        std::vector<float> out;
        std::wistringstream iss(s);
        while (iss)
        {
            float v = 0.0f;
            if (!(iss >> v)) break;
            out.push_back(v);
        }
        return out;
    }
    uint32_t AnimObject::ParseU32Prefix(std::wstring const& s, uint32_t defaultValue)
    {
        auto v = Trim(s);
        if (v.empty()) return defaultValue;

        size_t i = 0;
        while (i < v.size() && std::iswdigit(v[i])) ++i;
        if (i == 0) return defaultValue;

        return ParseU32(v.substr(0, i), defaultValue);
    }

    void AnimObject::ParseLoopingSpec(std::wstring const& s, Clip& clip)
    {
        auto v = ToLower(Trim(s));
        if (v.empty())
            return;

        // ---- "true"/"false" (and friends)
        if (v == L"true" || v == L"1" || v == L"yes" || v == L"on")
        {
            clip.looping = true;
            clip.endMode = Clip::EndMode::Loop;
            clip.loopFromFrame = 0;
            clip.endToClip.clear();
            clip.endToFrame = 0;
            clip.endOppositeDirection = false;
            return;
        }

        if (v == L"false" || v == L"0" || v == L"no" || v == L"off" || v == L"wait" || v == L"waits" || v == L"hold")
        {
            clip.looping = false;
            clip.endMode = Clip::EndMode::Wait;
            clip.loopFromFrame = 0;
            clip.endToClip.clear();
            clip.endToFrame = 0;
            clip.endOppositeDirection = false;
            return;
        }

        // ---- "frameN" means loop, but restart from that frame index
        if (v.rfind(L"frame", 0) == 0)
        {
            clip.looping = true;
            clip.endMode = Clip::EndMode::Loop;
            clip.loopFromFrame = ParseU32Prefix(v.substr(5), 0);
            clip.endToClip.clear();
            clip.endToFrame = 0;
            clip.endOppositeDirection = false;
            return;
        }

        // ---- Otherwise treat it as a "goto another clip" spec:
        //      e.g. "idle", or "jump_rise frame1(oppositedirection)"
        clip.looping = false;
        clip.endMode = Clip::EndMode::Goto;
        clip.loopFromFrame = 0;
        clip.endToClip.clear();
        clip.endToFrame = 0;
        clip.endOppositeDirection = false;

        std::wistringstream iss(v);
        std::wstring tok;
        if (!(iss >> tok))
            return;

        clip.endToClip = ToLower(Trim(tok));

        while (iss >> tok)
        {
            std::wstring cleaned = ToLower(tok);

            // Replace punctuation with spaces so "frame1(oppositedirection)" becomes "frame1 oppositedirection"
            for (auto& ch : cleaned)
            {
                if (ch == L'(' || ch == L')' || ch == L',' || ch == L';')
                    ch = L' ';
            }

            std::wistringstream sub(cleaned);
            std::wstring part;
            while (sub >> part)
            {
                part = ToLower(Trim(part));
                if (part.empty()) continue;

                if (part.rfind(L"frame", 0) == 0)
                {
                    clip.endToFrame = ParseU32Prefix(part.substr(5), 0);
                    continue;
                }

                if (part.find(L"opposite") != std::wstring::npos)
                {
                    clip.endOppositeDirection = true;
                    continue;
                }
            }
        }
    }


    Cfg::Textures AnimObject::FindTextureEnum(std::wstring const& token)
    {
        auto t = ToLower(Trim(token));

        // 1) try matching the registered TextureStore key (Cfg::textures map value)
        for (auto const& kv : Cfg::textures)
        {
            if (IEquals(kv.second, t))
            {
                return kv.first;
            }
        }

        // 2) try parsing as integer enum value
        try
        {
            int v = std::stoi(t);
            if (v >= 0 && v < static_cast<int>(Cfg::Textures::Count))
            {
                return static_cast<Cfg::Textures>(v);
            }
        }
        catch (...) {}

        // 3) fall back to None
        return Cfg::Textures::None;
    }

    void AnimObject::BuildSourceRects(Clip& clip)
    {
        clip.sourceRects.clear();

        uint32_t frames = clip.framesPerDir;
        if (frames == 0 && !clip.rects.empty())
        {
            if (clip.uniDirectional)
                frames = static_cast<uint32_t>(clip.rects.size());
            else
                frames = static_cast<uint32_t>(clip.rects.size() / 2u);

            clip.framesPerDir = frames;
        }

        const uint32_t pitch = std::max<uint32_t>((uint32_t)1u, clip.pitch);

        if (frames == 0 || clip.frameSize.x <= 0.0f || clip.frameSize.y <= 0.0f)
            return;

        const size_t expect = clip.uniDirectional ? frames : (frames * 2ull);
        clip.sourceRects.reserve(expect);

        // ------------------------------------------------------------
        // Explicit rect list (cells in the atlas grid): rects = (col,row) ...
        // ------------------------------------------------------------
        if (!clip.rects.empty())
        {
            if (clip.rects.size() != expect)
            {
                Cfg::debugPrint(
                    L"AnimObject: rects count mismatch (expected " + std::to_wstring(expect) +
                    L", got " + std::to_wstring(clip.rects.size()) + L")\n");
            }

            const size_t count = std::min<size_t>(clip.rects.size(), expect);
            for (size_t i = 0; i < count; ++i)
            {
                const float2 cell = clip.rects[i];
                const float x = clip.startPx.x + cell.x * clip.frameSize.x;
                const float y = clip.startPx.y + cell.y * clip.frameSize.y;
                clip.sourceRects.emplace_back(winrt::Windows::Foundation::Rect{ x, y, clip.frameSize.x, clip.frameSize.y });
            }

            // Pad if short
            while (clip.sourceRects.size() < expect)
            {
                clip.sourceRects.emplace_back(winrt::Windows::Foundation::Rect{ clip.startPx.x, clip.startPx.y, clip.frameSize.x, clip.frameSize.y });
            }
            return;
        }

        // ------------------------------------------------------------
        // Auto-scan: build rects from (start_col/start_row + pitch wrap)
        // ------------------------------------------------------------
        auto makeRects = [&](float2 startPx, uint32_t startCol, uint32_t startRow)
            {
                for (uint32_t i = 0; i < frames; ++i)
                {
                    uint32_t c = startCol + (i % pitch);
                    uint32_t r = startRow + (i / pitch);

                    const float x = startPx.x + (c * clip.frameSize.x);
                    const float y = startPx.y + (r * clip.frameSize.y);

                    clip.sourceRects.emplace_back(winrt::Windows::Foundation::Rect{ x, y, clip.frameSize.x, clip.frameSize.y });
                }
            };

        // Right frames
        makeRects(clip.startPx, clip.startCol, clip.startRow);

        if (!clip.uniDirectional)
        {
            // Default left block: directly below the right block
            const uint32_t rowsRight = (frames + pitch - 1) / pitch;

            const float2 leftPx = clip.startPxLeft.value_or(clip.startPx);
            const uint32_t leftCol = clip.startColLeft.value_or(clip.startCol);
            const uint32_t leftRow = clip.startRowLeft.value_or(clip.startRow + rowsRight);

            makeRects(leftPx, leftCol, leftRow);
        }
    }


    AnimObject::Clip* AnimObject::currentClip()
    {
        if (m_currentClip.empty()) return nullptr;
        auto it = m_clips.find(m_currentClip);
        return (it == m_clips.end()) ? nullptr : &it->second;
    }

    AnimObject::Clip const* AnimObject::currentClip() const
    {
        if (m_currentClip.empty()) return nullptr;
        auto it = m_clips.find(m_currentClip);
        return (it == m_clips.end()) ? nullptr : &it->second;
    }

    size_t AnimObject::currentFrameLinearIndex(Clip const& clip) const noexcept
    {
        const size_t dir = (clip.uniDirectional || m_facingRight) ? 0u : 1u;
        const size_t base = static_cast<size_t>(dir) * static_cast<size_t>(clip.framesPerDir);

        size_t idx = base + static_cast<size_t>(m_currentIndex);

        // Clamp defensively
        const size_t max = clip.uniDirectional ? static_cast<size_t>(clip.framesPerDir)
            : static_cast<size_t>(clip.framesPerDir) * 2u;

        if (max == 0) return 0;
        if (idx >= max) idx = max - 1;
        return idx;
    }

    void AnimObject::LoadFromAnmFile(std::wstring const& path)
    {
        clearClips();

        auto fullPath = ResolvePath(path);
        std::wifstream in(fullPath);
        in.imbue(std::locale::classic());

        if (!in.is_open())
        {
            Cfg::debugPrint(L"AnimObject::LoadFromAnmFile failed to open: " + std::wstring(path));
            return;
        }

        std::wstringstream buffer;
        buffer << in.rdbuf();
        LoadFromAnmText(buffer.str());
    }

    void AnimObject::LoadFromAnmText(std::wstring const& text)
    {
        // This parser expects the INI-ish format you showed:
        // [object]
        // position = x y
        // start_anim = idle (optional)
        //
        // [anim idle]
        // texture = ship
        // frame_size = w h
        // start_col = ...
        // start_row = ...
        // start_px  = ...
        // pitch = ...
        // frames = ...
        // uni_directional = true/false
        // offsets = (x,y) ... (can continue on next lines)
        // sizes   = (x,y) ...
        // delays  = f ...
        // looping = true/false
        // loop_wait = true/false
        // loop_delay = f

        clearClips();

        std::wistringstream in(text);

        enum class Section { None, Object, Anim };
        Section section = Section::None;

        std::wstring currentAnimName;
        Clip* clip = nullptr;

        std::wstring startAnimName; // optional [object] start_anim

        // For multiline list continuation
        std::wstring listKey;
        std::wstring listBuffer;

        auto flushList = [&]()
            {
                if (section != Section::Anim || clip == nullptr || listKey.empty())
                {
                    listKey.clear();
                    listBuffer.clear();
                    return;
                }

                if (listKey == L"offsets") clip->offsets = ParseFloat2List(listBuffer);
                else if (listKey == L"sizes") clip->sizes = ParseFloat2List(listBuffer);
                else if (listKey == L"delays") clip->delays = ParseFloatList(listBuffer);
                else if (listKey == L"rects")  clip->rects = ParseFloat2List(listBuffer);

                listKey.clear();
                listBuffer.clear();
            };

        auto ensureClipVectors = [&](Clip& c)
            {
                // Derive framesPerDir if not explicitly set
                if (c.framesPerDir == 0)
                {
                    if (!c.offsets.empty()) c.framesPerDir = static_cast<uint32_t>(c.offsets.size());
                    else if (!c.sizes.empty()) c.framesPerDir = static_cast<uint32_t>(c.sizes.size());
                    else if (!c.delays.empty()) c.framesPerDir = static_cast<uint32_t>(c.delays.size());
                    else if (!c.rects.empty())
                    {
                        if (c.uniDirectional) c.framesPerDir = static_cast<uint32_t>(c.rects.size());
                        else c.framesPerDir = static_cast<uint32_t>(c.rects.size() / 2u);
                    }
                }
                if (c.framesPerDir == 0) c.framesPerDir = 1;

                const uint32_t n = c.framesPerDir;
                const size_t expect = c.uniDirectional ? n : (2ull * n);

                auto fixVec2 = [&](std::vector<float2>& v, float2 fallback)
                    {
                        // If only RIGHT values were provided for a bi-directional clip, duplicate them for LEFT.
                        if (!c.uniDirectional && v.size() == n)
                        {
                            v.reserve(expect);
                            for (uint32_t i = 0; i < n; ++i)
                                v.push_back(v[i]);
                        }

                        if (v.empty())
                        {
                            v.assign(expect, fallback);
                        }
                        else if (v.size() == 1 && expect > 1)
                        {
                            // Allow single-value shorthand (replicate to all frames)
                            auto tmp = v[0];
                            v.assign(expect, tmp);
                        }
                        else if (v.size() < expect)
                        {
                            v.resize(expect, fallback);
                        }
                        else if (v.size() > expect)
                        {
                            v.resize(expect);
                        }
                    };

                auto fixFloat = [&](std::vector<float>& v, float fallback)
                    {
                        if (!c.uniDirectional && v.size() == n)
                        {
                            v.reserve(expect);
                            for (uint32_t i = 0; i < n; ++i)
                                v.push_back(v[i]);
                        }

                        if (v.empty())
                        {
                            v.assign(expect, fallback);
                        }
                        else if (v.size() == 1 && expect > 1)
                        {
                            auto tmp = v[0];
                            v.assign(expect, tmp);
                        }
                        else if (v.size() < expect)
                        {
                            v.resize(expect, fallback);
                        }
                        else if (v.size() > expect)
                        {
                            v.resize(expect);
                        }
                    };

                fixVec2(c.offsets, float2{ 0.0f, 0.0f });
                fixVec2(c.sizes, (c.frameSize.x > 0.0f && c.frameSize.y > 0.0f) ? c.frameSize : float2{ 0.0f, 0.0f });
                fixFloat(c.delays, 0.10f);

                if (c.loopFromFrame >= c.framesPerDir)
                    c.loopFromFrame = 0;

                BuildSourceRects(c); // <-- do this ONCE, here
            };
    
    std::wstring line;
    while (std::getline(in, line))
    {
        // strip comments (# ...)
        if (auto pos = line.find(L'#'); pos != std::wstring::npos)
        {
            line = line.substr(0, pos);
        }

        line = Trim(line);
        if (line.empty()) continue;

        // section header [ ... ]
        if (line.front() == L'[' && line.back() == L']')
        {
            flushList();

            auto header = ToLower(Trim(line.substr(1, line.size() - 2)));

            if (header == L"object")
            {
                section = Section::Object;
                clip = nullptr;
                currentAnimName.clear();
            }
            else if (header.rfind(L"anim", 0) == 0)
            {
                section = Section::Anim;

                // header like: "anim idle"
                auto name = Trim(header.substr(4));
                if (name.empty()) name = L"unnamed";

                currentAnimName = ToLower(name);

                auto [it, inserted] = m_clips.emplace(currentAnimName, Clip{});
                clip = &it->second;
            }
            else
            {
                section = Section::None;
                clip = nullptr;
                currentAnimName.clear();
            }

            continue;
        }

        // key/value or list continuation
        auto eq = line.find(L'=');
        if (eq == std::wstring::npos)
        {
            // continuation for list keys
            if (!listKey.empty())
            {
                listBuffer.append(L" ");
                listBuffer.append(line);
            }
            continue;
        }

        auto key = ToLower(Trim(line.substr(0, eq)));
        auto value = Trim(line.substr(eq + 1));

        // if we were accumulating a list, and a new key begins, flush the list first
        if (!listKey.empty() && key != listKey)
        {
            flushList();
        }

        if (section == Section::Object)
        {
            if (key == L"position")
            {
                worldPosition = ParseFloat2Pair(value, worldPosition);
            }
            else if (key == L"start_anim")
            {
                startAnimName = ToLower(Trim(value));
            }
            else if (key == L"world_size" || key == L"collision_size" || key == L"size")
            {
                // Collision box size (constant for this object)
                worldSize = ParseFloat2Pair(value, worldSize);
            }
        }
        else if (section == Section::Anim && clip)
        {
            if (key == L"texture")
            {
                clip->texID = FindTextureEnum(value);
            }
            else if (key == L"frame_size")
            {
                clip->frameSize = ParseFloat2Pair(value, clip->frameSize);
            }
            else if (key == L"start_col")
            {
                clip->startCol = ParseU32(value, clip->startCol);
            }
            else if (key == L"start_row")
            {
                clip->startRow = ParseU32(value, clip->startRow);
            }
            else if (key == L"start_px")
            {
                clip->startPx = ParseFloat2Pair(value, clip->startPx);
            }
            else if (key == L"start_col_left")
            {
                clip->startColLeft = ParseU32(value, clip->startCol);
            }
            else if (key == L"start_row_left")
            {
                clip->startRowLeft = ParseU32(value, clip->startRow);
            }
            else if (key == L"start_px_left")
            {
                clip->startPxLeft = ParseFloat2Pair(value, clip->startPx);
            }
            else if (key == L"pitch")
            {
                clip->pitch = std::max<uint32_t>(1, ParseU32(value, 1));
            }
            else if (key == L"frames")
            {
                clip->framesPerDir = ParseU32(value, 0);
            }
            else if (key == L"uni_directional")
            {
                clip->uniDirectional = ParseBool(value, true);
            }
            else if (key == L"offsets" || key == L"sizes" || key == L"delays" || key == L"rects")
            {
                listKey = key;
                listBuffer = value;
            }
            else if (key == L"looping")
            {
                ParseLoopingSpec(value, *clip);
            }
            else if (key == L"loop_wait")
            {
                clip->loopWait = ParseBool(value, false);
            }
            else if (key == L"loop_delay")
            {
                clip->loopDelay = ParseF32(value, 0.0f);
            }
        }
    }

    flushList();

    // Post-process each clip
    for (auto& kv : m_clips)
    {
        ensureClipVectors(kv.second);
    }

    // Collision size is constant (design choice).
    // If it wasn't explicitly set in [object], initialize it once from the first clip.
    if ((worldSize.x <= 0.0f || worldSize.y <= 0.0f) && !m_clips.empty())
    {
        auto const& c0 = m_clips.begin()->second;
        if (!c0.sizes.empty())
            worldSize = c0.sizes[0];
        else
            worldSize = c0.frameSize;
    }

    // Pick a start clip
    if (!startAnimName.empty() && hasClip(startAnimName))
    {
        Play(startAnimName, true);
    }
    else if (!m_clips.empty())
    {
        Play(m_clips.begin()->first, true);
    }

    // Make sure base values are ready immediately (even before the first global SyncObjects call)
    SyncToBase();
}

void AnimObject::Play(std::wstring const& name, bool restart, uint32_t startFrame)
{
    auto key = ToLower(Trim(name));

    auto it = m_clips.find(key);
    if (it == m_clips.end())
    {
        Cfg::debugPrint(L"AnimObject::Play: unknown anim \"" + key + L"\"\n");
        return;
    }

    if (m_currentClip != key || restart)
    {
        m_currentClip = key;

        uint32_t maxIndex = (it->second.framesPerDir > 0) ? (it->second.framesPerDir - 1u) : 0u;
        m_currentIndex = std::min<uint32_t>(startFrame, maxIndex);

        m_animElapsed = 0.0f;
        m_loopElapsed = 0.0f;
        m_waitingForLoop = false;
    }

    m_playing = true;
}

void AnimObject::Update(float dt)
{
    auto* clip = currentClip();
    if (!clip) return;
    if (!m_playing) return;
    if (clip->framesPerDir == 0) return;

    // Loop-wait behavior: sit on the last frame until loop_delay expires
    if (m_waitingForLoop)
    {
        m_loopElapsed += dt;
        if (m_loopElapsed >= clip->loopDelay)
        {
            m_loopElapsed = 0.0f;
            m_waitingForLoop = false;

            // Resume the loop at the configured loop-from frame
            uint32_t maxIndex = (clip->framesPerDir > 0) ? (clip->framesPerDir - 1u) : 0u;
            m_currentIndex = std::min<uint32_t>(clip->loopFromFrame, maxIndex);
        }
        return;
    }

    const size_t idx = currentFrameLinearIndex(*clip);
    float frameDelay = (idx < clip->delays.size()) ? clip->delays[idx] : 0.10f;
    if (frameDelay <= 0.0001f) frameDelay = 0.0001f;

    m_animElapsed += dt;

    if (m_animElapsed < frameDelay)
    {
        return;
    }

    // Advance one frame (keep it simple; if dt is huge you'll just skip less smoothly)
    m_animElapsed -= frameDelay;

    if (m_currentIndex + 1 < clip->framesPerDir)
    {
        m_currentIndex++;
        return;
    }

    // ----------------------------
    // End of animation reached
    // ----------------------------
    switch (clip->endMode)
    {
    case Clip::EndMode::Wait:
        // Freeze on last frame until something external calls Play(...)
        m_playing = false;
        return;

    case Clip::EndMode::Goto:
    {
        if (clip->endToClip.empty())
        {
            m_playing = false;
            return;
        }

        if (clip->endOppositeDirection)
            m_facingRight = !m_facingRight;

        // Swap to the target clip (case-insensitive keys are stored lower-case)
        const auto key = ToLower(Trim(clip->endToClip));
        auto it = m_clips.find(key);
        if (it == m_clips.end())
        {
            // Unknown target: fail safe by freezing
            m_playing = false;
            return;
        }

        m_currentClip = it->first;

        uint32_t maxIndex = (it->second.framesPerDir > 0) ? (it->second.framesPerDir - 1u) : 0u;
        m_currentIndex = std::min<uint32_t>(clip->endToFrame, maxIndex);

        m_animElapsed = 0.0f;
        m_loopElapsed = 0.0f;
        m_waitingForLoop = false;
        m_playing = true;
        return;
    }

    case Clip::EndMode::Loop:
    default:
        // Optional "loop wait" delay before restarting
        if (clip->loopWait && clip->loopDelay > 0.0f)
        {
            m_waitingForLoop = true;
            m_loopElapsed = 0.0f;
            // stay on last frame until delay expires
            return;
        }

        // Restart loop from configured frame
        {
            uint32_t maxIndex = (clip->framesPerDir > 0) ? (clip->framesPerDir - 1u) : 0u;
            m_currentIndex = std::min<uint32_t>(clip->loopFromFrame, maxIndex);
        }
        return;
    }
}

void AnimObject::SyncToBase()
{
    auto* clip = currentClip();
    if (!clip) return;
    if (clip->framesPerDir == 0) return;

    const size_t idx = currentFrameLinearIndex(*clip);

    // Copy into base single-values
    texID = clip->texID;

    // texPosition + frameSize from the computed source rects
    if (idx < clip->sourceRects.size())
    {
        auto const& r = clip->sourceRects[idx];
        texPosition = float2{ r.X, r.Y };
        frameSize = float2{ r.Width, r.Height };
    }
    else
    {
        texPosition = clip->startPx;
        frameSize = clip->frameSize;
    }

    // Collision box size is constant (design choice): do NOT animate worldSize per-frame.
    // If it hasn't been initialized yet (e.g. no [object] world_size), initialize it once.
    if ((worldSize.x <= 0.0f || worldSize.y <= 0.0f))
    {
        if (!clip->sizes.empty())
            worldSize = clip->sizes[0];
        else
            worldSize = frameSize;
    }

    // Sprite offset is allowed to vary per frame (draw at worldPosition - textureOffset).
    if (idx < clip->offsets.size())
        textureOffset = clip->offsets[idx];

    // Since we are using explicit left frames by default, no sprite flipping required.
    // If you later want a "flip" mode, we can add a flag and set flip = Horizontal when !facingRight.
    //flip = engine::CanvasSpriteFlip::None;
}

}