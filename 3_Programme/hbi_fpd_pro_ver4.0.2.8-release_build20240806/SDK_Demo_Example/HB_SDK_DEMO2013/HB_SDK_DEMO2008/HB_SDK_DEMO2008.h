// HB_SDK_DEMO2008.h : Main header file for the PROJECT_NAME application
// Defines the CHB_SDK_DEMO2008App class, which serves as the main application class.

#pragma once  // Ensures this header file is included only once during compilation.

#ifndef __AFXWIN_H__
#error "Include 'stdafx.h' before including this file for precompiled headers (PCH)."
#endif

#include "resource.h"  // Includes application resource definitions (icons, menus, dialogs, etc.).


// -----------------------------
// CHB_SDK_DEMO2008App Class
// -----------------------------
// The main application class, derived from CWinAppEx (MFC extended application class).
// This class manages the application's initialization, execution, and cleanup.
//
class CHB_SDK_DEMO2008App : public CWinAppEx
{
public:
	CHB_SDK_DEMO2008App();  // Constructor.

	// Overrides
public:
	virtual BOOL InitInstance();  // Called when the application starts. Initializes the instance.

	// Implementation
	DECLARE_MESSAGE_MAP()  // Declares the message map for handling Windows messages.

		virtual int ExitInstance();  // Called when the application exits. Handles cleanup tasks.
};

// Global application object, representing the running instance of the application.
extern CHB_SDK_DEMO2008App theApp;

