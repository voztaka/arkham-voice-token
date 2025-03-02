#pragma once

#include <string>
#include <map>
#include <vector>
#include <random>
#include <optional>
#include <memory>

class TokenManager {
public:
    TokenManager();
    
    void setTokenCount(const std::string& token, int count);
    int getTokenCount(const std::string& token) const;
    std::optional<std::string> getRandomToken();
    int getTokenUsageCount(const std::string& token) const;
    
    const std::map<std::string, int>& getAllTokenCounts() const;
    const std::map<std::string, int>& getAllTokenUsages() const;
    
    // Utility functions
    static std::string getResourcePath(const std::string& filename);

private:
    void initializeTokens();
    
    std::map<std::string, int> m_tokenCounts; // Stores the count of each token
    std::map<std::string, int> m_tokenUsages; // Stores how many times each token was chosen
    std::mt19937 m_rng; // Random number generator
};
