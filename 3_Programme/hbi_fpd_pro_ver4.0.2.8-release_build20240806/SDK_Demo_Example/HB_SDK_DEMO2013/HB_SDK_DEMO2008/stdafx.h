
// stdafx.h : Precompiled header file for standard system include files
// and project-specific headers that are used frequently but changed infrequently.

#pragma once  // Ensure the header is included only once during compilation.

#ifndef _SECURE_ATL
#define _SECURE_ATL 1  // Enable secure ATL (Active Template Library) features.
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN  // Exclude rarely-used Windows features for faster compilation.
#endif

#include "targetver.h"  // Specifies the Windows version to target.

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // Makes some CString constructors explicit.

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS  // Prevents MFC from suppressing common warnings.

#include <afxwin.h>   // Core MFC and standard components.
#include <afxext.h>   // MFC extensions.

#include <afxdisp.h>  // MFC Automation classes.

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>  // MFC support for Internet Explorer 4 Common Controls.
#endif

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>  // MFC support for Windows Common Controls.
#endif

#include <afxcontrolbars.h>  // MFC support for ribbons and control bars.

#include <afxsock.h>  // MFC socket extensions.

// -----------------------------
// Custom SDK and API Definitions
// -----------------------------

#define _DLL_EX_IM 0  // Custom macro definition (Usage depends on the project).

#include "HbiFpd.h"  // Include third-party SDK header.
#include <afxcontrolbars.h>  // Duplicate inclusion (Consider removing one instance).

// Link to external library.
#pragma comment(lib, "HBISDKApi.lib")

// Define a custom Windows message.
#define WM_USER_NOTICE_TEMPLATE_TOOL (WM_USER + 15)

// -----------------------------
// Windows Manifest Dependencies
// -----------------------------

#ifdef _UNICODE  // If compiling in Unicode mode.

#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")

#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")

#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")

#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#endif  // End of Unicode check.


