// DlgViewerSet.cpp : implementation file
//

#include "stdafx.h"
#include "LnpCAM.h"
#include "DlgViewerSet.h"
#include "afxdialogex.h"
#include "LnpCAMDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDlgViewerSet dialog

IMPLEMENT_DYNAMIC(CDlgViewerSet, CDialogEx)

CDlgViewerSet::CDlgViewerSet(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgViewerSet::IDD, pParent)
{

}

CDlgViewerSet::~CDlgViewerSet()
{
}

void CDlgViewerSet::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgViewerSet, CDialogEx)
//	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
//	ON_WM_COMPAREITEM()
ON_WM_ERASEBKGND()
ON_BN_CLICKED(IDC_VIEWSET_BTN_SECT, &CDlgViewerSet::OnBnClickedBtnSect)
ON_BN_CLICKED(IDC_VIEWSET_BTN_ORDER, &CDlgViewerSet::OnBnClickedBtnOrder)
ON_BN_CLICKED(IDC_VIEWSET_BTN_ORG, &CDlgViewerSet::OnBnClickedBtnOrg)
ON_BN_CLICKED(IDC_VIEWSET_BTN_PTTR, &CDlgViewerSet::OnBnClickedBtnPattern)
END_MESSAGE_MAP()


// CDlgViewerSet message handlers


//void CDlgViewerSet::OnPaint()
//{
//	CPaintDC dc(this); // device context for painting
//}


HBRUSH CDlgViewerSet::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

BOOL CDlgViewerSet::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	GetDlgItem(IDC_VIEWSET_BTN_ORG)->EnableWindow(false);
	GetDlgItem(IDC_VIEWSET_BTN_PTTR)->EnableWindow(false);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CDlgViewerSet::OnEraseBkgnd(CDC* pDC)
{
	CRect rt;
    GetClientRect(&rt);  // 클라이언트 영역의 크기 계산
	pDC->FillSolidRect(&rt, RGB(41, 56, 82));  // 클라이언트 영역 크기만큼 흰색으로 채운다
  
    return TRUE;  // 상위 클래스의 OnEraseBkgnd 함수를 호출하지 않아야 바꾼 배경색이 적용된다
}


void CDlgViewerSet::InitDlg(void)
{
	CRect rt;
	GetClientRect(&rt);

	int nWidth = rt.right - rt.left;
	
 	m_dlgOrder.Create(IDD_DLG_VIEWER_SET_ORDER, this);
 	m_dlgOrder.SetWindowPos(NULL,0,25,rt.right-rt.left,rt.bottom - 25,SWP_HIDEWINDOW);

	m_dlgORG.Create(IDD_DLG_VIEWER_SET_ORG, this);
	m_dlgORG.SetWindowPos(NULL,0,25,rt.right-rt.left,rt.bottom - 25,SWP_HIDEWINDOW);

	m_dlgPattern.Create(IDD_DLG_VIEWER_SET_PATTERN, this);
	m_dlgPattern.SetWindowPos(NULL,0,25,rt.right-rt.left,rt.bottom - 25,SWP_HIDEWINDOW);

	m_dlgSector.Create(IDD_DLG_VIEWER_SET_SECTOR, this);
	m_dlgSector.SetWindowPos(NULL,0,25,rt.right-rt.left,rt.bottom - 25,SWP_SHOWWINDOW);
}


void CDlgViewerSet::OnBnClickedBtnSect()
{
	SelectSetDlg(_SET_SECTOR);
}


void CDlgViewerSet::OnBnClickedBtnOrder()
{
	SelectSetDlg(_SET_ORDER);
}


void CDlgViewerSet::OnBnClickedBtnOrg()
{
	SelectSetDlg(_SET_ORG);
}


void CDlgViewerSet::OnBnClickedBtnPattern()
{
	SelectSetDlg(_SET_PATTERN);
}


