#define NOMINMAX  
#include "TokenManager.h"
#include <algorithm>
#include <chrono>

#ifdef _WIN32
    #include <windows.h>
#else 
    #include <unistd.h>
    #include <limits.h>
#endif

TokenManager::TokenManager() 
    : m_rng(std::chrono::steady_clock::now().time_since_epoch().count()) {
    initializeTokens();
}

std::string TokenManager::getResourcePath(const std::string& filename) {
#ifdef _WIN32
    char modulePath[MAX_PATH];
    GetModuleFileNameA(NULL, modulePath, MAX_PATH);
    std::string exePath(modulePath);
    std::string dirPath = exePath.substr(0, exePath.find_last_of("\\"));
    return dirPath + "\\resources\\" + filename;
#else
    // Get Linux executable path
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    std::string execPath = std::string(result, (count > 0) ? count : 0);
    std::string dirPath = execPath.substr(0, execPath.find_last_of("/"));
    return dirPath + "/resources/" + filename;
#endif
}

void TokenManager::initializeTokens() {
    std::vector<std::string> defaultTokens = {
        "skull", "cultist", "elder_thing", "tablet", "tentacle", 
        "elder_sign", "-1", "0", "+1", "-2", "-3", "-4"
    };
    
    for (const auto& token : defaultTokens) {
        m_tokenCounts[token] = 0;
        m_tokenUsages[token] = 0;
    }
}

void TokenManager::setTokenCount(const std::string& token, int count) {
    if (m_tokenCounts.find(token) != m_tokenCounts.end()) {
        m_tokenCounts[token] = std::max(0, count);
    }
}

int TokenManager::getTokenCount(const std::string& token) const {
    auto it = m_tokenCounts.find(token);
    return (it != m_tokenCounts.end()) ? it->second : 0;
}

std::optional<std::string> TokenManager::getRandomToken() {
    std::vector<std::string> availableTokens;
    
    // Build the pool of available tokens
    for (const auto& [token, count] : m_tokenCounts) {
        if (count > 0) {
            for (int i = 0; i < count; ++i) {
                availableTokens.push_back(token);
            }
        }
    }
    
    if (availableTokens.empty()) {
        return std::nullopt;
    }
    
    // Shuffle using Fisher-Yates algorithm
    for (int i = static_cast<int>(availableTokens.size()) - 1; i > 0; --i) {
        std::uniform_int_distribution<> dist(0, i);
        int j = dist(m_rng);
        std::swap(availableTokens[i], availableTokens[j]);
    }
    
    std::string selectedToken = availableTokens[0];
    ++m_tokenUsages[selectedToken];
    
    return selectedToken;
}

int TokenManager::getTokenUsageCount(const std::string& token) const {
    auto it = m_tokenUsages.find(token);
    return (it != m_tokenUsages.end()) ? it->second : 0;
}

const std::map<std::string, int>& TokenManager::getAllTokenCounts() const {
    return m_tokenCounts;
}

const std::map<std::string, int>& TokenManager::getAllTokenUsages() const {
    return m_tokenUsages;
}
