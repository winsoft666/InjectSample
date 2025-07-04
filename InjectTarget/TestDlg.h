#pragma once

class CTestDlg : public CDialog {
   public:
    CTestDlg(CWnd* pParent = NULL);

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_TEST_DIALOG };
#endif

   protected:
    virtual void DoDataExchange(CDataExchange* pDX);

   protected:
    HICON m_hIcon;

    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBtnSleepEx10sClicked();
    DECLARE_MESSAGE_MAP()
};
