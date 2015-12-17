#pragma once


// CDlgViewerSetOrder dialog

class CDlgViewerSetOrder : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgViewerSetOrder)

public:
	CDlgViewerSetOrder(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgViewerSetOrder();

// Dialog Data
	enum { IDD = IDD_DLG_VIEWER_SET_ORDER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedOrderChkView();
	afx_msg void OnBnClickedOrderBtnModify();

	BOOL m_bIsModify;
	void ShowControl(BOOL bFlag);
	afx_msg void OnBnClickedOrderBtnApply();
};
