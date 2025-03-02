#include "SerialCommunicator.h"
#include "Constants.h"
#include <sstream>

SerialCommunicator::SerialCommunicator(const std::string& portName, int baudRate)
    : m_portName(portName), 
      m_baudRate(baudRate),
      m_isRunning(false),
      m_shouldStop(false),
      m_isProcessing(false) {
}

SerialCommunicator::~SerialCommunicator() {
    stop();
}

bool SerialCommunicator::start(std::function<void()> onSignalCallback) {
    if (m_isRunning) {
        m_lastError = "Serial thread is already running";
        return false;
    }
    
    m_callback = std::move(onSignalCallback);
    m_shouldStop = false;
    m_isProcessing = false;
    
    try {
        m_thread = std::make_unique<std::thread>(&SerialCommunicator::threadFunction, this);
        m_isRunning = true;
        return true;
    }
    catch (const std::exception& e) {
        m_lastError = "Failed to start serial thread: ";
        m_lastError += e.what();
        return false;
    }
}

void SerialCommunicator::stop() {
    if (m_isRunning && m_thread) {
        m_shouldStop = true;
        if (m_thread->joinable()) {
            m_thread->join();
        }
        m_thread.reset();
        m_isRunning = false;
    }
}

bool SerialCommunicator::isRunning() const {
    return m_isRunning;
}

std::string SerialCommunicator::getLastError() const {
    return m_lastError;
}

void SerialCommunicator::threadFunction() {
    OutputDebugStringA("SerialListenerThread started.\n");
    
    // Open serial port
    HANDLE hSerial = CreateFileA(
        m_portName.c_str(),
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );
    
    if (hSerial == INVALID_HANDLE_VALUE) {
        m_lastError = "Failed to open COM port: " + m_portName;
        OutputDebugStringA(m_lastError.c_str());
        m_isRunning = false;
        return;
    }
    
    // Configure serial parameters
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        m_lastError = "Failed to get current serial parameters";
        OutputDebugStringA(m_lastError.c_str());
        CloseHandle(hSerial);
        m_isRunning = false;
        return;
    }
    
    dcbSerialParams.BaudRate = m_baudRate;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    
    if (!SetCommState(hSerial, &dcbSerialParams)) {
        m_lastError = "Could not set serial parameters";
        OutputDebugStringA(m_lastError.c_str());
        CloseHandle(hSerial);
        m_isRunning = false;
        return;
    }
    
    // Set timeouts
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    SetCommTimeouts(hSerial, &timeouts);
    
    char buffer[Constants::SERIAL_READ_BUFFER_SIZE];
    DWORD bytesRead;
    bool lastState = false;
    
    while (!m_shouldStop) {
        if (ReadFile(hSerial, buffer, sizeof(buffer)-1, &bytesRead, NULL)) {
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                
                // Debug output
                std::string debugMsg = "Serial data received: ";
                debugMsg += buffer;
                debugMsg += "\n";
                OutputDebugStringA(debugMsg.c_str());
                
                // Check if it's the signal we're looking for (1)
                bool currentState = (strstr(buffer, "1") != nullptr);
                
                // React to rising edge (low to high transition) and not currently processing
                if (currentState && !lastState && !m_isProcessing) {
                    OutputDebugStringA("HIGH signal detected, triggering callback\n");
                    m_isProcessing = true;
                    
                    // Call the provided callback function
                    if (m_callback) {
                        m_callback();
                    }
                    
                    // Set a delay before accepting new signals (debounce)
                    std::thread([this]() {
                        Sleep(Constants::TOKEN_PROCESSING_DELAY_MS);
                        m_isProcessing = false;
                        OutputDebugStringA("Processing completed\n");
                    }).detach();
                }
                
                lastState = currentState;
            }
        }
        else {
            OutputDebugStringA("ReadFile() failed.\n");
        }
        
        // Sleep briefly to avoid busy waiting
        Sleep(Constants::SERIAL_THREAD_SLEEP_MS);
    }
    
    OutputDebugStringA("SerialListenerThread ending.\n");
    CloseHandle(hSerial);
    m_isRunning = false;
}
