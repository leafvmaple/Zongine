#pragma once

#include <string>
#include <cstdlib>

namespace Zongine {
    std::wstring AnsiToWString(const std::string& ansiStr) {
        size_t wideSize = std::mbstowcs(nullptr, ansiStr.c_str(), 0) + 1;
        std::wstring wideStr(wideSize, L'\0');
        std::mbstowcs(&wideStr[0], ansiStr.c_str(), wideSize);
        return wideStr;
    }

    std::string WStringToAnsi(const std::wstring& wideStr) {
        size_t ansiSize = wcstombs(nullptr, wideStr.c_str(), 0) + 1;
        std::string ansiStr(ansiSize, '\0');
        wcstombs(&ansiStr[0], wideStr.c_str(), ansiSize);
        return ansiStr;
    }
}