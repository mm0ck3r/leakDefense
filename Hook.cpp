// dllmain.cpp
#include "pch.h"
#include <windows.h>

LRESULT CALLBACK NewScintillaProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
WNDPROC originalScintillaProc = NULL;

LRESULT CALLBACK NewScintillaProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_GETTEXT) {
        MessageBox(hwnd, L"WM_GETTEXT message detected!", L"Security Alert", MB_OK | MB_ICONWARNING);
        // 원래 동작을 유지하거나 메시지를 무시할 수 있습니다.
        // return 0; // 메시지 무시
    }
    // 원래 윈도우 프로시저 호출
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
