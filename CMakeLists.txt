cmake_minimum_required(VERSION 3.16)
project(arkham_voice_token VERSION 1.0.0 LANGUAGES CXX)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release)

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/TokenManager.cpp
    src/AudioPlayer.cpp
    src/SerialCommunicator.cpp
    src/Profile.cpp
    src/ProfileManager.cpp
)

# Header files
set(HEADERS
    src/MainWindow.h
    src/TokenManager.h
    src/AudioPlayer.h
    src/SerialCommunicator.h
    src/Constants.h
    src/Profile.h
    src/ProfileManager.h
)

# Add executable
add_executable(arkham_voice_token WIN32 ${SOURCES} ${HEADERS})

# Link libraries for Windows
if(WIN32)
    target_link_libraries(arkham_voice_token PRIVATE ole32 oleaut32 winmm comctl32)
endif()

# Copy resources to output directory
add_custom_command(
    TARGET arkham_voice_token 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
            "${CMAKE_SOURCE_DIR}/resources" 
            "$<TARGET_FILE_DIR:arkham_voice_token>/resources"
    COMMENT "Copying resources directory to output directory"
)