void CDlgViewerSet::SelectSetDlg(int nSel)
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	switch(nSel)
	{
	case _SET_SECTOR:
		m_dlgSector.ShowWindow(SW_SHOW);
		
		m_dlgOrder.ShowWindow(SW_HIDE);
		if( ((CButton*)m_dlgOrder.GetDlgItem(IDC_ORDER_CHK_VIEW))->GetCheck() == TRUE ){
			((CButton*)m_dlgOrder.GetDlgItem(IDC_ORDER_CHK_VIEW))->SetCheck(0);
			
			m_dlgOrder.GetDlgItem(IDC_ORDER_BTN_MODIFY)->ShowWindow(SW_HIDE);

			pDlg->m_Viewer.SetVIewOrder(FALSE);	
		}
		
		m_dlgORG.ShowWindow(SW_HIDE);
		
		m_dlgPattern.ShowWindow(SW_HIDE);
		break;
	case _SET_ORDER:
		m_dlgOrder.ShowWindow(SW_SHOW);
		if( ((CButton*)m_dlgSector.GetDlgItem(IDC_SECTOR_CHK_VIEW))->GetCheck() == TRUE ){
			((CButton*)m_dlgSector.GetDlgItem(IDC_SECTOR_CHK_VIEW))->SetCheck(0);
			
			m_dlgSector.GetDlgItem(IDC_SECTOR_BTN_MODIFY)->ShowWindow(SW_HIDE);

			pDlg->m_Viewer.SetViewSector();
		}

		m_dlgSector.ShowWindow(SW_HIDE);
		m_dlgORG.ShowWindow(SW_HIDE);
		m_dlgPattern.ShowWindow(SW_HIDE);
		break;
	case _SET_ORG:
		m_dlgORG.ShowWindow(SW_SHOW);
		m_dlgSector.ShowWindow(SW_HIDE);
		m_dlgOrder.ShowWindow(SW_HIDE);
		m_dlgPattern.ShowWindow(SW_HIDE);
		break;
	case _SET_PATTERN:
		m_dlgPattern.ShowWindow(SW_SHOW);
		m_dlgSector.ShowWindow(SW_HIDE);
		m_dlgOrder.ShowWindow(SW_HIDE);
		m_dlgORG.ShowWindow(SW_HIDE);
		break;
	}
}


void CDlgViewerSet::DisableDlg(BOOL bFlag, int nCurSel)
{
	if( bFlag == FALSE)
	{
		GetDlgItem(IDC_VIEWSET_BTN_SECT)->EnableWindow(TRUE);
		GetDlgItem(IDC_VIEWSET_BTN_ORDER)->EnableWindow(FALSE);
		GetDlgItem(IDC_VIEWSET_BTN_ORG)->EnableWindow(FALSE);
		GetDlgItem(IDC_VIEWSET_BTN_PTTR)->EnableWindow(FALSE);
	}
	else
	{
		switch(nCurSel)
		{
		case _SET_SECTOR:
			GetDlgItem(IDC_VIEWSET_BTN_SECT)->EnableWindow(TRUE);
			GetDlgItem(IDC_VIEWSET_BTN_ORDER)->EnableWindow(FALSE);
			GetDlgItem(IDC_VIEWSET_BTN_ORG)->EnableWindow(FALSE);
			GetDlgItem(IDC_VIEWSET_BTN_PTTR)->EnableWindow(FALSE);
			break;
		case _SET_ORDER:
			GetDlgItem(IDC_VIEWSET_BTN_SECT)->EnableWindow(FALSE);
			GetDlgItem(IDC_VIEWSET_BTN_ORDER)->EnableWindow(TRUE);
			GetDlgItem(IDC_VIEWSET_BTN_ORG)->EnableWindow(FALSE);
			GetDlgItem(IDC_VIEWSET_BTN_PTTR)->EnableWindow(FALSE);
			break;
		case _SET_ORG:
			GetDlgItem(IDC_VIEWSET_BTN_SECT)->EnableWindow(FALSE);
			GetDlgItem(IDC_VIEWSET_BTN_ORDER)->EnableWindow(FALSE);
			GetDlgItem(IDC_VIEWSET_BTN_ORG)->EnableWindow(TRUE);
			GetDlgItem(IDC_VIEWSET_BTN_PTTR)->EnableWindow(FALSE);
			break;
		case _SET_PATTERN:
			GetDlgItem(IDC_VIEWSET_BTN_SECT)->EnableWindow(FALSE);
			GetDlgItem(IDC_VIEWSET_BTN_ORDER)->EnableWindow(FALSE);
			GetDlgItem(IDC_VIEWSET_BTN_ORG)->EnableWindow(FALSE);
			GetDlgItem(IDC_VIEWSET_BTN_PTTR)->EnableWindow(TRUE);
			break;
		}
	}
	
}


void CDlgViewerSet::ModifyHideDlg()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	((CButton*)m_dlgOrder.GetDlgItem(IDC_ORDER_CHK_VIEW))->SetCheck(0);
	((CButton*)m_dlgSector.GetDlgItem(IDC_SECTOR_CHK_VIEW))->SetCheck(0);
	((CButton*)m_dlgORG.GetDlgItem(IDC_ORG_CHK_VIEW))->SetCheck(0);
	((CButton*)m_dlgPattern.GetDlgItem(IDC_PATTERN_CHK_VIEW))->SetCheck(0);

	SelectSetDlg(_SET_SECTOR);

	pDlg->m_dlgViewerSet.m_dlgSector.ShowControl(FASE);
	pDlg->m_dlgViewerSet.m_dlgORG.ShowControl(FALSE);
	pDlg->m_dlgViewerSet.m_dlgOrder.ShowControl(FALSE);
	pDlg->m_dlgViewerSet.m_dlgPattern.ShowControl(FALSE);
}


BOOL CDlgViewerSet::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)           
	{
		return TRUE; 
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}
