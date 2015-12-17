#pragma once


// CDlgToolInfo dialog

class CDlgToolInfo : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgToolInfo)

public:
	CDlgToolInfo(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgToolInfo();

// Dialog Data
	enum { IDD = IDD_DLG_TOOL_INFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnPaint();
	
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	CListCtrl m_listTool;
	void DisplayList();
	afx_msg void OnNMClickToolinfoListTool(NMHDR *pNMHDR, LRESULT *pResult);
	void GetCheckOutList(int nIndex);
	afx_msg void OnNMDblclkToolinfoListTool(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRclickToolinfoListTool(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedToolinfoBtnSectorsize();
	CListCtrl m_listInfo;
	CButton m_chkOptimize;
	afx_msg void OnLvnItemchangedToolinfoListTool(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCustomdrawToolinfoListTool(NMHDR *pNMHDR, LRESULT *pResult);
	CButton m_chkCenterAlign;
	CButton m_chk3StepMode;

	void Layer_Color(int color, int *nR, int *nG, int *nB);
	void SetOption();
};
