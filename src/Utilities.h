#include <string>
#include <windows.h>

namespace Utilities {
    std::wstring AnsiToWide(const std::string& str) {
        int size_needed = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
        std::wstring wstr(size_needed, 0);
        MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, &wstr[0], size_needed);
        return wstr;
    }

    std::string WideToAnsi(const std::wstring& wstr) {
        int size_needed = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
        std::string str(size_needed, 0);
        WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &str[0], size_needed, NULL, NULL);
        return str;
    }
}
