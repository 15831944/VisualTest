#include "StdAfx.h"
#include "LnpCAMProcess.h"
#include "LnpCAM.h"
#include "LnpCAMDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CLnpCAMProcess::CLnpCAMProcess(void)
{
	hProcessDisplayCoordTH = NULL;
	
	hProcessFileTH = NULL;
	
	hProcessProgressTH = NULL;

	m_bStopFileProcess = false;
}


CLnpCAMProcess::~CLnpCAMProcess(void)
{
	EndProcess();
}

void CLnpCAMProcess::EndProcess(void)
{
		
	m_bStopDisplayCoord = TRUE;
	m_bStopFileProcess = true;

	if(hProcessDisplayCoordTH != NULL)
		WaitForSingleObject(hProcessDisplayCoordTH->m_hThread, INFINITE);

// 	if(hProcessProgressTH != NULL)
// 		WaitForSingleObject(hProcessProgressTH->m_hThread, INFINITE);

	if(hProcessFileTH != NULL)
		WaitForSingleObject(hProcessFileTH->m_hThread, INFINITE);

	
}

void CLnpCAMProcess::StartDisplayCoord(void)
{
	m_bStopDisplayCoord = FALSE;

	if(hProcessDisplayCoordTH == NULL)
	{
		hProcessDisplayCoordTH = ::AfxBeginThread(DisplayCoord, this, THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED);
		hProcessDisplayCoordTH->ResumeThread();
	}
}

UINT DisplayCoord(LPVOID pParam)
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;
	CLnpCAMProcess *pProcess = (CLnpCAMProcess*)pParam;

	float fCoordX, fCoordY;
	CRect rt1, rt2;
	CString strTemp;
	double dAgoX = 0, dAgoY = 0;
	
	while(1)
	{
		Sleep(50);

		if(pProcess->m_bStopDisplayCoord== TRUE)
			break;
				
		pDlg->m_Viewer.GetCoord(&fCoordX, &fCoordY);

		if(dAgoX == fCoordX && dAgoY == fCoordY)
			continue;
		
		pDlg->m_dlgInfo.m_txtCoordX.GetWindowRect(&rt1);
				
		pDlg->m_dlgInfo.m_txtCoordY.GetWindowRect(&rt2);
		
		CRect rt3(rt1.left, rt1.top, rt2.right ,rt2.bottom);

		pDlg->m_dlgInfo.ScreenToClient(&rt3);	
		pDlg->m_dlgInfo.InvalidateRect(rt3, TRUE);
				
		strTemp.Format(L"%.3f", fCoordX);
		pDlg->m_dlgInfo.m_txtCoordX.SetWindowTextW(strTemp);

		strTemp.Format(L"%.3f", fCoordY);
		pDlg->m_dlgInfo.m_txtCoordY.SetWindowTextW(strTemp);

		dAgoX = fCoordX; dAgoY = fCoordY;
	}

	hProcessDisplayCoordTH = NULL;
	return 1;
}

void CLnpCAMProcess::StopFileProcessing(void)
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;
	m_bStopFileProcess = true;
}

void CLnpCAMProcess::StartFileLoading(void)
{
	if(hProcessFileTH == NULL && hProcessProgressTH == NULL)
	{
		m_bStopFileProcess = false;

		m_nProgressPos = 0;
		
		hProcessFileTH = ::AfxBeginThread(ProcessLoadFile, this, THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED);
		hProcessFileTH->ResumeThread();

		hProcessProgressTH = ::AfxBeginThread(ProcessProgressLoad, this, THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED);
		hProcessProgressTH->ResumeThread();
	}
}

