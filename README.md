# Arkham Voice Token CLI

A command-line application that provides voice feedback for Arkham Horror: The Card Game chaos tokens. This version maintains core functionality while removing graphical interface dependencies.

## Features

- Voice feedback for all standard chaos tokens
- Console-based interface with command input
- Supported tokens:
  - Elder Sign
  - Skull
  - Cultist
  - Tablet
  - Elder Thing
  - Tentacle
  - Numeric tokens (+1, 0, -1, -2, -3, -4)

## Installation

1. Ensure you have build-essential/cmake and Windows build tools installed
2. Clone the repository:
```bash
git clone https://github.com/yourusername/arkham-voice-token.git
cd arkham-voice-token
```
3. Configure with CMake:
```bash
mkdir build && cd build
cmake ..
```
4. Build the executable:
```bash
cmake --build .
```

## Usage

Run the compiled executable and use these commands:
```
draw       - Draw random token
set <token> <count> - Set token quantity
list       - Show token counts
stats      - Show usage statistics
help       - Show command help
exit       - Quit program
```

## Project Structure

- `main.cpp` - CLI entry point and command processor
- `tokenmanager.cpp/h` - Token management core logic
- `audioplayer.cpp/h` - Windows audio playback implementation
- `resources/` - Token sound files

## License

MIT License - See LICENSE file for details.