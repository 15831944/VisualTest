#pragma once
#include "afx.h"
class CLnpCAMProcess :
	public CObject
{
public:
	CLnpCAMProcess(void);
	~CLnpCAMProcess(void);

	//Stop Flags
	BOOL m_bStopDisplayCoord;

	void EndProcess(void);
	void StartDisplayCoord(void);

	bool m_bStopFileProcess;
	void StopFileProcessing(void);

	void StartFileLoading(void);
	void StartFileSaving(void);

	int m_nProgressPos;
	int m_nOriginCnt;
	CString m_strFileExt;

};

static CWinThread* hProcessFileTH;
UINT ProcessLoadFile(LPVOID pParam);
UINT ProcessSaveFile(LPVOID pParam);

static CWinThread* hProcessProgressTH;
UINT ProcessProgressLoad(LPVOID pParam);
UINT ProcessProgressSave(LPVOID pParam);

static CWinThread* hProcessDisplayCoordTH;
UINT DisplayCoord(LPVOID pParam);
