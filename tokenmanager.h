#ifndef TOKENMANAGER_H
#define TOKENMANAGER_H

#include <string>
#include <map>
#include <vector>
#include <random>

#ifdef _WIN32
#include <windows.h>
#endif

class TokenManager {
public:
    TokenManager();

    void setTokenCount(const std::string& token, int count);
    int getTokenCount(const std::string& token) const;
    std::string getRandomToken();
    int getTokenUsageCount(const std::string& token) const;
    std::map<std::string, int> getAllTokenCounts() const;
    std::map<std::string, int> getAllTokenUsages() const;
    void saveTokenData() const;

private:
    std::map<std::string, int> m_tokenCounts;      // Stores the count of each token
    std::map<std::string, int> m_tokenUsages;      // Stores how many times each token was chosen
    std::mt19937 m_rng;                           // Random number generator

    void initializeTokens();
    void loadTokenData();
    private:
    std::string getResourcePath(const std::string& filename) const;
};

#endif // TOKENMANAGER_H