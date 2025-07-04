#include "stdafx.h"
#include "Test.h"
#include "TestDlg.h"
#include "afxdialogex.h"
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CAboutDlg : public CDialogEx {
   public:
    CAboutDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

   protected:
    virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV 支持

   protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() :
    CDialogEx(IDD_ABOUTBOX) {
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

CTestDlg::CTestDlg(CWnd* pParent /*=NULL*/) :
    CDialog(IDD_TEST_DIALOG, pParent) {
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestDlg::DoDataExchange(CDataExchange* pDX) {
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTestDlg, CDialog)
ON_WM_SYSCOMMAND()
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()
ON_BN_CLICKED(IDC_BUTTON_SLEEPEX_10, &CTestDlg::OnBtnSleepEx10sClicked)
END_MESSAGE_MAP()

BOOL CTestDlg::OnInitDialog() {
    CDialog::OnInitDialog();

    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL) {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty()) {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    CString pidInfo;
    pidInfo.Format(TEXT("PID: %ld  HWND: 0x%X"), GetCurrentProcessId(), m_hWnd);

    SetDlgItemText(IDC_STATIC_PROCESS_INFO, pidInfo);

    return TRUE;
}

void CTestDlg::OnSysCommand(UINT nID, LPARAM lParam) {
    if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else {
        CDialog::OnSysCommand(nID, lParam);
    }
}

void CTestDlg::OnPaint() {
    if (IsIconic()) {
        CPaintDC dc(this);

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        dc.DrawIcon(x, y, m_hIcon);
    }
    else {
        CDialog::OnPaint();
    }
}

HCURSOR CTestDlg::OnQueryDragIcon() {
    return static_cast<HCURSOR>(m_hIcon);
}

void CTestDlg::OnBtnSleepEx10sClicked() {
    std::thread t = std::thread([this]() {
        DWORD dwTid = GetCurrentThreadId();
        CString msg;
        msg.Format(TEXT("Thread %ld will SleepEx(10000, TRUE)"), dwTid);
        MessageBox(msg, TEXT("TestInjectTarget"), MB_OK | MB_ICONASTERISK);
        SleepEx(10000, TRUE);
    });
    t.detach();
}
