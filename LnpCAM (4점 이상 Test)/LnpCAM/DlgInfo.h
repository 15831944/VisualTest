#pragma once


// CDlgInfo dialog

class CDlgInfo : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgInfo)

public:
	CDlgInfo(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgInfo();

// Dialog Data
	enum { IDD = IDD_DLG_INFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
//	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	CStatic m_txtCoordX;
	CStatic m_txtCoordY;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CProgressCtrl m_progressFileLoad;

	void SetProgressPosLoad(int nPos);
	void SetProgressRangeLoad(int nStart, int nEnd);
	void SetProgressMsg(CString str);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
