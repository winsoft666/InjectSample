#pragma once

#include <windows.h>
#include <tchar.h>

class InjectHelper {
public:
	InjectHelper();
	~InjectHelper();

public:
	static BOOL EnablePrivilege(LPCTSTR szPrivilege, BOOL fEnable);

	static HHOOK InjectSelfByHook(HWND hwnd, HOOKPROC hookProc);
	static BOOL EjectSelfByHook(HHOOK hook);

	static BOOL InjectDllByRemoteThread(DWORD dwProcessID, LPCTSTR pszDllPath);
	static BOOL EjectDllByRemoteThread(DWORD dwProcessID, LPCTSTR pszDllPath);

	static BOOL ScarFreeInject(DWORD dwProcessID, LPCTSTR pszDllPath);

	static HWND FindProcessWindow(DWORD dwProcessId, LPCTSTR lpClassName, LPCTSTR lpWindowName, BOOL bMainWindow = FALSE);
private:

};
