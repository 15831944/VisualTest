// DlgViewerSetSector.cpp : implementation file
//

#include "stdafx.h"
#include "LnpCAM.h"
#include "DlgViewerSetSector.h"
#include "afxdialogex.h"
#include "LnpCAMDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDlgViewerSetSector dialog

IMPLEMENT_DYNAMIC(CDlgViewerSetSector, CDialogEx)

CDlgViewerSetSector::CDlgViewerSetSector(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgViewerSetSector::IDD, pParent)
	, m_dMoveTotalX(0)
	, m_dMoveTotalY(0)
{
	m_bIsModify = FALSE;
}

CDlgViewerSetSector::~CDlgViewerSetSector()
{
}

void CDlgViewerSetSector::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgViewerSetSector, CDialogEx)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_SECTOR_CHK_VIEW, &CDlgViewerSetSector::OnBnClickedSectorChkView)
	ON_BN_CLICKED(IDC_SECTOR_BTN_MODIFY, &CDlgViewerSetSector::OnBnClickedSectorBtnModify)
	ON_BN_CLICKED(IDC_SECTOR_BTN_APPLY, &CDlgViewerSetSector::OnBnClickedSectorBtnApply)
	ON_BN_CLICKED(IDC_SECTOR_BTN_ADD_HOR, &CDlgViewerSetSector::OnBnClickedSectorBtnAddHor)
	ON_BN_CLICKED(IDC_SECTOR_BTN_ADD_VER, &CDlgViewerSetSector::OnBnClickedSectorBtnAddVer)
	ON_BN_CLICKED(IDC_SECTOR_BTN_MOVE, &CDlgViewerSetSector::OnBnClickedSectorBtnMove)
	ON_BN_CLICKED(IDC_SECTOR_BTN_RESET, &CDlgViewerSetSector::OnBnClickedSectorBtnReset)
	ON_BN_CLICKED(IDC_SECTOR_BTN_ADD_GARO, &CDlgViewerSetSector::OnBnClickedSectorBtnAddGaro)
	ON_BN_CLICKED(IDC_SECTOR_BTN_ADD_SERO, &CDlgViewerSetSector::OnBnClickedSectorBtnAddSero)
	ON_BN_CLICKED(IDC_SECTOR_BTN_ALIGN_CENTER, &CDlgViewerSetSector::OnBnClickedSectorBtnAlignCenter)
	ON_BN_CLICKED(IDC_SECTOR_BTN_REMOVE_GARO, &CDlgViewerSetSector::OnBnClickedSectorBtnRemoveGaro)
	ON_BN_CLICKED(IDC_SECTOR_BTN_REMOVE_SERO, &CDlgViewerSetSector::OnBnClickedSectorBtnRemoveSero)
	ON_BN_CLICKED(IDC_SECTOR_BTN_APPLY_INSPECT, &CDlgViewerSetSector::OnBnClickedSectorBtnApplyInspect)
END_MESSAGE_MAP()


// CDlgViewerSetSector message handlers


BOOL CDlgViewerSetSector::OnEraseBkgnd(CDC* pDC)
{
	CRect rt;
	GetClientRect(&rt);  // 클라이언트 영역의 크기 계산
	pDC->FillSolidRect(&rt, RGB(74, 97, 132));  // 클라이언트 영역 크기만큼 흰색으로 채운다

	return TRUE;  // 상위 클래스의 OnEraseBkgnd 함수를 호출하지 않아야 바꾼 배경색이 적용된다
}


HBRUSH CDlgViewerSetSector::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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


