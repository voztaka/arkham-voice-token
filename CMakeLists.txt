cmake_minimum_required(VERSION 3.16)

project(arkham_voice_token VERSION 1.0.0 LANGUAGES CXX)

# C++17 사용 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 실행 파일 출력 디렉토리 설정
# 기본적으로 빌드 폴더 내에 bin 디렉토리를 생성하도록 함
set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release)

# 실행 파일 추가
add_executable(arkham_voice_token
    main.cpp
    tokenmanager.cpp
    tokenmanager.h
)

if(WIN32)
    target_link_libraries(arkham_voice_token PRIVATE ole32 oleaut32 winmm)
    set_target_properties(arkham_voice_token PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# 실행 파일과 같은 폴더(출력 디렉토리)에 resources 폴더를 복사하는 POST_BUILD 커스텀 명령
add_custom_command(TARGET arkham_voice_token POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/resources"
    "$<TARGET_FILE_DIR:arkham_voice_token>/resources"
    COMMENT "Copying resources directory to output directory"
)
