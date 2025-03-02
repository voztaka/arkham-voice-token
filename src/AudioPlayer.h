#pragma once

#include <string>
#include <memory>

class AudioPlayer {
public:
    AudioPlayer();
    ~AudioPlayer();
    
    bool playTokenSound(const std::string& tokenName);
    
private:
    std::string m_lastError;
};
