// DlgMenu.cpp : implementation file
//

#include "stdafx.h"
#include "LnpCAM.h"
#include "DlgMenu.h"
#include "afxdialogex.h"
#include "LnpCAMDlg.h"


// CDlgMenu dialog

IMPLEMENT_DYNAMIC(CDlgMenu, CDialogEx)

CDlgMenu::CDlgMenu(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgMenu::IDD, pParent)
{

}

CDlgMenu::~CDlgMenu()
{
}

void CDlgMenu::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgMenu, CDialogEx)
//	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_MENU_BTN_OPEN, &CDlgMenu::OnBnClickedMenuBtnOpen)
	ON_BN_CLICKED(IDC_MENU_BTN_SAVE, &CDlgMenu::OnBnClickedMenuBtnSave)
END_MESSAGE_MAP()


// CDlgMenu message handlers


HBRUSH CDlgMenu::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if( nCtlColor == CTLCOLOR_STATIC )
	{
		pDC->SetTextColor(RGB(0,0,0));
		pDC->SetBkMode( TRANSPARENT );
		return (HBRUSH) ::GetStockObject(NULL_BRUSH);
	}
	else if( nCtlColor == CTLCOLOR_BTN )
	{
		return (HBRUSH) ::GetStockObject(NULL_BRUSH);
	}

	return hbr;
}


BOOL CDlgMenu::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgMenu::OnBnClickedMenuBtnOpen()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;
	pDlg->m_Process.StartFileLoading();
	return;
}


BOOL CDlgMenu::OnEraseBkgnd(CDC* pDC)
{
	CRect rt;
    GetClientRect(&rt);  // 클라이언트 영역의 크기 계산
    pDC->FillSolidRect(&rt, RGB(173, 186, 206));  // 클라이언트 영역 크기만큼 흰색으로 채운다

    return TRUE;  // 상위 클래스의 OnEraseBkgnd 함수를 호출하지 않아야 바꾼 배경색이 적용된다
}


BOOL CDlgMenu::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)           
	{
		return TRUE; 
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CDlgMenu::OnBnClickedMenuBtnSave()
{

	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;
	
	if(!pDlg->m_bIsFileOpen){
		AfxMessageBox(L"열린 파일이 없습니다.");
		return;
	}
	
	if(pDlg->m_Viewer.GetOrderModifyState(pDlg->m_nSelLayer) == -1 && pDlg->m_nFileType == 4 ){
		AfxMessageBox(L"순서 설정을 완료 후 저장하시오");
		return;
	}
	
	if(pDlg->m_nFileType == 4 && pDlg->m_Lpd.m_nLPDVersion < 2) // _FILE_LPD
	{
		AfxMessageBox(L" LpdFile Version 2 이상부터 호환 가능.");
		((CButton*)GetDlgItem(IDC_ORDER_CHK_VIEW))->SetCheck(0);
		return;
	}

	if( !pDlg->m_Entity.CheckSectorArea() )
	{
		AfxMessageBox(L"섹터 영역 설정이 잘못되었습니다. 다시 확인해주십시오.");
		return;
	}

	pDlg->m_dlgToolInfo.SetOption();
	pDlg->m_Entity.m_iProgressCount=0;
	pDlg->m_Process.StartFileSaving();
}
