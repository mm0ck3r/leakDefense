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

std::string GetCurrentExecutablePath() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string::size_type pos = std::string(path).find_last_of("\\/");
    return std::string(path).substr(0, pos);
}

void InitializeLogFile(const wchar_t* path) {
    std::wofstream logFile(path, std::ios::trunc); // Delete old log file
    logFile.close();
}

int main() {
    DWORD processID = 0;
    HWND hwnd;
    /*const char* dllPath = "C:\\Users\\KOREA\\Desktop\\defense\\Release\\Hook.dll";*/
    std::string dllPath = GetCurrentExecutablePath() + "\\Hook.dll";
    wchar_t path[MAX_PATH] = { 0, };
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, path))) {
        wcscat_s(path, MAX_PATH, L"\\detectLog.txt");
        InitializeLogFile(path); // Delete old log file
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
            InjectDLL(processID, dllPath.c_str());
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
//
//#include <windows.h>
//#include <iostream>
//
//LRESULT CALLBACK NewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
//
//// 원래 윈도우 프로시저를 저장할 변수
//WNDPROC originalWndProc = NULL;
//
//void SubclassScintilla(HWND hScintilla) {
//    // 원래 윈도우 프로시저를 저장
//    originalWndProc = (WNDPROC)GetClassLongPtr(hScintilla, GCLP_WNDPROC);
//    std::cout << originalWndProc << std::endl;
//    //// 새로운 윈도우 프로시저로 변경
//    //std::cout << SetWindowLongPtr(hScintilla, GWLP_WNDPROC, (LONG_PTR)NewWndProc) << std::endl;
//    if (!SetWindowLongPtr(hScintilla, GWLP_WNDPROC, (LONG_PTR)NewWndProc)) {
//        DWORD dwError = GetLastError();
//        std::cout << "SetWindowLongPtr 실패: " << dwError << "\n";
//    }
//}
//
//// 새로운 윈도우 프로시저
//LRESULT CALLBACK NewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
//    if (message == WM_GETTEXT) {
//        // WM_GETTEXT 메시지를 필터링하거나 처리
//        std::cout << "WM_GETTEXT 메시지 필터링\n";
//        return 0; // 메시지를 무시함
//    }
//
//    // 원래 윈도우 프로시저를 호출
//    return CallWindowProc(originalWndProc, hWnd, message, wParam, lParam);
//}
//#include <windows.h>
//
//BOOL AddMessageFilter(UINT message) {
//    typedef BOOL(WINAPI* PCHANGEWINDOWMESSAGEFILTER)(UINT, DWORD);
//    HMODULE hModUser32 = LoadLibrary(L"user32.dll");
//    if (hModUser32) {
//        PCHANGEWINDOWMESSAGEFILTER pChangeWindowMessageFilter = (PCHANGEWINDOWMESSAGEFILTER)GetProcAddress(hModUser32, "ChangeWindowMessageFilter");
//        if (pChangeWindowMessageFilter) {
//            return pChangeWindowMessageFilter(message, 1); // MSGFLT_ADD
//        }
//        FreeLibrary(hModUser32);
//    }
//    return FALSE;
//}
//
//int main() {
//    // 메시지 필터 추가
//    if (!AddMessageFilter(WM_GETTEXT)) {
//        DWORD dwError = GetLastError();
//        std::cout << "ChangeWindowMessageFilter 실패: " << dwError << "\n";
//        return 1;
//    }
//
//    // 나머지 코드
//    // Notepad++ 창 핸들 얻기
//    HWND hwndNotepad = FindWindow(L"Notepad++", NULL);
//    if (hwndNotepad == NULL) {
//        std::cout << "Notepad++ 창을 찾을 수 없습니다.\n";
//        return 1;
//    }
//
//    // Scintilla 자식 창 핸들 얻기
//    HWND hwndScintilla = FindWindowEx(hwndNotepad, NULL, L"Scintilla", NULL);
//    if (hwndScintilla == NULL) {
//        std::cout << "Scintilla 창을 찾을 수 없습니다.\n";
//        return 1;
//    }
//
//    // Scintilla 윈도우 프로시저 서브클래싱
//    SubclassScintilla(hwndScintilla);
//
//    // 메시지 루프 (필요한 경우)
//    MSG msg;
//    while (GetMessage(&msg, NULL, 0, 0)) {
//        TranslateMessage(&msg);
//        DispatchMessage(&msg);
//    }
//
//    return 0;
//}
//
//#include <windows.h>
//#include <iostream>
//
//LRESULT CALLBACK NewScintillaProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
//WNDPROC originalScintillaProc = NULL;
//
//LRESULT CALLBACK NewScintillaProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
//    if (msg == WM_GETTEXT) {
//        MessageBox(hwnd, L"WM_GETTEXT message detected!", L"Security Alert", MB_OK | MB_ICONWARNING);
//        // 원래 동작을 유지하거나 메시지를 무시할 수 있습니다.
//        // return 0; // 메시지 무시
//    }
//    // 원래 윈도우 프로시저 호출
//    return CallWindowProc(originalScintillaProc, hwnd, msg, wParam, lParam);
//}
//
//void SubclassScintilla() {
//    HWND hwndNotepad = FindWindow(L"Notepad++", NULL);
//    if (hwndNotepad) {
//        HWND hwndScintilla = FindWindowEx(hwndNotepad, NULL, L"Scintilla", NULL);
//        if (hwndScintilla) {
//            if (!SetWindowLongPtr(hwndScintilla, GWLP_WNDPROC, (LONG_PTR)NewScintillaProc)) {
//                DWORD dwError = GetLastError();
//                std::cout << dwError << std::endl;
//            }
//        }
//    }
//}
//
//int main() {
//    std::cout << "Waiting for Notepad++ to start..." << std::endl;
//
//    // Notepad++ 창을 찾을 때까지 잠시 대기
//    while (!FindWindow(L"Notepad++", NULL)) {
//        Sleep(100);
//    }
//
//    std::cout << "Notepad++ found. Subclassing Scintilla control." << std::endl;
//
//    // Scintilla 서브클래싱
//    SubclassScintilla();
//
//    // 메시지 루프
//    MSG msg;
//    while (GetMessage(&msg, NULL, 0, 0)) {
//        TranslateMessage(&msg);
//        DispatchMessage(&msg);
//    }
//
//    return (int)msg.wParam;
//}
