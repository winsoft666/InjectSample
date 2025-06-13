#include "pch.h"
#include "framework.h"
#include "InjectTool.h"
#include "InjectToolDlg.h"
#include "afxdialogex.h"
#include <Shlwapi.h>
#include "../common/InjectHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

typedef HHOOK (*PFN_InjectByWinHook)(HWND hwnd);
typedef BOOL (*PFN_EjectByWinHook)(HHOOK hook);

CInjectToolDlg::CInjectToolDlg(CWnd* pParent /*=nullptr*/) :
    CDialogEx(IDD_INJECTTOOL_DIALOG, pParent) {
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CInjectToolDlg::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CInjectToolDlg, CDialogEx)
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()
ON_BN_CLICKED(IDC_BUTTON_REMOTE_THREAD_INJECT, &CInjectToolDlg::OnBnClickedButtonRemoteThreadInject)
ON_BN_CLICKED(IDC_BUTTON_REMOTE_THREAD_EJECT, &CInjectToolDlg::OnBnClickedButtonRemoteThreadEject)
ON_BN_CLICKED(IDC_BUTTON_WINDOW_HOOK_INJECT, &CInjectToolDlg::OnBnClickedButtonWindowHookInject)
ON_BN_CLICKED(IDC_BUTTON_WINDOW_HOOK_EJECT, &CInjectToolDlg::OnBnClickedButtonWindowHookEject)
ON_BN_CLICKED(IDC_BUTTON_APC_INJECT, &CInjectToolDlg::OnBnClickedButtonApcInject)
ON_BN_CLICKED(IDC_BUTTON_APC_EJECT, &CInjectToolDlg::OnBnClickedButtonApcEject)
ON_BN_CLICKED(IDC_BUTTON_SET_DEBUG_PRIV, &CInjectToolDlg::OnBnClickedButtonSetDebugPriv)
END_MESSAGE_MAP()

BOOL CInjectToolDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);   // Set big icon
    SetIcon(m_hIcon, FALSE);  // Set small icon

    // TODO: Add extra initialization here
    TCHAR szDir[MAX_PATH] = {0};
    GetModuleFileName(NULL, szDir, MAX_PATH);
    PathRemoveFileSpec(szDir);
    PathAddBackslash(szDir);

    SetDlgItemText(IDC_EDIT_REMOTE_THREAD_DLL_PATH, CString(szDir) + TEXT("SimpleDLL.dll"));
    SetDlgItemText(IDC_EDIT_WIN_HOOK_DLL_PATH, CString(szDir) + TEXT("TroyDLL.dll"));
    SetDlgItemText(IDC_EDIT_APC_DLL_PATH, CString(szDir) + TEXT("SimpleDLL.dll"));

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CInjectToolDlg::OnPaint() {
    if (IsIconic()) {
        CPaintDC dc(this);  // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else {
        CDialogEx::OnPaint();
    }
}

