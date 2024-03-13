#include <windows.h>
#include <tchar.h>
#include <Tlhelp32.h>
#include <windowsx.h>
#include <string>
#include <sstream>
#include "resource.h"
#include "InjectHelper.h"

HMODULE g_hRemoteThreadDll = NULL;
HMODULE g_hInjectDll = NULL;
HHOOK g_hook = NULL;

typedef HHOOK (*PFN_InjectByWinHook)(HWND hwnd);
typedef BOOL (*PFN_EjectByWinHook)(HHOOK hook);

DWORD_PTR ParseHex(std::wstring const& text) {
    std::wstringstream ss;
    ss << std::hex;
    if (text.substr(0, 2) == L"0x" || text.substr(0, 2) == L"0X")
        ss << text.substr(2);
    else
        ss << text;
    DWORD_PTR value;
    ss >> value;
    return value;
}

void InjectByRemoteThread(HWND hwnd) {
    DWORD dwProcessID = 0;
    BOOL f;
    TCHAR szDllPath[MAX_PATH];
    dwProcessID = GetDlgItemInt(hwnd, IDC_EDIT_PROCESS_ID, &f, FALSE);
    if (dwProcessID == 0) {
        MessageBox(hwnd, TEXT("Inject Failed, no process id specified!"), TEXT("DLL Inject Tool"), MB_OK | MB_ICONERROR);
        return;
    }

    GetDlgItemText(hwnd, IDC_EDIT_DLL_PATH, szDllPath, MAX_PATH);
    g_hRemoteThreadDll = InjectHelper::InjectDllByRemoteThread(dwProcessID, szDllPath);
    if (g_hRemoteThreadDll)
        MessageBox(hwnd, TEXT("Inject Successful!"), TEXT("DLL Inject Tool"), MB_OK | MB_ICONINFORMATION);
    else
        MessageBox(hwnd, TEXT("Inject Failed!"), TEXT("DLL Inject Tool"), MB_OK | MB_ICONERROR);
}

void EjectByRemoteThread(HWND hwnd) {
    DWORD dwProcessID;
    BOOL f;
    dwProcessID = GetDlgItemInt(hwnd, IDC_EDIT_PROCESS_ID, &f, FALSE);

    //TCHAR szDllPath[MAX_PATH];
    //GetDlgItemText(hwnd, IDC_EDIT_DLL_PATH, szDllPath, MAX_PATH);

    if (InjectHelper::EjectDllByRemoteThread(dwProcessID, g_hRemoteThreadDll))
        MessageBox(hwnd, TEXT("Eject successful!"), TEXT("DLL Inject Tool"), MB_OK | MB_ICONINFORMATION);
    else
        MessageBox(hwnd, TEXT("Eject failed!"), TEXT("DLL Inject Tool"), MB_OK | MB_ICONERROR);
}

void InjectByWindowHook(HWND hwnd) {
    TCHAR szDllPath[MAX_PATH];
    GetDlgItemText(hwnd, IDC_EDIT_DLL_PATH, szDllPath, MAX_PATH);

    TCHAR szHWND[MAX_PATH];
    GetDlgItemText(hwnd, IDC_EDIT_WND_HWND, szHWND, MAX_PATH);

    HWND hHookWindow = NULL;
    if (_tcslen(szHWND) > 0) {
        hHookWindow = (HWND)ParseHex(szHWND);
    }

    if (hHookWindow == NULL) {
        MessageBox(hwnd, TEXT("Inject failed, no window handle specified!"), TEXT("DLL Inject Tool"), MB_OK | MB_ICONERROR);
        return;
    }

    g_hInjectDll = LoadLibrary(szDllPath);
    if (g_hInjectDll == NULL) {
        MessageBox(hwnd, TEXT("Inject failed, can't load dll!"), TEXT("DLL Inject Tool"), MB_OK | MB_ICONERROR);
        return;
    }

    PFN_InjectByWinHook pfnInject = (PFN_InjectByWinHook)GetProcAddress(g_hInjectDll, "InjectByWinHook");
    if (pfnInject == NULL) {
        MessageBox(hwnd, TEXT("Inject failed, can't get InjectByWinHook function address from dll!"), TEXT("Inject"), MB_OK | MB_ICONERROR);
        return;
    }

    g_hook = pfnInject(hHookWindow);

    if (g_hook) {
        MessageBox(hwnd, TEXT("Inject successful!"), TEXT("DLL Inject Tool"), MB_OK | MB_ICONINFORMATION);
    }
    else {
        MessageBox(hwnd, TEXT("Inject failed!"), TEXT("DLL Inject Tool"), MB_OK | MB_ICONERROR);
    }
}

