#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <process.h>
#include "InjectHelper.h"
#include "DlgTroy.h"

HHOOK g_hook = NULL;
WNDPROC g_oldProc = NULL;
DlgTroy* g_pDlgInject = NULL;
HMODULE g_hDllModule = NULL;

#define WUM_CREATE_USER_WINDOW (WM_USER+1)

LRESULT CALLBACK WndProc_Trampoline(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CreateUserWindowByDuilib() {
	HWND hParent = InjectHelper::FindProcessWindow(GetCurrentProcessId(), TEXT("Button"), TEXT("Button1"), FALSE);
	
	DWORD oldStyle = GetWindowLong(hParent, GWL_STYLE);
	SetWindowLong(hParent, GWL_STYLE, oldStyle | WS_CLIPCHILDREN);

	RECT rect;
	GetWindowRect(hParent, &rect);


	CPaintManagerUI::SetInstance(g_hDllModule);

	g_pDlgInject = new DlgTroy();
	if (g_pDlgInject->Create(hParent, TEXT("关于"), UI_WNDSTYLE_CHILD, 0L, 0, 0, 203, 203)) {
		::SetWindowPos(g_pDlgInject->GetHWND(), NULL, (rect.right - rect.left - 203) / 2, (rect.bottom - rect.top - 203) / 2, 0, 0, SWP_NOSIZE);
		g_pDlgInject->ShowWindow();
	}

	return TRUE;
}

LRESULT CALLBACK WndProc_Trampoline(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WUM_CREATE_USER_WINDOW:
		CreateUserWindowByDuilib();
		break;
	}

	return CallWindowProc(g_oldProc, hwnd, message, wParam, lParam);
}

unsigned int __stdcall PluginProc(LPVOID pArg) {
	MessageBox(NULL, TEXT("我已经被注入啦"), TEXT("信息"), MB_OK | MB_ICONASTERISK);

	HWND hMainWindow = InjectHelper::FindProcessWindow(GetCurrentProcessId(), TEXT("#32770"), TEXT("Test"), TRUE);

	g_oldProc = (WNDPROC)SetWindowLong(hMainWindow, DWL_DLGPROC, (LONG)WndProc_Trampoline);


	::PostMessage(hMainWindow, WUM_CREATE_USER_WINDOW, 0, 0);

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  fdwReason, LPVOID lpReserved) {
	HANDLE hThread = NULL;

    switch(fdwReason) {
        case DLL_PROCESS_ATTACH:
        {
			g_hDllModule = hModule;
			// 使用注册表方式和CreateRemoteThread方式注入时，一般在此处创建线程
			//

			hThread = (HANDLE)_beginthreadex(NULL, 0, PluginProc, NULL, 0, NULL);
			if (hThread) {
				CloseHandle(hThread); // 关闭句柄，防止句柄泄漏
			}
            break;
        }
        case DLL_THREAD_ATTACH:
        {
            break;
        }
        case DLL_THREAD_DETACH:
        {
            break;
        }
        case DLL_PROCESS_DETACH:
        {
            break;
        }
    }
    return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//                  使用SetWindowsHookEx钩子方式注入
//////////////////////////////////////////////////////////////////////////
//
LRESULT CALLBACK HookProc_GetMsg(int code, WPARAM wParam, LPARAM lParam) {
	char szMsg[512] = { 0 };
	sprintf_s(szMsg, 512, "code: %d, wParam: %d, lParam: %d", code, wParam, lParam);
	OutputDebugStringA(szMsg);

	return CallNextHookEx(g_hook, code, wParam, lParam);;
}

HHOOK InjectDllByHook(HWND hwnd) {
	return InjectHelper::InjectSelfByHook(hwnd, HookProc_GetMsg);
}

BOOL EjectDllByHook(HHOOK hook) {
	return InjectHelper::EjectSelfByHook(hook);
}