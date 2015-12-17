// DlgToolInfo.cpp : implementation file
//

#include "stdafx.h"
#include "LnpCAM.h"
#include "DlgToolInfo.h"
#include "afxdialogex.h"
#include "LnpCAMDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDlgToolInfo dialog

IMPLEMENT_DYNAMIC(CDlgToolInfo, CDialogEx)

CDlgToolInfo::CDlgToolInfo(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgToolInfo::IDD, pParent)
{
}

CDlgToolInfo::~CDlgToolInfo()
{
}

void CDlgToolInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TOOLINFO_LIST_TOOL, m_listTool);
	DDX_Control(pDX, IDC_TOOLINFO_LIST_INFO, m_listInfo);
	DDX_Control(pDX, IDC_TOOLINFO_CHK_OPTIMIZE, m_chkOptimize);
	DDX_Control(pDX, IDC_TOOLINFO_CHK_CENTERALIGN, m_chkCenterAlign);
	DDX_Control(pDX, IDC_TOOLINFO_CHK_3STEP_MODE, m_chk3StepMode);
}


BEGIN_MESSAGE_MAP(CDlgToolInfo, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_NOTIFY(NM_CLICK, IDC_TOOLINFO_LIST_TOOL, &CDlgToolInfo::OnNMClickToolinfoListTool)
	ON_NOTIFY(NM_DBLCLK, IDC_TOOLINFO_LIST_TOOL, &CDlgToolInfo::OnNMDblclkToolinfoListTool)
	ON_NOTIFY(NM_RCLICK, IDC_TOOLINFO_LIST_TOOL, &CDlgToolInfo::OnRclickToolinfoListTool)
ON_BN_CLICKED(IDC_TOOLINFO_BTN_SECTORSIZE, &CDlgToolInfo::OnBnClickedToolinfoBtnSectorsize)
ON_NOTIFY(LVN_ITEMCHANGED, IDC_TOOLINFO_LIST_TOOL, &CDlgToolInfo::OnLvnItemchangedToolinfoListTool)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_TOOLINFO_LIST_TOOL, &CDlgToolInfo::OnCustomdrawToolinfoListTool)
END_MESSAGE_MAP()


// CDlgToolInfo message handlers


//void CDlgToolInfo::OnPaint()
//{
//	CPaintDC dc(this); // device context for painting
//	// TODO: Add your message handler code here
//	// Do not call CDialogEx::OnPaint() for painting messages
//}


HBRUSH CDlgToolInfo::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if( nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetTextColor(RGB(255,255,255));
		pDC->SetBkMode( TRANSPARENT );
		return (HBRUSH) ::GetStockObject(NULL_BRUSH);


	}

	return hbr;
}


BOOL CDlgToolInfo::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_listTool.ModifyStyle(LVS_OWNERDRAWFIXED, 0, 0);
	ListView_SetExtendedListViewStyle(::GetDlgItem(m_hWnd,IDC_TOOLINFO_LIST_TOOL),
		LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES);

	LV_COLUMN ivCol;
	LPWSTR szlist[3] = {L" ",L"Name",L" "};
	int nWith[3]={20,133,20};
	for(int i=0; i<3; i++)
	{
		ivCol.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
		ivCol.fmt = LVCFMT_CENTER;
		ivCol.pszText = szlist[i];
		ivCol.iSubItem = 1;
		ivCol.cx = nWith[i];
		m_listTool.InsertColumn(i,&ivCol);
		m_listTool.SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	}

	GetDlgItem(IDC_TOOLINFO_EDIT_SECTORSIZE)->SetWindowText(L"30");
	m_chkOptimize.SetCheck(TRUE);

	/* Info Dlg */

	CString str;
	
	LV_COLUMN lvcolumn;

	LPWSTR list[3] = {L"1",L"Index",L"Data"};
	int width[3] = {0,100,75};

	m_listInfo.ModifyStyle(LVS_OWNERDRAWFIXED, 0, 0);
	ListView_SetExtendedListViewStyle(::GetDlgItem(m_hWnd,IDC_TOOLINFO_LIST_INFO),
		LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	for(int i=0;i<3;i++)
	{
		lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH ;
		lvcolumn.fmt = LVCFMT_CENTER;
		lvcolumn.pszText = list[i];
		lvcolumn.iSubItem=i;
		lvcolumn.cx = width[i];

		m_listInfo.InsertColumn(i,&lvcolumn);
	}

	m_listInfo.DeleteAllItems();

	SetWindowTheme(GetDlgItem(IDC_TOOLINFO_CHK_3STEP_MODE)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_TOOLINFO_CHK_OPTIMIZE)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_TOOLINFO_CHK_CENTERALIGN)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_TOOLINFO_CHK_REMOVEBLANK)->GetSafeHwnd(), L"", L"");

	GetDlgItem(IDC_TOOLINFO_EDIT_OVERLAP)->SetWindowText(L"0");
	GetDlgItem(IDC_TOOLINFO_EDIT_HOLESIZE)->SetWindowText(L"0.00");


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CDlgToolInfo::OnEraseBkgnd(CDC* pDC)
{
	CRect rt;
    GetClientRect(&rt);  // 클라이언트 영역의 크기 계산
    pDC->FillSolidRect(&rt, RGB(74, 97, 132));  // 클라이언트 영역 크기만큼 흰색으로 채운다

    return TRUE;  // 상위 클래스의 OnEraseBkgnd 함수를 호출하지 않아야 바꾼 배경색이 적용된다
}


