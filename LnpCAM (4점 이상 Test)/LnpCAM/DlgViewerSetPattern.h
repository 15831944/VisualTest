#pragma once


// CDlgViewerSetPattern dialog

class CDlgViewerSetPattern : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgViewerSetPattern)

public:
	CDlgViewerSetPattern(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgViewerSetPattern();

// Dialog Data
	enum { IDD = IDD_DLG_VIEWER_SET_PATTERN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedPatternChkView();
	afx_msg void OnBnClickedPatternBtnModify();

	BOOL m_bIsModify;
	void ShowControl(BOOL bFlag);
	afx_msg void OnBnClickedPatternBtnApply();
};
