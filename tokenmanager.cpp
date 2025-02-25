#include "tokenmanager.h"
#include <algorithm>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#endif

TokenManager::TokenManager()
    : m_rng(std::chrono::steady_clock::now().time_since_epoch().count())
{
    loadTokenData();
}



std::string TokenManager::getResourcePath(const std::string& filename) const {
#ifdef _WIN32
    char modulePath[MAX_PATH];
    GetModuleFileNameA(NULL, modulePath, MAX_PATH);
    std::string exePath(modulePath);
    std::string dirPath = exePath.substr(0, exePath.find_last_of("\\"));
    return dirPath + "\\resources\\" + filename;
#else
    // Non-Windows implementation (not needed for your case)
    return "resources/" + filename;
#endif
}

void TokenManager::loadTokenData()
{
    initializeTokens();
}

void TokenManager::initializeTokens()
{
    std::vector<std::string> defaultTokens = {
        "skull", "cultist", "elder_thing", "tablet",
        "tentacle", "elder_sign", "-1", "0", "+1", "-2"
    };

    for (const auto& token : defaultTokens) {
        m_tokenCounts[token] = 0;
        m_tokenUsages[token] = 0;
    }
}

void TokenManager::saveTokenData() const
{
    // No longer needed as we're not saving to a file
}


void TokenManager::setTokenCount(const std::string& token, int count)
{
    if (m_tokenCounts.find(token) != m_tokenCounts.end()) {
        m_tokenCounts[token] = (std::max)(0, count);
        saveTokenData();
    }
}

int TokenManager::getTokenCount(const std::string& token) const
{
    auto it = m_tokenCounts.find(token);
    return (it != m_tokenCounts.end()) ? it->second : 0;
}

std::string TokenManager::getRandomToken()
{
    std::vector<std::string> availableTokens;
    
    for (const auto& pair : m_tokenCounts) {
        const std::string& token = pair.first;
        int count = pair.second;
        
        if (count > 0) {
            for (int i = 0; i < count; ++i) {
                availableTokens.push_back(token);
            }
        }
    }
    
    if (availableTokens.empty()) {
        return std::string();
    }
    
    for (int i = static_cast<int>(availableTokens.size()) - 1; i > 0; --i) {
        std::uniform_int_distribution<> dist(0, i);
        int j = dist(m_rng);
        std::swap(availableTokens[i], availableTokens[j]);
    }
    
    std::string selectedToken = availableTokens[0];
    ++m_tokenUsages[selectedToken];
    saveTokenData();
    
    return selectedToken;
}


int TokenManager::getTokenUsageCount(const std::string& token) const
{
    auto it = m_tokenUsages.find(token);
    return (it != m_tokenUsages.end()) ? it->second : 0;
}

std::map<std::string, int> TokenManager::getAllTokenCounts() const
{
    return m_tokenCounts;
}

std::map<std::string, int> TokenManager::getAllTokenUsages() const
{
    return m_tokenUsages;
}