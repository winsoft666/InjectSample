#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <process.h>
#include "InjectHelper.h"
#include "DlgTroy.h"

#pragma data_seg("SharedVar")
HHOOK g_hook = NULL;
#pragma data_seg()

#pragma comment(linker, "/SECTION:SharedVar,RWS")

DlgTroy* g_pDlg = NULL;
HMODULE g_hDllModule = NULL;

#define WUM_CREATE_USER_WINDOW (WM_USER + 1)

BOOL CreateWindowByDuilib() {
    HWND hParent = InjectHelper::FindProcessWindow(GetCurrentProcessId(), TEXT("Button"), TEXT("Button1"), FALSE);

    DWORD oldStyle = GetWindowLong(hParent, GWL_STYLE);
    SetWindowLong(hParent, GWL_STYLE, oldStyle | WS_CLIPCHILDREN);

    RECT rect;
    GetWindowRect(hParent, &rect);

    CPaintManagerUI::SetInstance(g_hDllModule);

    g_pDlg = new DlgTroy();
    if (g_pDlg->Create(hParent, TEXT("Troy"), UI_WNDSTYLE_CHILD, 0L, 0, 0, 203, 203)) {
        ::SetWindowPos(g_pDlg->GetHWND(), NULL, (rect.right - rect.left - 203) / 2, (rect.bottom - rect.top - 203) / 2, 0, 0, SWP_NOSIZE);
        g_pDlg->ShowWindow();
    }

    return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
    HANDLE hThread = NULL;

    switch (fdwReason) {
        case DLL_PROCESS_ATTACH: {
            OutputDebugStringA("DLL_PROCESS_ATTACH");
            // Troy.dll will be load twice (DLLInjectTool.exe, TestInjectTarget.exe)
            // So can not show messagebox here.
            g_hDllModule = hModule;
            break;
        }
        case DLL_THREAD_ATTACH: {
            OutputDebugStringA("DLL_THREAD_DETACH");
            break;
        }
        case DLL_THREAD_DETACH: {
            OutputDebugStringA("DLL_THREAD_DETACH");
            break;
        }
        case DLL_PROCESS_DETACH: {
            OutputDebugStringA("DLL_PROCESS_DETACH");
            if (g_pDlg) {
                g_pDlg->Close();
                g_pDlg = NULL;
            }
            break;
        }
    }
    return TRUE;
}

LRESULT CALLBACK HookProc_GetMsg(int code, WPARAM wParam, LPARAM lParam) {
    static BOOL isFirst = TRUE;
    if (isFirst) {
        isFirst = FALSE;
        MessageBox(NULL, TEXT("I have been injected!"), TEXT("Troy"), MB_OK | MB_ICONASTERISK);

        CreateWindowByDuilib();
    }

    char szMsg[512] = {0};
    sprintf_s(szMsg, 512, "[Troy] code: %d, wParam: %d, lParam: %d", code, wParam, lParam);
    OutputDebugStringA(szMsg);

    return CallNextHookEx(g_hook, code, wParam, lParam);
}

// Export functions
//
HHOOK InjectByWinHook(HWND hwnd) {
    // This function be called in DLLInjectTool.exe, we need share g_hook to TestInjectTarget.exe
    g_hook = InjectHelper::InjectSelfByWinHook(hwnd, HookProc_GetMsg);
    return g_hook;
}

BOOL EjectByWinHook(HHOOK hook) {
    return InjectHelper::EjectSelfByWinHook(hook);
}