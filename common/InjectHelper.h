#pragma once

#include <windows.h>
#include <tchar.h>

class InjectHelper {
   public:
    static BOOL EnablePrivilege(LPCTSTR szPrivilege, BOOL fEnable);

    static HHOOK InjectSelfByWinHook(HWND hwnd, HOOKPROC hookProc);
    static BOOL EjectSelfByWinHook(HHOOK hook);

    static HMODULE InjectDllByRemoteThread(DWORD dwProcessID, LPCTSTR pszDllPath);
    static BOOL EjectDllByRemoteThread(DWORD dwProcessID, HMODULE hDLL);
    static BOOL EjectDllByRemoteThread(DWORD dwProcessID, LPCTSTR pszDllPath);

    static BOOL InjectDllByAPC(DWORD dwProcessID, DWORD dwThreadID, LPCTSTR pszDllPath);
    static BOOL EjectDllByAPC(DWORD dwProcessID, LPCTSTR pszDllPath);

    static BOOL ScarFreeInject(DWORD dwProcessID, LPCTSTR pszDllPath);

    static HWND FindProcessWindow(DWORD dwProcessId,
                                  LPCTSTR lpClassName,
                                  LPCTSTR lpWindowName,
                                  BOOL bMainWindow = FALSE);
};
