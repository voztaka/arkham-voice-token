#include "AudioPlayer.h"
#include "TokenManager.h"
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

AudioPlayer::AudioPlayer() = default;
AudioPlayer::~AudioPlayer() = default;

bool AudioPlayer::playTokenSound(const std::string& tokenName) {
    if (tokenName.empty()) {
        m_lastError = "Invalid token name";
        return false;
    }
    
    std::string audioPath = TokenManager::getResourcePath(tokenName + ".mp3");
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
}
