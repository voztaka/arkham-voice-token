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
                  << "  draw       - Draw a random token\n"
                  << "  set <token> <count> - Set token quantity\n"
                  << "  list       - Show all token counts\n"
                  << "  stats      - Show usage statistics\n"
                  << "  help       - Show this help\n"
                  << "  exit       - Exit program\n\n";
    }

    void handleDraw() {
        auto token = tokenManager.getRandomToken();
        if (token) {
            std::cout << "Drawn token: " << *token << "\n";
            if (audioPlayer.playTokenSound(*token)) {
                std::cout << "Playing sound for " << *token << "\n";
            } else {
                std::cout << "Error playing sound\n";
            }
        } else {
            std::cout << "No tokens available!\n";
        }
    }

    void handleSet(const std::string& token, int count) {
        tokenManager.setTokenCount(token, count);
        std::cout << "Set " << token << " count to " << count << "\n";
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

        std::string input;
        while (true) {
            std::cout << "> ";
            std::getline(std::cin, input);

            std::transform(input.begin(), input.end(), input.begin(), ::tolower);
            if (input == "exit") {
                break;
            } else if (input == "draw") {
                handleDraw();
            } else if (input == "list") {
                handleList();
            } else if (input == "stats") {
                handleStats();
            } else if (input == "help") {
                printHelp();
            } else if (input.rfind("set ", 0) == 0) {
                std::string token;
                int count;
                size_t space = input.find(' ', 4);
                if (space != std::string::npos) {
                    token = input.substr(4, space - 4);
                    try {
                        count = std::stoi(input.substr(space + 1));
                        handleSet(token, count);
                    } catch (...) {
                        std::cout << "Invalid count value\n";
                    }
                } else {
                    std::cout << "Usage: set <token> <count>\n";
                }
            } else {
                std::cout << "Unknown command. Type 'help' for available commands.\n";
            }
        }
    }
};

int main() {
    CLIInterface cli;
    cli.run();
    return 0;
}
