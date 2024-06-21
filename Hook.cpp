// dllmain.cpp
#pragma warning(disable:4996)
#include "pch.h"
#include <windows.h>
#include <fstream>
#include <ctime>
#include <shlobj.h>

LRESULT CALLBACK NewScintillaProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
WNDPROC originalScintillaProc = NULL;

void LogMessage(const wchar_t* message) {
    // load detectLog.txt
    wchar_t path[MAX_PATH] = { 0, };
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, path))) {
        wcscat_s(path, MAX_PATH, L"\\detectLog.txt");

        // load now time (timestamp -> ymdhms format)
        std::time_t now = std::time(nullptr);
        struct tm timeInfo;
        localtime_s(&timeInfo, &now);

        wchar_t timeStr[100];
        wcsftime(timeStr, 100, L"%Y-%m-%d %H:%M:%S", &timeInfo);

        // write log
        std::wofstream detectLog;
        detectLog.open(path, std::ios_base::app);
        detectLog << timeStr << L" " << message << std::endl;
        detectLog.close();
    }
}

//char 에서 wchar_t 로의 형변환 함수
//https://goguri.tistory.com/1393
wchar_t* ConverCtoWC(const char* str)
{
    //wchar_t형 변수 선언
    wchar_t* pStr;
    //멀티 바이트 크기 계산 길이 반환
    int strSize = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, NULL);
    //wchar_t 메모리 할당
    pStr = new WCHAR[strSize];
    //형 변환
    MultiByteToWideChar(CP_ACP, 0, str, strlen(str) + 1, pStr, strSize);
    return pStr;
}

LRESULT CALLBACK NewScintillaProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_GETTEXT || msg == 2182 || msg == 2162 || msg == 2161) { // 2182: SCI_GETTEXT 2162: SCI_GETTEXTRANGE 2161: SCI_GETSELTEXT
        LogMessage(L"Hacking Detected!!");
        /*MessageBox(hwnd, L"WM_GETTEXT message detected!", L"Security Alert", MB_OK | MB_ICONWARNING);*/
        // return 0; // 메시지 무시
    }
    // 원래 윈도우 프로시저 호출
    //if (hwnd == FindWindowEx(FindWindow(L"Notepad++", NULL), NULL, L"Scintilla", NULL)) {
    //    char buffer[100];
    //    _itoa(msg, buffer, 10);
    //    LogMessage(ConverCtoWC(buffer));
    //}
    return CallWindowProc(originalScintillaProc, hwnd, msg, wParam, lParam);
}

void SubclassScintilla() {
    HWND hwndNotepad = FindWindow(L"Notepad++", NULL);
    if (hwndNotepad) {
        HWND hwndScintilla = FindWindowEx(hwndNotepad, NULL, L"Scintilla", NULL);
        if (hwndScintilla) {
            originalScintillaProc = (WNDPROC)SetWindowLongPtr(hwndScintilla, GWLP_WNDPROC, (LONG_PTR)NewScintillaProc);
        }
    }
}

extern "C" __declspec(dllexport) void StartSubclassing() {
    SubclassScintilla();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        StartSubclassing();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