void EjectByWindowHook(HWND hwnd) {
    PFN_EjectByWinHook pfnEject = (PFN_EjectByWinHook)GetProcAddress(g_hInjectDll, "EjectByWinHook");
    if (pfnEject == NULL) {
        MessageBox(hwnd, TEXT("Eject failed, can't get EjectByWinHook function address"), TEXT("Eject"), MB_OK | MB_ICONERROR);
        return;
    }

    if (pfnEject(g_hook))
        MessageBox(hwnd, TEXT("Eject successful"), TEXT("DLL Inject Tool"), MB_OK | MB_ICONINFORMATION);
    else
        MessageBox(hwnd, TEXT("Eject failed"), TEXT("DLL Inject Tool"), MB_OK | MB_ICONERROR);

    FreeLibrary(g_hInjectDll);
}

void InjectByAPC(HWND hwnd) {
    DWORD dwProcessID = 0, dwThreadID = 0;
    BOOL f;
    TCHAR szDllPath[MAX_PATH];
    dwProcessID = GetDlgItemInt(hwnd, IDC_EDIT_APC_PROCESS_ID, &f, FALSE);
    if (dwProcessID == 0) {
        MessageBox(hwnd, TEXT("Inject Failed, no process id specified!"), TEXT("DLL Inject Tool"), MB_OK | MB_ICONERROR);
        return;
    }

    dwThreadID = GetDlgItemInt(hwnd, IDC_EDIT_APC_THREAD_ID, &f, FALSE);

    GetDlgItemText(hwnd, IDC_EDIT_DLL_PATH, szDllPath, MAX_PATH);

    if (InjectHelper::InjectDllByAPC(dwProcessID, dwThreadID, szDllPath))
        MessageBox(hwnd, TEXT("Inject Successful!"), TEXT("DLL Inject Tool"), MB_OK | MB_ICONINFORMATION);
    else
        MessageBox(hwnd, TEXT("Inject Failed!"), TEXT("DLL Inject Tool"), MB_OK | MB_ICONERROR);
}

void EjectByAPC(HWND hwnd) {
}

void Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
    switch (id) {
        case IDCANCEL:
            EndDialog(hwnd, id);
            break;
        case IDC_BUTTON_REMOTE_THREAD_INJECT: {
            InjectByRemoteThread(hwnd);
            break;
        }
        case IDC_BUTTON_REMOTE_THREAD_EJECT: {
            EjectByRemoteThread(hwnd);
            break;
        }
        case IDC_BUTTON_WINDOW_HOOK_INJECT: {
            InjectByWindowHook(hwnd);
            break;
        }
        case IDC_BUTTON_WINDOW_HOOK_EJECT: {
            EjectByWindowHook(hwnd);
            break;
        }
        case IDC_BUTTON_APC_INJECT: {
            InjectByAPC(hwnd);
            break;
        }
        case IDC_BUTTON_APC_EJECT: {
            EjectByAPC(hwnd);
            break;
        }
    }
}

BOOL Dlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {
    SetDlgItemText(hwnd, IDC_EDIT_DLL_PATH, TEXT(""));
    return TRUE;
}

BOOL WINAPI Dlg_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_INITDIALOG: {
            SetDlgMsgResult(hwnd, uMsg, HANDLE_WM_INITDIALOG((hwnd), (wParam), (lParam), (Dlg_OnInitDialog)));
            break;
        }
        case WM_COMMAND: {
            SetDlgMsgResult(hwnd, uMsg, HANDLE_WM_COMMAND((hwnd), (wParam), (lParam), (Dlg_OnCommand)));
            break;
        }
    }

    return (FALSE);
}

int WINAPI _tWinMain(HINSTANCE hInstExe, HINSTANCE, PTSTR pszCmdLine, int) {
    DialogBox(hInstExe, MAKEINTRESOURCE(IDD_DllInjecter), NULL, Dlg_Proc);

    return 0;
}