BOOL CDlgViewerSetSector::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	/* Control Transparent */
	SetWindowTheme(GetDlgItem(IDC_SECTOR_CHK_VIEW)->GetSafeHwnd(), L"", L"");
	//SetWindowTheme(GetDlgItem(IDC_SECTOR_BTN_MODIFY)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_SECTOR_BTN_MODIFY)->GetSafeHwnd(), L"explorer", NULL);


	GetDlgItem(IDC_SECTOR_BTN_MODIFY)->ShowWindow(SW_HIDE);

	for(int i=0; i<12; i++)
	{
		GetDlgItem(IDC_SECTOR_EDIT_IDEAL_X1+i)->SetWindowText(L"0.000");
		GetDlgItem(IDC_SECTOR_EDIT_IDEAL_Y1+i)->SetWindowText(L"0.000");
		GetDlgItem(IDC_SECTOR_EDIT_REAL_X1+i)->SetWindowText(L"0.000");
		GetDlgItem(IDC_SECTOR_EDIT_REAL_Y1+i)->SetWindowText(L"0.000");
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CDlgViewerSetSector::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)           
	{
		return TRUE; 
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CDlgViewerSetSector::OnBnClickedSectorChkView()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	if(!pDlg->m_bIsFileOpen) {
		((CButton*)GetDlgItem(IDC_SECTOR_CHK_VIEW))->SetCheck(0);
		return;
	}

	if(pDlg->m_nFileType == 4 && pDlg->m_Lpd.m_nLPDVersion < 2) // _FILE_LPD
	{
		AfxMessageBox(L" LpdFile Version 2 이상부터 호환 가능.");
		((CButton*)GetDlgItem(IDC_SECTOR_CHK_VIEW))->SetCheck(0);
		return;
	}

	if(((CButton*)GetDlgItem(IDC_SECTOR_CHK_VIEW))->GetCheck() == TRUE )
		GetDlgItem(IDC_SECTOR_BTN_MODIFY)->ShowWindow(SW_SHOW);
	else
		GetDlgItem(IDC_SECTOR_BTN_MODIFY)->ShowWindow(SW_HIDE);

	pDlg->m_Viewer.SetViewSector();
}


void CDlgViewerSetSector::OnBnClickedSectorBtnModify()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;
		
	if(!pDlg->m_bIsFileOpen) return;

	if(pDlg->m_nFileType == 4) // _FILE_LPD
	{
		AfxMessageBox(L" LpdFile은 편집할 수 없습니다.");
		return;
	}

	pDlg->m_Entity.m_bSectorApply = FALSE;
	ShowControl(TRUE);
	pDlg->m_Viewer.SetModifySector();
}


void CDlgViewerSetSector::OnBnClickedSectorBtnApply()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	ShowControl(FALSE);
// 	pDlg->m_Entity.ApplyUserSector();
// 	pDlg->m_Viewer.InitInputNumMEM();
	pDlg->m_Viewer.SetModifySector();
}


void CDlgViewerSetSector::ShowControl(BOOL bFlag)
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	if(m_bIsModify == bFlag)
		return;

	m_bIsModify = bFlag;

	pDlg->m_dlgViewerSet.DisableDlg(bFlag, _SET_SECTOR);

	GetDlgItem(IDC_SECTOR_GROUP1)->ShowWindow(bFlag);
	GetDlgItem(IDC_SECTOR_TEXT_MODIFY)->ShowWindow(bFlag);
	GetDlgItem(IDC_SECTOR_BTN_ADD_HOR)->ShowWindow(bFlag);
	GetDlgItem(IDC_SECTOR_BTN_ADD_VER)->ShowWindow(bFlag);

 	GetDlgItem(IDC_SECTOR_BTN_MODIFY)->EnableWindow(!bFlag);
	GetDlgItem(IDC_SECTOR_BTN_APPLY)->ShowWindow(bFlag);

	CRect rt;
	GetDlgItem(IDC_SECTOR_CHK_VIEW)->GetWindowRect(&rt);
	pDlg->ScreenToClient(rt); pDlg->InvalidateRect(rt, TRUE);
	GetDlgItem(IDC_SECTOR_CHK_VIEW)->EnableWindow(!bFlag);

}


void CDlgViewerSetSector::OnBnClickedSectorBtnAddHor()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	pDlg->m_Viewer.Insert_Row_Line();
}


void CDlgViewerSetSector::OnBnClickedSectorBtnAddVer()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	pDlg->m_Viewer.Insert_Col_Line();
}


void CDlgViewerSetSector::OnBnClickedSectorBtnMove()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	CString strTemp;
	double dOffsetX, dOffsetY;

	GetDlgItem(IDC_SECTOR_EDIT_OFFSETX)->GetWindowText(strTemp); dOffsetX = _ttof(strTemp);
	GetDlgItem(IDC_SECTOR_EDIT_OFFSETY)->GetWindowText(strTemp); dOffsetY = _ttof(strTemp);

	pDlg->m_Viewer.SetSectorMove(dOffsetX, dOffsetY);

	double dMoveX, dMoveY;
	pDlg->m_Entity.GetSectorTotalMove(&dMoveX, &dMoveY);

	CString strVal;
	strVal.Format(L"%.3f", dMoveX);GetDlgItem(IDC_SECTOR_EDIT_OFFSETX)->SetWindowText(strVal);
	strVal.Format(L"%.3f", dMoveY);GetDlgItem(IDC_SECTOR_EDIT_OFFSETY)->SetWindowText(strVal);
}


