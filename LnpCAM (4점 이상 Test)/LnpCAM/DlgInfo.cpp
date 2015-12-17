// DlgInfo.cpp : implementation file
//

#include "stdafx.h"
#include "LnpCAM.h"
#include "DlgInfo.h"
#include "afxdialogex.h"
#include "LnpCAMDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDlgInfo dialog

IMPLEMENT_DYNAMIC(CDlgInfo, CDialogEx)

CDlgInfo::CDlgInfo(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgInfo::IDD, pParent)
{

}

CDlgInfo::~CDlgInfo()
{
}

void CDlgInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INFO_TEXT_COORDX, m_txtCoordX);
	DDX_Control(pDX, IDC_INFO_TEXT_COORDY, m_txtCoordY);
	DDX_Control(pDX, IDC_INFO_PROGRESS_LOAD, m_progressFileLoad);
}


BEGIN_MESSAGE_MAP(CDlgInfo, CDialogEx)
//	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CDlgInfo message handlers


BOOL CDlgInfo::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	m_progressFileLoad.ShowWindow(false);

	//SetTimer(100, 100, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


//void CDlgInfo::OnPaint()
//{
//	CPaintDC dc(this); // device context for painting
//	// TODO: Add your message handler code here
//	// Do not call CDialogEx::OnPaint() for painting messages
//}


HBRUSH CDlgInfo::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

 	if( nCtlColor == CTLCOLOR_STATIC || nCtlColor == CTLCOLOR_BTN )
 	{
 		pDC->SetTextColor(RGB(255,255,255));
 		pDC->SetBkMode( TRANSPARENT );
		return (HBRUSH) ::GetStockObject(NULL_BRUSH);
 	}

	if( pWnd->GetDlgCtrlID() == IDC_INFO_PROGRESS_LOAD )
	{
		pDC->SetTextColor(RGB(0,0,0));
		pDC->SetBkMode( TRANSPARENT );
		return (HBRUSH) ::GetStockObject(NULL_BRUSH);
	}

	return hbr;
}


BOOL CDlgInfo::OnEraseBkgnd(CDC* pDC)
{
	CRect rt;
    GetClientRect(&rt);  // 클라이언트 영역의 크기 계산
    pDC->FillSolidRect(&rt, RGB(41, 56, 82));  // 클라이언트 영역 크기만큼 흰색으로 채운다

    return TRUE;  // 상위 클래스의 OnEraseBkgnd 함수를 호출하지 않아야 바꾼 배경색이 적용된다
}


BOOL CDlgInfo::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)           
	{
		return TRUE; 
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CDlgInfo::SetProgressPosLoad(int nPos)
{
	m_progressFileLoad.SetPos(nPos);
	m_progressFileLoad.InvalidateRect(false);
}

void CDlgInfo::SetProgressRangeLoad(int nStart, int nEnd)
{
	m_progressFileLoad.SetRange32(nStart, nEnd);
}

void CDlgInfo::SetProgressMsg(CString str)
{
	if(str == L"")
		return;

	CRect rt;
	GetDlgItem(IDC_INFO_TEXT_FILE_LOAD)->GetWindowRect(&rt);
	ScreenToClient(&rt);
	InvalidateRect(rt, TRUE);

	GetDlgItem(IDC_INFO_TEXT_FILE_LOAD)->SetWindowText(str);
}



void CDlgInfo::OnTimer(UINT_PTR nIDEvent)
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;
	CString str;

	if(pDlg->m_bIsFileOpen)
	{
		//str = pDlg->m_Entity.GetProgressMsg();

		//SetProgressMsg(str);
	}

	CDialogEx::OnTimer(nIDEvent);
}
