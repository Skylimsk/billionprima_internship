
// HB_SDK_DEMO2008.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CHB_SDK_DEMO2008App:
// See HB_SDK_DEMO2008.cpp for the implementation of this class
//

class CHB_SDK_DEMO2008App : public CWinAppEx
{
public:
	CHB_SDK_DEMO2008App();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CHB_SDK_DEMO2008App theApp;