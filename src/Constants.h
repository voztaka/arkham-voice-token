#pragma once

namespace Constants {
    // Window control IDs
    constexpr int ID_DRAW_BUTTON = 1001;
    constexpr int ID_STATS_EDIT = 1002;
    constexpr int ID_TOKEN_START = 2000;
    
    // Default serial port
    constexpr char DEFAULT_COM_PORT[] = "COM5";
    constexpr int DEFAULT_BAUD_RATE = 9600;
    
    // Other constants
    constexpr int TOKEN_PROCESSING_DELAY_MS = 3000;
    constexpr int SERIAL_READ_BUFFER_SIZE = 64;
    constexpr int SERIAL_THREAD_SLEEP_MS = 10;
    constexpr int ID_COM_PORT_COMBO = 1003;
    constexpr int ID_REFRESH_PORTS_BUTTON = 1004;

       // 프로필 관리 ID
    constexpr int ID_ADD_PROFILE_BUTTON = 3000;
    constexpr int ID_PROFILE_TAB = 3001;
    
    // 프로필별 컨트롤 ID (프로필 인덱스 오프셋과 함께 사용)
    constexpr int ID_PROFILE_NAME_EDIT = 4000;
    constexpr int ID_MAX_ACTIONS_EDIT = 4100;
    constexpr int ID_CURRENT_ACTIONS_EDIT = 4200;
    constexpr int ID_ACTIONS_UP_BUTTON = 4300;
    constexpr int ID_ACTIONS_DOWN_BUTTON = 4400;
    constexpr int ID_ACTIONS_RESET_BUTTON = 4500;
    constexpr int ID_DAMAGE_EDIT = 4600;
    constexpr int ID_DAMAGE_UP_BUTTON = 4700;
    constexpr int ID_DAMAGE_DOWN_BUTTON = 4800;
    constexpr int ID_HORROR_EDIT = 4900;
    constexpr int ID_HORROR_UP_BUTTON = 5000;
    constexpr int ID_HORROR_DOWN_BUTTON = 5100;
    constexpr int ID_CLUES_EDIT = 5200;
    constexpr int ID_CLUES_UP_BUTTON = 5300;
    constexpr int ID_CLUES_DOWN_BUTTON = 5400;
    constexpr int ID_RESOURCES_EDIT = 5500;
    constexpr int ID_RESOURCES_UP_BUTTON = 5600;
    constexpr int ID_RESOURCES_DOWN_BUTTON = 5700;
}
