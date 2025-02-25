# Arkham Voice Token

A desktop application that provides voice feedback for Arkham Horror: The Card Game chaos tokens. This application enhances the gaming experience by playing corresponding sound effects when drawing chaos tokens.

## Features

- Voice feedback for all standard chaos tokens
- Support for the following tokens:
  - Elder Sign
  - Skull
  - Cultist
  - Tablet
  - Elder Thing
  - Tentacle
  - Numeric tokens (+1, 0, -1, -2)
- Clean and intuitive user interface

## Prerequisites

- CMake (3.0 or higher)
- C++ compiler with C++11 support

## Building from Source

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/arkham-voice-token.git
   cd arkham-voice-token
   ```

2. Create a build directory and navigate to it:
   ```bash
   mkdir build
   cd build
   ```

3. Generate build files with CMake:
   ```bash
   cmake ..
   ```

4. Build the project:
   - On Windows with Visual Studio:
     Open the generated solution file and build using Visual Studio
   - On Unix-like systems:
     ```bash
     make
     ```

## Usage

1. Launch the application
2. Click on the token buttons to play corresponding voice feedback
3. Enjoy enhanced immersion in your Arkham Horror: The Card Game sessions

## Project Structure

- `main.cpp` - Application entry point
- `mainwindow.cpp/h` - Main window UI implementation
- `tokenmanager.cpp/h` - Token management and audio playback logic
- `resources/` - Audio files for token voices

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Fantasy Flight Games for Arkham Horror: The Card Game