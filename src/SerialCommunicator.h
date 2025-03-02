#pragma once

#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <memory>
#include <windows.h>

class SerialCommunicator {
public:
    SerialCommunicator(const std::string& portName = "COM5", int baudRate = 9600);
    ~SerialCommunicator();
    
    bool start(std::function<void()> onSignalCallback);
    void stop();
    
    bool isRunning() const;
    std::string getLastError() const;
    
private:
    void threadFunction();
    
    std::string m_portName;
    int m_baudRate;
    std::atomic<bool> m_isRunning;
    std::atomic<bool> m_shouldStop;
    std::atomic<bool> m_isProcessing;
    std::string m_lastError;
    std::unique_ptr<std::thread> m_thread;
    std::function<void()> m_callback;
};
