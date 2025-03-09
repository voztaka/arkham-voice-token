#pragma once

#include <string>
#include <vector>
#include <map>
#include <random>
#include <optional>
#include <memory>

struct Difficulty {
    std::string id;
    std::string name;
    std::map<std::string, int> tokenCounts;
};

struct Scenario {
    std::string id;
    std::string name;
    std::vector<Difficulty> difficulties;
};

class TokenManager {
public:
    TokenManager();
    
    // Load scenarios from YAML
    bool loadScenariosFromYaml(const std::string& yamlPath);
    
    // Get available scenarios and difficulties
    const std::vector<Scenario>& getScenarios() const;
    
    // Set current scenario and difficulty
    bool setScenario(const std::string& scenarioId);
    bool setDifficulty(const std::string& difficultyId);
    
    // Get current scenario and difficulty names
    std::string getCurrentScenarioName() const;
    std::string getCurrentDifficultyName() const;
    
    // Token management
    std::optional<std::string> getRandomToken();
    int getTokenCount(const std::string& token) const;
    int getTokenUsageCount(const std::string& token) const;
    
    const std::map<std::string, int>& getAllTokenCounts() const;
    const std::map<std::string, int>& getAllTokenUsages() const;
    
    // File path utility
    static std::string getResourcePath(const std::string& filename);
    static std::string getConfigPath(const std::string& filename);
    
private:
    void resetTokenUsages();
    
    std::vector<Scenario> m_scenarios;
    int m_currentScenarioIndex = -1;
    int m_currentDifficultyIndex = -1;
    
    std::map<std::string, int> m_tokenUsages; // How many times each token was drawn
    std::mt19937 m_rng; // Random number generator
};
