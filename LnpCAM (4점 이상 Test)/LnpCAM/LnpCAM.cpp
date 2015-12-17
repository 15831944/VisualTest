
// LnpCAM.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "LnpCAM.h"
#include "LnpCAMDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLnpCAMApp

BEGIN_MESSAGE_MAP(CLnpCAMApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CLnpCAMApp construction

CLnpCAMApp::CLnpCAMApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CLnpCAMApp object

CLnpCAMApp theApp;


// CLnpCAMApp initialization

BOOL CLnpCAMApp::InitInstance()
{
	/* for GDI+ */
	GdiplusStartupInput gdiplusStartupInput;
	if(::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput,NULL)!=Ok)
	{
		AfxMessageBox(_T("ERROR: Failed to initialize GDI+"));
		return FALSE;
	}

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CLnpCAMDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	
	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}



int CLnpCAMApp::ExitInstance()
{
	::GdiplusShutdown(m_gdiplusToken);

	return CWinApp::ExitInstance();
}


BOOL CLnpCAMApp::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_MOUSEWHEEL)
	{
		CPoint point;
		GetCursorPos (&point);

		HWND hWnd = ::WindowFromPoint (point);
		HWND fWnd = GetFocus();

		if(fWnd != hWnd){
			SendMessage(hWnd, pMsg->message, pMsg->wParam, pMsg->lParam);
			return TRUE;
		}
	}

	return CWinApp::PreTranslateMessage(pMsg);
}
