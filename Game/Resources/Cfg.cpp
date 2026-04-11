#include "pch.h"
#include "Cfg.h"
#include "../Engine/Sprite.h"
#include <chrono>
#include <algorithm>

using winrt::Windows::Foundation::Uri;

std::unordered_map<Cfg::Textures, std::wstring> Cfg::textures = {};
std::unordered_map<Cfg::Sounds, std::wstring> Cfg::sounds = {};
std::unordered_map<Cfg::Music, std::wstring> Cfg::music = {};
std::unordered_map<Cfg::Fonts, std::wstring> Cfg::fonts = {};




winrt::Windows::Foundation::IAsyncAction Cfg::InitializeAsync(winrt::Microsoft::Graphics::Canvas::UI::Xaml::CanvasAnimatedControl const& sender)
{
    // Textures

    engine::Sprite::SetResourceCreator(sender);
    engine::Sprite::ClearCache();
    co_await initTextures();

    engine::FontManager::Instance().Clear();
    initFonts();

    initSounds();
    initMusic();

    co_await engine::SoundManager::Instance().InitializeAsync();


    co_return;
}

void Cfg::initMusic()
{
    engine::SoundManager::Instance().RegisterMusic(L"theme", Uri(L"ms-appx:///Assets/Audio/theme.mp3"));
    music.emplace(Music::Theme1, L"theme");
}

void Cfg::initSounds()
{
    engine::SoundManager::Instance().RegisterSfx(L"blip", Uri(L"ms-appx:///Assets/Audio/buster_shot.wav"));
    sounds.emplace(Sounds::Blip, L"blip");

    // Placeholder hit SFX (using blip.wav for now). Swap the wavs later.
    engine::SoundManager::Instance().RegisterSfx(L"player_hit", Uri(L"ms-appx:///Assets/Audio/hurt.wav"));
    sounds.emplace(Sounds::PlayerHit, L"player_hit");

    engine::SoundManager::Instance().RegisterSfx(L"enemy_hit", Uri(L"ms-appx:///Assets/Audio/small_hit.wav"));
    sounds.emplace(Sounds::EnemyHit, L"enemy_hit");

    

        engine::SoundManager::Instance().RegisterSfx(L"electric_shot", Uri(L"ms-appx:///Assets/Audio/electric_shot.wav"));
    sounds.emplace(Sounds::ElectricShot, L"electric_shot");

    // Placeholder hit SFX (using blip.wav for now). Swap the wavs later.
    engine::SoundManager::Instance().RegisterSfx(L"missile_launch", Uri(L"ms-appx:///Assets/Audio/missile_launch.wav"));
    sounds.emplace(Sounds::MissileLaunch, L"missile_launch");

    engine::SoundManager::Instance().RegisterSfx(L"enemy_die", Uri(L"ms-appx:///Assets/Audio/enemy_die.wav"));
    sounds.emplace(Sounds::EnemyDie, L"enemy_die");
}

winrt::Windows::Foundation::IAsyncAction Cfg::initTextures()
{
    co_await engine::Sprite::RegisterAndLoadAsync(
        L"ship",
        Uri(L"ms-appx:///Assets/Textures/Characters/Player/ship.png"));
    textures.emplace(Textures::Ship, L"ship");

    co_await engine::Sprite::RegisterAndLoadAsync(
        L"PlayerAtlas",
        Uri(L"ms-appx:///Assets/Textures/Characters/Player/player_atlas.png"));
    textures.emplace(Textures::PlayerAtlas, L"PlayerAtlas");

    co_await engine::Sprite::RegisterAndLoadAsync(
        L"Tileset1",
        Uri(L"ms-appx:///Assets/Textures/Tilesets/tileset2.png"));
    textures.emplace(Textures::Tileset1, L"Tileset1");

    co_await engine::Sprite::RegisterAndLoadAsync(
        L"under",
        Uri(L"ms-appx:///Assets/Textures/Misc/under.png"));
    textures.emplace(Textures::Under, L"under");

    co_await engine::Sprite::RegisterAndLoadAsync(
        L"WhitePixel",
        Uri(L"ms-appx:///Assets/Textures/Misc/white_pixel.png"));
    textures.emplace(Textures::WhitePixel, L"WhitePixel");

    co_await engine::Sprite::RegisterAndLoadAsync(
        L"BusterShot",
        Uri(L"ms-appx:///Assets/Textures/Characters/Player/buster_shot_21x26.png"));
    textures.emplace(Textures::BusterShot, L"BusterShot");

    co_await engine::Sprite::RegisterAndLoadAsync(
        L"BlueyAtlas",
        Uri(L"ms-appx:///Assets/Textures/Characters/Enemies/Bluey102x127_Weapons102X67_5.png"));
    textures.emplace(Textures::BlueyAtlas, L"BlueyAtlas");

    co_await engine::Sprite::RegisterAndLoadAsync(
        L"ShellyAtlas",
        Uri(L"ms-appx:///Assets/Textures/Characters/Enemies/shelly.png"));
    textures.emplace(Textures::ShellyAtlas, L"ShellyAtlas");

    co_return;
}

std::shared_ptr<engine::Font> Cfg::GetFont(std::wstring fontKey_)
{
    return engine::FontManager::Instance().Get(fontKey_);
}

std::shared_ptr<engine::Font> Cfg::GetFont(Fonts f_)
{
    auto it = fonts.find(f_);
    if (it == fonts.end()) return nullptr;
    return engine::FontManager::Instance().Get(it->second);
}
std::wstring Cfg::GetTexKey(std::wstring const& tex_)
{
    return tex_;
}

std::wstring Cfg::GetTexKey(Textures tex_)
{
    auto it = textures.find(tex_);
    if (it == textures.end())
    {
        return L"";
    }
    return it->second;
}

void Cfg::PlaySfx(std::wstring snd_, float volume_)
{
    engine::SoundManager::Instance().PlaySfx(snd_, volume_);
}

void Cfg::PlaySfx(Sounds snd_, float volume_)
{
    auto it = sounds.find(snd_);
    if (it == sounds.end()) return;
    engine::SoundManager::Instance().PlaySfx(it->second, volume_);
}

winrt::Windows::Foundation::IAsyncAction Cfg::PlayMusicAsync(std::wstring snd_, bool loop_, float volume_)
{
    co_await engine::SoundManager::Instance().PlayMusicAsync(snd_, loop_, volume_);

    co_return;
}

winrt::Windows::Foundation::IAsyncAction Cfg::PlayMusicAsync(Music snd_, bool loop_, float volume_)
{
    auto it = music.find(snd_);
    if (it == music.end()) co_return;
    co_await engine::SoundManager::Instance().PlayMusicAsync(it->second, loop_, volume_);


    co_return;
}

void Cfg::initFonts()
{
    engine::FontManager::Instance().RegisterSystem(L"ui", L"Segoe UI");
    fonts.emplace(Fonts::UI, L"ui");

    // Example packaged font:
    engine::FontManager::Instance().RegisterAppFont(
        L"default",
        Uri(L"ms-appx:///Assets/Fonts/bubbly.ttf"),
        L"Spicy Sale");
    fonts.emplace(Fonts::Default, L"default");

    engine::FontManager::Instance().RegisterAppFont(
        L"bubbly",
        Uri(L"ms-appx:///Assets/Fonts/bubbly.ttf"),
        L"Spicy Sale");
    fonts.emplace(Fonts::Bubbly, L"bubbly");
}




