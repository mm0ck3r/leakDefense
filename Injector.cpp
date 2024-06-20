#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <shlobj.h>
#include <fstream>
#include <knownfolders.h>  
#include <vector>
#include <string>

void InjectDLL(DWORD pid, LPCSTR dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (NULL == hProcess) {
        printf("Process not found\n");
        return;
    }
    LPVOID lpAddr = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (lpAddr) {
        WriteProcessMemory(hProcess, lpAddr, dllPath, strlen(dllPath) + 1, NULL);
    }
    else {
        printf("VirtualAllocEx() failure.\n");
        return;
    }
    LPTHREAD_START_ROUTINE pfnLoadLibraryA = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (pfnLoadLibraryA) {
        HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pfnLoadLibraryA, lpAddr, 0, NULL);
        DWORD dwExitCode = NULL;
        if (hThread) {
            printf("Injection successful!\n");
            WaitForSingleObject(hThread, INFINITE);
            if (GetExitCodeThread(hThread, &dwExitCode)) printf("Injected DLL ImageBase: %#x\n", dwExitCode);
            CloseHandle(hThread);
        }
        else {
            std::cout << "Injection failure.\n";
        }
    }
    VirtualFreeEx(hProcess, lpAddr, 0, MEM_RELEASE);
}

std::wstring lastTimestamp = L"";

void ReadLogFile(const wchar_t* path) {
    std::wifstream logFile(path);
    std::wstring line;
    std::vector<std::wstring> newLines;

    while (std::getline(logFile, line)) {
        if (line > lastTimestamp) {
            newLines.push_back(line);
        }
    }

    if (!newLines.empty()) {
        lastTimestamp = newLines.back();
        for (const auto& l : newLines) {
            std::wcout << l << std::endl;
        }
    }

    logFile.close();
}

int main() {
    DWORD processID = 0;
    HWND hwnd;
    const char* dllPath = "C:\\Users\\KOREA\\Desktop\\defense\\Release\\Hook.dll";
    wchar_t path[MAX_PATH] = { 0, };
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, path))) {
        wcscat_s(path, MAX_PATH, L"\\detectLog.txt");
    }
    else {
        std::cout << "Failed to open detectLog.txt" << std::endl;
    }
    printf("Looking for \"notepad++.exe\".......\n");
    while (1) {
        hwnd = FindWindow(L"Notepad++", NULL);
        if (hwnd)
        {
            GetWindowThreadProcessId(hwnd, &processID);
            InjectDLL(processID, dllPath);
            break;
        }
        Sleep(500);
    }
    std::cout << "Now into logging mode\nWhen Notepad++ Quit, this defnese will be quit." << std::endl;
    while (1) {
        hwnd = FindWindow(L"Notepad++", NULL);
        if (hwnd) { // running
            ReadLogFile(path);
        }
        else {
            std::cout << "ByeBye~~!!" << std::endl;
            break;
        }
    }
    return 0;
}
