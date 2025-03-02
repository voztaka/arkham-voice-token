#include "MainWindow.h"
#include "Constants.h"
#include <commctrl.h>
#include "Utilities.h"
#include <string>
#include <sstream>

#pragma comment(lib, "comctl32.lib")

// Static member initialization
constexpr wchar_t MainWindow::CLASS_NAME[];

MainWindow::MainWindow(HINSTANCE hInstance)
    : m_hInstance(hInstance), 
      m_hwnd(NULL),
      m_tokenManager(std::make_unique<TokenManager>()),
      m_audioPlayer(std::make_unique<AudioPlayer>()),
      m_serialComm(std::make_unique<SerialCommunicator>(Constants::DEFAULT_COM_PORT, Constants::DEFAULT_BAUD_RATE)) {
}

MainWindow::~MainWindow() {
    // Smart pointers will clean up automatically
}

void MainWindow::refreshComPorts() {
    SendMessage(m_comPortCombo, CB_RESETCONTENT, 0, 0);
    
    auto ports = SerialCommunicator::getAvailableComPorts();
    for (const auto& port : ports) {
        std::wstring wport = Utilities::AnsiToWide(port); 
        SendMessage(m_comPortCombo, CB_ADDSTRING, 0, (LPARAM)wport.c_str());
    }
    
    if (!ports.empty()) {
        SendMessage(m_comPortCombo, CB_SETCURSEL, 0, 0);
    }
}


