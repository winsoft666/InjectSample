#include "InjectHelper.h"
#include <Tlhelp32.h>
#include <vector>

typedef struct tagWNDINFO {
	DWORD dwProcessId;
	std::vector<HWND> vHWNDs; // for return.
	LPCTSTR lpClassName;
	LPCTSTR lpWindowName;
} WNDINFO, *LPWNDINFO;

BOOL CALLBACK MyEnumProc(HWND hWnd, LPARAM lParam) 
{
	LPWNDINFO pWndInfo = (LPWNDINFO)lParam;

	DWORD dwProcId = 0;
	GetWindowThreadProcessId(hWnd, &dwProcId);

	if (dwProcId == pWndInfo->dwProcessId) {
		pWndInfo->vHWNDs.push_back(hWnd);
		return TRUE;
	}
	return TRUE;
}

InjectHelper::InjectHelper() {

}

InjectHelper::~InjectHelper() {

}

HWND InjectHelper::FindProcessWindow(DWORD dwProcessId, LPCTSTR lpClassName, LPCTSTR lpWindowName, BOOL bMainWindow /*= FALSE*/) {
	HWND hResult = NULL;

	WNDINFO wndInfo;
	wndInfo.dwProcessId = dwProcessId;
	wndInfo.lpClassName = lpClassName;
	wndInfo.lpWindowName = lpWindowName;

	EnumWindows(MyEnumProc, (LPARAM)&wndInfo);

	for (size_t i = 0; i < wndInfo.vHWNDs.size(); i++) {
		if (bMainWindow) {
			if (GetParent(wndInfo.vHWNDs[i]) == NULL) {
				TCHAR szClassName[MAX_PATH] = { 0 };
				TCHAR szWindowName[MAX_PATH] = { 0 };
				GetClassName(wndInfo.vHWNDs[i], szClassName, MAX_PATH);
				GetWindowText(wndInfo.vHWNDs[i], szWindowName, MAX_PATH);

				if ((lpClassName && _tcscmp(lpClassName, szClassName) == 0 && lpWindowName && _tcscmp(lpWindowName, szWindowName) == 0) ||
					(lpClassName && _tcscmp(lpClassName, szClassName) == 0) || 
					(lpWindowName && _tcscmp(lpWindowName, szWindowName) == 0)) {
					hResult = wndInfo.vHWNDs[i];
					break;
				}
			}
		}
		else {
			hResult = FindWindowEx(wndInfo.vHWNDs[i], NULL, lpClassName, lpWindowName);
			if (hResult) {
				break;
			}
		}
	}

	return hResult;
}

BOOL InjectHelper::EnablePrivilege(LPCTSTR szPrivilege, BOOL fEnable) {
	BOOL fOk = FALSE;
	HANDLE hToken = NULL;

	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) {
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		LookupPrivilegeValue(NULL, szPrivilege, &tp.Privileges[0].Luid);
		tp.Privileges->Attributes = fEnable ? SE_PRIVILEGE_ENABLED : 0;
		AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
		fOk = (GetLastError() == ERROR_SUCCESS);

		CloseHandle(hToken);
	}

	return fOk;
}

HHOOK InjectHelper::InjectSelfByHook(HWND hwnd, HOOKPROC hookProc) {
	DWORD dwThreadId = 0;
	HHOOK hHook = NULL;

	__try {
		if (!EnablePrivilege(SE_DEBUG_NAME, TRUE)) {
			__leave;
		}

		dwThreadId = GetWindowThreadProcessId(hwnd, NULL);
		if (dwThreadId == 0) {
			__leave;
		}

		// 获取DLL自身的句柄
		HMODULE hModule = NULL;
		GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			(LPCWSTR)InjectSelfByHook, &hModule);

		hHook = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)hookProc, hModule, dwThreadId);
	}
	__finally {

	}

	return (hHook);
}

BOOL InjectHelper::EjectSelfByHook(HHOOK hook) {
	return UnhookWindowsHookEx(hook);
}