UINT ProcessLoadFile(LPVOID pParam)
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;
	CLnpCAMProcess *pProcess = (CLnpCAMProcess*)pParam;

	bool bIsFileOpen = false;

	pDlg->m_nSelLayer = -1;
	CString strFileList;

	pProcess->m_nProgressPos = 0;

	strFileList = _T("*.dxf;*.bas;*.drl;*.gbr;*.lpd;|*.dxf;*.bas;*.drl;*.gbr;*.lpd;|");
	strFileList = strFileList + L"DXF File(*.dxf)|*.dxf|";
	strFileList = strFileList + L"BAS File(*.bas)|*.bas|";
	strFileList = strFileList + L"DRL File(*.drl)|*.drl|";
	strFileList = strFileList + L"GBR File(*.gbr)|*.gbr|";
	strFileList = strFileList + L"LPD File(*.lpd)|*.lpd|";
	
	CString strPath;
	CString strName;
	CString strTitle;
	pProcess->m_strFileExt = L"";	

	if( !_tcscmp(AfxGetApp()->m_lpCmdLine, _T("") ) )
	{
		CFileDialog dlgOpen(TRUE, L"File", L"", OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
	 		strFileList	,NULL);

		if(dlgOpen.DoModal() != IDOK){
			pProcess->m_bStopFileProcess = true;
			hProcessFileTH = NULL;
			return 0;
		}

		strPath = dlgOpen.GetPathName();
		strName = dlgOpen.GetFileName();
		pProcess->m_strFileExt = dlgOpen.GetFileExt();
		strTitle = dlgOpen.GetFileTitle();
	}
	else
	{
		strPath = AfxGetApp()->m_lpCmdLine;
		strPath.Replace(L"\"", L"");

		strName = PathFindFileName(strPath);

		pProcess->m_strFileExt = PathFindExtension(strPath);
		pProcess->m_strFileExt.Replace(L".", L"");
		
		strTitle = strName;
		strTitle.Replace(pProcess->m_strFileExt, L"");
		AfxGetApp()->m_lpCmdLine = _T("");
	}

	pDlg->m_dlgViewerSet.GetDlgItem(IDC_VIEWSET_BTN_ORDER)->EnableWindow(FALSE);
		
	if(pProcess->m_strFileExt == "dxf")
	{
		pProcess->m_nOriginCnt = pDlg->m_Dxf.GetProgressTotal();
		pProcess->m_nProgressPos = 5;

		if(!pDlg->m_Dxf.LoadDxfFile(strPath)){
			pProcess->m_bStopFileProcess = true;
			hProcessFileTH = NULL;
			return 0;
		}

		bIsFileOpen = true;
		pDlg->m_nFileType = 0;
	}

	else if(pProcess->m_strFileExt == "bas")
	{
		pProcess->m_nOriginCnt = 20;
		pProcess->m_nProgressPos = 5;

		if(!pDlg->m_Bas.Load(strPath)){
			pProcess->m_bStopFileProcess = true;
			hProcessFileTH = NULL;
			return 0;
		}
		bIsFileOpen = true;
		pDlg->m_nFileType = 1;
	}

	else if(pProcess->m_strFileExt == "drl")
	{
		pProcess->m_nOriginCnt = 20;
		pProcess->m_nProgressPos = 5;

		if(!pDlg->m_Drl.Load_DrlFile(strPath)){
			pProcess->m_bStopFileProcess = true;
			hProcessFileTH = NULL;
			return 0;
		}
		bIsFileOpen = true;
		pDlg->m_nFileType = 2;
	}

	else if(pProcess->m_strFileExt == "gbr")
	{
		pProcess->m_nOriginCnt = 20;
		pProcess->m_nProgressPos = 5;

		if(!pDlg->m_Gbr.LoadGbrFile(strPath)){
			pProcess->m_bStopFileProcess = true;
			hProcessFileTH = NULL;
			return 0;
		}
		bIsFileOpen = true;
		pDlg->m_nFileType = 3;
	}

	else if(pProcess->m_strFileExt == "lpd")
	{
		pProcess->m_nOriginCnt = 20;
		pProcess->m_nProgressPos = 5;

		if(!pDlg->m_Lpd.LoadLpdFile(strPath)){
			pProcess->m_bStopFileProcess = true;
			hProcessFileTH = NULL;
			return 0;
		}
		pProcess->m_strFileExt = L"lpd";
		pDlg->TransformLpd2Entity();

		bIsFileOpen = true;
		pDlg->m_nFileType = 4;

		pDlg->m_dlgViewerSet.GetDlgItem(IDC_VIEWSET_BTN_ORDER)->EnableWindow(TRUE);

		//Set Overlap Size
		CString strVal;
		strVal.Format(L"%d", int(pDlg->m_Lpd.m_dOverlapSize*1000));pDlg->m_dlgToolInfo.GetDlgItem(IDC_TOOLINFO_EDIT_OVERLAP)->SetWindowText(strVal);		

		//Set OFfsetSize Size
		strVal.Format(L"%.3f", pDlg->m_Lpd.m_dOffsetX);pDlg->m_dlgViewerSet.m_dlgSector.GetDlgItem(IDC_SECTOR_EDIT_OFFSETX)->SetWindowText(strVal);	
		strVal.Format(L"%.3f", pDlg->m_Lpd.m_dOffsetY);pDlg->m_dlgViewerSet.m_dlgSector.GetDlgItem(IDC_SECTOR_EDIT_OFFSETY)->SetWindowText(strVal);	
	}
	else
	{
		hProcessFileTH = NULL;
		pProcess->m_bStopFileProcess = true;
		return 0;
	}

	if(!bIsFileOpen){
		hProcessFileTH = NULL;
		pProcess->m_bStopFileProcess = true;
		return 0;
	}

	pProcess->m_nProgressPos = 20;

	if(pProcess->m_bStopFileProcess){
		hProcessFileTH = NULL;
		return 0;
	}

	pDlg->m_Viewer.SetFileLoad(FALSE);

	//Load Entity.dat File
	if(pDlg->m_nFileType != _FILE_LPD){

		pDlg->m_Entity.SetProgress(0);

		if(!pDlg->m_Entity.LoadEntityFile()){
			pProcess->m_bStopFileProcess = true;
			hProcessFileTH = NULL;
			return 0;
		}
	}
	
	if(pProcess->m_bStopFileProcess){
		hProcessFileTH = NULL;
		return 0;
	}

	//File VIewer
	pDlg->m_Viewer.SetFileLoad(TRUE);
	pDlg->m_Viewer.Initialize();

	pDlg->m_nLayerCnt = pDlg->m_Entity.GetLayerCount();

	if(pDlg->m_strLayerName != NULL){
		delete[] pDlg->m_strLayerName;
		pDlg->m_strLayerName = NULL;
	}
	pDlg->m_strLayerName = new CString[pDlg->m_nLayerCnt];
			
	for(int i=0; i<pDlg->m_nLayerCnt; i++)
	{
		pDlg->m_strLayerName[i] = pDlg->m_Entity.GetLayerName(i);
	}
	pDlg->m_dlgToolInfo.DisplayList();

	pDlg->m_bIsFileOpen = TRUE;
	pDlg->m_dlgViewerSet.ModifyHideDlg();

	pDlg->m_strFileName = strName;
	if( pDlg->m_nFileType != _FILE_LPD )
		pDlg->m_strFilePath = strPath;
	else
		pDlg->m_strFilePath = pDlg->m_Lpd.m_strFilePath;
	pDlg->m_strFileTitle = strTitle;

	//List Select
	pDlg->m_nSelLayer = 0;
	pDlg->m_Entity.SelectLayer(pDlg->m_nSelLayer);
	pDlg->m_dlgToolInfo.m_listTool.SetItemState(0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);

	CString strTemp;

	strTemp.Format(L"%.2f", pDlg->m_Entity.GetLayerHoleSize(0) * 1000);
	pDlg->m_dlgToolInfo.GetDlgItem(IDC_TOOLINFO_EDIT_HOLESIZE)->SetWindowTextW(strTemp);

	pProcess->m_nProgressPos = 94;
	Sleep(200);

	//File Name Disp
	pDlg->SetWindowTextW(L"LnpCAM - " + strPath);
	
	pProcess->m_bStopFileProcess = true;

	if(pProcess->m_strFileExt == "dxf")
	{
		double dMoveX, dMoveY;
		pDlg->m_Entity.GetSectorTotalMove(&dMoveX, &dMoveY);

		CString strVal;
		strVal.Format(L"%.3f", dMoveX);pDlg->m_dlgViewerSet.m_dlgSector.GetDlgItem(IDC_SECTOR_EDIT_OFFSETX)->SetWindowText(strVal);
		strVal.Format(L"%.3f", dMoveY);pDlg->m_dlgViewerSet.m_dlgSector.GetDlgItem(IDC_SECTOR_EDIT_OFFSETY)->SetWindowText(strVal);
	}

	hProcessFileTH = NULL;
	return 1;
}

