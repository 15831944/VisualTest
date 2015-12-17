
// LnpCAMDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LnpCAM.h"
#include "LnpCAMDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLnpCAMDlg dialog


CLnpCAMDlg::CLnpCAMDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLnpCAMDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	makeFolders();

	m_strLayerName = NULL;

	m_bIsFileOpen = FALSE;

	m_nInspectionCnt = 2;

	for(int i=0; i<m_nInspectionCnt; i++)
	{
		m_dInspectionPosX[i] = i*100;
		m_dInspectionPosY[i] = i*100;
	}

	m_Viewer.CreateView(&m_Entity);

	m_nSelLayer = -1;

	m_nFileType = -1;

	//File Name
	m_strFileName = L"";
	m_strFilePath = L"";
	m_strFileTitle = L"";
}

void CLnpCAMDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLnpCAMDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_DROPFILES()
END_MESSAGE_MAP()


// CLnpCAMDlg message handlers

BOOL CLnpCAMDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	
	SetWindowPos(NULL, 0, -3, 1280 + 6, 1024 + 6, SWP_NOSIZE);
	//ShowWindow(SW_MAXIMIZE);
	InitDlg();
	StartProcess();
	//m_Viewer.InitViewer(&m_dlgViewer);
	m_Viewer.InitViewer(&m_dlgViewer);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLnpCAMDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLnpCAMDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CLnpCAMDlg::InitDlg(void)
{
	CRect rt;
	GetClientRect(&rt);

	int nWidth = rt.right - rt.left;
	
	int midStart = 50;
	int btmStart = rt.bottom - rt.top - 50;
	int midHeight = btmStart - midStart;
	int edge = 2;

	
	m_dlgMenu.Create(IDD_DLG_MENU, this);
	m_dlgMenu.SetWindowPos(NULL, 0, 0, nWidth, midStart,SWP_SHOWWINDOW); 

	m_dlgInfo.Create(IDD_DLG_INFO, this);
	m_dlgInfo.SetWindowPos(NULL, 0, btmStart+edge*2, nWidth, rt.bottom - btmStart,SWP_SHOWWINDOW);

	m_dlgToolInfo.Create(IDD_DLG_TOOL_INFO, this);
	m_dlgToolInfo.SetWindowPos(NULL, edge, midStart+edge, 180-edge, midHeight,SWP_SHOWWINDOW);
			
	m_dlgViewer.Create(IDD_DLG_VIEWER, this);
	m_dlgViewer.SetWindowPos(NULL, 180+edge, midStart+edge, 800, midHeight,SWP_SHOWWINDOW);  

	m_dlgViewerSet.Create(IDD_DLG_VIEWER_SET, this);
	m_dlgViewerSet.SetWindowPos(NULL, 980+edge*2, midStart+edge, nWidth - 980 - edge*3, midHeight,SWP_SHOWWINDOW);
	m_dlgViewerSet.InitDlg();

	if( _tcscmp( AfxGetApp()->m_lpCmdLine, _T("")) != 0 )
		m_Process.StartFileLoading();
}


BOOL CLnpCAMDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rt;
    GetClientRect(&rt);  // 클라이언트 영역의 크기 계산
    pDC->FillSolidRect(&rt, RGB(41, 56, 82));  // 클라이언트 영역 크기만큼 흰색으로 채운다

    return TRUE;  // 상위 클래스의 OnEraseBkgnd 함수를 호출하지 않아야 바꾼 배경색이 적용된다
}


void CLnpCAMDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	TerminateProcess();

	if(m_strLayerName != NULL){
		delete[] m_strLayerName;
		m_strLayerName = NULL;
	}
}


void CLnpCAMDlg::StartProcess(void)
{
	m_Process.StartDisplayCoord();
}


void CLnpCAMDlg::TerminateProcess(void)
{
	m_Process.m_bStopDisplayCoord = TRUE;

	//Wait For Close Thread;
	Sleep(100);
}

void CLnpCAMDlg::OnClose()
{
	m_Process.EndProcess();

	CDialogEx::OnClose();
}


