#pragma once

#include <string>
#include <memory>

class AudioPlayer {
public:
    AudioPlayer();
    ~AudioPlayer();
    
    bool playTokenSound(const std::string& tokenName);
    const std::string& getLastError() const { return m_lastError; }
    
private:
    std::string m_lastError;
    bool m_initialized = false;
};
