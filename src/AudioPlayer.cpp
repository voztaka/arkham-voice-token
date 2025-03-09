#include <fstream>

#include "AudioPlayer.h"
#include "TokenManager.h"

#ifdef _WIN32
// Windows implementation
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#else
// Linux implementation
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#endif

AudioPlayer::AudioPlayer() {
#ifndef _WIN32
    // Initialize SDL and SDL_mixer on Linux
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        m_lastError = "SDL initialization failed: " + std::string(SDL_GetError());
        m_initialized = false;
        return;
    }
    
    // Initialize SDL_mixer with common audio format settings
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        m_lastError = "SDL_mixer initialization failed: " + std::string(Mix_GetError());
        m_initialized = false;
        return;
    }
    
    m_initialized = true;
#else
    m_initialized = true;
#endif
}

AudioPlayer::~AudioPlayer() {
#ifndef _WIN32
    if (m_initialized) {
        Mix_CloseAudio();
        SDL_Quit();
    }
#endif
}

bool AudioPlayer::playTokenSound(const std::string& tokenName) {
    if (tokenName.empty()) {
        m_lastError = "Invalid token name";
        return false;
    }
    
    if (!m_initialized) {
        m_lastError = "Audio player not initialized";
        return false;
    }
    
    std::string audioPath = TokenManager::getResourcePath(tokenName + ".mp3");
    std::ifstream f(audioPath.c_str());
    if (!f.good()) {
        m_lastError = "File does not exist at path: " + audioPath;
        return false;
    }

#ifdef _WIN32
    // Windows implementation using MCI
    std::string openCmd = "open \"" + audioPath + "\" type mpegvideo alias tokenSound";
    
    DWORD error = mciSendStringA(openCmd.c_str(), NULL, 0, NULL);
    if (error) {
        char errorText[256];
        mciGetErrorStringA(error, errorText, sizeof(errorText));
        m_lastError = errorText;
        return false;
    }
    
    error = mciSendStringA("play tokenSound wait", NULL, 0, NULL);
    if (error) {
        char errorText[256];
        mciGetErrorStringA(error, errorText, sizeof(errorText));
        m_lastError = errorText;
        mciSendStringA("close tokenSound", NULL, 0, NULL);
        return false;
    }
    
    mciSendStringA("close tokenSound", NULL, 0, NULL);
    return true;
#else
    // Linux implementation using SDL_mixer
    Mix_Music* music = Mix_LoadMUS(audioPath.c_str());
    if (!music) {
        m_lastError = "Failed to load sound file: " + std::string(Mix_GetError());
        return false;
    }
    
    if (Mix_PlayMusic(music, 1) == -1) {
        m_lastError = "Failed to play sound: " + std::string(Mix_GetError());
        Mix_FreeMusic(music);
        return false;
    }
    
    // Wait until the music finishes playing
    while (Mix_PlayingMusic()) {
        SDL_Delay(100);
    }
    
    Mix_FreeMusic(music);
    return true;
#endif
}
