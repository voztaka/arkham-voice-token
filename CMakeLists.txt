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
    src/TokenManager.cpp
    src/AudioPlayer.cpp
)

# Header files
set(HEADERS
    src/TokenManager.h
    src/AudioPlayer.h
)

# Add executable
add_executable(arkham_voice_token ${SOURCES} ${HEADERS})
target_link_options(arkham_voice_token PRIVATE "/SUBSYSTEM:CONSOLE")

# Link libraries for Windows
if(WIN32)
    target_link_libraries(arkham_voice_token PRIVATE winmm)
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