void CDlgViewerSetSector::OnBnClickedSectorBtnReset()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	//pDlg->m_Viewer.SetSectorMove(dOffsetX, dOffsetY);
	pDlg->m_dlgToolInfo.SetOption();

	pDlg->m_Entity.InitSectorLine();
	pDlg->m_Viewer.View_Draw_Line();

	double dMoveX, dMoveY;
	pDlg->m_Entity.GetSectorTotalMove(&dMoveX, &dMoveY);

	CString strVal;
	strVal.Format(L"%.3f", dMoveX);GetDlgItem(IDC_SECTOR_EDIT_OFFSETX)->SetWindowText(strVal);
	strVal.Format(L"%.3f", dMoveY);GetDlgItem(IDC_SECTOR_EDIT_OFFSETY)->SetWindowText(strVal);
}


void CDlgViewerSetSector::OnBnClickedSectorBtnAddGaro()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	pDlg->m_Entity.ExtensionSectorLine(_ROW_LINE, TRUE);
	pDlg->m_Viewer.Cal_Sector_Point();
	pDlg->m_Viewer.View_Draw_Line();
}


void CDlgViewerSetSector::OnBnClickedSectorBtnAddSero()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	pDlg->m_Entity.ExtensionSectorLine(_COL_LINE, TRUE);
	pDlg->m_Viewer.Cal_Sector_Point();
	pDlg->m_Viewer.View_Draw_Line();
}


void CDlgViewerSetSector::OnBnClickedSectorBtnAlignCenter()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	pDlg->m_Entity.SetSectorCenterAlign();
	pDlg->m_Viewer.View_Draw_Line();

	double dMoveX, dMoveY;
	pDlg->m_Entity.GetSectorTotalMove(&dMoveX, &dMoveY);

	CString strVal;
	strVal.Format(L"%.3f", dMoveX);GetDlgItem(IDC_SECTOR_EDIT_OFFSETX)->SetWindowText(strVal);
	strVal.Format(L"%.3f", dMoveY);GetDlgItem(IDC_SECTOR_EDIT_OFFSETY)->SetWindowText(strVal);
}


void CDlgViewerSetSector::OnBnClickedSectorBtnRemoveGaro()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	pDlg->m_Entity.ExtensionSectorLine(_ROW_LINE, FALSE);
	pDlg->m_Viewer.Cal_Sector_Point();
	pDlg->m_Viewer.View_Draw_Line();
}


void CDlgViewerSetSector::OnBnClickedSectorBtnRemoveSero()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	pDlg->m_Entity.ExtensionSectorLine(_COL_LINE, FALSE);
	pDlg->m_Viewer.Cal_Sector_Point();
	pDlg->m_Viewer.View_Draw_Line();
}


void CDlgViewerSetSector::OnBnClickedSectorBtnApplyInspect()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;
	CString strVal;

	for(int i=0; i<12;i++)
	{
		GetDlgItem(IDC_SECTOR_EDIT_IDEAL_X1+i)->GetWindowText(strVal);
		pDlg->m_Entity.m_fIdealPointX[i] = _ttof(strVal);
		strVal.Format(L"%.3f", pDlg->m_Entity.m_fIdealPointX[i]);
		GetDlgItem(IDC_SECTOR_EDIT_IDEAL_X1+i)->SetWindowText(strVal);

		GetDlgItem(IDC_SECTOR_EDIT_IDEAL_Y1+i)->GetWindowText(strVal);
		pDlg->m_Entity.m_fIdealPointY[i] = _ttof(strVal);
		strVal.Format(L"%.3f", pDlg->m_Entity.m_fIdealPointY[i]);
		GetDlgItem(IDC_SECTOR_EDIT_IDEAL_Y1+i)->SetWindowText(strVal);

		GetDlgItem(IDC_SECTOR_EDIT_REAL_X1+i)->GetWindowText(strVal);
		pDlg->m_Entity.m_fRealPointX[i] = _ttof(strVal);
		strVal.Format(L"%.3f", pDlg->m_Entity.m_fRealPointX[i]);
		GetDlgItem(IDC_SECTOR_EDIT_REAL_X1+i)->SetWindowText(strVal);

		GetDlgItem(IDC_SECTOR_EDIT_REAL_Y1+i)->GetWindowText(strVal);
		pDlg->m_Entity.m_fRealPointY[i] = _ttof(strVal);
		strVal.Format(L"%.3f", pDlg->m_Entity.m_fRealPointY[i]);
		GetDlgItem(IDC_SECTOR_EDIT_REAL_Y1+i)->SetWindowText(strVal);
	}

	pDlg->m_Entity.ApplyInspectResult();
	pDlg->m_Viewer.View_Draw_Line();
}
