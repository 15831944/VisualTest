// DlgViewerSetOrder.cpp : implementation file
//

#include "stdafx.h"
#include "LnpCAM.h"
#include "DlgViewerSetOrder.h"
#include "afxdialogex.h"
#include "LnpCAMDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDlgViewerSetOrder dialog

IMPLEMENT_DYNAMIC(CDlgViewerSetOrder, CDialogEx)

CDlgViewerSetOrder::CDlgViewerSetOrder(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgViewerSetOrder::IDD, pParent)
{
	m_bIsModify = FALSE;
}

CDlgViewerSetOrder::~CDlgViewerSetOrder()
{
}

void CDlgViewerSetOrder::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgViewerSetOrder, CDialogEx)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_ORDER_CHK_VIEW, &CDlgViewerSetOrder::OnBnClickedOrderChkView)
	ON_BN_CLICKED(IDC_ORDER_BTN_MODIFY, &CDlgViewerSetOrder::OnBnClickedOrderBtnModify)
	ON_BN_CLICKED(IDC_ORDER_BTN_APPLY, &CDlgViewerSetOrder::OnBnClickedOrderBtnApply)
END_MESSAGE_MAP()


// CDlgViewerSetOrder message handlers


BOOL CDlgViewerSetOrder::OnEraseBkgnd(CDC* pDC)
{
	CRect rt;
    GetClientRect(&rt);  // 클라이언트 영역의 크기 계산
    pDC->FillSolidRect(&rt, RGB(74, 97, 132));  // 클라이언트 영역 크기만큼 흰색으로 채운다

    return TRUE;  // 상위 클래스의 OnEraseBkgnd 함수를 호출하지 않아야 바꾼 배경색이 적용된다
}


HBRUSH CDlgViewerSetOrder::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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


BOOL CDlgViewerSetOrder::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	GetDlgItem(IDC_ORDER_BTN_MODIFY)->ShowWindow(SW_HIDE);

	SetWindowTheme(GetDlgItem(IDC_ORDER_CHK_VIEW)->GetSafeHwnd(), L"", L"");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CDlgViewerSetOrder::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)           
	{
		return TRUE; 

		return FALSE;            
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CDlgViewerSetOrder::OnBnClickedOrderChkView()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;
	
	if(pDlg->m_nSelLayer == -1){
		((CButton*)GetDlgItem(IDC_ORDER_CHK_VIEW))->SetCheck(0);
		GetDlgItem(IDC_ORDER_BTN_MODIFY)->ShowWindow(SW_HIDE);
		return;
	}

	if(!pDlg->m_bIsFileOpen) {
		((CButton*)GetDlgItem(IDC_ORDER_CHK_VIEW))->SetCheck(0);
		return;
	}

	if(pDlg->m_nFileType == 4 && pDlg->m_Lpd.m_nLPDVersion < 2) // _FILE_LPD
	{
		AfxMessageBox(L" LpdFile Version 2 이상부터 호환 가능.");
		((CButton*)GetDlgItem(IDC_ORDER_CHK_VIEW))->SetCheck(0);
		return;
	}

	if(((CButton*)GetDlgItem(IDC_ORDER_CHK_VIEW))->GetCheck() == TRUE )
		GetDlgItem(IDC_ORDER_BTN_MODIFY)->ShowWindow(SW_SHOW);
	else
		GetDlgItem(IDC_ORDER_BTN_MODIFY)->ShowWindow(SW_HIDE);

	pDlg->m_Viewer.SetVIewOrder(((CButton*)GetDlgItem(IDC_ORDER_CHK_VIEW))->GetCheck());	
}


void CDlgViewerSetOrder::OnBnClickedOrderBtnModify()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	if(pDlg->m_Entity.m_bSectorApply == FALSE) return;

	if(!pDlg->m_bIsFileOpen) return;

	if(pDlg->m_nSelLayer == -1) return;

	if(((CButton*)GetDlgItem(IDC_ORDER_CHK_VIEW))->GetCheck() == FALSE){
		AfxMessageBox(L"오더 보기를 먼저 설정 해 주세요");
		return;
	}

	if(pDlg->m_nFileType == 4 && pDlg->m_Lpd.m_nLPDVersion < 2) // _FILE_LPD
	{
		AfxMessageBox(L" LpdFile Version 2 이상부터 호환 가능.");
		((CButton*)GetDlgItem(IDC_ORDER_CHK_VIEW))->SetCheck(0);
		return;
	}

	ShowControl(TRUE);
	pDlg->m_Viewer.SetModifyOrder(TRUE);
}


void CDlgViewerSetOrder::OnBnClickedOrderBtnApply()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	ShowControl(FALSE);

	if(pDlg->m_nSelLayer == -1) return;

	if(pDlg->m_Viewer.m_nInputNum[pDlg->m_Entity.m_nSelLayer] != pDlg->m_Entity.m_pLayer[pDlg->m_Entity.m_nSelLayer].iRealBlockCount)
		pDlg->m_Viewer.SetModifyOrder(FALSE);

	//pDlg->m_Viewer.View_Draw_Line(FALSE);
	pDlg->m_Viewer.m_bOrderModify = FALSE;
	pDlg->m_Viewer.View_Draw_Line();
}


void CDlgViewerSetOrder::ShowControl(BOOL bFlag)
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	if(m_bIsModify == bFlag)
		return;

	m_bIsModify = bFlag;

	pDlg->m_dlgViewerSet.DisableDlg(bFlag, _SET_ORDER);

	GetDlgItem(IDC_ORDER_BTN_MODIFY)->EnableWindow(!bFlag);
	GetDlgItem(IDC_ORDER_BTN_APPLY)->ShowWindow(bFlag);

	CRect rt;
	GetDlgItem(IDC_ORDER_CHK_VIEW)->GetWindowRect(&rt);
	pDlg->ScreenToClient(rt); pDlg->InvalidateRect(rt, TRUE);
	GetDlgItem(IDC_ORDER_CHK_VIEW)->EnableWindow(!bFlag);

}