UINT ProcessProgressLoad(LPVOID pParam)
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;
	CLnpCAMProcess *pProcess = (CLnpCAMProcess*)pParam;

	int nPos = 0;

	int nOriginCount;
	bool bFirst = true;

	pDlg->m_dlgInfo.SetProgressRangeLoad(0, 100);
	pDlg->m_dlgInfo.SetProgressMsg(L"파일 읽기 시작");

	while( !pProcess->m_bStopFileProcess )
	{
		nPos = pProcess->m_nProgressPos;

		switch( nPos )
		{
		case 5:
			{
				if( bFirst == true ){
					pDlg->m_dlgInfo.m_progressFileLoad.ShowWindow(true);
					bFirst = false;
				}

				if(pProcess->m_strFileExt = L"dxf")
					nOriginCount = pDlg->m_Dxf.GetProgress();
				else if(pProcess->m_strFileExt = L"bas")
					nOriginCount = 0;
				else if(pProcess->m_strFileExt = L"gbr")
					nOriginCount = 0;
				else if(pProcess->m_strFileExt = L"lpd")
					nOriginCount = 0;
				else if(pProcess->m_strFileExt = L"drl")
					nOriginCount = 0;

				nPos = nPos + (int) (nOriginCount * 20.0 / pProcess->m_nOriginCnt);

				if(nPos > 20)
					break;

				pDlg->m_dlgInfo.SetProgressPosLoad( nPos );	
				break;
			}
		case 20:
			{
				nOriginCount = pDlg->m_Entity.GetProgressTotal();

				if(nOriginCount == 0)
					break;

				nPos = nPos + (int) ( pDlg->m_Entity.GetProgress() * 70.0 / nOriginCount );

				if(nPos > 90)
					break;

				pDlg->m_dlgInfo.SetProgressPosLoad( nPos );	
				break;
			}
		default:
			int x = pDlg->m_Entity.GetProgress();
			pDlg->m_dlgInfo.SetProgressPosLoad( nPos );	
		}


		Sleep(100);
	}
	pDlg->m_dlgInfo.SetProgressMsg(L"파일 읽기 종료");
	Sleep(100);

	pDlg->m_dlgInfo.m_progressFileLoad.ShowWindow(false);

	hProcessProgressTH = NULL;
	return 1;
}

