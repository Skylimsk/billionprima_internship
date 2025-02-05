// HB_SDK_DEMO2008.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "HB_SDK_DEMO2008.h"
#include "HB_SDK_DEMO2008Dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// -----------------------------
// CHB_SDK_DEMO2008App Message Map
// -----------------------------
// Maps Windows messages to their respective handlers.
BEGIN_MESSAGE_MAP(CHB_SDK_DEMO2008App, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)  // Handles the Help command.
END_MESSAGE_MAP()

// -----------------------------
// CHB_SDK_DEMO2008App Construction
// -----------------------------
// Constructor: Perform any necessary setup.
CHB_SDK_DEMO2008App::CHB_SDK_DEMO2008App()
{
	// TODO: Add application-specific construction code here.
	// Any major initialization should be done in InitInstance().
}

// The global application instance.
CHB_SDK_DEMO2008App theApp;

// -----------------------------
// CHB_SDK_DEMO2008App Initialization
// -----------------------------
// This function is called when the application starts.
BOOL CHB_SDK_DEMO2008App::InitInstance()
{
	// Required for proper visual styles on Windows XP and later.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Enable common controls for Windows UI elements.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	// Call base class initialization.
	CWinAppEx::InitInstance();

	// Initialize Windows Sockets for networking.
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);  // Show error message if socket initialization fails.
		return FALSE;  // Terminate application.
	}

	AfxEnableControlContainer();  // Enable support for embedded controls.

	// -----------------------------
	// Application Registry Key Setup
	// -----------------------------
	// This key is used to store application settings in the Windows registry.
	// Modify the string to match your company or organization name.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	// -----------------------------
	// Console Output (Optional Debugging Tool)
	// -----------------------------
	// The following code can be used to open a console window for debugging.
	// Uncomment if console output is required.

	/*
	AllocConsole();  // Open a console window.
	SetConsoleTitle(_T("DETECTOR_TOOL"));  // Set console window title.
	freopen("CONOUT$", "w", stdout);  // Redirect standard output to console.
	*/

	// -----------------------------
	// Create and Display the Main Dialog
	// -----------------------------
	CHB_SDK_DEMO2008Dlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();  // Display the modal dialog.

	if (nResponse == IDOK)
	{
		// TODO: Handle OK button click (when dialog is closed with "OK").
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Handle Cancel button click (when dialog is closed with "Cancel").
	}

	// Since the dialog has been closed, return FALSE to exit the application
	// instead of starting the application's message pump.
	return FALSE;
}

// -----------------------------
// CHB_SDK_DEMO2008App Exit Cleanup
// -----------------------------
// Called when the application exits.
int CHB_SDK_DEMO2008App::ExitInstance()
{
	// TODO: Add cleanup code here if needed.

	// -----------------------------
	// Close Console (If Used)
	// -----------------------------
	// If a console was opened for debugging, close it before exiting.
	/*
	HWND hwndFound = FindWindow(NULL, _T("DETECTOR_TOOL"));
	if (hwndFound != NULL) {
		FreeConsole();
		hwndFound = NULL;
	}
	*/

	// Call base class cleanup function.
	return CWinAppEx::ExitInstance();
}
