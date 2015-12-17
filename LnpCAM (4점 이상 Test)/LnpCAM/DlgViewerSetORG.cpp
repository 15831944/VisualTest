// DlgViewerSetORG.cpp : implementation file
//

#include "stdafx.h"
#include "LnpCAM.h"
#include "DlgViewerSetORG.h"
#include "afxdialogex.h"
#include "LnpCAMDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDlgViewerSetORG dialog

IMPLEMENT_DYNAMIC(CDlgViewerSetORG, CDialogEx)

CDlgViewerSetORG::CDlgViewerSetORG(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgViewerSetORG::IDD, pParent)
{
	m_bIsModify = FALSE;
}

CDlgViewerSetORG::~CDlgViewerSetORG()
{
}

void CDlgViewerSetORG::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgViewerSetORG, CDialogEx)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_ORG_CHK_VIEW, &CDlgViewerSetORG::OnBnClickedOrgChkView)
	ON_BN_CLICKED(IDC_ORG_BTN_MODIFY, &CDlgViewerSetORG::OnBnClickedOrgBtnModify)
	ON_BN_CLICKED(IDC_ORG_BTN_APPLY, &CDlgViewerSetORG::OnBnClickedOrgBtnApply)
END_MESSAGE_MAP()


// CDlgViewerSetORG message handlers


BOOL CDlgViewerSetORG::OnEraseBkgnd(CDC* pDC)
{
	CRect rt;
	GetClientRect(&rt);  // 클라이언트 영역의 크기 계산
	pDC->FillSolidRect(&rt, RGB(74, 97, 132));  // 클라이언트 영역 크기만큼 흰색으로 채운다

	return TRUE;  // 상위 클래스의 OnEraseBkgnd 함수를 호출하지 않아야 바꾼 배경색이 적용된다
}


HBRUSH CDlgViewerSetORG::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if( nCtlColor == CTLCOLOR_STATIC )
	{
		pDC->SetTextColor(RGB(255,255,255));
		pDC->SetBkMode( TRANSPARENT );
		return (HBRUSH) ::GetStockObject(NULL_BRUSH);
	}
	else if( nCtlColor == CTLCOLOR_BTN )
	{
		return (HBRUSH) ::GetStockObject(NULL_BRUSH);
	}

	return hbr;
}


BOOL CDlgViewerSetORG::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowTheme(GetDlgItem(IDC_ORG_CHK_VIEW)->GetSafeHwnd(), L"", L"");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CDlgViewerSetORG::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)           
	{
		return TRUE; 
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CDlgViewerSetORG::OnBnClickedOrgChkView()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	if(!pDlg->m_bIsFileOpen) {
		((CButton*)GetDlgItem(IDC_ORG_CHK_VIEW))->SetCheck(0);
		return;
	}

	pDlg->m_Viewer.SetViewORG();
}


void CDlgViewerSetORG::OnBnClickedOrgBtnModify()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	if(!pDlg->m_bIsFileOpen) return;

	ShowControl(TRUE);
	//pDlg->m_Viewer.SetModifyORG();
}


void CDlgViewerSetORG::OnBnClickedOrgBtnApply()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;
	
	float fMoveX, fMoveY;
	CString str;

	ShowControl(FALSE);
	
	GetDlgItem(IDC_ORG_EDIT_COORDX)->GetWindowText(str);
	fMoveX = (float)_ttof(str);

	GetDlgItem(IDC_ORG_EDIT_COORDY)->GetWindowText(str);
	fMoveY = (float)_ttof(str);

	pDlg->m_Viewer.SetModifyORG(fMoveX, fMoveY);
}


void CDlgViewerSetORG::ShowControl(BOOL bFlag)
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	if(m_bIsModify == bFlag)
		return;

	m_bIsModify = bFlag;

	pDlg->m_dlgViewerSet.DisableDlg(bFlag, _SET_ORG);

	GetDlgItem(IDC_ORG_EDIT_COORDX)->ShowWindow(bFlag);
	GetDlgItem(IDC_ORG_EDIT_COORDY)->ShowWindow(bFlag);
	GetDlgItem(IDC_ORG_GROUP1)->ShowWindow(bFlag);
	GetDlgItem(IDC_ORG_TEXT_COORDX)->ShowWindow(bFlag);
	GetDlgItem(IDC_ORG_TEXT_COORDY)->ShowWindow(bFlag);
	GetDlgItem(IDC_ORG_TEXT_START)->ShowWindow(bFlag);

	GetDlgItem(IDC_ORG_BTN_MODIFY)->EnableWindow(!bFlag);
	GetDlgItem(IDC_ORG_BTN_APPLY)->ShowWindow(bFlag);
}

