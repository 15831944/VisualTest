#pragma once


// CDlgMenu dialog

class CDlgMenu : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgMenu)

public:
	CDlgMenu(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgMenu();

// Dialog Data
	enum { IDD = IDD_DLG_MENU };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedMenuBtnOpen();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedMenuBtnSave();
};