DWORD_PTR CInjectToolDlg::ParseHex(std::wstring const& text) {
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

HCURSOR CInjectToolDlg::OnQueryDragIcon() {
    return static_cast<HCURSOR>(m_hIcon);
}

void CInjectToolDlg::OnBnClickedButtonRemoteThreadInject() {
    DWORD dwProcessID = 0;
    BOOL f;
    TCHAR szDllPath[MAX_PATH];
    dwProcessID = GetDlgItemInt(IDC_EDIT_REMOTE_THREAD_PID, &f, FALSE);
    if (dwProcessID == 0) {
        MessageBox(TEXT("Inject Failed, no process id specified!"), TEXT("Inject Tool"), MB_OK | MB_ICONERROR);
        return;
    }

    GetDlgItemText(IDC_EDIT_REMOTE_THREAD_DLL_PATH, szDllPath, MAX_PATH);
    m_hRemoteThreadDll = InjectHelper::InjectDllByRemoteThread(dwProcessID, szDllPath);
    if (m_hRemoteThreadDll)
        MessageBox(TEXT("Inject successful!"), TEXT("Inject Tool"), MB_OK | MB_ICONINFORMATION);
    else
        MessageBox(TEXT("Inject failed!"), TEXT("Inject Tool"), MB_OK | MB_ICONERROR);
}

void CInjectToolDlg::OnBnClickedButtonRemoteThreadEject() {
    DWORD dwProcessID;
    BOOL f;
    dwProcessID = GetDlgItemInt(IDC_EDIT_REMOTE_THREAD_PID, &f, FALSE);

    if (InjectHelper::EjectDllByRemoteThread(dwProcessID, m_hRemoteThreadDll))
        MessageBox(TEXT("Eject successful!"), TEXT("Inject Tool"), MB_OK | MB_ICONINFORMATION);
    else
        MessageBox(TEXT("Eject failed!"), TEXT("Inject Tool"), MB_OK | MB_ICONERROR);
}

void CInjectToolDlg::OnBnClickedButtonWindowHookInject() {
    TCHAR szDllPath[MAX_PATH];
    GetDlgItemText(IDC_EDIT_WIN_HOOK_DLL_PATH, szDllPath, MAX_PATH);

    TCHAR szHWND[MAX_PATH];
    GetDlgItemText(IDC_EDIT_WND_HWND, szHWND, MAX_PATH);

    HWND hHookWindow = NULL;
    if (_tcslen(szHWND) > 0) {
        hHookWindow = (HWND)ParseHex(szHWND);
    }

    if (hHookWindow == NULL) {
        MessageBox(TEXT("Inject failed, no window handle specified!"), TEXT("Inject Tool"), MB_OK | MB_ICONERROR);
        return;
    }

    m_hWinHookDll = LoadLibrary(szDllPath);
    if (m_hWinHookDll == NULL) {
        MessageBox(TEXT("Inject failed, can't load dll!"), TEXT("Inject Tool"), MB_OK | MB_ICONERROR);
        return;
    }

    PFN_InjectByWinHook pfnInject = (PFN_InjectByWinHook)GetProcAddress(m_hWinHookDll, "InjectByWinHook");
    if (pfnInject == NULL) {
        MessageBox(TEXT("Inject failed, can't get InjectByWinHook function address from dll!"), TEXT("Inject"), MB_OK | MB_ICONERROR);
        return;
    }

    m_hook = pfnInject(hHookWindow);

    if (m_hook) {
        MessageBox(TEXT("Inject successful!"), TEXT("Inject Tool"), MB_OK | MB_ICONINFORMATION);
    }
    else {
        MessageBox(TEXT("Inject failed!"), TEXT("Inject Tool"), MB_OK | MB_ICONERROR);
    }
}

void CInjectToolDlg::OnBnClickedButtonWindowHookEject() {
    PFN_EjectByWinHook pfnEject = (PFN_EjectByWinHook)GetProcAddress(m_hWinHookDll, "EjectByWinHook");
    if (pfnEject == NULL) {
        MessageBox(TEXT("Eject failed, can't get EjectByWinHook function address!"), TEXT("Eject"), MB_OK | MB_ICONERROR);
        return;
    }

    if (pfnEject(m_hook))
        MessageBox(TEXT("Eject successful"), TEXT("Inject Tool"), MB_OK | MB_ICONINFORMATION);
    else
        MessageBox(TEXT("Eject failed"), TEXT("Inject Tool"), MB_OK | MB_ICONERROR);

    FreeLibrary(m_hWinHookDll);
}

void CInjectToolDlg::OnBnClickedButtonApcInject() {
    DWORD dwProcessID = 0, dwThreadID = 0;
    BOOL f;
    TCHAR szDllPath[MAX_PATH];
    dwProcessID = GetDlgItemInt(IDC_EDIT_APC_PID, &f, FALSE);
    if (dwProcessID == 0) {
        MessageBox(TEXT("Inject failed, no process id specified!"), TEXT("Inject Tool"), MB_OK | MB_ICONERROR);
        return;
    }

    dwThreadID = GetDlgItemInt(IDC_EDIT_APC_TID, &f, FALSE);

    GetDlgItemText(IDC_EDIT_APC_DLL_PATH, szDllPath, MAX_PATH);

    if (InjectHelper::InjectDllByAPC(dwProcessID, dwThreadID, szDllPath))
        MessageBox(TEXT("Inject successful!"), TEXT("Inject Tool"), MB_OK | MB_ICONINFORMATION);
    else
        MessageBox(TEXT("Inject failed!"), TEXT("Inject Tool"), MB_OK | MB_ICONERROR);
}

void CInjectToolDlg::OnBnClickedButtonApcEject() {
    MessageBox(TEXT("The DLL injected by APC not support eject!"), TEXT("Inject Tool"), MB_OK | MB_ICONERROR);
}

void CInjectToolDlg::OnBnClickedButtonSetDebugPriv() {
    if (InjectHelper::EnablePrivilege(SE_DEBUG_NAME, TRUE)) {
        MessageBox(TEXT("Set debug privilege successful!"), TEXT("Inject Tool"), MB_OK | MB_ICONINFORMATION);
    }
    else {
        MessageBox(TEXT("Set debug privilege failed!"), TEXT("Inject Tool"), MB_OK | MB_ICONERROR);
    }
}