BOOL CLnpCAMDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)           
	{
		return TRUE; 
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


int CLnpCAMDlg::WriteLpdFile()
{
	
//	FILE* JobFileOut;
// 	if( m_bLoadStopFlag == TRUE )
// 		return 0;

// 	char path[1000];		/** path local variable 01*/
// 	int i;
// 
// 	
// 	wsprintfA(path, "%S", m_strCoordFolder + L"\test.lpd"); 
// 
// 	if((JobFileOut =_fsopen(path, "w",_SH_DENYNO))==NULL)
// 		return 0;
// 
// 	fprintf(JobFileOut, "Start\n");
// 	fprintf(JobFileOut, "<JobInformation>\n");
// 	fprintf(JobFileOut, "\n");
// 
// 	fprintf(JobFileOut, "InspectionCount %d\n", m_nInspectionCnt);
// 
// 	for(i = 0; i < m_nInspectionCnt; i++)
// 	{													
// 		fprintf(JobFileOut, "InspectionPoint%d X: %.3f Y: %.3f\n", i+1, m_dInspectionPosX[i], m_dInspectionPosY[i]);
// 	}
// 	fprintf(JobFileOut, "\n");
// 
// 	fprintf(JobFileOut, "Version %d\n", /* m_nVersion */ 100 );
// 	fprintf(JobFileOut, "BlockSize %d\n", /* m_iBlockSize */ 40);
// 	fprintf(JobFileOut, "ToolCount %d\n", m_nLayerCnt);
// 
// 	for(i=0;i<m_nLayerCnt;i++)
// 	{
// 		fprintf(JobFileOut, "\nToolName %S\n", m_pLayer[i].strLayerName);
// 		fprintf(JobFileOut, "ToolPointCount %d\n", m_pLayer[i].iEntityCount);
// 		fprintf(JobFileOut, "ToolColor %d\n", m_pLayer[i].i_LayerColor);
// 		fprintf(JobFileOut, "BlockCount %d\n", m_pLayer[i].m_iRealBlockCount);
// 
// 		for(int j=0;j<m_pLayer[i].m_iRealBlockCount;j++)
// 		{
// 			m_iProgressCount++;
// 			int BlockNum = m_pLayer[i].pBlockPos[m_pLayer[i].pBlockOrder[j]].lBlockNumber;
// 			int PointCount = m_pLayer[i].pInBlockPointCount[BlockNum];
// 
// 			fprintf(JobFileOut, "\nBlockNumber %d\n", BlockNum);
// 			fprintf(JobFileOut, "BlockPointCount %d\n", PointCount);
// 			fprintf(JobFileOut, "BlockPositionX %.3f\n", m_pLayer[i].pBlockPos[m_pLayer[i].pBlockOrder[j]].fX);
// 			fprintf(JobFileOut, "BlockPositionY %.3f\n", m_pLayer[i].pBlockPos[m_pLayer[i].pBlockOrder[j]].fY);
// 			int TempNum = BlockNum;
// 			for(int k=0;k<m_pLayer[i].m_iRealBlockCount;k++)
// 			{
// 				if(BlockNum == m_pLayer[i].pCutting[k].lBlockNumber)
// 				{
// 					TempNum = k;
// 					break;
// 				}
// 			}
// 
// 			for(int k=0;k<PointCount;k++)
// 			{
// 				fprintf(JobFileOut, "PointX %.3f  PointY %.3f\n", 
// 					m_pLayer[i].pCutting[TempNum].m_pPoint[k].fX,
// 					m_pLayer[i].pCutting[TempNum].m_pPoint[k].fY);
// 
// 				if( m_bLoadStopFlag == TRUE )
// 				{
// 					fclose(BlockFileIn);
// 					return 0;
// 				}
// 
// 			}
// 		}
// 	}
// 
// 	fprintf(JobFileOut, "End\n");
// 	fprintf(JobFileOut, "0\n");
// 
// 	fclose(BlockFileIn);
// 
 	return 1;
}


int CLnpCAMDlg::ReadLpdFile()
{
	return 1;
}


void CLnpCAMDlg::makeFolders()
{
	CString TempStr;			/** TempStr local variable 01*/
	CString TempStr2;			/** TempStr2 local variable 02*/
	int TempLength;				/** TempLength local variable 03*/
	int TempNumber;				/** TempNumber local variable 04*/
	TCHAR folder[1000];			/** folder local variable 05*/

	////////////////// 블럭 실행 디렉토리 만들기 ///////////////////////////
	GetModuleFileName(AfxGetInstanceHandle(), folder, 1000);

	TempStr = folder;
	TempLength = TempStr.GetLength();
	TempStr.MakeReverse();
	TempNumber = TempStr.Find(_T("\\"),0);
	TempStr = TempStr.Right(TempLength-TempNumber-1);
	TempStr.MakeReverse();

	m_strCoordFolder	= TempStr+_T("\\Coord");
// 	m_strCommonFolder	= m_strCoordFolder+_T("\\Comm");
// 	m_strDxfFolder		= m_strCoordFolder+_T("\\Dxf");
// 	m_strBlockFolder	= m_strDxfFolder+_T("\\Block");
// 	m_strEntityFolder	= m_strDxfFolder+_T("\\Entity");

	CreateDirectory(m_strCoordFolder,	NULL);
// 	CreateDirectory(m_strCommonFolder,	NULL);
// 	CreateDirectory(m_strBlockFolder,	NULL);
// 	CreateDirectory(m_strDxfFolder,		NULL);
// 	CreateDirectory(m_strEntityFolder,	NULL);
}

void CLnpCAMDlg::TransformLpd2Entity(void)
{
	m_Entity.InitializeLayerMemory();

	m_Entity.m_fTotalMinX = 9999999;
	m_Entity.m_fTotalMinY = 9999999;
	m_Entity.m_fTotalMaxX = -9999999;
	m_Entity.m_fTotalMaxY = -9999999;

	/*Start Set Buffer Entity */
	
	int i;							/** i local variable 04*/
	//int temp_cnt;					/** temp_cnt local variable 09*/

	m_Entity.m_dSectorTotalMoveX = 0;
	m_Entity.m_dSectorTotalMoveY = 0;

	m_Entity.m_nInspectionCnt = m_Lpd.m_nInspectionCount;

	for(i = 0 ; i < m_nInspectionCnt; i++)
	{
		m_Entity.m_dInspectionPosX[i] = m_Lpd.m_dInspectionPosX[i];
		m_Entity.m_dInspectionPosY[i] = m_Lpd.m_dInspectionPosY[i];
	}

	m_Entity.m_iBlockSize = (int)m_Lpd.m_dSectorSize;

	m_Entity.m_lFilePointCount = m_Lpd.m_nEntityCount;

	m_Entity.m_iLayerCount = m_Lpd.m_nLayerCnt;

	if(m_Entity.m_iLayerCount <1)
		return;

	m_Entity.m_pBlockPos = NULL;
	m_Entity.AllocLayerMem(m_Entity.m_iLayerCount);

	m_Entity.m_nLayerBufferSize = m_Entity.m_iLayerCount;

	for(i=0;i<m_Entity.m_iLayerCount;i++)
	{
		m_Entity.m_pLayer[i].m_nOptimizeType = _ENTITY_LINE;
		m_Entity.m_pLayer[i].pCutBlock = NULL;
		m_Entity.m_pLayer[i].pEntity = NULL; // 구조체 포인터 변수 
		m_Entity.m_pLayer[i].pOriginEntity = NULL; // 구조체 포인터 변수 
		m_Entity.m_pLayer[i].pBlockOrder = NULL;
		m_Entity.m_pLayer[i].pInBlockEntityCount = NULL;

		m_Entity.m_pLayer[i].strLayerName = m_Lpd.m_pLayer[i].strLayerName;		

		m_Entity.AllocLayerEntityMem(i, m_Lpd.m_pLayer[i].nEntityCount);

		m_Entity.m_pLayer[i].iOriginCount = 0;

		m_Entity.m_pLayer[i].fHoleSize = m_Lpd.m_pLayer[i].dHoleSize;
	}

	/* Fill layer information */

	for(i=0;i<m_Entity.m_iLayerCount;i++)
	{
		int nIndex = 0;

		m_Entity.AllocCutBlock(i, m_Lpd.m_pLayer[i].nSectorCount);

		for(int j=0; j< m_Lpd.m_pLayer[i].nSectorCount; j++)
		{
			m_Entity.AllocCutBlockEntity(i, j, m_Lpd.m_pLayer[i].pBlock[j].nEntityCnt);

			for(int k=0; k < m_Lpd.m_pLayer[i].pBlock[j].nEntityCnt; k++)
			{
				m_Entity.m_pLayer[i].pOriginEntity[nIndex].tempLine = NULL;
				m_Entity.m_pLayer[i].pOriginEntity[nIndex].tempSize = 0;

				if( m_Lpd.m_pLayer[i].pBlock[j].pEntity[k].nType == _ENTITY_POINT )
				{
					m_Entity.m_pLayer[i].m_nOptimizeType = _ENTITY_POINT;

					m_Entity.m_pLayer[i].pOriginEntity[nIndex].nType = _ENTITY_POINT;
					m_Entity.m_pLayer[i].pOriginEntity[nIndex].nColor = i;
					m_Entity.m_pLayer[i].pOriginEntity[nIndex].cX = m_Lpd.m_pLayer[i].pBlock[j].pEntity[k].cX;
					m_Entity.m_pLayer[i].pOriginEntity[nIndex].cY = m_Lpd.m_pLayer[i].pBlock[j].pEntity[k].cY;

					m_Entity.m_pLayer[i].pCutBlock[j].m_pEntity[k].nType = _ENTITY_POINT;
					m_Entity.m_pLayer[i].pCutBlock[j].m_pEntity[k].nColor = i;
					m_Entity.m_pLayer[i].pCutBlock[j].m_pEntity[k].cX = m_Lpd.m_pLayer[i].pBlock[j].pEntity[k].cX;
					m_Entity.m_pLayer[i].pCutBlock[j].m_pEntity[k].cY = m_Lpd.m_pLayer[i].pBlock[j].pEntity[k].cY;

					m_Entity.CheckMinMaxTotalPoint(
						m_Entity.m_pLayer[i].pOriginEntity[nIndex].cX,
						m_Entity.m_pLayer[i].pOriginEntity[nIndex].cY);

					nIndex++;
					m_Entity.m_pLayer[i].iOriginCount++;
				}
				else if( m_Lpd.m_pLayer[i].pBlock[j].pEntity[k].nType == _ENTITY_LINE)
				{
					m_Entity.m_pLayer[i].pOriginEntity[nIndex].nType = _ENTITY_LINE;
					m_Entity.m_pLayer[i].pOriginEntity[nIndex].nColor = i;
					m_Entity.m_pLayer[i].pOriginEntity[nIndex].sX = m_Lpd.m_pLayer[i].pBlock[j].pEntity[k].sX;
					m_Entity.m_pLayer[i].pOriginEntity[nIndex].sY = m_Lpd.m_pLayer[i].pBlock[j].pEntity[k].sY;
					m_Entity.m_pLayer[i].pOriginEntity[nIndex].eX = m_Lpd.m_pLayer[i].pBlock[j].pEntity[k].eX;
					m_Entity.m_pLayer[i].pOriginEntity[nIndex].eY = m_Lpd.m_pLayer[i].pBlock[j].pEntity[k].eY;

					m_Entity.m_pLayer[i].pCutBlock[j].m_pEntity[k].nType = _ENTITY_LINE;
					m_Entity.m_pLayer[i].pCutBlock[j].m_pEntity[k].nColor = i;
					m_Entity.m_pLayer[i].pCutBlock[j].m_pEntity[k].sX = m_Lpd.m_pLayer[i].pBlock[j].pEntity[k].sX;
					m_Entity.m_pLayer[i].pCutBlock[j].m_pEntity[k].sY = m_Lpd.m_pLayer[i].pBlock[j].pEntity[k].sY;
					m_Entity.m_pLayer[i].pCutBlock[j].m_pEntity[k].eX = m_Lpd.m_pLayer[i].pBlock[j].pEntity[k].eX;
					m_Entity.m_pLayer[i].pCutBlock[j].m_pEntity[k].eY = m_Lpd.m_pLayer[i].pBlock[j].pEntity[k].eY;

					m_Entity.CheckMinMaxTotalLine(
						m_Entity.m_pLayer[i].pOriginEntity[nIndex].sX,
						m_Entity.m_pLayer[i].pOriginEntity[nIndex].sY,
						m_Entity.m_pLayer[i].pOriginEntity[nIndex].eX,
						m_Entity.m_pLayer[i].pOriginEntity[nIndex].eY);

					nIndex++;
					m_Entity.m_pLayer[i].iOriginCount++;

				}	
			}
		}

		m_Entity.m_pLayer[i].iEntityCount = m_Entity.m_pLayer[i].iOriginCount;

		for(int j=0; j<m_Lpd.m_pLayer[i].nSectorCount; j++)
			m_Entity.m_pLayer[i].pCutBlock[j].lOrderNumber = _SECT_EMPTY;
		
		m_Entity.m_pLayer[i].iRealBlockCount = 0;

		for(int j=0; j<m_Lpd.m_pLayer[i].nSectorCount; j++)
		{
			int nBlockNum = m_Lpd.m_pLayer[i].pBlockOrder[j];
			
			m_Entity.m_pLayer[i].pBlockOrder[j] = nBlockNum;

			if(nBlockNum >= 0){
				m_Entity.m_pLayer[i].pCutBlock[nBlockNum].lOrderNumber = j;
				m_Entity.m_pLayer[i].iRealBlockCount++;
			}
		}
	}
	/* End Set Buffer Entity */////////////////////////////////////////////////

	/* Alloc Sector Line */

	if(m_Lpd.m_nLPDVersion < 2){

		m_Entity.m_SectLine.RowCnt = m_Lpd.m_nBlockCntX + 1;
		m_Entity.m_SectLine.ColCnt = m_Lpd.m_nBlockCntY + 1;
		m_Entity.AllocSectLineMem(m_Entity.m_SectLine.RowCnt, m_Entity.m_SectLine.ColCnt);

		m_Entity.MakeBlockAllLayer();

		return;
	}

	m_Entity.DeleteSectorLineMem();

	m_Entity.m_SectLine.RowCnt = m_Lpd.m_nBlockLineCntX;
	m_Entity.m_SectLine.ColCnt = m_Lpd.m_nBlockLineCntY;

	m_Entity.AllocSectLineMem(m_Entity.m_SectLine.RowCnt, m_Entity.m_SectLine.ColCnt);
	
	for(i=0; i<m_Entity.m_SectLine.RowCnt; i++)
	{
		m_Entity.m_SectLine.Row[i].sx = m_Lpd.m_SectLineX[i].sX;
		m_Entity.m_SectLine.Row[i].sy = m_Lpd.m_SectLineX[i].sY;
		m_Entity.m_SectLine.Row[i].ex = m_Lpd.m_SectLineX[i].eX;
		m_Entity.m_SectLine.Row[i].ey = m_Lpd.m_SectLineX[i].eY;
	}

	for(i=0; i<m_Entity.m_SectLine.ColCnt; i++)
	{
		m_Entity.m_SectLine.Col[i].sx = m_Lpd.m_SectLineY[i].sX;
		m_Entity.m_SectLine.Col[i].sy = m_Lpd.m_SectLineY[i].sY;
		m_Entity.m_SectLine.Col[i].ex = m_Lpd.m_SectLineY[i].eX;
		m_Entity.m_SectLine.Col[i].ey = m_Lpd.m_SectLineY[i].eY;
	}
	
	m_Entity.MakeBlockAllLayer();

	
}

void CLnpCAMDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: Add your message handler code here and/or call default

	TCHAR strPathName[1024];
	//ㅊㄴ
	
	int nCount = ::DragQueryFile(hDropInfo, 0xFFFFFFFF, strPathName, 1024);

	if(nCount > 1){
		AfxMessageBox(L"하나의 파일만 선택해 주십시오");
	}
	else{
		DragQueryFile(hDropInfo, 0, strPathName, 1024);
		
		AfxGetApp()->m_lpCmdLine = strPathName;
		m_Process.StartFileLoading();		
	}
		
	CDialogEx::OnDropFiles(hDropInfo);
}

