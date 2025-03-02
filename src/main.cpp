#include "MainWindow.h"
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    MainWindow mainWindow(hInstance);
    
    if (!mainWindow.create()) {
        return 1;
    }
    
    return mainWindow.run(nCmdShow);
}
