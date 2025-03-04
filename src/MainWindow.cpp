#define NOMINMAX  
#include "MainWindow.h"
#include "Constants.h"
#include "Profile.h"
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
      m_profileManager(std::make_unique<ProfileManager>()),
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
            else if (LOWORD(wParam) >= Constants::ID_TOKEN_START && 
                    LOWORD(wParam) < Constants::ID_ADD_PROFILE_BUTTON) {
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
            // 프로필 관련 명령 처리
            handleProfileAction(wParam, lParam);
            return 0;

        case WM_NOTIFY:
            // 탭 컨트롤 선택 변경 처리
            if (((LPNMHDR)lParam)->idFrom == Constants::ID_PROFILE_TAB) {
                if (((LPNMHDR)lParam)->code == TCN_SELCHANGE) {
                    int newTab = TabCtrl_GetCurSel(m_profileTab);
                    
                    // 프로필 컨트롤 표시/숨김 변경
                    for (int i = 0; i < m_profileManager->getProfileCount(); i++) {
                        for (HWND control : m_profileControls[i]) {
                            ShowWindow(control, i == newTab ? SW_SHOW : SW_HIDE);
                        }
                    }
                    return 0;
                }
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

    initializeProfileControls();

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



// 프로필 컨트롤 초기화 함수
void MainWindow::initializeProfileControls() {
    // 먼저 탭 컨트롤 생성
    createProfileTab();
    
    // 각 프로필에 대한 컨트롤 생성
    for (size_t i = 0; i < m_profileManager->getProfileCount(); i++) {
        createProfileControls(i);
    }
    
    // 프로필 추가 버튼 생성
    CreateWindowW(
        L"BUTTON", L"+",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        700, 50, 30, 30,
        m_hwnd, (HMENU)Constants::ID_ADD_PROFILE_BUTTON,
        m_hInstance, NULL
    );
    
    // 초기 프로필 표시 업데이트
    updateAllProfileDisplays();
}

// 프로필 탭 생성 함수
void MainWindow::createProfileTab() {
    m_profileTab = CreateWindowW(
        WC_TABCONTROLW, NULL,
        WS_VISIBLE | WS_CHILD | TCS_TABS,
        400, 50, 280, 350,
        m_hwnd, (HMENU)Constants::ID_PROFILE_TAB,
        m_hInstance, NULL
    );
    
    // 기존 프로필에 대한 탭 추가
    for (size_t i = 0; i < m_profileManager->getProfileCount(); i++) {
        const Profile* profile = m_profileManager->getProfile(i);
        if (profile) {
            std::wstring wname(profile->getName().begin(), profile->getName().end());
            
            TCITEMW tie = {0};
            tie.mask = TCIF_TEXT;
            tie.pszText = const_cast<LPWSTR>(wname.c_str());
            
            TabCtrl_InsertItem(m_profileTab, i, &tie);
        }
    }
}

// 프로필 컨트롤 생성 함수
void MainWindow::createProfileControls(int profileIndex) {
    const int baseX = 410;
    const int baseY = 90;
    const int editWidth = 80;
    const int buttonWidth = 30;
    const int rowHeight = 30;
    
    Profile* profile = m_profileManager->getProfile(profileIndex);
    if (!profile) return;
    
    std::vector<HWND>& controls = m_profileControls[profileIndex];
    controls.clear();
    
    // 이름 입력 영역
    CreateWindowW(
        L"STATIC", L"Name:",
        WS_VISIBLE | WS_CHILD,
        baseX, baseY, 60, rowHeight,
        m_hwnd, NULL, m_hInstance, NULL
    );
    
    HWND nameEdit = CreateWindowW(
        L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        baseX + 60, baseY, editWidth + 50, rowHeight,
        m_hwnd, (HMENU)(Constants::ID_PROFILE_NAME_EDIT + profileIndex),
        m_hInstance, NULL
    );
    controls.push_back(nameEdit);
    
    // 최대 액션 입력 영역
    CreateWindowW(
        L"STATIC", L"Maximum Actions:",
        WS_VISIBLE | WS_CHILD,
        baseX, baseY + rowHeight, 60, rowHeight,
        m_hwnd, NULL, m_hInstance, NULL
    );
    
    HWND maxActionsEdit = CreateWindowW(
        L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
        baseX + 60, baseY + rowHeight, editWidth, rowHeight,
        m_hwnd, (HMENU)(Constants::ID_MAX_ACTIONS_EDIT + profileIndex),
        m_hInstance, NULL
    );
    controls.push_back(maxActionsEdit);
    
    // 현재 액션 영역
    CreateWindowW(
        L"STATIC", L"Current Actions:",
        WS_VISIBLE | WS_CHILD,
        baseX, baseY + rowHeight * 2, 60, rowHeight,
        m_hwnd, NULL, m_hInstance, NULL
    );
    
    HWND currentActionsEdit = CreateWindowW(
        L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | ES_READONLY,
        baseX + 60, baseY + rowHeight * 2, editWidth, rowHeight,
        m_hwnd, (HMENU)(Constants::ID_CURRENT_ACTIONS_EDIT + profileIndex),
        m_hInstance, NULL
    );
    controls.push_back(currentActionsEdit);
    
    // 액션 증가/감소 버튼
    HWND actionsUpButton = CreateWindowW(
        L"BUTTON", L"+",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        baseX + 60 + editWidth + 5, baseY + rowHeight * 2, buttonWidth, rowHeight,
        m_hwnd, (HMENU)(Constants::ID_ACTIONS_UP_BUTTON + profileIndex),
        m_hInstance, NULL
    );
    controls.push_back(actionsUpButton);
    
    HWND actionsDownButton = CreateWindowW(
        L"BUTTON", L"-",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        baseX + 60 + editWidth + 5 + buttonWidth + 5, baseY + rowHeight * 2, buttonWidth, rowHeight,
        m_hwnd, (HMENU)(Constants::ID_ACTIONS_DOWN_BUTTON + profileIndex),
        m_hInstance, NULL
    );
    controls.push_back(actionsDownButton);
    
    // 리셋 버튼
    HWND resetButton = CreateWindowW(
        L"BUTTON", L"Reset",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        baseX + 60 + editWidth + 5 + (buttonWidth + 5) * 2, baseY + rowHeight * 2, 50, rowHeight,
        m_hwnd, (HMENU)(Constants::ID_ACTIONS_RESET_BUTTON + profileIndex),
        m_hInstance, NULL
    );
    controls.push_back(resetButton);
    
    // Damage 영역
    CreateWindowW(
        L"STATIC", L"Damage:",
        WS_VISIBLE | WS_CHILD,
        baseX, baseY + rowHeight * 3, 60, rowHeight,
        m_hwnd, NULL, m_hInstance, NULL
    );
    
    HWND damageEdit = CreateWindowW(
        L"EDIT", L"0",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | ES_READONLY,
        baseX + 60, baseY + rowHeight * 3, editWidth, rowHeight,
        m_hwnd, (HMENU)(Constants::ID_DAMAGE_EDIT + profileIndex),
        m_hInstance, NULL
    );
    controls.push_back(damageEdit);
    
    HWND damageUpButton = CreateWindowW(
        L"BUTTON", L"+",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        baseX + 60 + editWidth + 5, baseY + rowHeight * 3, buttonWidth, rowHeight,
        m_hwnd, (HMENU)(Constants::ID_DAMAGE_UP_BUTTON + profileIndex),
        m_hInstance, NULL
    );
    controls.push_back(damageUpButton);
    
    HWND damageDownButton = CreateWindowW(
        L"BUTTON", L"-",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        baseX + 60 + editWidth + 5 + buttonWidth + 5, baseY + rowHeight * 3, buttonWidth, rowHeight,
        m_hwnd, (HMENU)(Constants::ID_DAMAGE_DOWN_BUTTON + profileIndex),
        m_hInstance, NULL
    );
    controls.push_back(damageDownButton);
    
    // Horror 영역
    CreateWindowW(
        L"STATIC", L"Horror:",
        WS_VISIBLE | WS_CHILD,
        baseX, baseY + rowHeight * 4, 60, rowHeight,
        m_hwnd, NULL, m_hInstance, NULL
    );
    
    HWND horrorEdit = CreateWindowW(
        L"EDIT", L"0",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | ES_READONLY,
        baseX + 60, baseY + rowHeight * 4, editWidth, rowHeight,
        m_hwnd, (HMENU)(Constants::ID_HORROR_EDIT + profileIndex),
        m_hInstance, NULL
    );
    controls.push_back(horrorEdit);
    
    HWND horrorUpButton = CreateWindowW(
        L"BUTTON", L"+",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        baseX + 60 + editWidth + 5, baseY + rowHeight * 4, buttonWidth, rowHeight,
        m_hwnd, (HMENU)(Constants::ID_HORROR_UP_BUTTON + profileIndex),
        m_hInstance, NULL
    );
    controls.push_back(horrorUpButton);
    
    HWND horrorDownButton = CreateWindowW(
        L"BUTTON", L"-",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        baseX + 60 + editWidth + 5 + buttonWidth + 5, baseY + rowHeight * 4, buttonWidth, rowHeight,
        m_hwnd, (HMENU)(Constants::ID_HORROR_DOWN_BUTTON + profileIndex),
        m_hInstance, NULL
    );
    controls.push_back(horrorDownButton);
    
    // Clues 영역
    CreateWindowW(
        L"STATIC", L"Clues:",
        WS_VISIBLE | WS_CHILD,
        baseX, baseY + rowHeight * 5, 60, rowHeight,
        m_hwnd, NULL, m_hInstance, NULL
    );
    
    HWND cluesEdit = CreateWindowW(
        L"EDIT", L"0",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | ES_READONLY,
        baseX + 60, baseY + rowHeight * 5, editWidth, rowHeight,
        m_hwnd, (HMENU)(Constants::ID_CLUES_EDIT + profileIndex),
        m_hInstance, NULL
    );
    controls.push_back(cluesEdit);
    
    HWND cluesUpButton = CreateWindowW(
        L"BUTTON", L"+",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        baseX + 60 + editWidth + 5, baseY + rowHeight * 5, buttonWidth, rowHeight,
        m_hwnd, (HMENU)(Constants::ID_CLUES_UP_BUTTON + profileIndex),
        m_hInstance, NULL
    );
    controls.push_back(cluesUpButton);
    
    HWND cluesDownButton = CreateWindowW(
        L"BUTTON", L"-",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        baseX + 60 + editWidth + 5 + buttonWidth + 5, baseY + rowHeight * 5, buttonWidth, rowHeight,
        m_hwnd, (HMENU)(Constants::ID_CLUES_DOWN_BUTTON + profileIndex),
        m_hInstance, NULL
    );
    controls.push_back(cluesDownButton);
    
    // Resources 영역
    CreateWindowW(
        L"STATIC", L"Resources:",
        WS_VISIBLE | WS_CHILD,
        baseX, baseY + rowHeight * 6, 60, rowHeight,
        m_hwnd, NULL, m_hInstance, NULL
    );
    
    HWND resourcesEdit = CreateWindowW(
        L"EDIT", L"0",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | ES_READONLY,
        baseX + 60, baseY + rowHeight * 6, editWidth, rowHeight,
        m_hwnd, (HMENU)(Constants::ID_RESOURCES_EDIT + profileIndex),
        m_hInstance, NULL
    );
    controls.push_back(resourcesEdit);
    
    HWND resourcesUpButton = CreateWindowW(
        L"BUTTON", L"+",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        baseX + 60 + editWidth + 5, baseY + rowHeight * 6, buttonWidth, rowHeight,
        m_hwnd, (HMENU)(Constants::ID_RESOURCES_UP_BUTTON + profileIndex),
        m_hInstance, NULL
    );
    controls.push_back(resourcesUpButton);
    
    HWND resourcesDownButton = CreateWindowW(
        L"BUTTON", L"-",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        baseX + 60 + editWidth + 5 + buttonWidth + 5, baseY + rowHeight * 6, buttonWidth, rowHeight,
        m_hwnd, (HMENU)(Constants::ID_RESOURCES_DOWN_BUTTON + profileIndex),
        m_hInstance, NULL
    );
    controls.push_back(resourcesDownButton);
    
    // 초기값으로 UI 설정
    updateProfileDisplay(profileIndex);
    
    // 처음에는 모든 컨트롤이 표시되지만, 현재 탭이 아닌 것은 숨김
    int currentTab = TabCtrl_GetCurSel(m_profileTab);
    for (HWND control : controls) {
        ShowWindow(control, currentTab == profileIndex ? SW_SHOW : SW_HIDE);
    }
}

// 프로필 표시 업데이트
void MainWindow::updateProfileDisplay(int profileIndex) {
    Profile* profile = m_profileManager->getProfile(profileIndex);
    if (!profile || profileIndex >= ProfileManager::MAX_PROFILES) return;
    
    std::vector<HWND>& controls = m_profileControls[profileIndex];
    if (controls.empty()) return;
    
    // 이름 업데이트
    std::wstring wname(profile->getName().begin(), profile->getName().end());
    SetWindowTextW(controls[0], wname.c_str());
    
    // 최대 액션 업데이트
    SetWindowTextW(controls[1], std::to_wstring(profile->getMaxActions()).c_str());
    
    // 현재 액션 업데이트
    SetWindowTextW(controls[2], std::to_wstring(profile->getCurrentActions()).c_str());
    
    // Damage 업데이트
    SetWindowTextW(controls[6], std::to_wstring(profile->getDamage()).c_str());
    
    // Horror 업데이트
    SetWindowTextW(controls[9], std::to_wstring(profile->getHorror()).c_str());
    
    // Clues 업데이트
    SetWindowTextW(controls[12], std::to_wstring(profile->getClues()).c_str());
    
    // Resources 업데이트
    SetWindowTextW(controls[15], std::to_wstring(profile->getResources()).c_str());
    
    // 탭 텍스트 업데이트
    TCITEMW tie = {0};
    tie.mask = TCIF_TEXT;
    tie.pszText = const_cast<LPWSTR>(wname.c_str());
    TabCtrl_SetItem(m_profileTab, profileIndex, &tie);
}

// 모든 프로필 표시 업데이트
void MainWindow::updateAllProfileDisplays() {
    for (size_t i = 0; i < m_profileManager->getProfileCount(); i++) {
        updateProfileDisplay(i);
    }
}

// 현재 선택된 프로필 인덱스 가져오기
int MainWindow::getCurrentProfileIndex() {
    return TabCtrl_GetCurSel(m_profileTab);
}

// 프로필 액션 처리
void MainWindow::handleProfileAction(WPARAM wParam, LPARAM lParam) {
    WORD cmdId = LOWORD(wParam);
    
    // 프로필 추가 버튼
    if (cmdId == Constants::ID_ADD_PROFILE_BUTTON) {
        if (m_profileManager->getProfileCount() < ProfileManager::MAX_PROFILES) {
            std::string name = "Investigator " + std::to_string(m_profileManager->getProfileCount() + 1);
            if (m_profileManager->addProfile(name)) {
                // 새 프로필 위한 탭 생성
                std::wstring wname(name.begin(), name.end());
                TCITEMW tie = {0};
                tie.mask = TCIF_TEXT;
                tie.pszText = const_cast<LPWSTR>(wname.c_str());
                int newTabIndex = TabCtrl_GetItemCount(m_profileTab);
                TabCtrl_InsertItem(m_profileTab, newTabIndex, &tie);
                
                // 새 프로필 위한 컨트롤 생성
                createProfileControls(newTabIndex);
                
                // 새 탭 선택
                TabCtrl_SetCurSel(m_profileTab, newTabIndex);
                
                // 프로필 컨트롤 표시/숨김 갱신
                for (int i = 0; i < ProfileManager::MAX_PROFILES; i++) {
                    if (i < m_profileManager->getProfileCount()) {
                        for (HWND control : m_profileControls[i]) {
                            ShowWindow(control, i == newTabIndex ? SW_SHOW : SW_HIDE);
                        }
                    }
                }
            }
        } else {
            MessageBoxW(m_hwnd, L"최대 프로필 수에 도달했습니다.", L"알림", MB_OK | MB_ICONINFORMATION);
        }
        return;
    }
    
    // 이름 변경 처리
    if (cmdId >= Constants::ID_PROFILE_NAME_EDIT && 
        cmdId < Constants::ID_PROFILE_NAME_EDIT + ProfileManager::MAX_PROFILES) {
        if (HIWORD(wParam) == EN_KILLFOCUS) {
            int profileIdx = cmdId - Constants::ID_PROFILE_NAME_EDIT;
            Profile* profile = m_profileManager->getProfile(profileIdx);
            if (profile) {
                wchar_t buffer[256];
                GetWindowTextW((HWND)lParam, buffer, 256);
                std::wstring wname(buffer);
                std::string name(wname.begin(), wname.end());
                profile->setName(name);
                
                // 탭 이름 업데이트
                TCITEMW tie = {0};
                tie.mask = TCIF_TEXT;
                tie.pszText = buffer;
                TabCtrl_SetItem(m_profileTab, profileIdx, &tie);
            }
        }
        return;
    }
    
    // 최대 액션 변경 처리
    if (cmdId >= Constants::ID_MAX_ACTIONS_EDIT && 
        cmdId < Constants::ID_MAX_ACTIONS_EDIT + ProfileManager::MAX_PROFILES) {
        if (HIWORD(wParam) == EN_KILLFOCUS) {
            int profileIdx = cmdId - Constants::ID_MAX_ACTIONS_EDIT;
            Profile* profile = m_profileManager->getProfile(profileIdx);
            if (profile) {
                wchar_t buffer[16];
                GetWindowTextW((HWND)lParam, buffer, 16);
                int maxActions = std::max(1, _wtoi(buffer));
                profile->setMaxActions(maxActions);
                
                // UI 업데이트
                updateProfileDisplay(profileIdx);
            }
        }
        return;
    }
    
    // 액션 증가 버튼
    if (cmdId >= Constants::ID_ACTIONS_UP_BUTTON && 
        cmdId < Constants::ID_ACTIONS_UP_BUTTON + ProfileManager::MAX_PROFILES) {
        int profileIdx = cmdId - Constants::ID_ACTIONS_UP_BUTTON;
        Profile* profile = m_profileManager->getProfile(profileIdx);
        if (profile) {
            profile->incrementActions();
            updateProfileDisplay(profileIdx);
        }
        return;
    }
    
    // 액션 감소 버튼
    if (cmdId >= Constants::ID_ACTIONS_DOWN_BUTTON && 
        cmdId < Constants::ID_ACTIONS_DOWN_BUTTON + ProfileManager::MAX_PROFILES) {
        int profileIdx = cmdId - Constants::ID_ACTIONS_DOWN_BUTTON;
        Profile* profile = m_profileManager->getProfile(profileIdx);
        if (profile) {
            profile->decrementActions();
            updateProfileDisplay(profileIdx);
        }
        return;
    }
    
    // 액션 리셋 버튼
    if (cmdId >= Constants::ID_ACTIONS_RESET_BUTTON && 
        cmdId < Constants::ID_ACTIONS_RESET_BUTTON + ProfileManager::MAX_PROFILES) {
        int profileIdx = cmdId - Constants::ID_ACTIONS_RESET_BUTTON;
        Profile* profile = m_profileManager->getProfile(profileIdx);
        if (profile) {
            profile->resetActions();
            updateProfileDisplay(profileIdx);
        }
        return;
    }
    
    // Damage 증가 버튼
    if (cmdId >= Constants::ID_DAMAGE_UP_BUTTON && 
        cmdId < Constants::ID_DAMAGE_UP_BUTTON + ProfileManager::MAX_PROFILES) {
        int profileIdx = cmdId - Constants::ID_DAMAGE_UP_BUTTON;
        Profile* profile = m_profileManager->getProfile(profileIdx);
        if (profile) {
            profile->incrementDamage();
            updateProfileDisplay(profileIdx);
        }
        return;
    }
    
    // Damage 감소 버튼
    if (cmdId >= Constants::ID_DAMAGE_DOWN_BUTTON && 
        cmdId < Constants::ID_DAMAGE_DOWN_BUTTON + ProfileManager::MAX_PROFILES) {
        int profileIdx = cmdId - Constants::ID_DAMAGE_DOWN_BUTTON;
        Profile* profile = m_profileManager->getProfile(profileIdx);
        if (profile) {
            profile->decrementDamage();
            updateProfileDisplay(profileIdx);
        }
        return;
    }
    
    // Horror 증가 버튼
    if (cmdId >= Constants::ID_HORROR_UP_BUTTON && 
        cmdId < Constants::ID_HORROR_UP_BUTTON + ProfileManager::MAX_PROFILES) {
        int profileIdx = cmdId - Constants::ID_HORROR_UP_BUTTON;
        Profile* profile = m_profileManager->getProfile(profileIdx);
        if (profile) {
            profile->incrementHorror();
            updateProfileDisplay(profileIdx);
        }
        return;
    }
    
    // Horror 감소 버튼
    if (cmdId >= Constants::ID_HORROR_DOWN_BUTTON && 
        cmdId < Constants::ID_HORROR_DOWN_BUTTON + ProfileManager::MAX_PROFILES) {
        int profileIdx = cmdId - Constants::ID_HORROR_DOWN_BUTTON;
        Profile* profile = m_profileManager->getProfile(profileIdx);
        if (profile) {
            profile->decrementHorror();
            updateProfileDisplay(profileIdx);
        }
        return;
    }
    
    // Clues 증가 버튼
    if (cmdId >= Constants::ID_CLUES_UP_BUTTON && 
        cmdId < Constants::ID_CLUES_UP_BUTTON + ProfileManager::MAX_PROFILES) {
        int profileIdx = cmdId - Constants::ID_CLUES_UP_BUTTON;
        Profile* profile = m_profileManager->getProfile(profileIdx);
        if (profile) {
            profile->incrementClues();
            updateProfileDisplay(profileIdx);
        }
        return;
    }
    
    // Clues 감소 버튼
    if (cmdId >= Constants::ID_CLUES_DOWN_BUTTON && 
        cmdId < Constants::ID_CLUES_DOWN_BUTTON + ProfileManager::MAX_PROFILES) {
        int profileIdx = cmdId - Constants::ID_CLUES_DOWN_BUTTON;
        Profile* profile = m_profileManager->getProfile(profileIdx);
        if (profile) {
            profile->decrementClues();
            updateProfileDisplay(profileIdx);
        }
        return;
    }
    
    // Resources 증가 버튼
    if (cmdId >= Constants::ID_RESOURCES_UP_BUTTON && 
        cmdId < Constants::ID_RESOURCES_UP_BUTTON + ProfileManager::MAX_PROFILES) {
        int profileIdx = cmdId - Constants::ID_RESOURCES_UP_BUTTON;
        Profile* profile = m_profileManager->getProfile(profileIdx);
        if (profile) {
            profile->incrementResources();
            updateProfileDisplay(profileIdx);
        }
        return;
    }
    
    // Resources 감소 버튼
    if (cmdId >= Constants::ID_RESOURCES_DOWN_BUTTON && 
        cmdId < Constants::ID_RESOURCES_DOWN_BUTTON + ProfileManager::MAX_PROFILES) {
        int profileIdx = cmdId - Constants::ID_RESOURCES_DOWN_BUTTON;
        Profile* profile = m_profileManager->getProfile(profileIdx);
        if (profile) {
            profile->decrementResources();
            updateProfileDisplay(profileIdx);
        }
        return;
    }
}