void CLnpCAMProcess::StartFileSaving(void)
{
	if(hProcessFileTH == NULL && hProcessProgressTH == NULL)
	{
		m_bStopFileProcess = false;

		hProcessFileTH = ::AfxBeginThread(ProcessSaveFile, this, THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED);
		hProcessFileTH->ResumeThread();

		if(m_strFileExt == "dxf"){
			hProcessProgressTH = ::AfxBeginThread(ProcessProgressSave, this, THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED);
			hProcessProgressTH->ResumeThread();
		}
	}
}

UINT ProcessSaveFile(LPVOID pParam)
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;
	CLnpCAMProcess *pProcess = (CLnpCAMProcess*)pParam;

	CFileDialog dlgOpen(FALSE, L"lpd", pDlg->m_strFileTitle, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		L"LPD File(*.lpd)|*.lpd|", NULL);

	if(dlgOpen.DoModal() != IDOK){
		hProcessFileTH = NULL;
		return 0;
	}

	if( pDlg->m_nFileType != 4 )
	{
		if( pDlg->m_Entity.m_b3StepMode )
			pDlg->m_Entity.m_nLayerBufferSize = pDlg->m_Entity.m_iLayerCount + 2;
		else
			pDlg->m_Entity.m_nLayerBufferSize = pDlg->m_Entity.m_iLayerCount;
	}

	if(pDlg->m_nFileType != _FILE_LPD)
	{
		pDlg->m_Entity.ApplyUserSector();
		pDlg->m_Viewer.InitInputNumMEM();
	}

	pDlg->m_Entity.m_strBaseFile = pDlg->m_strFilePath;
	pDlg->m_Entity.SaveJobData(dlgOpen.GetPathName());

	hProcessFileTH = NULL;
	return 1;
}


UINT ProcessProgressSave(LPVOID pParam)
{
	CLnpCAMDlg *pDlg = (CLnpCAMDlg*)AfxGetApp()->m_pMainWnd;
	CLnpCAMProcess *pProcess = (CLnpCAMProcess*)pParam;

	int nPos = 0;

	int nOriginCount;
	bool bFirst = true;

	pDlg->m_Entity.m_strProgressMsg = L"";
	pDlg->m_dlgInfo.SetProgressMsg(L"파일 저장 시작");
	
	pDlg->m_dlgInfo.SetProgressRangeLoad(0, 100);

	nOriginCount = pDlg->m_Entity.GetProgressTotal();

	while( !pProcess->m_bStopFileProcess )
	{
		nPos = (int) ( pDlg->m_Entity.GetProgress() * 100 / nOriginCount );

		if(bFirst && nPos > 0){
			pDlg->m_dlgInfo.m_progressFileLoad.ShowWindow(SW_SHOW);
			pDlg->m_dlgInfo.m_progressFileLoad.ShowWindow(SW_SHOW);
			bFirst = false;
		}

		pDlg->m_dlgInfo.SetProgressPosLoad( nPos );	
	
		pDlg->m_dlgInfo.SetProgressMsg(pDlg->m_Entity.GetProgressMsg());

		Sleep(100);

		if(hProcessFileTH == NULL){
			break;
		}
	}

	if(pDlg->m_nFileType == _FILE_DXF)
		pDlg->m_dlgInfo.SetProgressMsg(L"파일 저장 종료" + pDlg->m_Entity.GetProgressMsg());
	else
		pDlg->m_dlgInfo.SetProgressMsg(L"파일 저장 종료");
	pDlg->m_dlgInfo.SetProgressPosLoad( 100 );
	pDlg->m_dlgInfo.m_progressFileLoad.ShowWindow(SW_HIDE);

	hProcessProgressTH = NULL;
	return 1;
}