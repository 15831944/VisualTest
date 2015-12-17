
// LnpCAM.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CLnpCAMApp:
// See LnpCAM.cpp for the implementation of this class
//

class CLnpCAMApp : public CWinApp
{
public:
	CLnpCAMApp();

	ULONG_PTR m_gdiplusToken;

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

extern CLnpCAMApp theApp;