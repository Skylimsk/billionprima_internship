#pragma once

// -----------------------------------------
// CDetectorSettingDlg - Detector Settings Dialog
// -----------------------------------------
// This class defines the settings dialog for the detector tool.
// It is derived from `CDialogEx` and provides UI interaction methods.
//
class CDetectorSettingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDetectorSettingDlg)  // Enables dynamic creation of this dialog class.

public:
	// Constructor & Destructor
	CDetectorSettingDlg(CWnd* pParent = nullptr);  // Standard constructor with optional parent window.
	virtual ~CDetectorSettingDlg();  // Destructor.

#ifdef AFX_DESIGN_TIME
	// Dialog ID for use in resource files.
	enum { IDD = IDD_DETECTOR_SETTING };
#endif

	HICON m_hIcon;  // Handle to the dialog's icon.

protected:
	// -----------------------------
	// Data Exchange and Validation
	// -----------------------------
	// Used for exchanging data between UI controls and variables.
	virtual void DoDataExchange(CDataExchange* pDX);

	// Message map for event handling.
	DECLARE_MESSAGE_MAP()

public:
	// -----------------------------
	// Initialization & Event Handlers
	// -----------------------------
	virtual BOOL OnInitDialog();  // Called when the dialog is initialized.
	afx_msg void OnBnClickedBtnSaveSetting();  // Handler for the "Save Setting" button.

public:
	// -----------------------------
	// Helper Functions
	// -----------------------------
	void InitCtrlData();  // Initializes control values when the dialog loads.
	void SaveCfg2IniFile();  // Saves the settings to an INI configuration file.
};
