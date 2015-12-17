#pragma once

// CDlgViewer dialog

class CDlgViewer : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgViewer)

public:
	CDlgViewer(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgViewer();

// Dialog Data
	enum { IDD = IDD_DLG_VIEWER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
