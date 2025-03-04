#pragma once

#include "TokenManager.h"
#include "AudioPlayer.h"
#include "SerialCommunicator.h"
#include "ProfileManager.h"
#include <windows.h>
#include <vector>
#include <memory>
#include <string>

class MainWindow {
public:
    MainWindow(HINSTANCE hInstance);
    ~MainWindow();
    
    bool create();
    int run(int nCmdShow);
    
private:
    static LRESULT CALLBACK staticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    void initializeControls();
    void updateStatistics();
    void drawToken();
    
    HINSTANCE m_hInstance;
    HWND m_hwnd;
    std::vector<HWND> m_tokenControls;

    HWND m_comPortCombo;
    void refreshComPorts();
    
    std::unique_ptr<TokenManager> m_tokenManager;
    std::unique_ptr<AudioPlayer> m_audioPlayer;
    std::unique_ptr<SerialCommunicator> m_serialComm;
    
    static constexpr wchar_t CLASS_NAME[] = L"ArkhamTokenWindow";

      // 새로운 프로필 관련 멤버
    std::unique_ptr<ProfileManager> m_profileManager;
    HWND m_profileTab;
    std::vector<HWND> m_profileControls[ProfileManager::MAX_PROFILES];
    
    // 프로필 UI 메서드
    void initializeProfileControls();
    void createProfileTab();
    void createProfileControls(int profileIndex);
    void updateProfileDisplay(int profileIndex);
    void updateAllProfileDisplays();
    int getCurrentProfileIndex();
    void handleProfileAction(WPARAM wParam, LPARAM lParam);
};
