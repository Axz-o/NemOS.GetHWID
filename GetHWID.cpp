// GetHWID.cpp — NemOS: wylicza HWID komputera i kopiuje go do schowka.
// HWID = FNV-1a64( serial partycji systemowej | nazwa CPU | plyta glowna ).
// Kompilacja (MSVC, x64 Native Tools):  cl /EHsc /std:c++17 GetHWID.cpp
// C++17, tylko Windows API, brak zewnętrznych bibliotek.

#include <windows.h>

#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#pragma comment(lib, "user32.lib")   // schowek (Clipboard API)
#pragma comment(lib, "advapi32.lib") // rejestr (RegOpenKeyExA)

namespace {

constexpr WORD kDefaultColor = 7; // jasnoszary (domyslny kolor konsoli)
constexpr WORD kGreen = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
constexpr WORD kRed = FOREGROUND_RED | FOREGROUND_INTENSITY;
constexpr WORD kYellow = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;

void SetColor(WORD color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

std::string ReadRegistryString(HKEY root, const char* subKey, const char* valueName) {
    HKEY hKey = nullptr;
    std::string result;
    if (RegOpenKeyExA(root, subKey, 0, KEY_READ | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS) {
        char buffer[512] = {};
        DWORD size = sizeof(buffer) - 1;
        DWORD type = 0;
        const LONG rc = RegQueryValueExA(hKey, valueName, nullptr, &type,
                                         reinterpret_cast<LPBYTE>(buffer), &size);
        if (rc == ERROR_SUCCESS && (type == REG_SZ || type == REG_EXPAND_SZ)) {
            buffer[sizeof(buffer) - 1] = '\0';
            result = buffer;
        }
        RegCloseKey(hKey);
    }
    return result;
}

std::string GetSystemVolumeSerial() {
    char sysDir[MAX_PATH] = {};
    const UINT len = GetSystemDirectoryA(sysDir, MAX_PATH);
    if (len == 0 || len >= MAX_PATH || sysDir[1] != ':') {
        return "";
    }
    std::string root;
    root += sysDir[0];
    root += ":\\";
    DWORD serial = 0;
    DWORD maxComponent = 0;
    DWORD fsFlags = 0;
    if (!GetVolumeInformationA(root.c_str(), nullptr, 0, &serial,
                               &maxComponent, &fsFlags, nullptr, 0)) {
        return "";
    }
    char out[16] = {};
    snprintf(out, sizeof(out), "%08lX", static_cast<unsigned long>(serial));
    return out;
}

uint64_t Fnv1a64(const std::string& text) {
    uint64_t hash = 14695981039346656037ULL;
    for (unsigned char c : text) {
        hash ^= c;
        hash *= 1099511628211ULL;
    }
    return hash;
}

std::string GetHWID() {
    const std::string serial = GetSystemVolumeSerial();
    const std::string cpu = ReadRegistryString(
        HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        "ProcessorNameString");
    std::string board = ReadRegistryString(
        HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\BIOS", "BaseBoardProduct");
    if (board.empty()) {
        board = ReadRegistryString(
            HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\BIOS", "SystemProductName");
    }
    if (serial.empty() && cpu.empty() && board.empty()) {
        return "";
    }
    std::ostringstream raw;
    raw << serial << '|' << cpu << '|' << board;
    std::ostringstream hex;
    hex << std::uppercase << std::hex << std::setw(16) << std::setfill('0')
        << Fnv1a64(raw.str());
    return hex.str();
}

bool CopyTextToClipboard(const std::string& text) {
    if (!OpenClipboard(nullptr)) {
        return false;
    }
    bool ok = false;
    EmptyClipboard();
    const size_t bytes = text.size() + 1;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (hMem) {
        void* locked = GlobalLock(hMem);
        if (locked) {
            memcpy(locked, text.c_str(), bytes);
            GlobalUnlock(hMem);
            if (SetClipboardData(CF_TEXT, hMem)) {
                ok = true; // od tej pory pamiecia zarzadza schowek
            } else {
                GlobalFree(hMem);
            }
        } else {
            GlobalFree(hMem);
        }
    }
    CloseClipboard();
    return ok;
}

} // namespace

int main() {
    std::cout << "==============================\n";
    std::cout << "  NemOS - pobieranie HWID\n";
    std::cout << "==============================\n";

    const std::string hwid = GetHWID();
    if (hwid.empty()) {
        SetColor(kRed);
        std::cout << "Blad: nie udalo sie wyliczyc HWID.\n";
        SetColor(kDefaultColor);
        return 1;
    }

    std::cout << "Twoj HWID: [" << hwid << "]\n";

    if (CopyTextToClipboard(hwid)) {
        SetColor(kGreen);
        std::cout << "Skopiowano HWID do schowka. Wklej go w panelu WWW.\n";
    } else {
        SetColor(kYellow);
        std::cout << "Uwaga: nie udalo sie skopiowac do schowka. Przepisz HWID recznie.\n";
    }
    SetColor(kDefaultColor);

    std::cout << "Nacisnij Enter, aby zamknac...";
    std::string dummy;
    std::getline(std::cin, dummy);
    return 0;
}
