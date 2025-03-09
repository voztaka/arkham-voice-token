#include "TokenManager.h"
#include "AudioPlayer.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <iomanip>

class CLIInterface {
    TokenManager tokenManager;
    AudioPlayer audioPlayer;

    void printHelp() {
        std::cout << "\nArkham Voice Token CLI\n"
                << "Commands:\n"
                << "  draw                   - Draw a random token\n"
                << "  scenarios              - List available scenarios\n"
                << "  scenario <scenario_id> - Select a scenario\n"
                << "  difficulties           - List difficulties for current scenario\n"
                << "  difficulty <diff_id>   - Select a difficulty\n"
                << "  list                   - Show all token counts\n"
                << "  stats                  - Show usage statistics\n"
                << "  help                   - Show this help\n"
                << "  exit                   - Exit program\n\n";
    }

    void printScenarios() {
        std::cout << "\nAvailable Scenarios:\n";
        const auto& scenarios = tokenManager.getScenarios();
        for (const auto& scenario : scenarios) {
            std::cout << "  " << scenario.id << " - " << scenario.name << "\n";
        }
    }

    void printDifficulties() {
        std::cout << "\nAvailable Difficulties:\n";
        const auto& scenarios = tokenManager.getScenarios();
        for (const auto& scenario : scenarios) {
            if (scenario.name == tokenManager.getCurrentScenarioName()) {
                for (const auto& difficulty : scenario.difficulties) {
                    std::cout << "  " << difficulty.id << " - " << difficulty.name << "\n";
                }
                break;
            }
        }
    }

    void handleScenario(const std::string& scenarioId) {
        if (tokenManager.setScenario(scenarioId)) {
            std::cout << "Selected scenario: " << tokenManager.getCurrentScenarioName() << "\n";
        } else {
            std::cout << "Invalid scenario ID. Use 'scenarios' to see available options.\n";
        }
    }

    void handleDifficulty(const std::string& difficultyId) {
        if (tokenManager.setDifficulty(difficultyId)) {
            std::cout << "Selected difficulty: " << tokenManager.getCurrentDifficultyName() << "\n";
        } else {
            std::cout << "Invalid difficulty ID. Use 'difficulties' to see available options.\n";
        }
    }    

    void handleDraw() {
        auto token = tokenManager.getRandomToken();
        if (token) {
            std::cout << "Drawn token: " << *token << "\n";
            if (audioPlayer.playTokenSound(*token)) {
                std::cout << "Playing sound for " << *token << "\n";
            } else {
                std::cout << "Error playing sound: " << audioPlayer.getLastError() << "\n";
            }
        } else {
            std::cout << "No tokens available!\n";
        }
    }

    void handleList() {
        std::cout << "\nToken Inventory:\n";
        for (const auto& [token, count] : tokenManager.getAllTokenCounts()) {
            std::cout << "  " << token << ": " << count << "\n";
        }
    }

    void handleStats() {
        std::cout << "\nToken Probability Statistics:\n";
    
        // Get current token counts and usage statistics
        const auto& tokenCounts = tokenManager.getAllTokenCounts();
        const auto& tokenUsages = tokenManager.getAllTokenUsages();
    
        // Calculate weights for each token
        std::map<std::string, double> tokenWeights;
        double totalWeight = 0.0;
    
        for (const auto& [token, count] : tokenCounts) {
            // Find usage count (default to 0 if not found)
            int usageCount = 0;
            auto it = tokenUsages.find(token);
            if (it != tokenUsages.end()) {
                usageCount = it->second;
            }
        
            // Calculate weight: original count / (1 + times drawn)
            double weight = static_cast<double>(count) / (1.0 + usageCount);
            tokenWeights[token] = weight;
            totalWeight += weight;
        }
    
        // Display probabilities for each token
        for (const auto& [token, weight] : tokenWeights) {
            // Calculate probability as weight / total weight
            double probability = (totalWeight > 0) ? (weight / totalWeight) * 100.0 : 0.0;
        
            int usageCount = 0;
            auto it = tokenUsages.find(token);
            if (it != tokenUsages.end()) {
                usageCount = it->second;
            }
        
            std::cout << "  " << token << ": " 
                    << std::fixed << std::setprecision(2) << probability << "% "
                    << "(Count: " << tokenCounts.at(token) 
                    << ", Drawn: " << usageCount << ")\n";
        }
    }

public:
    void run() {
        std::cout << "Arkham Horror Voice Token CLI\n";
        printHelp();
    
        std::cout << "Current scenario: " << tokenManager.getCurrentScenarioName() << "\n";
        std::cout << "Current difficulty: " << tokenManager.getCurrentDifficultyName() << "\n";
    
        std::string input;
        while (true) {
            std::cout << "> ";
            std::getline(std::cin, input);
        
            // Parse command and arguments
            std::string command;
            std::string arg;
        
            size_t spacePos = input.find(' ');
            if (spacePos != std::string::npos) {
                command = input.substr(0, spacePos);
                arg = input.substr(spacePos + 1);
            } else {
                command = input;
            }
        
            // Convert command to lowercase for case-insensitive comparison
            std::transform(command.begin(), command.end(), command.begin(), ::tolower);
        
            if (command == "exit") {
                break;
            } else if (command == "draw") {
                handleDraw();
            } else if (command == "list") {
                handleList();
            } else if (command == "stats") {
                handleStats();
            } else if (command == "scenarios") {
                printScenarios();
            } else if (command == "scenario") {
                handleScenario(arg);
            } else if (command == "difficulties") {
                printDifficulties();
            } else if (command == "difficulty") {
                handleDifficulty(arg);
            } else if (command == "help") {
                printHelp();
            } else {
                std::cout << "Unknown command. Type 'help' for a list of commands.\n";
            }
        }
    }
};

int main() {
    CLIInterface cli;
    cli.run();
    return 0;
}
