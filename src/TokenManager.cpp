#include "TokenManager.h"
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <fstream>
#include <chrono>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <limits.h>
#endif

TokenManager::TokenManager() 
    : m_rng(std::chrono::steady_clock::now().time_since_epoch().count()) {
    // Try to load default scenario file
    loadScenariosFromYaml(getConfigPath("scenarios.yaml"));
}

bool TokenManager::loadScenariosFromYaml(const std::string& yamlPath) {
    try {
        // Check if file exists
        std::ifstream f(yamlPath.c_str());
        if (!f.good()) {
            std::cerr << "Config file not found: " << yamlPath << std::endl;
            return false;
        }
        
        // Load YAML file
        YAML::Node config = YAML::LoadFile(yamlPath);
        
        if (!config["scenarios"] || !config["scenarios"].IsSequence()) {
            std::cerr << "Invalid scenario configuration format" << std::endl;
            return false;
        }
        
        m_scenarios.clear();
        
        // Parse scenarios
        for (const auto& scenarioNode : config["scenarios"]) {
            Scenario scenario;
            scenario.id = scenarioNode["id"].as<std::string>();
            scenario.name = scenarioNode["name"].as<std::string>();
            
            // Parse difficulties
            for (const auto& diffNode : scenarioNode["difficulties"]) {
                Difficulty difficulty;
                difficulty.id = diffNode["id"].as<std::string>();
                difficulty.name = diffNode["name"].as<std::string>();
                
                // Parse tokens
                const auto& tokensNode = diffNode["tokens"];
                for (const auto& token : tokensNode) {
                    std::string tokenName = token.first.as<std::string>();
                    int tokenCount = token.second.as<int>();
                    difficulty.tokenCounts[tokenName] = tokenCount;
                }
                
                scenario.difficulties.push_back(difficulty);
            }
            
            m_scenarios.push_back(scenario);
        }
        
        resetTokenUsages();
        return true;
        
    } catch (const YAML::Exception& e) {
        std::cerr << "Error parsing YAML file: " << e.what() << std::endl;
        return false;
    }
}

const std::vector<Scenario>& TokenManager::getScenarios() const {
    return m_scenarios;
}

bool TokenManager::setScenario(const std::string& scenarioId) {
    for (size_t i = 0; i < m_scenarios.size(); i++) {
        if (m_scenarios[i].id == scenarioId) {
            m_currentScenarioIndex = i;
            m_currentDifficultyIndex = -1;
            resetTokenUsages();
            return true;
        }
    }
    return false;
}

bool TokenManager::setDifficulty(const std::string& difficultyId) {
    if (m_currentScenarioIndex < 0 || m_currentScenarioIndex >= static_cast<int>(m_scenarios.size())) {
        return false;
    }
    
    const auto& difficulties = m_scenarios[m_currentScenarioIndex].difficulties;
    for (size_t i = 0; i < difficulties.size(); i++) {
        if (difficulties[i].id == difficultyId) {
            m_currentDifficultyIndex = i;
            resetTokenUsages();
            return true;
        }
    }
    return false;
}

std::string TokenManager::getCurrentScenarioName() const {
    if (m_currentScenarioIndex >= 0 && m_currentScenarioIndex < static_cast<int>(m_scenarios.size())) {
        return m_scenarios[m_currentScenarioIndex].name;
    }
    return "No Scenario Selected";
}

std::string TokenManager::getCurrentDifficultyName() const {
    if (m_currentScenarioIndex >= 0 && m_currentScenarioIndex < static_cast<int>(m_scenarios.size()) &&
        m_currentDifficultyIndex >= 0 && m_currentDifficultyIndex < static_cast<int>(m_scenarios[m_currentScenarioIndex].difficulties.size())) {
        return m_scenarios[m_currentScenarioIndex].difficulties[m_currentDifficultyIndex].name;
    }
    return "No Difficulty Selected";
}

std::optional<std::string> TokenManager::getRandomToken() {
    if (m_currentScenarioIndex < 0 || m_currentScenarioIndex >= static_cast<int>(m_scenarios.size()) ||
        m_currentDifficultyIndex < 0 || m_currentDifficultyIndex >= static_cast<int>(m_scenarios[m_currentScenarioIndex].difficulties.size())) {
        return std::nullopt;
    }
    
    const auto& tokenCounts = m_scenarios[m_currentScenarioIndex].difficulties[m_currentDifficultyIndex].tokenCounts;
    
    // Count total available tokens
    int totalTokens = 0;
    for (const auto& [token, count] : tokenCounts) {
        totalTokens += count;
    }
    
    if (totalTokens <= 0) {
        return std::nullopt;
    }
    
    // Select a random token
    std::uniform_int_distribution<int> dist(1, totalTokens);
    int randomIndex = dist(m_rng);
    
    int currentIndex = 0;
    for (const auto& [token, count] : tokenCounts) {
        currentIndex += count;
        if (randomIndex <= currentIndex) {
            // Track usage
            m_tokenUsages[token]++;
            return token;
        }
    }
    
    return std::nullopt;
}

int TokenManager::getTokenCount(const std::string& token) const {
    if (m_currentScenarioIndex < 0 || m_currentScenarioIndex >= static_cast<int>(m_scenarios.size()) ||
        m_currentDifficultyIndex < 0 || m_currentDifficultyIndex >= static_cast<int>(m_scenarios[m_currentScenarioIndex].difficulties.size())) {
        return 0;
    }
    
    const auto& tokenCounts = m_scenarios[m_currentScenarioIndex].difficulties[m_currentDifficultyIndex].tokenCounts;
    auto it = tokenCounts.find(token);
    if (it != tokenCounts.end()) {
        return it->second;
    }
    return 0;
}

int TokenManager::getTokenUsageCount(const std::string& token) const {
    auto it = m_tokenUsages.find(token);
    if (it != m_tokenUsages.end()) {
        return it->second;
    }
    return 0;
}

const std::map<std::string, int>& TokenManager::getAllTokenCounts() const {
    static std::map<std::string, int> emptyMap;
    
    if (m_currentScenarioIndex < 0 || m_currentScenarioIndex >= static_cast<int>(m_scenarios.size()) ||
        m_currentDifficultyIndex < 0 || m_currentDifficultyIndex >= static_cast<int>(m_scenarios[m_currentScenarioIndex].difficulties.size())) {
        return emptyMap;
    }
    
    return m_scenarios[m_currentScenarioIndex].difficulties[m_currentDifficultyIndex].tokenCounts;
}

const std::map<std::string, int>& TokenManager::getAllTokenUsages() const {
    return m_tokenUsages;
}

void TokenManager::resetTokenUsages() {
    m_tokenUsages.clear();
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

std::string TokenManager::getConfigPath(const std::string& filename) {
#ifdef _WIN32
    char modulePath[MAX_PATH];
    GetModuleFileNameA(NULL, modulePath, MAX_PATH);
    std::string exePath(modulePath);
    std::string dirPath = exePath.substr(0, exePath.find_last_of("\\"));
    return dirPath + "\\config\\" + filename;
#else
    // Get Linux executable path
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    std::string execPath = std::string(result, (count > 0) ? count : 0);
    std::string dirPath = execPath.substr(0, execPath.find_last_of("/"));
    return dirPath + "/config/" + filename;
#endif
}
