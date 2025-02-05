// DetectorSettingDlg.cpp: Implementation file for Detector Settings Dialog.
//

#include "stdafx.h"
#include "HB_SDK_DEMO2008.h"
#include "DetectorSettingDlg.h"
#include "afxdialogex.h"
#include "HB_SDK_DEMO2008Dlg.h"

// External reference to the main application dialog.
extern CHB_SDK_DEMO2008Dlg* theDemo;

// --------------------------------------
// CDetectorSettingDlg - Implementation
// --------------------------------------
IMPLEMENT_DYNAMIC(CDetectorSettingDlg, CDialogEx)

// Constructor: Initializes the dialog and loads the application icon.
CDetectorSettingDlg::CDetectorSettingDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DETECTOR_SETTING, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);  // Set dialog icon.
}

// Destructor: Cleans up resources.
CDetectorSettingDlg::~CDetectorSettingDlg()
{
}

// Handles data exchange between UI controls and class variables.
void CDetectorSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

// --------------------------------------
// Message Map for Event Handling
// --------------------------------------
BEGIN_MESSAGE_MAP(CDetectorSettingDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_SAVE_SETTING, &CDetectorSettingDlg::OnBnClickedBtnSaveSetting)  // Save button event.
END_MESSAGE_MAP()

// --------------------------------------
// OnInitDialog - Dialog Initialization
// --------------------------------------
// Called when the dialog is created.
BOOL CDetectorSettingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set dialog icons.
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	// Initialize control data.
	InitCtrlData();

	return TRUE;  // Return TRUE unless setting focus to a control.
}

// --------------------------------------
// InitCtrlData - Initialize UI Controls
// --------------------------------------
void CDetectorSettingDlg::InitCtrlData()
{
	if (theDemo != NULL)
	{
		// Set username text field.
		SetDlgItemText(IDC_EDIT_DETECTOR_USER_NAME, theDemo->m_username);

		// Reset some internal settings.
		if (theDemo->m_bSupportDual) theDemo->m_bSupportDual = false;
		if (theDemo->m_uDefaultFpdid != 0) theDemo->m_uDefaultFpdid = 0;

		// Retrieve configuration data.
		CFPD_BASE_CFG* pCfg = NULL;
		DWORD dwAddress = 0;
		char buff[16] = { 0 };
		unsigned char* pIP = NULL;
		pCfg = theDemo->m_fpd_base;

		if (pCfg != NULL)
		{
			((CEdit*)GetDlgItem(IDC_EDIT_DETECTOR_A_ID))->SetWindowTextA("0");

			// Set communication type dropdown.
			if (pCfg->commType >= 0 && pCfg->commType <= 3)
				((CComboBox*)GetDlgItem(IDC_COMBO_COMM_TYPE_A))->SetCurSel(pCfg->commType);
			else
				((CComboBox*)GetDlgItem(IDC_COMBO_COMM_TYPE_A))->SetCurSel(0);

			// Set destination IP address.
			dwAddress = inet_addr(pCfg->DstIP);
			pIP = (unsigned char*)&dwAddress;
			((CIPAddressCtrl*)GetDlgItem(IDC_IPADDRESS_DETECTOR_IP_A))->SetAddress(*pIP, *(pIP + 1), *(pIP + 2), *(pIP + 3));

			// Set destination port.
			memset(buff, 0x00, 16);
			sprintf(buff, "%u", pCfg->DstPort);
			SetDlgItemText(IDC_EDIT_DETECTOR_PORT_A, buff);

			// Set local IP address.
			dwAddress = inet_addr(pCfg->SrcIP);
			pIP = (unsigned char*)&dwAddress;
			((CIPAddressCtrl*)GetDlgItem(IDC_IPADDRESS_LOCAL_IP_A))->SetAddress(*pIP, *(pIP + 1), *(pIP + 2), *(pIP + 3));

			// Set local port.
			memset(buff, 0x00, 16);
			sprintf(buff, "%u", pCfg->SrcPort);
			SetDlgItemText(IDC_EDIT_LOCAL_PORT_A, buff);
		}
	}
}

