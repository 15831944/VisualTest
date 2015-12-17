#pragma once


// CDlgViewerSetSector dialog

class CDlgViewerSetSector : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgViewerSetSector)

public:
	CDlgViewerSetSector(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgViewerSetSector();

// Dialog Data
	enum { IDD = IDD_DLG_VIEWER_SET_SECTOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedSectorChkView();
	afx_msg void OnBnClickedSectorBtnModify();

	BOOL m_bIsModify;
	void ShowControl(BOOL bFlag);
	afx_msg void OnBnClickedSectorBtnApply();
	afx_msg void OnBnClickedSectorBtnAddHor();
	afx_msg void OnBnClickedSectorBtnAddVer();
	afx_msg void OnBnClickedSectorBtnMove();
	afx_msg void OnBnClickedSectorBtnReset();
	double m_dMoveTotalX;
	double m_dMoveTotalY;
	afx_msg void OnBnClickedSectorBtnAddGaro();
	afx_msg void OnBnClickedSectorBtnAddSero();
	afx_msg void OnBnClickedSectorBtnAlignCenter();
	afx_msg void OnBnClickedSectorBtnRemoveGaro();
	afx_msg void OnBnClickedSectorBtnRemoveSero();
	afx_msg void OnBnClickedSectorBtnApplyInspect();
};