bool MainWindow::create() {
    // Initialize Common Controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_UPDOWN_CLASS | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    // Register window class
    WNDCLASSW wc = {};
    wc.lpfnWndProc = MainWindow::staticWindowProc;
    wc.hInstance = m_hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    if (!RegisterClassW(&wc)) {
        MessageBoxW(NULL, L"Failed to register window class", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    // Create window
    m_hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Arkham Horror Token Manager",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600,
        NULL, NULL,
        m_hInstance,
        this  // Pass 'this' pointer to connect the window with this instance
    );

    if (m_hwnd == NULL) {
        MessageBoxW(NULL, L"Failed to create window", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    // Create controls
    initializeControls();

    // Start the serial communication
    if (!m_serialComm->start([this]() { 
        // This is the callback that will be executed when a signal is received
        PostMessage(m_hwnd, WM_COMMAND, Constants::ID_DRAW_BUTTON, 0);
    })) {
        OutputDebugStringA(("Serial error: " + m_serialComm->getLastError() + "\n").c_str());
        // Don't return false here - app can still work without serial
    }

    return true;
}

int MainWindow::run(int nCmdShow) {
    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);

    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK MainWindow::staticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis = nullptr;
    
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<MainWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        return pThis->windowProc(hwnd, uMsg, wParam, lParam);
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT MainWindow::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == Constants::ID_DRAW_BUTTON) {
                OutputDebugStringA("WM_COMMAND: Draw Token button pressed.\n");
                drawToken();
                return 0;
            }
            else if (LOWORD(wParam) >= Constants::ID_TOKEN_START) {
                int tokenIndex = LOWORD(wParam) - Constants::ID_TOKEN_START;
                if (tokenIndex >= 0 && tokenIndex < m_tokenControls.size()) {
                    wchar_t buffer[16];
                    GetWindowTextW(m_tokenControls[tokenIndex], buffer, 16);
                    int value = _wtoi(buffer);
                    
                    const auto& tokens = m_tokenManager->getAllTokenCounts();
                    auto it = tokens.begin();
                    std::advance(it, tokenIndex);
                    
                    m_tokenManager->setTokenCount(it->first, value);
                    
                    std::string debugMsg = "WM_COMMAND: Updated token '";
                    debugMsg += it->first;
                    debugMsg += "' to ";
                    debugMsg += std::to_string(value);
                    debugMsg += "\n";
                    OutputDebugStringA(debugMsg.c_str());
                }
                return 0;
            }
            if (LOWORD(wParam) == Constants::ID_COM_PORT_COMBO && HIWORD(wParam) == CBN_SELCHANGE) {
                int idx = SendMessage(m_comPortCombo, CB_GETCURSEL, 0, 0);
                if (idx != CB_ERR) {
                    wchar_t buffer[20];
                    SendMessage(m_comPortCombo, CB_GETLBTEXT, idx, (LPARAM)buffer);
                    
                    std::wstring wport(buffer);
                    std::string port(wport.begin(), wport.end());
                    
                    m_serialComm->setPortName(port);
                    
                    m_serialComm->start([this]() {
                        PostMessage(m_hwnd, WM_COMMAND, Constants::ID_DRAW_BUTTON, 0);
                    });
                }
                return 0;
            }
            else if (LOWORD(wParam) == Constants::ID_REFRESH_PORTS_BUTTON) {
                refreshComPorts();
                return 0;
            }
            break;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void MainWindow::initializeControls() {
    // Create Draw Token button
    CreateWindowW(
        L"BUTTON", 
        L"Draw Token",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        10, 10, 100, 30,
        m_hwnd, 
        (HMENU)Constants::ID_DRAW_BUTTON,
        m_hInstance,
        NULL
    );

    // Create token count controls
    const auto& tokens = m_tokenManager->getAllTokenCounts();
    int y = 50;
    int id = Constants::ID_TOKEN_START;
    
    for (const auto& [token, count] : tokens) {
        std::wstring wtoken(token.begin(), token.end());
        
        // Create label
        CreateWindowW(
            L"STATIC",
            wtoken.c_str(),
            WS_VISIBLE | WS_CHILD,
            10, y, 100, 20,
            m_hwnd,
            NULL,
            m_hInstance,
            NULL
        );

        CreateWindowW(
            L"STATIC",
            L"COM Port:",
            WS_VISIBLE | WS_CHILD,
            120, 10, 70, 30,
            m_hwnd,
            NULL,
            m_hInstance,
            NULL
        );

        m_comPortCombo = CreateWindowW(
            L"COMBOBOX",
            NULL,
            WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL,
            200, 10, 100, 200,
            m_hwnd,
            (HMENU)Constants::ID_COM_PORT_COMBO,
            m_hInstance,
            NULL
        );

        CreateWindowW(
            L"BUTTON",
            L"Refresh",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            310, 10, 70, 30,
            m_hwnd,
            (HMENU)Constants::ID_REFRESH_PORTS_BUTTON,
            m_hInstance,
            NULL
        );
    
        refreshComPorts();
        
        // Create edit control for count
        HWND hEdit = CreateWindowW(
            L"EDIT",
            L"0",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
            120, y, 50, 20,
            m_hwnd,
            (HMENU)id,
            m_hInstance,
            NULL
        );
        
        // Create up-down buddy control
        HWND hUpDown = CreateWindowW(
            UPDOWN_CLASSW,
            NULL,
            WS_VISIBLE | WS_CHILD | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_NOTHOUSANDS,
            0, 0, 0, 0,
            m_hwnd,
            (HMENU)(id + 1000),
            m_hInstance,
            NULL
        );
        
        // Create drawn count label
        CreateWindowW(
            L"STATIC",
            L"Drawn: 0",
            WS_VISIBLE | WS_CHILD,
            180, y, 150, 20,
            m_hwnd,
            (HMENU)(id + 2000),
            m_hInstance,
            NULL
        );
        
        // Set the buddy window
        SendMessage(hUpDown, UDM_SETBUDDY, (WPARAM)hEdit, 0);
        
        // Set the range (0-99)
        SendMessage(hUpDown, UDM_SETRANGE32, 0, 99);
        
        m_tokenControls.push_back(hEdit);
        y += 30;
        id++;
    }
}

void MainWindow::updateStatistics() {
    const auto& counts = m_tokenManager->getAllTokenCounts();
    const auto& usages = m_tokenManager->getAllTokenUsages();
    
    int i = 0;
    for (const auto& [token, count] : counts) {
        SetWindowTextA(m_tokenControls[i], std::to_string(count).c_str());
        
        HWND hDrawnLabel = GetDlgItem(m_hwnd, Constants::ID_TOKEN_START + i + 2000);
        if (hDrawnLabel) {
            std::string drawnText = "Drawn: " + std::to_string(usages.at(token));
            SetWindowTextA(hDrawnLabel, drawnText.c_str());
        }
        
        i++;
    }
}

void MainWindow::drawToken() {
    OutputDebugStringA("DrawToken() invoked.\n");
    
    auto tokenOpt = m_tokenManager->getRandomToken();
    if (tokenOpt) {
        const std::string& token = *tokenOpt;
        
        // Play audio
        if (!m_audioPlayer->playTokenSound(token)) {
            OutputDebugStringA("Failed to play audio.\n");
        } else {
            OutputDebugStringA("Audio played successfully.\n");
        }
        
        // Update display
        updateStatistics();
    }
    else {
        MessageBoxW(m_hwnd, L"No tokens available to draw", L"Information", MB_OK | MB_ICONINFORMATION);
        OutputDebugStringA("No token available to draw!\n");
    }
}
