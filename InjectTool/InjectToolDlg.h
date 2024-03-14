#pragma once
#include <string>
#include <sstream>

class CInjectToolDlg : public CDialogEx {
    // Construction
   public:
    CInjectToolDlg(CWnd* pParent = nullptr);  // standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_INJECTTOOL_DIALOG };
#endif

   protected:
    virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV support

    // Implementation
   protected:
    HICON m_hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()
   public:
    afx_msg void OnBnClickedButtonRemoteThreadInject();
    afx_msg void OnBnClickedButtonRemoteThreadEject();
    afx_msg void OnBnClickedButtonWindowHookInject();
    afx_msg void OnBnClickedButtonWindowHookEject();
    afx_msg void OnBnClickedButtonApcInject();
    afx_msg void OnBnClickedButtonApcEject();

   private:
    DWORD_PTR ParseHex(std::wstring const& text);

   private:
    HMODULE m_hRemoteThreadDll = NULL;

    // Win Hook
    HMODULE m_hWinHookDll = NULL;
    HHOOK m_hook = NULL;
public:
    afx_msg void OnBnClickedButtonSetDebugPriv();
};
