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
}