void CDlgToolInfo::DisplayList(void)
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	CString strTemp;
	
	m_listTool.DeleteAllItems();

	int n=0;

	LV_ITEM ivItem;
	ivItem.mask = LVIF_TEXT;

	for(int i=0; i<pDlg->m_nLayerCnt; i++)
	{
		ivItem.iItem = i;

		ivItem.iSubItem=0;
		//strTemp.Format(L"%d",i);
		strTemp.Format(L"");
		ivItem.pszText=(LPWSTR)(LPCWSTR)strTemp;
		m_listTool.InsertItem(&ivItem);

		ivItem.iSubItem=1;
		ivItem.pszText=(LPWSTR)(LPCWSTR)pDlg->m_strLayerName[i];
		m_listTool.SetItem(&ivItem);

		ivItem.iSubItem=2;
		ivItem.pszText=(LPWSTR)(LPCWSTR)L"";
		m_listTool.SetItem(&ivItem);

		m_listTool.SetCheck(i, TRUE);  
	}
}


void CDlgToolInfo::OnNMClickToolinfoListTool(NMHDR *pNMHDR, LRESULT *pResult)
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	LPNMITEMACTIVATE temp = (LPNMITEMACTIVATE) pNMHDR;

	HWND hWnd1 =  ::GetDlgItem (m_hWnd,IDC_TOOLINFO_LIST_TOOL);

	*pResult = TRUE;

	if(pDlg->m_dlgViewerSet.m_dlgOrder.GetDlgItem(IDC_ORDER_BTN_MODIFY)->IsWindowEnabled() == FALSE ||
		pDlg->m_dlgViewerSet.m_dlgSector.GetDlgItem(IDC_SECTOR_BTN_MODIFY)->IsWindowEnabled() == FALSE )
	{
		m_listTool.SetItemState(pDlg->m_nSelLayer, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		AfxMessageBox(L"하던 작업을 먼저 마무리 해 주세요.");
		return;
	}

	int nItem = temp->iItem;
	int nSubItem = temp->iSubItem;

	if(nSubItem == -1 || nItem == -1)
		return ;

	pDlg->m_Entity.SelectLayer(nItem);

	if( ((CButton*)pDlg->m_dlgViewerSet.m_dlgOrder.GetDlgItem(IDC_ORDER_CHK_VIEW))->GetCheck()   
	   && pDlg->m_nSelLayer != nItem)
	{
		pDlg->m_Viewer.View_Draw_Line();
	}

	pDlg->m_nSelLayer = nItem;
		
	if(nSubItem == 0)
	{
		if(m_listTool.GetCheck(nItem))  
			m_listTool.SetCheck(nItem, false);  
		else  
			m_listTool.SetCheck(nItem, true);  

		GetCheckOutList(m_listTool.GetItemCount());	
	}
	else
	{
		CString strTemp;
		strTemp.Format(L"%.2f", pDlg->m_Entity.GetLayerHoleSize(nItem) * 1000);
		GetDlgItem(IDC_TOOLINFO_EDIT_HOLESIZE)->SetWindowText(strTemp);
	}
}


void CDlgToolInfo::GetCheckOutList(int nIndex)
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	int nCheckTime=0;
	int i, nNum;

	BOOL test;

	nNum = 0;

	for( i = 0; i < nIndex; i++)
	{
		test = ListView_GetCheckState(::GetDlgItem (m_hWnd,IDC_TOOLINFO_LIST_TOOL), i);

		pDlg->m_Viewer.SetLayerViewer(i, test);

	}
}


