#pragma once


// CDlgViewerSetORG dialog

class CDlgViewerSetORG : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgViewerSetORG)

public:
	CDlgViewerSetORG(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgViewerSetORG();

// Dialog Data
	enum { IDD = IDD_DLG_VIEWER_SET_ORG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedOrgChkView();
	afx_msg void OnBnClickedOrgBtnModify();
	afx_msg void OnBnClickedOrgBtnApply();

	BOOL m_bIsModify;
	void ShowControl(BOOL bFlag);
	
};