// --------------------------------------
// OnBnClickedBtnSaveSetting - Save Button Handler
// --------------------------------------
void CDetectorSettingDlg::OnBnClickedBtnSaveSetting()
{
	this->UpdateData(TRUE);  // Update UI values to variables.

	if (theDemo == NULL)
	{
		::AfxMessageBox("Error: theDemo is NULL!");
		return;
	}

	if (theDemo->m_IsOpen)
	{
		::AfxMessageBox("Warning: Disconnect FPD before saving settings!");
		return;
	}

	// Validate configuration storage.
	if (theDemo->m_fpdbasecfg.size() == 0)
	{
		::AfxMessageBox("Error: m_fpdbasecfg is invalid!");
		return;
	}

	// Update configuration values.
	CFPD_BASE_CFG* pCfg = NULL;
	DWORD dwAddress = 0;
	char buff[16] = { 0 };
	unsigned char* pIP = NULL;
	int j = 0;

	memset(theDemo->m_username, 0x00, 64);
	((CEdit*)GetDlgItem(IDC_EDIT_DETECTOR_USER_NAME))->GetWindowTextA(theDemo->m_username, 63);

	if (theDemo->m_bSupportDual) theDemo->m_bSupportDual = false;
	if (theDemo->m_uDefaultFpdid != 0) theDemo->m_uDefaultFpdid = 0;

	pCfg = theDemo->m_fpd_base;

	if (pCfg != NULL)
	{
		pCfg->detectorId = 0;
		pCfg->commType = ((CComboBox*)GetDlgItem(IDC_COMBO_COMM_TYPE_A))->GetCurSel();

		// Set destination IP address.
		((CIPAddressCtrl*)GetDlgItem(IDC_IPADDRESS_DETECTOR_IP_A))->GetAddress(dwAddress);
		pIP = (unsigned char*)&dwAddress;
		j = sprintf(pCfg->DstIP, "%u.%u.%u.%u", *(pIP + 3), *(pIP + 2), *(pIP + 1), *pIP);
		pCfg->DstIP[j] = '\0';
		memset(buff, 0x00, 16);

		// Set destination port.
		((CEdit*)GetDlgItem(IDC_EDIT_DETECTOR_PORT_A))->GetWindowTextA(buff, 8);
		pCfg->DstPort = atoi(buff);
		memset(buff, 0x00, 16);

		// Set local IP address.
		((CIPAddressCtrl*)GetDlgItem(IDC_IPADDRESS_LOCAL_IP_A))->GetAddress(dwAddress);
		pIP = (unsigned char*)&dwAddress;
		j = sprintf(pCfg->SrcIP, "%u.%u.%u.%u", *(pIP + 3), *(pIP + 2), *(pIP + 1), *pIP);
		pCfg->SrcIP[j] = '\0';
		memset(buff, 0x00, 16);

		// Set local port.
		((CEdit*)GetDlgItem(IDC_EDIT_LOCAL_PORT_A))->GetWindowTextA(buff, 8);
		pCfg->SrcPort = atoi(buff);
	}

	// Update main window title with username.
	size_t len = strlen(theDemo->m_username);
	if (len > 0)
	{
		theDemo->SetWindowTextA(theDemo->m_username);
	}
	else
	{
		theDemo->SetWindowTextA(_T("shanghaihaobo"));
	}

	// Save settings to file.
	SaveCfg2IniFile();
}

// --------------------------------------
// SaveCfg2IniFile - Save Configuration to INI File
// --------------------------------------
void CDetectorSettingDlg::SaveCfg2IniFile()
{
	if (theDemo == NULL)
	{
		::AfxMessageBox("Error: theDemo is NULL!");
		return;
	}

	if (theDemo->m_IsOpen)
	{
		::AfxMessageBox("Warning: Disconnect FPD before saving settings!");
		return;
	}

	unsigned int usize = theDemo->m_fpdbasecfg.size();
	if (usize == 0)
	{
		::AfxMessageBox("err:m_fpdbasecfg is error!");
		return;
	}

	// Construct configuration file path.
	char szPath[MAX_PATH] = { 0 };
	sprintf(szPath, "%s\\DETECTOR_CFG.INI", theDemo->m_path);

	// Save username.
	WritePrivateProfileString("FPD_CFG", "USER_NAME", theDemo->m_username, szPath);

	char tmp[16] = { 0 };
	memset(tmp, 0x00, 16);
	int j = 0;

	// Save detector configuration.
	CFPD_BASE_CFG* pCfg = theDemo->m_fpd_base;

	if (pCfg != NULL)
	{
		// Set detector ID to 0 and save it in the INI file.
		pCfg->detectorId = 0;
		WritePrivateProfileString("DETECTOR_A", "DETECTOR_ID", "0", szPath);

		// Convert communication type to string and save it in the INI file.
		j = sprintf(tmp, "%u", pCfg->commType);
		tmp[j] = '\0';
		WritePrivateProfileString("DETECTOR_A", "COMM_TYPE", tmp, szPath);

		// Save detector IP address to the INI file.
		WritePrivateProfileString("DETECTOR_A", "DETECTOR_IP", pCfg->DstIP, szPath);

		// Convert detector port to string and save it in the INI file.
		j = sprintf(tmp, "%u", pCfg->DstPort);
		tmp[j] = '\0';
		WritePrivateProfileString("DETECTOR_A", "DETECTOR_PORT", tmp, szPath);

		// Save local IP address to the INI file.
		WritePrivateProfileString("DETECTOR_A", "LOCAL_IP", pCfg->SrcIP, szPath);

		// Convert local port to string and save it in the INI file.
		j = sprintf(tmp, "%u", pCfg->SrcPort);
		tmp[j] = '\0';
		WritePrivateProfileString("DETECTOR_A", "LOCAL_PORT", tmp, szPath);
	}

	// Show a success message once all settings are saved.
	::AfxMessageBox("Save success!");

}