void CDlgToolInfo::OnNMDblclkToolinfoListTool(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// Remove Mouse Event = pResult : TRUE
	*pResult = TRUE;
}


void CDlgToolInfo::OnRclickToolinfoListTool(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// Remove Mouse Event = pResult : TRUE
	*pResult = TRUE;
}


BOOL CDlgToolInfo::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)           
	{
		return TRUE; 
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CDlgToolInfo::OnBnClickedToolinfoBtnSectorsize()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;
	CString strTemp;
	int nCheck;

// Sector Size
	GetDlgItem(IDC_TOOLINFO_EDIT_SECTORSIZE)->GetWindowText(strTemp);
	pDlg->m_Entity.SetBlockSize((int)_ttof(strTemp));

// Hole Size
	if(pDlg->m_bIsFileOpen == TRUE && pDlg->m_nSelLayer != -1) {
		GetDlgItem(IDC_TOOLINFO_EDIT_HOLESIZE)->GetWindowText(strTemp);
		pDlg->m_Entity.SetLayerHoleSize(pDlg->m_nSelLayer,  _ttof(strTemp) / 1000);
		strTemp.Format(L"%.2f", pDlg->m_Entity.GetLayerHoleSize(pDlg->m_nSelLayer) * 1000);
	}

// Overrap
	GetDlgItem(IDC_TOOLINFO_EDIT_OVERLAP)->GetWindowText(strTemp);
	pDlg->m_Entity.m_fOverlap = _ttof(strTemp) / 1000;

// CheckBox - Optimize 
	nCheck = ((CButton*)pDlg->m_dlgToolInfo.GetDlgItem(IDC_TOOLINFO_CHK_OPTIMIZE))->GetCheck();
	pDlg->m_Entity.SetOptimization(nCheck);

// CheckBox - RemoveBlank 
	nCheck = ((CButton*)pDlg->m_dlgToolInfo.GetDlgItem(IDC_TOOLINFO_CHK_REMOVEBLANK))->GetCheck();
	pDlg->m_Entity.SetRemoveBlank(nCheck);

// CheckBox - Center Align 
	nCheck = pDlg->m_dlgToolInfo.m_chkCenterAlign.GetCheck();
	pDlg->m_Entity.SetCenterAlign(nCheck);

// CheckBox - 3Step Mode 
	nCheck = pDlg->m_dlgToolInfo.m_chk3StepMode.GetCheck();
	pDlg->m_Entity.m_b3StepMode = nCheck;

	AfxMessageBox(L"다음 파일 로딩시 부터 적용 됩니다.");
}



void CDlgToolInfo::OnLvnItemchangedToolinfoListTool(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}


void CDlgToolInfo::OnCustomdrawToolinfoListTool(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVCUSTOMDRAW pLVCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);

	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;

	int nItem = static_cast<int> (pLVCD->nmcd.dwItemSpec);
    int nSubItem = pLVCD->iSubItem;

	CDC *pDC;
	CRect rcSubItem, rcItem;
	CString strText;

	switch(pLVCD->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
        break;

	case CDDS_ITEMPREPAINT|CDDS_SUBITEM:
	{
		m_listTool.GetItemRect(nItem, rcItem, LVIR_BOUNDS);
		m_listTool.GetSubItemRect(nItem, nSubItem, LVIR_BOUNDS, rcSubItem);

		strText = m_listTool.GetItemText(nItem, nSubItem);
        pDC = CDC::FromHandle(pLVCD->nmcd.hdc);

		if(pLVCD->iSubItem == 0)
		{
			*pResult = CDRF_DODEFAULT;
			break;
		}
		else if(pLVCD->iSubItem == 2)
		{
			int nR, nG, nB;
			Layer_Color(pDlg->m_Entity.m_pLayer[pLVCD->nmcd.dwItemSpec].pOriginEntity[0].nColor, &nR, &nG, &nB);
			pDC->FillSolidRect(rcSubItem, RGB(nR, nG, nB));
			pDC->SetTextColor(RGB(255,255,255));
		}
		else if(m_listTool.GetItemState(pLVCD->nmcd.dwItemSpec, LVIS_SELECTED))
		{
			pDC->FillSolidRect(rcSubItem, RGB(49, 105, 198));
			pDC->DrawFocusRect(rcSubItem);
			pDC->SetTextColor(RGB(255,255,255));
		}
		else if(0 == ((pLVCD->nmcd.dwItemSpec)%2))
		{
			pDC->FillSolidRect(rcSubItem, RGB(249, 249, 249));
			pDC->SetTextColor(RGB(85,85,85));
		}else
		{
			pDC->FillSolidRect(rcSubItem, RGB(255, 255, 255));
			pDC->SetTextColor(RGB(85,85,85));
		}

		// 텍스트 출력
		pDC->DrawText(strText, &rcSubItem, DT_SINGLELINE | DT_CENTER| DT_VCENTER );

		CDC::DeleteTempMap();

		*pResult = CDRF_SKIPDEFAULT;
		break;
	}
	case CDDS_ITEMPREPAINT:
		*pResult = CDRF_NOTIFYSUBITEMDRAW|CDRF_SKIPPOSTPAINT;
		break;
	default:
		*pResult = CDRF_DODEFAULT;
		break;
	}
}

