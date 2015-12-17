// DlgViewerSetPattern.cpp : implementation file
//

#include "stdafx.h"
#include "LnpCAM.h"
#include "DlgViewerSetPattern.h"
#include "afxdialogex.h"
#include "LnpCAMDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDlgViewerSetPattern dialog

IMPLEMENT_DYNAMIC(CDlgViewerSetPattern, CDialogEx)

CDlgViewerSetPattern::CDlgViewerSetPattern(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgViewerSetPattern::IDD, pParent)
{
	m_bIsModify = FALSE;
}

CDlgViewerSetPattern::~CDlgViewerSetPattern()
{
}

void CDlgViewerSetPattern::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgViewerSetPattern, CDialogEx)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_PATTERN_CHK_VIEW, &CDlgViewerSetPattern::OnBnClickedPatternChkView)
	ON_BN_CLICKED(IDC_PATTERN_BTN_MODIFY, &CDlgViewerSetPattern::OnBnClickedPatternBtnModify)
	ON_BN_CLICKED(IDC_PATTERN_BTN_APPLY, &CDlgViewerSetPattern::OnBnClickedPatternBtnApply)
END_MESSAGE_MAP()


// CDlgViewerSetPattern message handlers


BOOL CDlgViewerSetPattern::OnEraseBkgnd(CDC* pDC)
{
	CRect rt;
	GetClientRect(&rt);  // 클라이언트 영역의 크기 계산
	pDC->FillSolidRect(&rt, RGB(74, 97, 132));  // 클라이언트 영역 크기만큼 흰색으로 채운다

	return TRUE;  // 상위 클래스의 OnEraseBkgnd 함수를 호출하지 않아야 바꾼 배경색이 적용된다
}


HBRUSH CDlgViewerSetPattern::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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


BOOL CDlgViewerSetPattern::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowTheme(GetDlgItem(IDC_PATTERN_CHK_VIEW)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_PATTERN_RADIO1)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_PATTERN_RADIO2)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_PATTERN_RADIO3)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_PATTERN_RADIO4)->GetSafeHwnd(), L"", L"");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CDlgViewerSetPattern::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)           
	{
		return TRUE; 
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CDlgViewerSetPattern::OnBnClickedPatternChkView()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	if(!pDlg->m_bIsFileOpen) {
		((CButton*)GetDlgItem(IDC_PATTERN_CHK_VIEW))->SetCheck(0);
		return;
	}

	pDlg->m_Viewer.SetViewPattern();
}


void CDlgViewerSetPattern::OnBnClickedPatternBtnModify()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	if(!pDlg->m_bIsFileOpen) return;

	ShowControl(TRUE);
	pDlg->m_Viewer.SetModifyPattern();
}


void CDlgViewerSetPattern::ShowControl(BOOL bFlag)
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	if(m_bIsModify == bFlag)
		return;

	m_bIsModify = bFlag;

	pDlg->m_dlgViewerSet.DisableDlg(bFlag, _SET_PATTERN);


 	GetDlgItem(IDC_PATTERN_RADIO1)->ShowWindow(bFlag);
	GetDlgItem(IDC_PATTERN_RADIO2)->ShowWindow(bFlag);
	GetDlgItem(IDC_PATTERN_RADIO3)->ShowWindow(bFlag);
	GetDlgItem(IDC_PATTERN_RADIO4)->ShowWindow(bFlag);
	GetDlgItem(IDC_PATTERN_TEXT_MATCH1)->ShowWindow(bFlag);
	GetDlgItem(IDC_PATTERN_TEXT_MATCH2)->ShowWindow(bFlag);
	GetDlgItem(IDC_PATTERN_TEXT_MATCH3)->ShowWindow(bFlag);
	GetDlgItem(IDC_PATTERN_TEXT_MATCH4)->ShowWindow(bFlag);
	GetDlgItem(IDC_PATTERN_TEXT_MATCHX)->ShowWindow(bFlag);
	GetDlgItem(IDC_PATTERN_TEXT_MATCHY)->ShowWindow(bFlag);
	GetDlgItem(IDC_PATTERN_TEXT_MODIFY1)->ShowWindow(bFlag);
	GetDlgItem(IDC_PATTERN_TEXT_MODIFY2)->ShowWindow(bFlag);
	GetDlgItem(IDC_PATTERN_GROUP1)->ShowWindow(bFlag);
	GetDlgItem(IDC_PATTERN_GROUP2)->ShowWindow(bFlag);
	GetDlgItem(IDC_PATTERN_EDIT_COORDX1)->ShowWindow(bFlag);
	GetDlgItem(IDC_PATTERN_EDIT_COORDX2)->ShowWindow(bFlag);
	GetDlgItem(IDC_PATTERN_EDIT_COORDX3)->ShowWindow(bFlag);
	GetDlgItem(IDC_PATTERN_EDIT_COORDX4)->ShowWindow(bFlag);
	GetDlgItem(IDC_PATTERN_EDIT_COORDY1)->ShowWindow(bFlag);
	GetDlgItem(IDC_PATTERN_EDIT_COORDY2)->ShowWindow(bFlag);
	GetDlgItem(IDC_PATTERN_EDIT_COORDY3)->ShowWindow(bFlag);
	GetDlgItem(IDC_PATTERN_EDIT_COORDY4)->ShowWindow(bFlag);

 	GetDlgItem(IDC_PATTERN_BTN_MODIFY)->EnableWindow(!bFlag);
 	GetDlgItem(IDC_PATTERN_BTN_APPLY)->ShowWindow(bFlag);

}

void CDlgViewerSetPattern::OnBnClickedPatternBtnApply()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	ShowControl(FALSE);
	pDlg->m_Viewer.SetModifyPattern();
}
