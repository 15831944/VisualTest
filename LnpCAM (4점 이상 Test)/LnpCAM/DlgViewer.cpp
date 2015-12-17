// DlgViewer.cpp : implementation file
//

#include "stdafx.h"
#include "LnpCAM.h"
#include "DlgViewer.h"
#include "afxdialogex.h"
#include "LnpCAMDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDlgViewer dialog

IMPLEMENT_DYNAMIC(CDlgViewer, CDialogEx)

CDlgViewer::CDlgViewer(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgViewer::IDD, pParent)
{

}

CDlgViewer::~CDlgViewer()
{
}

void CDlgViewer::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgViewer, CDialogEx)
//	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CDlgViewer message handlers


//void CDlgViewer::OnPaint()
//{
//	CPaintDC dc(this); // device context for painting
//	// TODO: Add your message handler code here
//	// Do not call CDialogEx::OnPaint() for painting messages
//}


HBRUSH CDlgViewer::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if( nCtlColor == CTLCOLOR_STATIC )
	{
		pDC->SetTextColor(RGB(255,255,255));
		pDC->SetBkMode( TRANSPARENT );
		return (HBRUSH) ::GetStockObject(NULL_BRUSH);
	}
	return hbr;
}


BOOL CDlgViewer::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CDlgViewer::OnEraseBkgnd(CDC* pDC)
{
	CRect rt;
    GetClientRect(&rt);  // Ŭ���̾�Ʈ ������ ũ�� ���
    pDC->FillSolidRect(&rt, RGB(255, 255, 255));  // Ŭ���̾�Ʈ ���� ũ�⸸ŭ ������� ä���

    return TRUE;  // ���� Ŭ������ OnEraseBkgnd �Լ��� ȣ������ �ʾƾ� �ٲ� ������ ����ȴ�
}


BOOL CDlgViewer::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)           
	{
		return TRUE; 
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}