void CDlgToolInfo::Layer_Color(int color, int *nR, int *nG, int *nB)
{
	switch (color){
	case 0:
		*nR=104;*nG=0;*nB=0;
		break;
	case 1:
		*nR=255;*nG=0;*nB=0;
		break;
	case 2:
		*nR=255;*nG=255;*nB=0;
		break;
	case 3:
		*nR=0;*nG=255;*nB=0;
		break;
	case 4:
		*nR=0;*nG=255;*nB=255;
		break;
	case 5:
		*nR=0;*nG=0;*nB=255;
		break;
	case 6:
		*nR=255;*nG=0;*nB=255;
		break;
	case 7:
		*nR=255;*nG=255;*nB=255;
		break;
	case 8:
		*nR=65;*nG=65;*nB=65;
		break;
	case 9:
		*nR=128;*nG=128;*nB=128;
		break;
	case 10:
		*nR=255;*nG=0;*nB=0;
		break;
	case 11:
		*nR=255;*nG=170;*nB=170;
		break;
	case 12:
		*nR=189;*nG=0;*nB=0;
		break;
	case 13:
		*nR=189;*nG=126;*nB=126;
		break;
	case 14:
		*nR=129;*nG=0;*nB=0;
		break;
	case 15:
		*nR=129;*nG=86;*nB=86;
		break;
	case 16:
		*nR=104;*nG=0;*nB=0;
		break;
	case 17:
		*nR=104;*nG=69;*nB=69;
		break;
	case 18:
		*nR=79;*nG=0;*nB=0;
		break;
	case 19:
		*nR=79;*nG=53;*nB=53;
		break;
	case 20:
		*nR=255;*nG=63;*nB=0;
		break;
	case 21:
		*nR=255;*nG=191;*nB=170;
		break;
	case 22:
		*nR=189;*nG=46;*nB=0;
		break;
	case 23:
		*nR=189;*nG=141;*nB=126;
		break;
	case 24:
		*nR=129;*nG=31;*nB=0;
		break;
	case 25:
		*nR=129;*nG=96;*nB=86;
		break;
	case 26:
		*nR=104;*nG=25;*nB=0;
		break;
	case 27:
		*nR=104;*nG=78;*nB=69;
		break;
	case 28:
		*nR=79;*nG=19;*nB=0;
		break;
	case 29:
		*nR=79;*nG=59;*nB=53;
		break;
	case 30:
		*nR=255;*nG=127;*nB=0;
		break;
	case 31:
		*nR=255;*nG=212;*nB=170;
		break;
	case 32:
		*nR=189;*nG=94;*nB=0;
		break;
	case 33:
		*nR=189;*nG=157;*nB=126;
		break;
	case 34:
		*nR=129;*nG=64;*nB=0;
		break;
	case 35:
		*nR=129;*nG=107;*nB=86;
		break;
	case 36:
		*nR=104;*nG=52;*nB=0;
		break;
	case 37:
		*nR=104;*nG=86;*nB=69;
		break;
	case 38:
		*nR=79;*nG=39;*nB=0;
		break;
	case 39:
		*nR=79;*nG=66;*nB=53;
		break;
	case 40:
		*nR=255;*nG=191;*nB=0;
		break;
	case 41:
		*nR=255;*nG=234;*nB=170;
		break;
	case 42:
		*nR=189;*nG=141;*nB=0;
		break;
	case 43:
		*nR=189;*nG=173;*nB=126;
		break;
	case 44:
		*nR=129;*nG=96;*nB=0;
		break;
	case 45:
		*nR=129;*nG=118;*nB=86;
		break;
	case 46:
		*nR=104;*nG=78;*nB=0;
		break;
	case 47:
		*nR=104;*nG=95;*nB=69;
		break;
	case 48:
		*nR=79;*nG=59;*nB=0;
		break;
	case 49:
		*nR=79;*nG=73;*nB=53;
		break;
	case 50:
		*nR=255;*nG=255;*nB=0;
		break;
	case 51:
		*nR=255;*nG=255;*nB=170;
		break;
	case 52:
		*nR=189;*nG=189;*nB=0;
		break;
	case 53:
		*nR=189;*nG=189;*nB=126;
		break;
	case 54:
		*nR=129;*nG=129;*nB=0;
		break;
	case 55:
		*nR=129;*nG=129;*nB=86;
		break;
	case 56:
		*nR=104;*nG=104;*nB=0;
		break;
	case 57:
		*nR=104;*nG=104;*nB=69;
		break;
	case 58:
		*nR=79;*nG=79;*nB=0;
		break;
	case 59:
		*nR=79;*nG=79;*nB=53;
		break;
	case 60:
		*nR=191;*nG=255;*nB=0;
		break;
	case 61:
		*nR=234;*nG=255;*nB=170;
		break;
	case 62:
		*nR=141;*nG=189;*nB=0;
		break;
	case 63:
		*nR=173;*nG=189;*nB=126;
		break;
	case 64:
		*nR=96;*nG=129;*nB=0;
		break;
	case 65:
		*nR=118;*nG=129;*nB=86;
		break;
	case 66:
		*nR=78;*nG=104;*nB=0;
		break;
	case 67:
		*nR=95;*nG=104;*nB=69;
		break;
	case 68:
		*nR=59;*nG=79;*nB=0;
		break;
	case 69:
		*nR=73;*nG=79;*nB=53;
		break;
	case 70:
		*nR=127;*nG=255;*nB=0;
		break;
	case 71:
		*nR=212;*nG=255;*nB=170;
		break;
	case 72:
		*nR=94;*nG=189;*nB=0;
		break;
	case 73:
		*nR=157;*nG=189;*nB=126;
		break;
	case 74:
		*nR=64;*nG=129;*nB=0;
		break;
	case 75:
		*nR=107;*nG=129;*nB=86;
		break;
	case 76:
		*nR=52;*nG=104;*nB=0;
		break;
	case 77:
		*nR=86;*nG=104;*nB=69;
		break;
	case 78:
		*nR=39;*nG=79;*nB=0;
		break;
	case 79:
		*nR=66;*nG=79;*nB=53;
		break;
	case 80:
		*nR=63;*nG=255;*nB=0;
		break;
	case 81:
		*nR=191;*nG=255;*nB=170;
		break;
	case 82:
		*nR=46;*nG=189;*nB=0;
		break;
	case 83:
		*nR=141;*nG=189;*nB=126;
		break;
	case 84:
		*nR=31;*nG=129;*nB=0;
		break;
	case 85:
		*nR=96;*nG=129;*nB=86;
		break;
	case 86:
		*nR=25;*nG=104;*nB=0;
		break;
	case 87:
		*nR=78;*nG=104;*nB=69;
		break;
	case 88:
		*nR=19;*nG=79;*nB=0;
		break;
	case 89:
		*nR=59;*nG=79;*nB=53;
		break;
	case 90:
		*nR=0;*nG=255;*nB=0;
		break;
	case 91:
		*nR=170;*nG=255;*nB=170;
		break;
	case 92:
		*nR=0;*nG=189;*nB=0;
		break;
	case 93:
		*nR=126;*nG=189;*nB=126;
		break;
	case 94:
		*nR=0;*nG=129;*nB=0;
		break;
	case 95:
		*nR=86;*nG=129;*nB=86;
		break;
	case 96:
		*nR=0;*nG=104;*nB=0;
		break;
	case 97:
		*nR=69;*nG=104;*nB=69;
		break;
	case 98:
		*nR=0;*nG=79;*nB=0;
		break;
	case 99:
		*nR=53;*nG=79;*nB=53;
		break;
	case 100:
		*nR=0;*nG=255;*nB=63;
		break;
	case 101:
		*nR=170;*nG=255;*nB=191;
		break;
	case 102:
		*nR=0;*nG=189;*nB=46;
		break;
	case 103:
		*nR=126;*nG=189;*nB=141;
		break;
	case 104:
		*nR=0;*nG=129;*nB=31;
		break;
	case 105:
		*nR=86;*nG=129;*nB=96;
		break;
	case 106:
		*nR=0;*nG=104;*nB=25;
		break;
	case 107:
		*nR=69;*nG=104;*nB=78;
		break;
	case 108:
		*nR=0;*nG=79;*nB=19;
		break;
	case 109:
		*nR=53;*nG=79;*nB=59;
		break;
	case 110:
		*nR=0;*nG=255;*nB=127;
		break;
	case 111:
		*nR=170;*nG=255;*nB=212;
		break;
	case 112:
		*nR=0;*nG=189;*nB=94;
		break;
	case 113:
		*nR=126;*nG=189;*nB=157;
		break;
	case 114:
		*nR=0;*nG=129;*nB=64;
		break;
	case 115:
		*nR=86;*nG=129;*nB=107;
		break;
	case 116:
		*nR=0;*nG=104;*nB=52;
		break;
	case 117:
		*nR=69;*nG=104;*nB=86;
		break;
	case 118:
		*nR=0;*nG=79;*nB=39;
		break;
	case 119:
		*nR=53;*nG=79;*nB=66;
		break;
	case 120:
		*nR=0;*nG=255;*nB=191;
		break;
	case 121:
		*nR=170;*nG=255;*nB=234;
		break;
	case 122:
		*nR=0;*nG=189;*nB=141;
		break;
	case 123:
		*nR=126;*nG=189;*nB=173;
		break;
	case 124:
		*nR=0;*nG=129;*nB=96;
		break;
	case 125:
		*nR=86;*nG=129;*nB=118;
		break;
	case 126:
		*nR=0;*nG=104;*nB=78;
		break;
	case 127:
		*nR=69;*nG=104;*nB=95;
		break;
	case 128:
		*nR=0;*nG=79;*nB=59;
		break;
	case 129:
		*nR=53;*nG=79;*nB=73;
		break;
	case 130:
		*nR=0;*nG=255;*nB=255;
		break;
	case 131:
		*nR=170;*nG=255;*nB=255;
		break;
	case 132:
		*nR=0;*nG=189;*nB=189;
		break;
	case 133:
		*nR=126;*nG=189;*nB=189;
		break;
	case 134:
		*nR=0;*nG=129;*nB=129;
		break;
	case 135:
		*nR=86;*nG=129;*nB=129;
		break;
	case 136:
		*nR=0;*nG=104;*nB=104;
		break;
	case 137:
		*nR=69;*nG=104;*nB=104;
		break;
	case 138:
		*nR=0;*nG=79;*nB=79;
		break;
	case 139:
		*nR=53;*nG=79;*nB=79;
		break;
	case 140:
		*nR=0;*nG=191;*nB=255;
		break;
	case 141:
		*nR=170;*nG=234;*nB=255;
		break;
	case 142:
		*nR=0;*nG=141;*nB=189;
		break;
	case 143:
		*nR=126;*nG=173;*nB=189;
		break;
	case 144:
		*nR=0;*nG=96;*nB=129;
		break;
	case 145:
		*nR=86;*nG=118;*nB=129;
		break;
	case 146:
		*nR=0;*nG=78;*nB=104;
		break;
	case 147:
		*nR=69;*nG=95;*nB=104;
		break;
	case 148:
		*nR=0;*nG=59;*nB=79;
		break;
	case 149:
		*nR=53;*nG=73;*nB=79;
		break;
	case 150:
		*nR=0;*nG=127;*nB=255;
		break;
	case 151:
		*nR=170;*nG=212;*nB=255;
		break;
	case 152:
		*nR=0;*nG=94;*nB=189;
		break;
	case 153:
		*nR=126;*nG=157;*nB=189;
		break;
	case 154:
		*nR=0;*nG=64;*nB=129;
		break;
	case 155:
		*nR=86;*nG=107;*nB=129;
		break;
	case 156:
		*nR=0;*nG=52;*nB=104;
		break;
	case 157:
		*nR=69;*nG=86;*nB=104;
		break;
	case 158:
		*nR=0;*nG=39;*nB=79;
		break;
	case 159:
		*nR=53;*nG=66;*nB=79;
		break;
	case 160:
		*nR=0;*nG=63;*nB=255;
		break;
	case 161:
		*nR=170;*nG=191;*nB=255;
		break;
	case 162:
		*nR=0;*nG=46;*nB=189;
		break;
	case 163:
		*nR=126;*nG=141;*nB=189;
		break;
	case 164:
		*nR=0;*nG=31;*nB=129;
		break;
	case 165:
		*nR=86;*nG=96;*nB=129;
		break;
	case 166:
		*nR=0;*nG=25;*nB=104;
		break;
	case 167:
		*nR=69;*nG=78;*nB=104;
		break;
	case 168:
		*nR=0;*nG=19;*nB=79;
		break;
	case 169:
		*nR=53;*nG=59;*nB=79;
		break;
	case 170:
		*nR=0;*nG=0;*nB=255;
		break;
	case 171:
		*nR=170;*nG=170;*nB=255;
		break;
	case 172:
		*nR=0;*nG=0;*nB=189;
		break;
	case 173:
		*nR=126;*nG=126;*nB=189;
		break;
	case 174:
		*nR=0;*nG=0;*nB=129;
		break;
	case 175:
		*nR=86;*nG=86;*nB=129;
		break;
	case 176:
		*nR=0;*nG=0;*nB=104;
		break;
	case 177:
		*nR=69;*nG=69;*nB=104;
		break;
	case 178:
		*nR=0;*nG=0;*nB=79;
		break;
	case 179:
		*nR=53;*nG=53;*nB=79;
		break;
	case 180:
		*nR=63;*nG=0;*nB=255;
		break;
	case 181:
		*nR=191;*nG=170;*nB=255;
		break;
	case 182:
		*nR=46;*nG=0;*nB=189;
		break;
	case 183:
		*nR=141;*nG=126;*nB=189;
		break;
	case 184:
		*nR=31;*nG=0;*nB=129;
		break;
	case 185:
		*nR=96;*nG=86;*nB=129;
		break;
	case 186:
		*nR=25;*nG=0;*nB=104;
		break;
	case 187:
		*nR=78;*nG=69;*nB=104;
		break;
	case 188:
		*nR=19;*nG=0;*nB=79;
		break;
	case 189:
		*nR=59;*nG=53;*nB=79;
		break;
	case 190:
		*nR=127;*nG=0;*nB=255;
		break;
	case 191:
		*nR=212;*nG=170;*nB=255;
		break;
	case 192:
		*nR=94;*nG=0;*nB=189;
		break;
	case 193:
		*nR=157;*nG=126;*nB=189;
		break;
	case 194:
		*nR=64;*nG=0;*nB=129;
		break;
	case 195:
		*nR=107;*nG=86;*nB=129;
		break;
	case 196:
		*nR=52;*nG=0;*nB=104;
		break;
	case 197:
		*nR=86;*nG=69;*nB=104;
		break;
	case 198:
		*nR=39;*nG=0;*nB=79;
		break;
	case 199:
		*nR=66;*nG=53;*nB=79;
		break;
	case 200:
		*nR=191;*nG=0;*nB=255;
		break;
	case 201:
		*nR=234;*nG=170;*nB=255;
		break;
	case 202:
		*nR=141;*nG=0;*nB=189;
		break;
	case 203:
		*nR=173;*nG=126;*nB=189;
		break;
	case 204:
		*nR=96;*nG=0;*nB=129;
		break;
	case 205:
		*nR=118;*nG=86;*nB=129;
		break;
	case 206:
		*nR=78;*nG=0;*nB=104;
		break;
	case 207:
		*nR=95;*nG=69;*nB=104;
		break;
	case 208:
		*nR=59;*nG=0;*nB=79;
		break;
	case 209:
		*nR=73;*nG=53;*nB=79;
		break;
	case 210:
		*nR=255;*nG=0;*nB=255;
		break;
	case 211:
		*nR=255;*nG=170;*nB=255;
		break;
	case 212:
		*nR=189;*nG=0;*nB=189;
		break;
	case 213:
		*nR=189;*nG=126;*nB=189;
		break;
	case 214:
		*nR=129;*nG=0;*nB=129;
		break;
	case 215:
		*nR=129;*nG=86;*nB=129;
		break;
	case 216:
		*nR=104;*nG=0;*nB=104;
		break;
	case 217:
		*nR=104;*nG=69;*nB=104;
		break;
	case 218:
		*nR=79;*nG=0;*nB=79;
		break;
	case 219:
		*nR=79;*nG=53;*nB=79;
		break;
	case 220:
		*nR=255;*nG=0;*nB=191;
		break;
	case 221:
		*nR=255;*nG=170;*nB=234;
		break;
	case 222:
		*nR=189;*nG=0;*nB=141;
		break;
	case 223:
		*nR=189;*nG=126;*nB=173;
		break;
	case 224:
		*nR=129;*nG=0;*nB=96;
		break;
	case 225:
		*nR=129;*nG=86;*nB=118;
		break;
	case 226:
		*nR=104;*nG=0;*nB=78;
		break;
	case 227:
		*nR=104;*nG=69;*nB=95;
		break;
	case 228:
		*nR=79;*nG=0;*nB=59;
		break;
	case 229:
		*nR=79;*nG=53;*nB=73;
		break;
	case 230:
		*nR=255;*nG=0;*nB=127;
		break;
	case 231:
		*nR=255;*nG=170;*nB=212;
		break;
	case 232:
		*nR=189;*nG=0;*nB=94;
		break;
	case 233:
		*nR=189;*nG=126;*nB=157;
		break;
	case 234:
		*nR=129;*nG=0;*nB=64;
		break;
	case 235:
		*nR=129;*nG=86;*nB=107;
		break;
	case 236:
		*nR=104;*nG=0;*nB=52;
		break;
	case 237:
		*nR=104;*nG=69;*nB=86;
		break;
	case 238:
		*nR=79;*nG=0;*nB=39;
		break;
	case 239:
		*nR=79;*nG=53;*nB=66;
		break;
	case 240:
		*nR=255;*nG=0;*nB=63;
		break;
	case 241:
		*nR=255;*nG=170;*nB=191;
		break;
	case 242:
		*nR=189;*nG=0;*nB=46;
		break;
	case 243:
		*nR=189;*nG=126;*nB=141;
		break;
	case 244:
		*nR=129;*nG=0;*nB=31;
		break;
	case 245:
		*nR=129;*nG=86;*nB=96;
		break;
	case 246:
		*nR=104;*nG=0;*nB=25;
		break;
	case 247:
		*nR=104;*nG=69;*nB=78;
		break;
	case 248:
		*nR=79;*nG=0;*nB=19;
		break;
	case 249:
		*nR=79;*nG=53;*nB=59;
		break;
	case 250:
		*nR=51;*nG=51;*nB=51;
		break;
	case 251:
		*nR=80;*nG=80;*nB=80;
		break;
	case 252:
		*nR=105;*nG=105;*nB=105;
		break;
	case 253:
		*nR=130;*nG=130;*nB=130;
		break;
	case 254:
		*nR=190;*nG=190;*nB=190;
		break;
	case 255:
		*nR=255;*nG=255;*nB=255;
		break;
	}
}

