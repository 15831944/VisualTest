// LnpCAMDlg.h : header file
//
#include "DlgToolInfo.h"
#include "DlgInfo.h"
#include "DlgMenu.h"
#include "DlgViewer.h"
#include "DlgViewerSet.h"

//DLL
#include "FileDxf.h"
#include "FileEntity.h"
#include "FileEntityViewer.h"
#include "FileDrl.h"
#include "FileBas.h"
#include "FileGbr.h"
#include "FileLpd.h"

//Process
#include "LnpCAMProcess.h"

#define	_ENTITY_POINT	1
#define	_ENTITY_DRILL	1
#define	_ENTITY_LINE	2
#define	_ENTITY_RECT	3
#define	_ENTITY_CIRCLE	4
#define	_ENTITY_ARC		5
#define _ENTITY_ELLIPSE 6

#define _FILE_DXF		0
#define _FILE_DRL		1
#define _FILE_BAS		2
#define _FILE_GBR		3
#define _FILE_LPD		4

#pragma once

// CLnpCAMDlg dialog
class CLnpCAMDlg : public CDialogEx
{
// Construction
public:
	CLnpCAMDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_LNPCAM_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	//Classes
	CDlgToolInfo		m_dlgToolInfo;
	CDlgInfo			m_dlgInfo;
	CDlgMenu			m_dlgMenu;
	CDlgViewer			m_dlgViewer;
	CDlgViewerSet		m_dlgViewerSet;
	
	//DLL	
	//CFileViewer			m_Viewer;
	CFileEntity			m_Entity;
	CFileEntityViewer	m_Viewer;
	CFileDxf			m_Dxf;
	CFileBas			m_Bas;
	CFileDrl			m_Drl;
	CFileGbr			m_Gbr;
	CFileLpd			m_Lpd;

	//Process
	CLnpCAMProcess		m_Process;

	//Entity Value
	int m_nLayerCnt;
	CString *m_strLayerName;

	int m_nInspectionCnt;
	double m_dInspectionPosX[4], m_dInspectionPosY[4];

	//Flags
	BOOL m_bIsFileOpen;

	int m_nSelLayer;

	//File Type
	int m_nFileType;

	//Folders
	CString m_strCoordFolder;

	//FileInfo
	CString m_strFileName;
	CString m_strFilePath;
	CString m_strFileTitle;

public:
	void makeFolders();
	int WriteLpdFile();
	int ReadLpdFile();
	
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	void InitDlg(void);
	void StartProcess(void);
	void TerminateProcess(void);
	void TransformLpd2Entity();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDropFiles(HDROP hDropInfo);
};

