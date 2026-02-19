#pragma once
#include <vector>
#include <variant>
#include <utility>
#include <string>
#include <unordered_map>
#include "../Engine/TextureStore.h"
#include "../Engine/SoundManager.h"
#include "../Engine/FontManager.h"
#include "../Engine/Text.h"
#include <winrt/Windows.UI.h>
#include <winrt/Microsoft.Graphics.Canvas.h>
#include <winrt/Windows.Foundation.h>
#include <chrono>
#include <algorithm>


struct Cfg
{
	Cfg() = delete;
	Cfg(const Cfg&) = delete;
	Cfg& operator=(const Cfg&) = delete;


	static void debugPrint(const std::wstring& msg) {
		std::wstring str = L"[ODS] ";
		str.append(msg);
		OutputDebugStringW(str.c_str());
	};



	//globals 
	//static sol::state lua; // globals are bad, but we'll use it for simpler implementation
	static winrt::Windows::Foundation::IAsyncAction InitializeAsync(winrt::Microsoft::Graphics::Canvas::UI::Xaml::CanvasAnimatedControl const& sender);

	// Resource Enums 
	enum class Textures : int { Under, Ship, PlayerAtlas, Tileset1, BusterShot, BlueyAtlas, Default, Count, None };
	enum class Sounds : int { Blip, PlayerHit, EnemyHit, Default,
			ElectricShot, MissileLaunch,  EnemyDie,
		Count, None };
	enum class Music : int { Theme1, Default, Count, None };
	enum class Fonts : int { UI, Bubbly, Default, Count, None };


	// resource buckets for each type of resource
	static std::unordered_map<Textures, std::wstring> textures;
	static std::unordered_map<Sounds, std::wstring> sounds;
	static std::unordered_map<Music, std::wstring> music;
	static std::unordered_map<Fonts, std::wstring> fonts;

	static std::shared_ptr<engine::Font> GetFont(std::wstring fontKey_);
	static std::shared_ptr<engine::Font> GetFont(Fonts f_);


	static std::shared_ptr<engine::Texture> GetTex(std::wstring tex_);
	static std::shared_ptr<engine::Texture> GetTex(Textures tex_);

	static void PlaySfx(std::wstring snd_, float volume_ = 1.0f);
	static void PlaySfx(Sounds snd_, float volume_ = 1.0f);

	static winrt::Windows::Foundation::IAsyncAction PlayMusicAsync(std::wstring snd_, bool loop_ = true, float volume_ = 0.5f);
	static winrt::Windows::Foundation::IAsyncAction PlayMusicAsync(Music snd_, bool loop_ = true, float volume_ = 0.5f);


private:
	// initalize the resources for the entire game
	static void initFonts();
	static void initMusic();
	static void initSounds();
	static winrt::Windows::Foundation::IAsyncAction initTextures();
};