void CDlgToolInfo::SetOption()
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;
	CString strTemp;
	int nCheck;

	// Sector Size
	GetDlgItem(IDC_TOOLINFO_EDIT_SECTORSIZE)->GetWindowText(strTemp);
	pDlg->m_Entity.SetBlockSize((int)_ttof(strTemp));

	// Hole Size
	if(pDlg->m_bIsFileOpen == TRUE && pDlg->m_nSelLayer != -1) {
		GetDlgItem(IDC_TOOLINFO_EDIT_HOLESIZE)->GetWindowText(strTemp);
		pDlg->m_Entity.SetLayerHoleSize(pDlg->m_nSelLayer,  _ttof(strTemp) / 1000);
		strTemp.Format(L"%.2f", pDlg->m_Entity.GetLayerHoleSize(pDlg->m_nSelLayer) * 1000);
	}

	// Overrap
	GetDlgItem(IDC_TOOLINFO_EDIT_OVERLAP)->GetWindowText(strTemp);
	pDlg->m_Entity.m_fOverlap = _ttof(strTemp) / 1000;

	// CheckBox - Optimize 
	nCheck = ((CButton*)pDlg->m_dlgToolInfo.GetDlgItem(IDC_TOOLINFO_CHK_OPTIMIZE))->GetCheck();
	pDlg->m_Entity.SetOptimization(nCheck);

	// CheckBox - RemoveBlank 
	nCheck = ((CButton*)pDlg->m_dlgToolInfo.GetDlgItem(IDC_TOOLINFO_CHK_REMOVEBLANK))->GetCheck();
	pDlg->m_Entity.SetRemoveBlank(nCheck);

	// CheckBox - Center Align 
	nCheck = pDlg->m_dlgToolInfo.m_chkCenterAlign.GetCheck();
	pDlg->m_Entity.SetCenterAlign(nCheck);

	// CheckBox - 3Step Mode 
	nCheck = pDlg->m_dlgToolInfo.m_chk3StepMode.GetCheck();
	pDlg->m_Entity.m_b3StepMode = nCheck;
}