BOOL InjectHelper::InjectDllByRemoteThread(DWORD dwProcessID, LPCTSTR pszDllPath) {
	BOOL bRet = FALSE;
	HANDLE hTargeProcess = NULL;
	HANDLE hRemoteThread = NULL;
	PVOID pVM4LoadLibrary = NULL;
	PVOID pVM4DllPath = NULL;
	SIZE_T dllPathSize = 0;

	__try {
		if (pszDllPath == NULL) {
			__leave;
		}

		if (!EnablePrivilege(SE_DEBUG_NAME, TRUE)) {
			__leave;
		}

		hTargeProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID);
		if (!hTargeProcess) {
			__leave;
		}

		dllPathSize = _tcslen(pszDllPath)*sizeof(TCHAR);
		pVM4DllPath = VirtualAllocEx(hTargeProcess, NULL, dllPathSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (!pVM4DllPath) {
			__leave;
		}

		if (!WriteProcessMemory(hTargeProcess, pVM4DllPath, pszDllPath, dllPathSize, NULL)) {
			__leave;
		}

		HMODULE hKernel32 = GetModuleHandle(TEXT("kernel32.dll"));
#ifdef UNICODE
		LPVOID pLoadLibraryAAddr = (LPVOID)GetProcAddress(hKernel32, "LoadLibraryW");
#else
		LPVOID pLoadLibraryAAddr = (LPVOID)GetProcAddress(hKernel32, "LoadLibraryA");
#endif
		
		hRemoteThread = CreateRemoteThread(hTargeProcess, NULL, 0, (DWORD(WINAPI *)(LPVOID))pLoadLibraryAAddr, pVM4DllPath, 0, NULL);
		if (!hRemoteThread) {
			__leave;
		}

		WaitForSingleObject(hRemoteThread, INFINITE);

		bRet = TRUE;
	}
	__finally {
		if (hTargeProcess && pVM4DllPath) {
			VirtualFreeEx(hTargeProcess, pVM4DllPath, dllPathSize, MEM_RELEASE);
		}

		if (hRemoteThread) {
			CloseHandle(hRemoteThread);
		}

		if (hTargeProcess) {
			CloseHandle(hTargeProcess);
		}
	}

	return bRet;
}

BOOL InjectHelper::EjectDllByRemoteThread(DWORD dwProcessID, LPCTSTR pszDllPath) {
	BOOL bOk = FALSE;
	HANDLE hTHSnapshot = NULL;
	HANDLE hProcess = NULL;
	HANDLE hThread = NULL;

	__try {
		hTHSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessID);
		if (hTHSnapshot == INVALID_HANDLE_VALUE) {
			__leave;
		}

		MODULEENTRY32W me = { sizeof(me) };
		BOOL bFound = FALSE;
		BOOL bMoreMods = Module32First(hTHSnapshot, &me);
		for (; bMoreMods; bMoreMods = Module32Next(hTHSnapshot, &me)) {
			bFound = (_tcsicmp(me.szModule, pszDllPath) == 0) ||
				(_tcsicmp(me.szExePath, pszDllPath) == 0);
			if (bFound) 
				break;
		}

		if (!bFound) {
			__leave;
		}

		hProcess = OpenProcess(
			PROCESS_QUERY_INFORMATION |
			PROCESS_CREATE_THREAD |
			PROCESS_VM_OPERATION,  // For CreateRemoteThread
			FALSE, dwProcessID);
		if (hProcess == NULL) {
			__leave;
		}

		PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)
			GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "FreeLibrary");
		if (pfnThreadRtn == NULL) {
			__leave;
		}

		hThread = CreateRemoteThread(hProcess, NULL, 0, pfnThreadRtn, me.modBaseAddr, 0, NULL);
		if (hThread == NULL) {
			__leave;
		}

		WaitForSingleObject(hThread, INFINITE);

		bOk = TRUE;
	}
	__finally {
		if (hTHSnapshot != NULL) {
			CloseHandle(hTHSnapshot);
		}
		if (hThread != NULL) {
			CloseHandle(hThread);
		}
		if (hProcess != NULL) {
			CloseHandle(hProcess);
		}
	}

	return(bOk);
}

BOOL InjectHelper::ScarFreeInject(DWORD dwProcessID, LPCTSTR pszDllPath) {
	return FALSE;
}
