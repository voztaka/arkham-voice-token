#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <memory>
#include "tokenmanager.h"

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comctl32.lib")

// Window control IDs
#define ID_DRAW_BUTTON 1001
#define ID_STATS_EDIT 1002
#define ID_TOKEN_START 2000

// Global variables
HWND g_hwnd;
TokenManager* g_tokenManager;
std::vector<HWND> g_tokenControls;

// Forward declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitializeControls(HWND hwnd);
void UpdateStatistics(HWND hwnd);
void DrawToken(HWND hwnd);

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

    // Show window
    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);

    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup
    delete g_tokenManager;

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == ID_DRAW_BUTTON) {
                DrawToken(hwnd);
                return 0;
            } else if (LOWORD(wParam) >= ID_TOKEN_START) {
                int tokenIndex = LOWORD(wParam) - ID_TOKEN_START;
                if (tokenIndex >= 0 && tokenIndex < g_tokenControls.size()) {
                    // Get the current value from the edit control
                    wchar_t buffer[16];
                    GetWindowTextW(g_tokenControls[tokenIndex], buffer, 16);
                    int value = _wtoi(buffer);

                    // Get the token name
                    const auto& tokens = g_tokenManager->getAllTokenCounts();
                    auto it = tokens.begin();
                    std::advance(it, tokenIndex);
                    
                    // Update the token count
                    g_tokenManager->setTokenCount(it->first, value);
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
        // Convert string to wide string
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
            (HMENU)(id + 2000),  // Use a new range of IDs for drawn labels
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
    // Update token counts and drawn counts
    const auto& counts = g_tokenManager->getAllTokenCounts();
    const auto& usages = g_tokenManager->getAllTokenUsages();
    
    int i = 0;
    for (const auto& [token, count] : counts) {
        // Update count control
        SetWindowTextA(g_tokenControls[i], std::to_string(count).c_str());
        
        // Update drawn count label
        HWND hDrawnLabel = GetDlgItem(hwnd, ID_TOKEN_START + i + 2000);
        if (hDrawnLabel) {
            std::string drawnText = "Drawn: " + std::to_string(usages.at(token));
            SetWindowTextA(hDrawnLabel, drawnText.c_str());
        }
        
        i++;
    }
}

void DrawToken(HWND hwnd) {
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
            return;
        }
        error = mciSendStringA("play tokenSound wait", NULL, 0, NULL);
        if (error) {
            char errorText[256];
            mciGetErrorStringA(error, errorText, sizeof(errorText));
            MessageBoxA(hwnd, errorText, "MCI Play Error", MB_OK | MB_ICONERROR);
            // 에러 발생시 토큰 사운드 닫기 (리소스 해제)
            mciSendStringA("close tokenSound", NULL, 0, NULL);
            return;
        }
        mciSendStringA("close tokenSound", NULL, 0, NULL);
        
        // Update display
        UpdateStatistics(hwnd);
    } else {
        MessageBoxW(hwnd, L"No tokens available to draw", L"Information", MB_OK | MB_ICONINFORMATION);
    }
}