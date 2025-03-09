#include "TokenManager.h"
#include "AudioPlayer.h"
#include <iostream>
#include <string>
#include <algorithm>

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
        std::cout << "\nToken Usage Statistics:\n";
        for (const auto& [token, usage] : tokenManager.getAllTokenUsages()) {
            std::cout << "  " << token << ": " << usage << " draws\n";
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
