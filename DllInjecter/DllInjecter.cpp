#include <windows.h>
#include <tchar.h>
#include <Tlhelp32.h>
#include <windowsx.h>
#include "resource.h"
#include "InjectHelper.h"

HMODULE g_hInjectDll = NULL;
HHOOK g_hook = NULL;

typedef HHOOK(*PFN_InjectDllByHook)(HWND hwnd);
typedef BOOL(*PFN_EjectDllByHook)(HHOOK hook);


void Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
    switch(id) {
        case IDCANCEL:
            EndDialog(hwnd, id);
            break;
        case IDC_BUTTON_INJECT: {
            DWORD dwProcessID;
            BOOL f;
            TCHAR szDllPath[MAX_PATH];
            dwProcessID = GetDlgItemInt(hwnd, IDC_EDIT_PROCESS_ID, &f, FALSE);
            GetDlgItemText(hwnd, IDC_EDIT_DLL_PATH, szDllPath, MAX_PATH);
            if(InjectHelper::InjectDllByRemoteThread(dwProcessID, szDllPath))
                MessageBox(hwnd, TEXT("Inject Successful"), TEXT("Inject"), MB_OK | MB_ICONINFORMATION);
            else
                MessageBox(hwnd, TEXT("Inject Failed"), TEXT("Inject"), MB_OK | MB_ICONERROR);
            break;
        }

        case IDC_BUTTON_EJECT: {
            DWORD dwProcessID;
            BOOL f;
            TCHAR szDllPath[MAX_PATH];
            dwProcessID = GetDlgItemInt(hwnd, IDC_EDIT_PROCESS_ID, &f, FALSE);
            GetDlgItemText(hwnd, IDC_EDIT_DLL_PATH, szDllPath, MAX_PATH);
            if(InjectHelper::EjectDllByRemoteThread(dwProcessID, szDllPath))
                MessageBox(hwnd, TEXT("Eject successful"), TEXT("Eject"), MB_OK | MB_ICONINFORMATION);
            else
                MessageBox(hwnd, TEXT("Eject failed"), TEXT("Eject"), MB_OK | MB_ICONERROR);
            break;
        }
		case IDC_BUTTON_HOOK:
		{
			TCHAR szDllPath[MAX_PATH];
			GetDlgItemText(hwnd, IDC_EDIT_DLL_PATH, szDllPath, MAX_PATH);

			TCHAR szClassName[MAX_PATH];
			GetDlgItemText(hwnd, IDC_EDIT_CLASS_NAME, szClassName, MAX_PATH);

			HWND hHookWindow = FindWindow(szClassName, szClassName);
			if (hHookWindow == NULL) {
				MessageBox(hwnd, TEXT("Inject failed, can't find hook window"), TEXT("Inject"), MB_OK | MB_ICONERROR);
				break;
			}

			g_hInjectDll = LoadLibrary(szDllPath);
			if (g_hInjectDll == NULL) {
				MessageBox(hwnd, TEXT("Inject failed, can't find dll"), TEXT("Inject"), MB_OK | MB_ICONERROR);
				break;
			}

			PFN_InjectDllByHook pfnInject = (PFN_InjectDllByHook)GetProcAddress(g_hInjectDll, "InjectDllByHook");
			if (pfnInject == NULL) {
				MessageBox(hwnd, TEXT("Inject failed, can't get InjectDllByHook function address"), TEXT("Inject"), MB_OK | MB_ICONERROR);
				break;
			}

			g_hook = pfnInject(hHookWindow);

			if (g_hook) {
				MessageBox(hwnd, TEXT("Inject successful"), TEXT("Inject"), MB_OK | MB_ICONINFORMATION);
			}
			else {
				MessageBox(hwnd, TEXT("Inject failed"), TEXT("Inject"), MB_OK | MB_ICONERROR);
			}

			break;
		}
		case IDC_BUTTON_UNHOOK:
		{
			PFN_EjectDllByHook pfnEject = (PFN_EjectDllByHook)GetProcAddress(g_hInjectDll, "EjectDllByHook");
			if (pfnEject == NULL) {
				MessageBox(hwnd, TEXT("Eject failed, can't get EjectDllByHook function address"), TEXT("Eject"), MB_OK | MB_ICONERROR);
				break;
			}

			if (pfnEject(g_hook))
				MessageBox(hwnd, TEXT("Eject successful"), TEXT("Eject"), MB_OK | MB_ICONINFORMATION);
			else
				MessageBox(hwnd, TEXT("Eject failed"), TEXT("Eject"), MB_OK | MB_ICONERROR);
			
			FreeLibrary(g_hInjectDll);
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

    return(FALSE);
}

int WINAPI _tWinMain(HINSTANCE hInstExe, HINSTANCE, PTSTR pszCmdLine, int) {

    DialogBox(hInstExe, MAKEINTRESOURCE(IDD_DllInjecter), NULL, Dlg_Proc);

    return 0;
}