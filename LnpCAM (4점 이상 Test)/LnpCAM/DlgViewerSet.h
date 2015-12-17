#pragma once

//Dialog
#include "DlgViewerSetOrder.h"
#include "DlgViewerSetORG.h"
#include "DlgViewerSetPattern.h"
#include "DlgViewerSetSector.h"

// CDlgViewerSet dialog

class CDlgViewerSet : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgViewerSet)

public:
	CDlgViewerSet(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgViewerSet();

// Dialog Data
	enum { IDD = IDD_DLG_VIEWER_SET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	void InitDlg(void);

	//SubClasses
	CDlgViewerSetOrder		m_dlgOrder;
	CDlgViewerSetPattern	m_dlgPattern;
	CDlgViewerSetSector		m_dlgSector;
	CDlgViewerSetORG		m_dlgORG;

	afx_msg void OnBnClickedBtnSect();
	afx_msg void OnBnClickedBtnOrder();
	afx_msg void OnBnClickedBtnOrg();
	afx_msg void OnBnClickedBtnPattern();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void DisableDlg(BOOL bFlag, int nCurSel);
	void ModifyHideDlg();
	void SelectSetDlg(int nSel);
};
