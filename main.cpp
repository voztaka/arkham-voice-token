#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <memory>
#include <thread>
#include <sstream>
#include "tokenmanager.h"

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comctl32.lib")

// Window control IDs
#define ID_DRAW_BUTTON 1001
#define ID_STATS_EDIT  1002
#define ID_TOKEN_START 2000

// Global variables
HWND g_hwnd;
TokenManager* g_tokenManager;
std::vector<HWND> g_tokenControls;
volatile bool g_runSerialThread = true;

// Forward declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitializeControls(HWND hwnd);
void UpdateStatistics(HWND hwnd);
void DrawToken(HWND hwnd);

DWORD WINAPI SerialListenerThread(LPVOID lpParam)
{
    HWND hwnd = (HWND)lpParam;
    OutputDebugStringA("SerialListenerThread started.\n");

    // Adjust the COM port name as needed.
    HANDLE hSerial = CreateFileA("COM5",
                                 GENERIC_READ,
                                 0,
                                 NULL,
                                 OPEN_EXISTING,
                                 0,
                                 NULL);
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        OutputDebugStringA("Failed to open COM.\n");
        MessageBoxA(hwnd, "Failed to open COM", "Serial Port Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // Configure serial parameters
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        OutputDebugStringA("Failed to get current serial parameters.\n");
        MessageBoxA(hwnd, "Failed to get current serial parameters", "Serial Port Error", MB_OK | MB_ICONERROR);
        CloseHandle(hSerial);
        return 1;
    }
    
    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity   = NOPARITY;
    if (!SetCommState(hSerial, &dcbSerialParams))
    {
        OutputDebugStringA("Could not set serial parameters.\n");
        MessageBoxA(hwnd, "Could not set serial parameters", "Serial Port Error", MB_OK | MB_ICONERROR);
        CloseHandle(hSerial);
        return 1;
    }
    
    // Set timeouts
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout         = 50;
    timeouts.ReadTotalTimeoutConstant    = 50;
    timeouts.ReadTotalTimeoutMultiplier  = 10;
    SetCommTimeouts(hSerial, &timeouts);
    
    char buffer[64];
    DWORD bytesRead;

    bool lastState = false;
    volatile bool isProcessing = false;

    while (g_runSerialThread)
    {
        if (ReadFile(hSerial, buffer, sizeof(buffer)-1, &bytesRead, NULL))
        {
            if (bytesRead > 0)
            {
                buffer[bytesRead] = '\0';
                // 디버그로 읽어온 문자열 출력
                std::string debugMsg = "Serial data received: ";
                debugMsg += buffer;
                debugMsg += "\n";
                OutputDebugStringA(debugMsg.c_str());

                bool currentState = (strstr(buffer, "1") != nullptr);

                // Check if the Arduino sent a signal (adjust the protocol as needed)
                if (currentState && !lastState && !isProcessing)
                {
                    OutputDebugStringA("HIGH signal detected, posting WM_COMMAND\n");

                    isProcessing = true;
                    // Post a WM_COMMAND message to simulate clicking the Draw Token button
                    PostMessage(hwnd, WM_COMMAND, ID_DRAW_BUTTON, 0);

                    std::thread([&isProcessing](){
                        Sleep(3000);
                        isProcessing = false;
                        OutputDebugStringA("Processing completed");
                    }).detach();
                }

                lastState = currentState;
            }
        }
        else
        {
            OutputDebugStringA("ReadFile() failed.\n");
        }
        // Sleep briefly to avoid busy waiting
        Sleep(10);
    }
    OutputDebugStringA("SerialListenerThread ending.\n");
    CloseHandle(hSerial);
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Initialize Common Controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_UPDOWN_CLASS | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    // Register window class
    const wchar_t CLASS_NAME[] = L"ArkhamTokenWindow";
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    // Create window
    g_hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Arkham Horror Token Manager",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL,
        NULL,
        hInstance,
        NULL
    );
    if (g_hwnd == NULL) {
        MessageBoxW(NULL, L"Failed to create window", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Initialize TokenManager
    g_tokenManager = new TokenManager();

    // Create controls
    InitializeControls(g_hwnd);

    // Start the serial listener thread (for Arduino communication)
    HANDLE hSerialThread = CreateThread(NULL, 0, SerialListenerThread, g_hwnd, 0, NULL);
    if (hSerialThread == NULL)
    {
        MessageBoxW(g_hwnd, L"Failed to start serial thread", L"Error", MB_OK | MB_ICONERROR);
    }
    else
    {
        OutputDebugStringA("Serial listener thread successfully created.\n");
    }

    // Show window
    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);

    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup: stop serial thread
    g_runSerialThread = false;
    if (hSerialThread)
    {
        WaitForSingleObject(hSerialThread, 2000);
        CloseHandle(hSerialThread);
    }
    delete g_tokenManager;
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == ID_DRAW_BUTTON) {
                OutputDebugStringA("WM_COMMAND: Draw Token button pressed.\n");
                DrawToken(hwnd);
                return 0;
            } else if (LOWORD(wParam) >= ID_TOKEN_START) {
                int tokenIndex = LOWORD(wParam) - ID_TOKEN_START;
                if (tokenIndex >= 0 && tokenIndex < g_tokenControls.size()) {
                    wchar_t buffer[16];
                    GetWindowTextW(g_tokenControls[tokenIndex], buffer, 16);
                    int value = _wtoi(buffer);
                    const auto& tokens = g_tokenManager->getAllTokenCounts();
                    auto it = tokens.begin();
                    std::advance(it, tokenIndex);
                    g_tokenManager->setTokenCount(it->first, value);
                    std::string debugMsg = "WM_COMMAND: Updated token '";
                    debugMsg += it->first;
                    debugMsg += "' to ";
                    debugMsg += std::to_string(value);
                    debugMsg += "\n";
                    OutputDebugStringA(debugMsg.c_str());
                }
                return 0;
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void InitializeControls(HWND hwnd) {
    // Create Draw Token button
    CreateWindowW(
        L"BUTTON",
        L"Draw Token",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        10, 10, 100, 30,
        hwnd,
        (HMENU)ID_DRAW_BUTTON,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        NULL
    );

    // Create token count controls
    const auto& tokens = g_tokenManager->getAllTokenCounts();
    int y = 50;
    int id = ID_TOKEN_START;
    for (const auto& [token, count] : tokens) {
        std::wstring wtoken(token.begin(), token.end());
        // Create label
        CreateWindowW(
            L"STATIC",
            wtoken.c_str(),
            WS_VISIBLE | WS_CHILD,
            10, y, 100, 20,
            hwnd,
            NULL,
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
            NULL
        );
        // Create up-down control for count
        HWND hEdit = CreateWindowW(
            L"EDIT",
            L"0",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
            120, y, 50, 20,
            hwnd,
            (HMENU)id,
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
            NULL
        );
        // Create the up-down buddy control
        HWND hUpDown = CreateWindowW(
            UPDOWN_CLASSW,
            NULL,
            WS_VISIBLE | WS_CHILD | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_NOTHOUSANDS,
            0, 0, 0, 0,
            hwnd,
            (HMENU)(id + 1000),
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
            NULL
        );
        // Create drawn count label with more space
        CreateWindowW(
            L"STATIC",
            L"Drawn: 0",
            WS_VISIBLE | WS_CHILD,
            180, y, 150, 20,
            hwnd,
            (HMENU)(id + 2000),
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
            NULL
        );
        // Set the buddy window
        SendMessage(hUpDown, UDM_SETBUDDY, (WPARAM)hEdit, 0);
        // Set the range (0-99)
        SendMessage(hUpDown, UDM_SETRANGE32, 0, 99);
        g_tokenControls.push_back(hEdit);
        y += 30;
        id++;
    }
}

void UpdateStatistics(HWND hwnd) {
    const auto& counts = g_tokenManager->getAllTokenCounts();
    const auto& usages = g_tokenManager->getAllTokenUsages();

    int i = 0;
    for (const auto& [token, count] : counts) {
        SetWindowTextA(g_tokenControls[i], std::to_string(count).c_str());
        HWND hDrawnLabel = GetDlgItem(hwnd, ID_TOKEN_START + i + 2000);
        if (hDrawnLabel)
        {
            std::string drawnText = "Drawn: " + std::to_string(usages.at(token));
            SetWindowTextA(hDrawnLabel, drawnText.c_str());
        }
        i++;
    }
}

void DrawToken(HWND hwnd) {
    OutputDebugStringA("DrawToken() invoked.\n");
    std::string token = g_tokenManager->getRandomToken();
    if (!token.empty()) {
        // Play the corresponding audio file using absolute path
        char modulePath[MAX_PATH];
        GetModuleFileNameA(NULL, modulePath, MAX_PATH);
        std::string exePath(modulePath);
        std::string dirPath = exePath.substr(0, exePath.find_last_of("\\"));
        std::string audioPath = dirPath + "\\resources\\" + token + ".mp3";
        std::string openCmd = "open \"" + audioPath + "\" type mpegvideo alias tokenSound";
        DWORD error = mciSendStringA(openCmd.c_str(), NULL, 0, NULL);
        if (error) {
            char errorText[256];
            mciGetErrorStringA(error, errorText, sizeof(errorText));
            MessageBoxA(hwnd, errorText, "MCI Open Error", MB_OK | MB_ICONERROR);
            OutputDebugStringA("MCI Open Error encountered.\n");
            return;
        }
        error = mciSendStringA("play tokenSound wait", NULL, 0, NULL);
        if (error) {
            char errorText[256];
            mciGetErrorStringA(error, errorText, sizeof(errorText));
            MessageBoxA(hwnd, errorText, "MCI Play Error", MB_OK | MB_ICONERROR);
            mciSendStringA("close tokenSound", NULL, 0, NULL);
            OutputDebugStringA("MCI Play Error encountered.\n");
            return;
        }
        mciSendStringA("close tokenSound", NULL, 0, NULL);
        OutputDebugStringA("Audio played successfully.\n");
        
        // Update display
        UpdateStatistics(hwnd);
    } else {
        MessageBoxW(hwnd, L"No tokens available to draw", L"Information", MB_OK | MB_ICONINFORMATION);
        OutputDebugStringA("No token available to draw!\n");
    }
}
