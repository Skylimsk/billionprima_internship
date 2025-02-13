/**
 * @file HB_SDK_DEMO2008Dlg.cpp
 * @brief Implementation file for the main dialog and about dialog
 */

#include "stdafx.h"
#include "HB_SDK_DEMO2008.h"
#include "HB_SDK_DEMO2008Dlg.h"
#include "CDlgBinningType.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Global pointer to main dialog instance for callback access
CHB_SDK_DEMO2008Dlg* theDemo = NULL;

/**
 * @class CAboutDlg
 * @brief Dialog class for displaying application information and version details
 * 
 * This dialog shows information about the application including:
 * - Firmware version
 * - SDK version
 * - FPD (Flat Panel Detector) serial number
 */
class CAboutDlg : public CDialog
{
public:
    // Constructor
    CAboutDlg();

    // Dialog Data
    enum { IDD = IDD_ABOUTBOX };

protected:
    //Exchanges data between dialog controls and member variables
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Message map declaration
    DECLARE_MESSAGE_MAP()

public:
    //Initializes dialog when it's first created
    virtual BOOL OnInitDialog();

    //Handles custom control coloring
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

    //Button handler to retrieve and display firmware version
    afx_msg void OnBnClickedButtonGetFirmwareVer();

    //Button handler to retrieve and display SDK version
    afx_msg void OnBnClickedButtonGetSdkVer();

    //Button handler to retrieve and display FPD serial number
    afx_msg void OnBnClickedButtonGetFpdSn();

public:
    //Updates the displayed firmware version information
    void updateFirmwareVersionInfo();
};

//Constructor for About dialog
CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

// Handles data exchange between dialog controls and member variables using MFC's DDX/DDV mechanism
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_GET_FIRMWARE_VER, &CAboutDlg::OnBnClickedButtonGetFirmwareVer)
	ON_BN_CLICKED(IDC_BUTTON_GET_SDK_VER, &CAboutDlg::OnBnClickedButtonGetSdkVer)
	ON_BN_CLICKED(IDC_BUTTON_GET_FPD_SN, &CAboutDlg::OnBnClickedButtonGetFpdSn)
END_MESSAGE_MAP()

// Customizes appearance of dialog controls by setting transparent background and blue text for static controls
HBRUSH CAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	if (nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(0, 0, 255));
	}
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

// Initializes dialog controls and updates firmware version information when first created
BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	////if (strlen(theDemo->m_username) > 0)
	////{
	////	CString strTitle = _T("");
	////	strTitle.Format(_T("About %s"), theDemo->m_username);
	////	this->SetWindowTextA(strTitle);
	////}

	// 更新固件版本信息
	updateFirmwareVersionInfo();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

// Updates all version information fields in the About dialog with current firmware, SDK and serial number values
void CAboutDlg::updateFirmwareVersionInfo()
{
	if (theDemo != NULL)
	{
		// Validate FPD handle
		if (theDemo->m_pFpd == NULL)
		{
			::AfxMessageBox("m_pFpd is NULL!");
			return;
		}

		// Buffer for version information
		char buff[128];
		memset(buff, 0x00, 128);

		// Get and display SDK version
		int ret = HBI_GetSDKVerion(theDemo->m_pFpd, buff);
		if (0 == ret)
		{
			((CStatic *)GetDlgItem(IDC_STATIC_SDK_VERSION))->SetWindowTextA(buff);
		}

		// Return if detector is not connected
		if (!theDemo->m_IsOpen)
		{
			return;
		}

		// Get and display firmware version
		ret = HBI_GetFirmareVerion(theDemo->m_pFpd, buff);
		if (0 == ret)
		{
			((CStatic *)GetDlgItem(IDC_STATIC_FIRMWARE_VERSION))->SetWindowTextA(buff);
		}

		// Get and display serial number
		ret = HBI_GetFPDSerialNumber(theDemo->m_pFpd, buff);
		if (0 == ret)
		{
			((CStatic *)GetDlgItem(IDC_STATIC_SERIAL_NUMBER))->SetWindowTextA(buff);
		}
	}
}

// Retrieves and displays the current firmware version in response to button click
void CAboutDlg::OnBnClickedButtonGetFirmwareVer()
{
	// TODO: 在此添加控件通知处理程序代码
	if (theDemo != NULL)
	{
		// Validate FPD handle
		if (theDemo->m_pFpd == NULL) 
		{
			::AfxMessageBox("m_pFpd is NULL!");
			return;
		}

		// Get firmware version
		int ret = HBI_GetFirmareVerion(theDemo->m_pFpd, theDemo->szFirmVer);
		if (0 != ret) 
		{
			CString strStr = _T("");
			strStr.Format(_T("HBI_GetFirmareVerion Err,ret=%d"), ret);
			::AfxMessageBox(strStr);
			return;
		}

		// Display firmware version
		((CStatic *)GetDlgItem(IDC_STATIC_FIRMWARE_VERSION))->SetWindowTextA(theDemo->szFirmVer);
	}
}

// Retrieves and displays the current SDK version in response to button click
void CAboutDlg::OnBnClickedButtonGetSdkVer()
{
	if (theDemo != NULL)
	{
		// Validate FPD handle
		if (theDemo->m_pFpd == NULL) {
			::AfxMessageBox("m_pFpd is NULL!");
			return;
		}

		// Get SDK version
		int ret = HBI_GetSDKVerion(theDemo->m_pFpd, theDemo->szSdkVer);
		if (0 != ret)
		{
			CString strStr = _T("");
			strStr.Format(_T("HBI_GetSDKVerion Err,ret=%d"), ret);
			::AfxMessageBox(strStr);
			return;
		}

		// Display SDK version
		((CStatic*)GetDlgItem(IDC_STATIC_SDK_VERSION))->SetWindowTextA(theDemo->szFirmVer);
	}
}

// Retrieves and displays the detector serial number in response to button click
void CAboutDlg::OnBnClickedButtonGetFpdSn()
{
	// TODO: 在此添加控件通知处理程序代码
	if (theDemo != NULL)
	{
		// Validate FPD handle
		if (theDemo->m_pFpd == NULL) {
			::AfxMessageBox("m_pFpd is NULL!");
			return;
		}

		memset(theDemo->szSeiralNum, 0x00, 16);

		// Get SDK version
		int ret = HBI_GetFPDSerialNumber(theDemo->m_pFpd, theDemo->szSeiralNum);
		if (0 != ret)
		{
			CString strStr = _T("");
			strStr.Format(_T("HBI_GetFPDSerialNumber Err,ret=%d"), ret);
			::AfxMessageBox(strStr);
			return;
		}

		//Display SDK Version
		((CStatic *)GetDlgItem(IDC_STATIC_SERIAL_NUMBER))->SetWindowTextA(theDemo->szFirmVer);
	}
}

// Initializes the main dialog with default settings, creates events and starts the show thread
CHB_SDK_DEMO2008Dlg::CHB_SDK_DEMO2008Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHB_SDK_DEMO2008Dlg::IDD, pParent)
	, m_uFrameNum(0)
	, m_uDiscardNum(0)
	, m_uMaxFps(1)
	, m_uPrepareTime(0)
	, m_uLiveTime(0)
	, m_upacketInterval(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// Set global pointer to this dialog instance
	theDemo = this;

	// Initialize detector handles and config pointers
	m_pFpd  = NULL;
	m_pLastRegCfg = NULL;
	m_fpd_base = NULL;

	// Set default image dimensions
	m_imgWA = m_imgHA = 3072;
	m_ufpsA = 0;
	pIplimageA = NULL;

	// Initialize detector status and config
	m_nDetectorAStatus = FPD_DISCONN_STATUS;
	m_bSupportDual = false;
	m_uDefaultFpdid = 0;
	memset(m_username, 0x00, 64);

	init_base_cfg();

	// Initialize display parameters
	picA_factor = 1.0;
	m_bShowPic = true;
	pPicWndA = NULL;

	// Create event handle for synchronization
	m_hEventA = NULL;
	m_hEventA = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hEventA == NULL)
	{
		printf("err:%s", "CreateEventA failed!");
	}

	// Start display thread in suspended state
	RunFlag = 1;
	m_ThreadhdlA = (HANDLE)_beginthreadex(NULL, 0, &ShowThreadProcFunA, (void *)this, CREATE_SUSPENDED, &m_uThreadFunIDA);
	
	// Initialize connection status
	m_IsOpen = false;
	downloadfile = NULL;
	m_bIsDetectA = true;

	// Get system dialog background color
	COLORREF dd = GetSysColor(COLOR_3DFACE);
	m_r = GetRValue(dd);
	m_g = GetGValue(dd);
	m_b = GetBValue(dd);

	// Initialize SDK and register callback
	CFPD_BASE_CFG *pBaseCfg = get_base_cfg_ptr(m_uDefaultFpdid);
	if (pBaseCfg != NULL)
	{
		m_fpd_base = pBaseCfg;
		m_fpd_base->m_pFpdHand = HBI_Init(0);
	}
	m_pFpd = m_fpd_base->m_pFpdHand;

	// Register event callback
	if (m_pFpd != NULL)
	{
		HBI_RegEventCallBackFun(m_pFpd, handleCommandEventA, (void *)this);
	}
}

// Cleans up resources and handles when dialog is destroyed
CHB_SDK_DEMO2008Dlg::~CHB_SDK_DEMO2008Dlg()
{
	// Release DLL resources
	if (m_pFpd != NULL)
	{
		HBI_Destroy(m_pFpd);
	}
	//HBI_DestroyEx();

	// Close display thread
	CloseShowThread();

	// Free image buffer
	if (pIplimageA != NULL) 
	{
		cvReleaseImage(&pIplimageA);
		pIplimageA = NULL;
	}

	// Free download file buffer
	if (downloadfile != NULL)
	{
		delete downloadfile;
		downloadfile = NULL;
	}

	// Release base configuration
	free_base_cfg();
	m_pFpd = NULL;
	m_IsOpen = false;
	m_pLastRegCfg = NULL;
	m_fpd_base = NULL;

	////if (m_hBrush != NULL)
	////	DeleteObject(m_hBrush);
	////m_hBrush = NULL;
}

// Maps dialog control values to member variables and vice versa using DDX mechanism
void CHB_SDK_DEMO2008Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	// Map spin controls
	DDX_Control(pDX, IDC_SPIN_MAX_FRAME_SUM, m_ctlSpinAqcSum);
	DDX_Control(pDX, IDC_SPIN_DISCARD_FRAME_NUM, m_ctlSpinDiscardNum);
	DDX_Control(pDX, IDC_SPIN_MAX_FPS, m_ctlSpinMaxFps);

	// Map edit control values to member variables
	DDX_Text(pDX, IDC_EDIT_MAX_FRAME_SUM, m_uFrameNum);
	DDX_Text(pDX, IDC_EDIT_DISCARD_FRAME_NUM, m_uDiscardNum);
	DDX_Text(pDX, IDC_EDIT_MAX_FPS, m_uMaxFps);

	// Map connection status icon control
	DDX_Control(pDX, IDI_ICON_DISCONNECT, m_ctlConnStatus);

	// Map timing parameters
	DDX_Text(pDX, IDC_EDIT_PREPARE_TIME, m_uPrepareTime);
	DDX_Text(pDX, IDC_EDIT_LIVE_ACQ_TIME, m_uLiveTime);
	DDX_Text(pDX, IDC_EDIT_PACKET_INTERVAL_TIME, m_upacketInterval);
}

// Maps Windows messages and control notifications to handler functions
BEGIN_MESSAGE_MAP(CHB_SDK_DEMO2008Dlg, CDialog)
	// System messages
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	// Application messages
	ON_COMMAND(ID_APP_ABOUT, &CHB_SDK_DEMO2008Dlg::OnAppAbout)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_USER_CURR_CONTROL_DATA, &CHB_SDK_DEMO2008Dlg::OnUpdateCurControlData)

	// Menu commands
	ON_COMMAND(ID_FILE_GENERATETEMPLATE, &CHB_SDK_DEMO2008Dlg::OnFileTemplate)
	ON_COMMAND(ID_FILE_SETTING, &CHB_SDK_DEMO2008Dlg::OnFileDetectorSetting)

	// Connection control buttons
	ON_BN_CLICKED(IDC_BTN_CONN, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnConn)
	ON_BN_CLICKED(IDC_BTN_DISCONN, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnDisconn)
	ON_BN_CLICKED(IDC_BTN_LIVE_ACQ, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnLiveAcq)
	ON_BN_CLICKED(IDC_BTN_STOP_LIVE_ACQ, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnStopLiveAcq)

	// Display control buttons
	ON_BN_CLICKED(IDC_RADIO_SHOW_PIC, &CHB_SDK_DEMO2008Dlg::OnBnClickedRadioShowPic)
	ON_BN_CLICKED(IDC_RADIO_SAVE_PIC, &CHB_SDK_DEMO2008Dlg::OnBnClickedRadioSavePic)
	ON_BN_CLICKED(IDC_BUTTON_SINGLE_SHOT, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSingleShot)

	// Version information buttons
	ON_BN_CLICKED(IDC_BUTTON_FIRMWARE_VER, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonFirmwareVer)
	ON_BN_CLICKED(IDC_BUTTON_SOFTWARE_VER, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSoftwareVer)

	// Configuration and property buttons
	ON_BN_CLICKED(IDC_BTN_GET_IMAGE_PROPERTY, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnGetImageProperty)
	ON_BN_CLICKED(IDC_BUTTON_GET_CONFIG, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonGetConfig)
	ON_BN_CLICKED(IDC_BTN_SET_TRIGGER_CORRECTION, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnSetTriggerCorrection)
	ON_BN_CLICKED(IDC_BTN_SET_TRIGGER_MODE, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnSetTriggerMode)
	ON_BN_CLICKED(IDC_BTN_FIRMWARE_CORRECT_ENABLE, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnCorrectEnable)

	// Gain control buttons
	ON_BN_CLICKED(IDC_BUTTON_SET_GAIN_MODE, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSetGainMode)
	ON_BN_CLICKED(IDC_BUTTON_GET_GAIN_MODE, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonGetGainMode)
	ON_BN_CLICKED(IDC_BUTTON_SET_PGA_LEVEL, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonGetLiveAcqTime)

	// Acquisition and timing control buttons
	ON_BN_CLICKED(IDC_BUTTON_SET_BINNING, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSetBinning)
	ON_BN_CLICKED(IDC_BUTTON_SET_LIVE_ACQUISITION_TM, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSetLiveAcquisitionTm)
	ON_BN_CLICKED(IDC_BUTTON_GET_SERIAL_NUMBER, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonLiveAcquisitionTm)
	ON_EN_CHANGE(IDC_EDIT_MAX_FPS, &CHB_SDK_DEMO2008Dlg::OnEnChangeEditMaxFps)

	// Template and configuration buttons
	ON_BN_CLICKED(IDC_BTN_DOWNLOAD_TEMPLATE, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnDownloadTemplate)
	ON_BN_CLICKED(IDC_BUTTON_SET_PREPARE_TM, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSetPrepareTm)
	ON_BN_CLICKED(IDC_BUTTON_GET_PREPARE_TM, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonGetPrepareTm)
	ON_BN_CLICKED(IDC_BUTTON_GET_BINNING, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonGetBinning)
	ON_BN_CLICKED(IDC_BUTTON_SINGLE_PREPARE, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSinglePrepare)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_CONFIG, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonOpenConfig)

	// Status and callback messages
	ON_MESSAGE(WM_DETECTORA_CONNECT_STATUS, &CHB_SDK_DEMO2008Dlg::OnUpdateDetectorAStatus)
	ON_MESSAGE(WM_DOWNLOAD_TEMPLATE_CB_MSG, &CHB_SDK_DEMO2008Dlg::OnDownloadTemplateCBMessage)

	// FPS and binning update buttons
	ON_BN_CLICKED(IDC_BTN_UPDATE_TRIGGER_BINNING_FPS, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnUpdateTriggerBinningFps)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE_PGA_BINNING_FPS, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnUpdatePgaBinningFps)
	ON_BN_CLICKED(IDC_BTN_UPDATE_TRIGGER_PGA_BINNING_FPS, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnUpdateTriggerPgaBinningFps)

	// Additional menu commands
	ON_COMMAND(ID_FILE_OPEN_TEMPLATE_WIZARD, &CHB_SDK_DEMO2008Dlg::OnFileOpenTemplateWizard)
	ON_COMMAND(ID_FIRMWARE_UPDATE_TOOL, &CHB_SDK_DEMO2008Dlg::OnFirmwareUpdateTool)

	// Packet interval control buttons
	ON_BN_CLICKED(IDC_BTN_UPDATE_PACKET_INTERVAL_TIME, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnUpdatePacketIntervalTime)
	ON_BN_CLICKED(IDC_BTN_GET_PACKET_INTERVAL_TIME, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnGetPacketIntervalTime)

END_MESSAGE_MAP()

// Initializes dialog controls and user interface when dialog is first created
BOOL CHB_SDK_DEMO2008Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set dialog icons
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	InitCtrlData(); 

	// Get detector group box area
	GetDlgItem(IDC_STATIC_DETECTOR_A)->GetWindowRect(m_rectA);
	ScreenToClient(m_rectA);
	//GetDlgItem(IDC_STATIC_DETECTOR_B)->GetWindowRect(m_rectB);
	//ScreenToClient(m_rectB);
	m_rectA.left   -= 2;
//	m_rectA.top -= 2;
	m_rectA.right  += 2;
	m_rectA.bottom += 2;

	////获取系统默认背景颜色
	//COLORREF dd = GetSysColor(COLOR_3DFACE);
	//m_r = GetRValue(dd);
	//m_g = GetGValue(dd);
	//m_b = GetBValue(dd);
	
	// Get current module path
	memset(m_path, 0x00, MAX_PATH);
	GetModuleFileName(NULL, m_path, MAX_PATH);

	// Enable show picture radio button
	((CButton *)GetDlgItem(IDC_RADIO_SHOW_PIC))->SetCheck(TRUE);

	// Get executable directory
	PathRemoveFileSpec(m_path);

	// Read configuration file
	if (read_ini_cfg())
	{
		printf("err:read_ini_cfg failed!\n");
	}

	// Initialize FPD base configuration 
	if (m_fpd_base != NULL)
	{
		m_pLastRegCfg = m_fpd_base->m_pRegCfg;

		// Initialize control values from base config
		char buff[32] = { 0 };
		memset(buff, 0x00, 32);
		((CComboBox *)GetDlgItem(IDC_COMBO_TRIGGER_MODE))->SetCurSel(m_fpd_base->trigger_mode);
		((CComboBox *)GetDlgItem(IDC_COMBO_ENABLE_OFFSET))->SetCurSel(m_fpd_base->offset_enable);
		((CComboBox *)GetDlgItem(IDC_COMBO_ENABLE_GAIN))->SetCurSel(m_fpd_base->gain_enable);
		((CComboBox *)GetDlgItem(IDC_COMBO_ENABLE_DEFECT))->SetCurSel(m_fpd_base->defect_enable);
		((CEdit *)GetDlgItem(IDC_EDIT_MAX_FRAME_SUM))->SetWindowTextA(_T("0"));
		((CEdit *)GetDlgItem(IDC_EDIT_DISCARD_FRAME_NUM))->SetWindowTextA(_T("0"));
		((CComboBox *)GetDlgItem(IDC_COMBO_PGA_LEVEL))->SetCurSel(6);
		((CComboBox *)GetDlgItem(IDC_COMBO_BINNING))->SetCurSel(1);
		m_fpd_base->PRINT_FPD_INFO();
	}
	this->UpdateData(FALSE);

	// Get display area dimensions
	pPicWndA = GetDlgItem(IDC_STATIC_PICTURE_ZONE);
	if (pPicWndA != NULL)
	{
		pPicWndA->GetClientRect(&m_PicRectA);

		picctl_width  = m_PicRectA.Width();
		picctl_height = m_PicRectA.Height();
	}

	// Start display thread
	if (m_ThreadhdlA != NULL)  
	{
		ResumeThread(m_ThreadhdlA);
	}

	else 
	{
		printf("start ShowThreadProcFunA failed!\n");
	}
	return TRUE;  
}

// Handles system commands, including About dialog display
void CHB_SDK_DEMO2008Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

/*
If you add a minimize button to your dialog, you will need the code below
to draw the icon.  For MFC applications using the document/view model,
this is automatically done for you by the framework.

*/

// Handles dialog painting, including minimized icon and detector area border
void CHB_SDK_DEMO2008Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center and draw minimized icon
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}

	else
	{
	#if 1
		// Draw detector area border
		CPaintDC dc(this); // device context for painting
		CPen pen1(PS_SOLID, 1, RGB(0, 0, 255));
		CPen pen2(PS_SOLID, 1, RGB(m_r, m_g, m_b));
		CPen *pOldPen = dc.SelectObject(&pen1);
		dc.Rectangle(&m_rectA);
		dc.SelectObject(pOldPen);
	#endif
		CDialog::OnPaint();
	}
}

// Returns cursor icon when user drags minimized window
HCURSOR CHB_SDK_DEMO2008Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// Handles custom coloring for dialog controls based on detector state
HBRUSH CHB_SDK_DEMO2008Dlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	//static CBrush brh(RGB(255, 0, 0));

	// Set detector A colors based on connection state
	if (nCtlColor == CTLCOLOR_STATIC)
	{
		if (pWnd->GetDlgCtrlID() == IDC_STATIC_DETECTOR_A)
		{
			if (m_bIsDetectA)
			{
				pDC->SetBkColor(RGB(0, 0, 0));
				pDC->SetTextColor(RGB(0, 255, 0));
			}

			else
			{
				pDC->SetBkColor(RGB(m_r, m_g, m_b));
				pDC->SetTextColor(RGB(0, 0, 0));
			}	
		}

		//else if (pWnd->GetDlgCtrlID() == IDC_STATIC_DETECTOR_B)
		//{
		//	if (m_bIsDetectA)
		//	{
		//		pDC->SetBkColor(RGB(m_r, m_g, m_b));
		//		pDC->SetTextColor(RGB(0, 0, 0));
		//	}
		//	else
		//	{
		//		pDC->SetBkColor(RGB(0, 0, 0));
		//		pDC->SetTextColor(RGB(0, 255, 0));
		//	}		
		//}

		else
		{
			// Set colors for group boxes
			if ((pWnd->GetDlgCtrlID() == IDC_GROUP_BOX_CONN) ||
				(pWnd->GetDlgCtrlID() == IDC_GROUP_BOX_CORRECTION) ||
				(pWnd->GetDlgCtrlID() == IDC_GROUP_BOX_BINNING) ||
				(pWnd->GetDlgCtrlID() == IDC_GROUP_BOX_LIVE_ACQ) ||
				(pWnd->GetDlgCtrlID() == IDC_GROUP_BOX_SIGLE_ACQ) ||
				(pWnd->GetDlgCtrlID() == IDC_GROUP_BOX_CUSTOM))
			{
				pDC->SetBkColor(RGB(0, 0, 0));
				pDC->SetTextColor(RGB(0, 255, 0));
			}

			else
			{
				//pDC->SetBkMode(TRANSPARENT);
				pDC->SetTextColor(RGB(0, 0, 255));
			}
		}
	}

	// Set colors for radio buttons
	if (pWnd->GetDlgCtrlID() == IDC_RADIO_SHOW_PIC || pWnd->GetDlgCtrlID() == IDC_RADIO_SAVE_PIC)
	{
		pDC->SetBkColor(RGB(0, 0, 0));
		pDC->SetTextColor(RGB(0, 255, 0));
	}

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}

// Shows the About dialog when menu item is selected 
void CHB_SDK_DEMO2008Dlg::OnAppAbout()
{
	// TODO: Add your command handler code here
	CAboutDlg AboutDlg;
	AboutDlg.DoModal();
}

// Shows the detector settings dialog
void CHB_SDK_DEMO2008Dlg::OnFileDetectorSetting()
{
	// TODO: 在此添加命令处理程序代码
	CDetectorSettingDlg dlg;
	dlg.DoModal();
}

// Shows the template tool dialog if detector is connected
void CHB_SDK_DEMO2008Dlg::OnFileTemplate()
{
	// TODO: 在此添加命令处理程序代码
	if (m_pFpd == NULL) {
		::AfxMessageBox(_T("HBI_Init failed!"));
		return;
	}


	if (!m_IsOpen)
	{
		::AfxMessageBox(_T("warning:fpd is disconnect!"));
		return;
	}


	CTemplateTool dlg;
	dlg.DoModal();
}

// Updates acquisition time when max FPS value changes
void CHB_SDK_DEMO2008Dlg::OnEnChangeEditMaxFps()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	if (((CEdit *)GetDlgItem(IDC_EDIT_LIVE_ACQ_TIME))->GetSafeHwnd() != NULL) // important ???
	{
		this->UpdateData(TRUE);
		if (m_uMaxFps <= 0) m_uMaxFps = 1;
		m_uLiveTime = 1000 / m_uMaxFps;
		this->UpdateData(FALSE);
	}
}

// Initializes all dialog controls and loads status icons 
void CHB_SDK_DEMO2008Dlg::InitCtrlData()
{
	// Initialize offset template combo box
	((CComboBox *)GetDlgItem(IDC_COMBO_CONN_OFFSETTEMP))->SetCurSel(0);

	// Configure acquisition frame spin control
	m_ctlSpinAqcSum.SetRange(0, 3000);
	m_ctlSpinAqcSum.SetPos(0);
	m_ctlSpinAqcSum.SetBase(1);
	m_ctlSpinAqcSum.SetBuddy(GetDlgItem(IDC_EDIT_MAX_FRAME_SUM));

	// Configure discard frame spin control
	m_ctlSpinDiscardNum.SetRange(0, 10);
	m_ctlSpinDiscardNum.SetPos(0);
	m_ctlSpinDiscardNum.SetBase(1);
	m_ctlSpinDiscardNum.SetBuddy(GetDlgItem(IDC_EDIT_DISCARD_FRAME_NUM));

	// Configure max FPS spin control
	m_ctlSpinMaxFps.SetRange(1, 30);
	m_ctlSpinMaxFps.SetPos(0);
	m_ctlSpinMaxFps.SetBase(1);
	m_ctlSpinMaxFps.SetBuddy(GetDlgItem(IDC_EDIT_MAX_FPS));

	// Set acquisition mode
	((CComboBox *)GetDlgItem(IDC_COMBO_LIVE_ACQ_MODE))->SetCurSel(1);

	// Load all status icons
	m_hConnIco    = ::AfxGetApp()->LoadIconA(IDI_ICON_CONNECT);
	m_hDisConnIco = ::AfxGetApp()->LoadIconA(IDI_ICON_DISCONNECT);
	m_hReadyIco   = ::AfxGetApp()->LoadIconA(IDI_ICON_READY);
	m_hbusyIco    = ::AfxGetApp()->LoadIconA(IDI_ICON_BUSY);
	m_hprepareIco = ::AfxGetApp()->LoadIconA(IDI_ICON_PREPARE);
	m_hExposeIco     = ::AfxGetApp()->LoadIconA(IDI_ICON_EXPOSE);
	m_hOffsetIco     = ::AfxGetApp()->LoadIconA(IDI_ICON_CONTINUE_OFFSET);
	m_hConReadyIco   = ::AfxGetApp()->LoadIconA(IDI_ICON_CONTINUE_READY);
	m_hGainAckIco    = ::AfxGetApp()->LoadIconA(IDI_ICON_GAIN_ACK);
	m_hDefectAckIco  = ::AfxGetApp()->LoadIconA(IDI_ICON_DEFECT_ACK);
	m_hOffsetAckIco  = ::AfxGetApp()->LoadIconA(IDI_ICON_OFFSET_ACK);
	m_hUpdateFirmIco = ::AfxGetApp()->LoadIconA(IDI_ICON_UPDATE_FIRMWARE);
	m_hRetransIco    = ::AfxGetApp()->LoadIconA(IDI_ICON_RETRANS_MISS);
}

// Converts raw PGA register value to gain level (1-7)
int CHB_SDK_DEMO2008Dlg::GetPGA(unsigned short usValue)
{
	// Swap bytes (convert from little-endian to big-endian)
	unsigned short gainMode = ((usValue & 0xff) << 8) | ((usValue >> 8) & 0xff);

	// Extract bits [15:10] to get the PGA gain code
	int nPGA = (gainMode >> 10) & 0x3f;

	// Map PGA gain code to corresponding gain level
	if (nPGA == 0x02) return 1;			// Gain level 1 (0.6pC)
	else if (nPGA == 0x04) return 2;	// Gain level 2 (1.2pC)
	else if (nPGA == 0x08) return 3;	// Gain level 3 (2.4pC)
	else if (nPGA == 0x0c) return 4;	// Gain level 4 (3.6pC)
	else if (nPGA == 0x10) return 5;	// Gain level 5 (4.8pC)
	else if (nPGA == 0x18) return 6;	// Gain level 6 (7.2pC)
	else if (nPGA == 0x3e) return 7;	// Gain level 7 (9.6pC)
	else return 0;						// Invalid gain code
}

// Calculates scale factor for display window based on image and control dimensions 
void CHB_SDK_DEMO2008Dlg::AutoResize(int type)
{
	if (type == 0)
	{
		if ((picctl_width > m_imgWA) && (picctl_height > m_imgHA))
		{
			picA_factor = 1.0;
		}
		else
		{
			int iTemp = (int)((double)((double)picctl_width / (double)picctl_height) * 100);   //刷图区域的长宽比
			int iTempBK = (int)((double)((double)m_imgWA / (double)m_imgHA) * 100); //图片的长宽比
			if (iTemp < iTempBK)
				picA_factor = (double)picctl_width / (double)m_imgWA;
			else //设置新高度信息刷图	
				picA_factor = (double)picctl_height / (double)m_imgHA;
		}
	}
	else
	{}
}

// Updates UI elements based on detector status
void CHB_SDK_DEMO2008Dlg::UpdateUI(int type)
{

}

// Connects to detector device and initializes communication
void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnConn()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	int offset_tmp = ((CComboBox *)GetDlgItem(IDC_COMBO_CONN_OFFSETTEMP))->GetCurSel();
	
	// Validate configuration
	size_t nsize = m_fpdbasecfg.size();
	if (nsize == 0)
	{
		::AfxMessageBox(_T("err:m_fpdbasecfg error!"));
		return;
	}

	if (m_uDefaultFpdid != 0) m_uDefaultFpdid = 0;

	CFPD_BASE_CFG *pBaseCfg = m_fpd_base;
	if (pBaseCfg == NULL)
	{
		::AfxMessageBox(_T("err:m_fpdbasecfg error!"));
		return;
	}

	if (m_pFpd == NULL)
	{
		if (m_IsOpen) m_IsOpen = false;
		::AfxMessageBox(_T("HBI_Init failed!"));
		return;
	}

	// Configure communication parameters
	COMM_CFG commCfg;
	if (pBaseCfg->commType == 0)
		commCfg._type = FPD_COMM_TYPE::UDP_COMM_TYPE;
	else if (pBaseCfg->commType == 1)
		commCfg._type = FPD_COMM_TYPE::UDP_JUMBO_COMM_TYPE;
	else if (pBaseCfg->commType == 2)
		commCfg._type = FPD_COMM_TYPE::PCIE_COMM_TYPE;
	else
		commCfg._type = FPD_COMM_TYPE::UDP_COMM_TYPE;

	commCfg._loacalPort = pBaseCfg->SrcPort;
	commCfg._remotePort = pBaseCfg->DstPort;

	int j = sprintf(commCfg._localip, "%s", pBaseCfg->SrcIP);
	commCfg._localip[j] = '\0';

	j = sprintf(commCfg._remoteip, "%s", pBaseCfg->DstIP);
	commCfg._remoteip[j] = '\0';

	// Configure communication parameters
	int ret = HBI_ConnectDetector(m_pFpd, commCfg, offset_tmp);
	if (ret != 0)
	{
		if (m_IsOpen) m_IsOpen = false;
		::AfxMessageBox(_T("err:HBI_ConnectDetector failed!"));
		return;
	}

	conn_button_status();
	printf("HBI_ConnectDetector success!\n");
}

// Disconnects from detector device
void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnDisconn()
{
	// TODO: Add your control notification handler code here
	if (m_pFpd != NULL)
	{
		HBI_DisConnectDetector(m_pFpd);
		printf("HBI_DisConnectDetector DetectorA success!\n");
	}

	CFPD_BASE_CFG *pBaseCfg = m_fpd_base;
	if (pBaseCfg != NULL)
	{
		if (pBaseCfg->m_bOpenOfFpd) pBaseCfg->m_bOpenOfFpd = false;
	}

	disconn_button_status();
	PostMessage(WM_DETECTORA_CONNECT_STATUS, (WPARAM)FPD_DISCONN_STATUS, (LPARAM)0);
}

// Starts live image acquisition from detector
void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnLiveAcq()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	printf("OnBnClickedBtnLiveAcq!\n");
	if (m_pFpd == NULL) {
		::AfxMessageBox(_T("err:m_pFpd is NULL!"));
		return;
	}

	int nMode = ((CComboBox *)GetDlgItem(IDC_COMBO_LIVE_ACQ_MODE))->GetCurSel();
	////aqc_mode.eLivetype = PREOFFSET_IMAGE;						//Offsete Template+Only Image
	////aqc_mode.eLivetype = ONLY_IMAGE;							//Only Image
	////aqc_mode.eLivetype = ONLY_PREOFFSET;						//Only Template
	if (nMode == 0) m_aqc_mode.eLivetype = PREOFFSET_IMAGE;			// Offset Template + Image
	else if (nMode == 1) m_aqc_mode.eLivetype = ONLY_IMAGE;			// Only Image
	else if (nMode == 2) m_aqc_mode.eLivetype = ONLY_PREOFFSET;		// Only Template
	else if (nMode == 3) m_aqc_mode.eLivetype = OVERLAY_16BIT_IMG;	// 16-bit overlay
	else if (nMode == 4) m_aqc_mode.eLivetype = OVERLAY_32BIT_IMG;	// 32-bit overlay
	else 
		m_aqc_mode.eLivetype = ONLY_IMAGE;

	// Set acquisition parameters
	m_aqc_mode.eAqccmd    = LIVE_ACQ_DEFAULT_TYPE;
	m_aqc_mode.nAcqnumber = m_uFrameNum;
	m_aqc_mode.nframeid   = 0;
	m_aqc_mode.ngroupno   = 0;
	m_aqc_mode.ndiscard   = m_uDiscardNum;

	// Start acquisition
	printf("Do Live Acquisition:[cmd]=%d,[_acqMaxNum]=%d,[_discardNum]=%d,[_groupNum]=%d\n", m_aqc_mode.eAqccmd, m_aqc_mode.nAcqnumber, m_aqc_mode.ndiscard, m_aqc_mode.ngroupno);
	int ret = HBI_LiveAcquisition(m_pFpd, m_aqc_mode);

	if (ret == 0)
		printf("cmd=%d,sum=%d,num=%d,group=%d,Do Live Acquisition succss!\n", m_aqc_mode.eAqccmd, m_aqc_mode.nAcqnumber, m_aqc_mode.ndiscard, m_aqc_mode.ngroupno);
	
	else {
		printf("cmd=%d,sum=%d,num=%d,group=%d,Do Live Acquisition failed!\n", m_aqc_mode.eAqccmd, m_aqc_mode.nAcqnumber, m_aqc_mode.ndiscard, m_aqc_mode.ngroupno);
	}
}

// Stops live image acquisition
void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnStopLiveAcq()
{
	// TODO: Add your control notification handler code here
	printf("OnBnClickedBtnStopLiveAcq!\n");
	if (m_pFpd == NULL) {
		::AfxMessageBox(_T("err:m_pFpd is NULL!"));
		return;
	}

	int ret = HBI_StopAcquisition(m_pFpd);
	if (ret == 0)
		printf("Stop Live Acquisition succss!\n");

	else {
		printf("Stop Live Acquisition failed!\n");
	}
}

// Enables all control buttons when detector is connected
void CHB_SDK_DEMO2008Dlg::conn_button_status()
{
	// Disable connect button, enable disconnect and acquisition buttons
	((CButton *)GetDlgItem(IDC_BTN_CONN))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BTN_DISCONN))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BTN_LIVE_ACQ))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BTN_STOP_LIVE_ACQ))->EnableWindow(TRUE);

	// Enable all configuration and control buttons
	((CButton *)GetDlgItem(IDC_BUTTON_FIRMWARE_VER))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BTN_GET_IMAGE_PROPERTY))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BUTTON_SINGLE_SHOT))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BTN_SET_TRIGGER_MODE))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BUTTON_SET_GAIN_MODE))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BUTTON_SET_PGA_LEVEL))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BUTTON_SET_LIVE_ACQUISITION_TM))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BUTTON_GET_CONFIG))->EnableWindow(TRUE);
    ((CButton *)GetDlgItem(IDC_BTN_SET_TRIGGER_CORRECTION))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BTN_FIRMWARE_CORRECT_ENABLE))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BUTTON_GET_GAIN_MODE))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BUTTON_SET_BINNING))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BUTTON_GET_SERIAL_NUMBER))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BTN_DOWNLOAD_TEMPLATE))->EnableWindow(TRUE);

	((CButton *)GetDlgItem(IDC_BUTTON_GET_BINNING))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BUTTON_SET_PREPARE_TM))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BUTTON_GET_PREPARE_TM))->EnableWindow(TRUE);
	((CButton*)GetDlgItem(IDC_BUTTON_SINGLE_PREPARE))->EnableWindow(TRUE);

	((CButton*)GetDlgItem(IDC_BTN_UPDATE_PACKET_INTERVAL_TIME))->EnableWindow(TRUE);
	((CButton*)GetDlgItem(IDC_BTN_GET_PACKET_INTERVAL_TIME))->EnableWindow(TRUE);

	((CButton *)GetDlgItem(IDC_BTN_UPDATE_TRIGGER_BINNING_FPS))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BUTTON_UPDATE_PGA_BINNING_FPS))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BTN_UPDATE_TRIGGER_PGA_BINNING_FPS))->EnableWindow(TRUE);

	if (!m_IsOpen) m_IsOpen = true;
}

// Disables all control buttons when detector is disconnected
void CHB_SDK_DEMO2008Dlg::disconn_button_status()
{
	// Enable connect button, disable disconnect and acquisition buttons
	((CButton *)GetDlgItem(IDC_BTN_CONN))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BTN_DISCONN))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BTN_LIVE_ACQ))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BTN_STOP_LIVE_ACQ))->EnableWindow(FALSE);
	
	// Disable all configuration and control buttons
	((CButton *)GetDlgItem(IDC_BUTTON_FIRMWARE_VER))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BTN_GET_IMAGE_PROPERTY))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BUTTON_SINGLE_SHOT))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BTN_SET_TRIGGER_MODE))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BUTTON_SET_GAIN_MODE))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BUTTON_SET_PGA_LEVEL))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BUTTON_SET_LIVE_ACQUISITION_TM))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BUTTON_GET_CONFIG))->EnableWindow(FALSE);
    ((CButton *)GetDlgItem(IDC_BTN_SET_TRIGGER_CORRECTION))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BTN_FIRMWARE_CORRECT_ENABLE))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BUTTON_GET_GAIN_MODE))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BUTTON_SET_BINNING))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BUTTON_GET_SERIAL_NUMBER))->EnableWindow(FALSE);
	((CButton*)GetDlgItem(IDC_BTN_DOWNLOAD_TEMPLATE))->EnableWindow(FALSE);


	((CButton *)GetDlgItem(IDC_BUTTON_GET_BINNING))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BUTTON_SET_PREPARE_TM))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BUTTON_GET_PREPARE_TM))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BUTTON_SINGLE_PREPARE))->EnableWindow(FALSE);

	((CButton*)GetDlgItem(IDC_BTN_UPDATE_PACKET_INTERVAL_TIME))->EnableWindow(FALSE);
	((CButton*)GetDlgItem(IDC_BTN_GET_PACKET_INTERVAL_TIME))->EnableWindow(FALSE);

	((CButton *)GetDlgItem(IDC_BTN_UPDATE_TRIGGER_BINNING_FPS))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BUTTON_UPDATE_PGA_BINNING_FPS))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BTN_UPDATE_TRIGGER_PGA_BINNING_FPS))->EnableWindow(FALSE);

	if (m_IsOpen) m_IsOpen = false;
}

// Updates detector status icon based on current device state 
LRESULT CHB_SDK_DEMO2008Dlg::OnUpdateDetectorAStatus(WPARAM wparam, LPARAM lparam)
{
	int status = (int)wparam;
	// Set appropriate status icon based on detector state
	if (status == FPD_CONN_SUCCESS)
		m_ctlConnStatus.SetIcon(m_hConnIco);

	else if (status == FPD_PREPARE_STATUS)
		m_ctlConnStatus.SetIcon(m_hprepareIco);

	else if (status == FPD_READY_STATUS)
		m_ctlConnStatus.SetIcon(m_hReadyIco);

	else if (status == FPD_DOOFFSET_TEMPLATE)
		m_ctlConnStatus.SetIcon(m_hOffsetIco);

	else if (status == FPD_EXPOSE_STATUS)
		m_ctlConnStatus.SetIcon(m_hbusyIco);

	else if (status == FPD_CONTINUE_READY)
		m_ctlConnStatus.SetIcon(m_hConReadyIco);

	else if (status == FPD_DWONLOAD_GAIN)
		m_ctlConnStatus.SetIcon(m_hGainAckIco);

	else if (status == FPD_DWONLOAD_DEFECT)
		m_ctlConnStatus.SetIcon(m_hDefectAckIco);

	else if (status == FPD_DWONLOAD_OFFSET)
		m_ctlConnStatus.SetIcon(m_hOffsetAckIco);

	else if (status == FPD_UPDATE_FIRMARE)
		m_ctlConnStatus.SetIcon(m_hUpdateFirmIco);

	else if (status == FPD_RETRANS_MISS)
		m_ctlConnStatus.SetIcon(m_hRetransIco);

	else  /* if (status == FPD_STATUS_DISCONN)*/
	{
		m_ctlConnStatus.SetIcon(m_hDisConnIco);
		//if (m_uDefaultFpdid == 0)
		//	disconn_button_status();
		CFPD_BASE_CFG *pBaseCfg = theDemo->m_fpd_base;

		if (pBaseCfg != NULL)
		{
			if (pBaseCfg->m_bOpenOfFpd) pBaseCfg->m_bOpenOfFpd = false;
		}
	}

	if (m_nDetectorAStatus != status) m_nDetectorAStatus = status;

	return S_OK;
}

// Updates all dialog controls to reflect current detector configuration values
LRESULT CHB_SDK_DEMO2008Dlg::OnUpdateCurControlData(WPARAM wparam, LPARAM lparam)
{
	if (m_fpd_base != NULL)
	{
		// Update trigger mode selection
		if (m_fpd_base->trigger_mode >= 1 && m_fpd_base->trigger_mode <= 7)
			((CComboBox*)GetDlgItem(IDC_COMBO_TRIGGER_MODE))->SetCurSel(m_fpd_base->trigger_mode);
		else
			((CComboBox*)GetDlgItem(IDC_COMBO_TRIGGER_MODE))->SetCurSel(0);

		// Update offset enable selection 
		if (m_fpd_base->offset_enable >= 0 && m_fpd_base->offset_enable <= 3)
			((CComboBox*)GetDlgItem(IDC_COMBO_ENABLE_OFFSET))->SetCurSel(m_fpd_base->offset_enable);
		else
			((CComboBox*)GetDlgItem(IDC_COMBO_ENABLE_OFFSET))->SetCurSel(0);

		// Update gain enable selection
		if (m_fpd_base->gain_enable >= 0 && m_fpd_base->gain_enable <= 2)
			((CComboBox*)GetDlgItem(IDC_COMBO_ENABLE_GAIN))->SetCurSel(m_fpd_base->gain_enable);
		else
			((CComboBox*)GetDlgItem(IDC_COMBO_ENABLE_GAIN))->SetCurSel(0);

		// Update defect enable selection
		if (m_fpd_base->gain_enable >= 0 && m_fpd_base->gain_enable <= 2)
			((CComboBox*)GetDlgItem(IDC_COMBO_ENABLE_DEFECT))->SetCurSel(m_fpd_base->defect_enable);
		else
			((CComboBox*)GetDlgItem(IDC_COMBO_ENABLE_DEFECT))->SetCurSel(0);

		// Update timing values
		CString strstr = _T("");
		if (m_uPrepareTime != m_fpd_base->prepareTime) m_uPrepareTime = m_fpd_base->prepareTime;
		if (m_uLiveTime != m_fpd_base->liveAcqTime) m_uLiveTime = m_fpd_base->liveAcqTime;
		if (m_upacketInterval != m_fpd_base->packetInterval) m_upacketInterval = m_fpd_base->packetInterval;

		// Update display values
		strstr.Format(_T("%u"), m_uLiveTime);
		((CEdit*)GetDlgItem(IDC_EDIT_LIVE_ACQ_TIME))->SetWindowTextA(strstr);
		strstr.ReleaseBuffer();

		strstr.Format(_T("%u"), m_uPrepareTime);
		((CEdit*)GetDlgItem(IDC_EDIT_PREPARE_TIME))->SetWindowTextA(strstr);
		strstr.ReleaseBuffer();

		strstr.Format(_T("%u"), m_upacketInterval);
		((CEdit*)GetDlgItem(IDC_EDIT_PACKET_INTERVAL_TIME))->SetWindowTextA(strstr);
		strstr.ReleaseBuffer();

		// Calculate and update FPS
		if (m_uLiveTime <= 0) m_uLiveTime = 1000;
		unsigned int unValue = (UINT)(1000 / m_uLiveTime);
		if (unValue <= 0) unValue = 1;
		if (theDemo->m_uMaxFps != unValue) theDemo->m_uMaxFps = unValue;

		strstr.Format(_T("%u"), theDemo->m_uMaxFps);
		((CEdit*)GetDlgItem(IDC_EDIT_MAX_FPS))->SetWindowTextA(strstr);
		strstr.ReleaseBuffer();

		// Update PGA level selection
		if (m_fpd_base->nPGALevel >= 1 && m_fpd_base->nPGALevel <= 7)
			((CComboBox*)GetDlgItem(IDC_COMBO_PGA_LEVEL))->SetCurSel(m_fpd_base->nPGALevel);
		else
			((CComboBox*)GetDlgItem(IDC_COMBO_PGA_LEVEL))->SetCurSel(0);

		// Update binning selection
		if (m_fpd_base->nBinning >= 1 && m_fpd_base->nBinning <= 4)
			((CComboBox*)GetDlgItem(IDC_COMBO_BINNING))->SetCurSel(m_fpd_base->nBinning);
		else
			((CComboBox*)GetDlgItem(IDC_COMBO_BINNING))->SetCurSel(0);

	//	this->UpdateData(FALSE);
	}
	return S_OK;
}

// Updates all dialog control values to match current detector configuration state
void CHB_SDK_DEMO2008Dlg::UpdateCurControlData()
{
	if (m_fpd_base != NULL)
	{
		// Update trigger mode combo selection (valid range 1-7)
		if (m_fpd_base->trigger_mode >= 1 && m_fpd_base->trigger_mode <= 7)
			((CComboBox*)GetDlgItem(IDC_COMBO_TRIGGER_MODE))->SetCurSel(m_fpd_base->trigger_mode);
		else
			((CComboBox*)GetDlgItem(IDC_COMBO_TRIGGER_MODE))->SetCurSel(0);

		// Update offset correction mode (valid range 0-3)
		if (m_fpd_base->offset_enable >= 0 && m_fpd_base->offset_enable <= 3)
			((CComboBox*)GetDlgItem(IDC_COMBO_ENABLE_OFFSET))->SetCurSel(m_fpd_base->offset_enable);
		else
			((CComboBox*)GetDlgItem(IDC_COMBO_ENABLE_OFFSET))->SetCurSel(0);

		// Update gain correction mode (valid range 0-2)
		if (m_fpd_base->gain_enable >= 0 && m_fpd_base->gain_enable <= 2)
			((CComboBox*)GetDlgItem(IDC_COMBO_ENABLE_GAIN))->SetCurSel(m_fpd_base->gain_enable);
		else
			((CComboBox*)GetDlgItem(IDC_COMBO_ENABLE_GAIN))->SetCurSel(0);

		// Update defect correction mode (valid range 0-2)
		if (m_fpd_base->gain_enable >= 0 && m_fpd_base->gain_enable <= 2)
			((CComboBox*)GetDlgItem(IDC_COMBO_ENABLE_DEFECT))->SetCurSel(m_fpd_base->defect_enable);
		else
			((CComboBox*)GetDlgItem(IDC_COMBO_ENABLE_DEFECT))->SetCurSel(0);

		// Update timing parameters if changed
		CString strstr = _T("");
		if (m_uPrepareTime != m_fpd_base->prepareTime) m_uPrepareTime = m_fpd_base->prepareTime;
		if (m_uLiveTime != m_fpd_base->liveAcqTime) m_uLiveTime = m_fpd_base->liveAcqTime;
		if (m_upacketInterval != m_fpd_base->packetInterval) m_upacketInterval = m_fpd_base->packetInterval;

		// Update timing display values
		strstr.Format(_T("%u"), theDemo->m_uLiveTime);
		((CEdit*)GetDlgItem(IDC_EDIT_LIVE_ACQ_TIME))->SetWindowTextA(strstr);
		strstr.ReleaseBuffer();

		strstr.Format(_T("%u"), theDemo->m_uPrepareTime);
		((CEdit*)GetDlgItem(IDC_EDIT_PREPARE_TIME))->SetWindowTextA(strstr);
		strstr.ReleaseBuffer();

		strstr.Format(_T("%u"), m_upacketInterval);
		((CEdit*)GetDlgItem(IDC_EDIT_PACKET_INTERVAL_TIME))->SetWindowTextA(strstr);
		strstr.ReleaseBuffer();

		// Calculate and update FPS value
		if (m_uLiveTime <= 0) m_uLiveTime = 1000;
		unsigned int unValue = (UINT)(1000 / m_uLiveTime);
		if (unValue <= 0) unValue = 1;
		if (theDemo->m_uMaxFps != unValue) theDemo->m_uMaxFps = unValue;

		strstr.Format(_T("%u"), theDemo->m_uMaxFps);
		((CEdit*)GetDlgItem(IDC_EDIT_MAX_FPS))->SetWindowTextA(strstr);
		strstr.ReleaseBuffer();

		// Update PGA level selection (valid range 1-7)
		if (m_fpd_base->nPGALevel >= 1 && m_fpd_base->nPGALevel <= 7)
			((CComboBox*)GetDlgItem(IDC_COMBO_PGA_LEVEL))->SetCurSel(m_fpd_base->nPGALevel);
		else
			((CComboBox*)GetDlgItem(IDC_COMBO_PGA_LEVEL))->SetCurSel(0);

		// Update binning selection (valid range 1-4)
		if (m_fpd_base->nBinning >= 1 && m_fpd_base->nBinning <= 4)
			((CComboBox*)GetDlgItem(IDC_COMBO_BINNING))->SetCurSel(m_fpd_base->nBinning);
		else
			((CComboBox*)GetDlgItem(IDC_COMBO_BINNING))->SetCurSel(0);

		//	this->UpdateData(FALSE);
	}
}

// Saves raw image data to timestamped file in raw_dir folder 
int CHB_SDK_DEMO2008Dlg::SaveImage(unsigned char* imgbuff, int nbufflen, int type)
{
	// Get detector resolution dimensions
	int WIDTH = m_imgWA;
	int HEIGHT = m_imgHA;

	// Validate input buffer and expected size
	if ((imgbuff == NULL) || (nbufflen != (WIDTH * HEIGHT * 2)))
	{
		printf("Invalid image data or size!\n");
		return 0;
	}

	// Create timestamped filename
	char filename[MAX_PATH];
	memset(filename, 0x00, MAX_PATH);
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	sprintf(filename, "%s\\raw_dir\\A%4d%02d%02d%02d%02d%02d%03d.raw",
		m_path,
		sys.wYear,
		sys.wMonth,
		sys.wDay,
		sys.wHour,
		sys.wMinute,
		sys.wSecond,
		sys.wMilliseconds);

	// Save image buffer to file
	FILE* fp = fopen(filename, "wb");
	if (fp != NULL)
	{
		fwrite(imgbuff, 1, nbufflen, fp);
		fclose(fp);
		printf("SaveImage:%s success!\n", filename);
		return 1;
	}
	else
	{
		printf("SaveImage:%s failed!\n", filename);
		return 0;
	}
}

// Handles detector callbacks for command events, status updates, and image data processing
int CHB_SDK_DEMO2008Dlg::handleCommandEventA(void* _contex, int nDevId, unsigned char byteEventId, void *pvParam, int nlength, int param3, int param4)
{
	int ret = 1;
	if (theDemo == NULL) return ret;
	
	// Initialize data pointers for different event types
	ImageData *imagedata    = NULL;
	RegCfgInfo *pRegCfg     = NULL;

	// Handle configuration upload events
	if ((byteEventId == ECALLBACK_TYPE_ROM_UPLOAD) || (byteEventId == ECALLBACK_TYPE_RAM_UPLOAD) ||
		(byteEventId == ECALLBACK_TYPE_FACTORY_UPLOAD))
	{
		if (pvParam == NULL || nlength == 0)
		{
			printf("Callback parameter error!\n");
			return ret;
		}

		// Prevent parameter conflict
		if (0 != nDevId)
		{
			printf("warnning:handleCommandEventA:m_uDefaultFpdid=%u,nDevId=%d,eventid=0x%02x,nParam2=%d\n",theDemo->m_uDefaultFpdid, nDevId, byteEventId, nlength);
			return ret;
		}


		if (theDemo->m_fpd_base != NULL)
		{
			pRegCfg = theDemo->m_fpd_base->m_pRegCfg;
		}	
	}

	// Handle image data events
	else if ((byteEventId == ECALLBACK_TYPE_SINGLE_IMAGE) || (byteEventId == ECALLBACK_TYPE_MULTIPLE_IMAGE) ||
		(byteEventId == ECALLBACK_TYPE_PREVIEW_IMAGE) || (byteEventId == ECALLBACK_TYPE_OFFSET_TMP) ||
		(byteEventId == ECALLBACK_OVERLAY_16BIT_IMAGE) || (byteEventId == ECALLBACK_OVERLAY_32BIT_IMAGE))
	{
		if (pvParam == NULL || nlength == 0)
		{
			printf("Callback parameter error!\n");
			return ret;
		}

		// Prevent parameter conflict
		if (0 != nDevId)
		{
			printf("warnning:handleCommandEventA:m_uDefaultFpdid=%u,nDevId=%d,eventid=0x%02x,nParam2=%d\n", theDemo->m_uDefaultFpdid, nDevId, byteEventId, nlength);
			return ret;
		}

		imagedata = (ImageData *)pvParam;
	}
	else
	{}
	
	// Process different event types
	int status = -1;
	int j = 0;
	ret = 1;
	bool b = false;
	CHB_SDK_DEMO2008Dlg *pDlg = (CHB_SDK_DEMO2008Dlg *)_contex;

	switch (byteEventId)
	{
	case ECALLBACK_TYPE_FPD_STATUS: // Panel status: Connect/Disconnect/Ready/Busy
		printf("ECALLBACK_TYPE_FPD_STATUS,recode=%d\n", nlength);
		if (theDemo != NULL)
		{
			CString strMsg = _T("");
			if (nlength <= 0 && nlength >= -11)
			{
				//Process detector callback events
				if (nlength == 0)
					printf("ECALLBACK_TYPE_FPD_STATUS,Err:Network not connected!\n");
				else if (nlength == -1)
					printf("ECALLBACK_TYPE_FPD_STATUS,Err:Parameter error!\n");
				else if (nlength == -2)
					printf("ECALLBACK_TYPE_FPD_STATUS,Err:Description flags return failed!\n");
				else if (nlength == -3)
					printf("ECALLBACK_TYPE_FPD_STATUS,Err:Receive timeout!\n");
				else if (nlength == -4)
					printf("ECALLBACK_TYPE_FPD_STATUS,Err:Receive failed!\n");
				else if (nlength == -5)
					printf("ECALLBACK_TYPE_FPD_STATUS,Err:Port not readable!\n");
				else if (nlength == -6)
					printf("ECALLBACK_TYPE_FPD_STATUS,network card unusual!\n");
				else if (nlength == -7)
					printf("ECALLBACK_TYPE_FPD_STATUS,network card ok!\n");
				else if (nlength == -8)
					printf("ECALLBACK_TYPE_FPD_STATUS:update Firmware end!\n");
				else if (nlength == -9)
					printf("ECALLBACK_TYPE_FPD_STATUS:Optical fiber disconnected!\n");
				else if (nlength == -10)
					printf("ECALLBACK_TYPE_FPD_STATUS:read ddr failed,try restarting the PCIe driver!\n");
				else /*if (nlength == -11)*/
					printf("ECALLBACK_TYPE_FPD_STATUS:ECALLBACK_TYPE_FPD_STATUS:is not jumb!\n");
				status = (int)FPD_DISCONN_STATUS;
			}

			else if (nlength == FPD_CONN_SUCCESS) { // Connect
				printf("ECALLBACK_TYPE_FPD_STATUS,Start monitoring!\n");
				status = (int)FPD_CONN_SUCCESS;
			}

			else if (nlength == FPD_PREPARE_STATUS) { // Ready
				printf("ECALLBACK_TYPE_FPD_STATUS,ready!\n");
				status = (int)FPD_PREPARE_STATUS;
			}

			else if (nlength == FPD_READY_STATUS) { // Busy
				printf("ECALLBACK_TYPE_FPD_STATUS,busy!\n");
				status = (int)FPD_READY_STATUS;
			}

			else if (nlength == FPD_DOOFFSET_TEMPLATE) { // Prepare
				printf("ECALLBACK_TYPE_FPD_STATUS,prepare!\n");
				status = (int)FPD_DOOFFSET_TEMPLATE;
			}

			else if (nlength == FPD_EXPOSE_STATUS) { // Busy expose
				printf("ECALLBACK_TYPE_FPD_STATUS:Exposing!\n");
				status = FPD_EXPOSE_STATUS;
			}

			else if (nlength == FPD_CONTINUE_READY) { // continue ready
				printf("ECALLBACK_TYPE_FPD_STATUS:Continue ready!\n");
				status = FPD_CONTINUE_READY;
			}

			else if (nlength == FPD_DWONLOAD_GAIN) { // download gain template
				printf("ECALLBACK_TYPE_FPD_STATUS:Download gain template ack!\n");
				status = FPD_DWONLOAD_GAIN;
			}

			else if (nlength == FPD_DWONLOAD_DEFECT) { // download defect template
				printf("ECALLBACK_TYPE_FPD_STATUS:Download defect template ack!\n");
				status = FPD_DWONLOAD_DEFECT;
			}

			else if (nlength == FPD_DWONLOAD_OFFSET) { // download offset template
				printf("ECALLBACK_TYPE_FPD_STATUS:Download offset template ack!\n");
				status = FPD_DWONLOAD_OFFSET;
			}

			else if (nlength == FPD_UPDATE_FIRMARE) { // update firmware
				printf("ECALLBACK_TYPE_FPD_STATUS:Update firmware!\n");
				status = FPD_UPDATE_FIRMARE;
			}

			else if (nlength == FPD_RETRANS_MISS) { // update firmware
				printf("ECALLBACK_TYPE_FPD_STATUS:Retransmission!\n");
				status = FPD_RETRANS_MISS;
			}

			else
				printf("ECALLBACK_TYPE_FPD_STATUS,Err:Other error=%d\n", nlength);

			if (status != -1)
			{
				// Update icon 
				theDemo->PostMessage(WM_DETECTORA_CONNECT_STATUS, (WPARAM)status, (LPARAM)0);

				// Trigger disconnect message
				if (nlength <= 0 && nlength >= -10)
				{
					CFPD_BASE_CFG* pBaseCfg = theDemo->m_fpd_base;
					if (pBaseCfg != NULL)
					{
						if (pBaseCfg->m_pFpdHand != NULL)
						{
							HBI_DisConnectDetector(theDemo->m_pFpd);
							theDemo->UpdateUI();
						}
					}
				}
			}
		}
		break;

	//Process configuration and image events
	case ECALLBACK_TYPE_SET_CFG_OK:
		printf("ECALLBACK_TYPE_SET_CFG_OK:Reedback set rom param succuss!\n");
		break;

	case ECALLBACK_TYPE_ROM_UPLOAD: // Update configuration
		printf("ECALLBACK_TYPE_ROM_UPLOAD:\n");
		if (theDemo->m_fpd_base != NULL)
		{
			if (pRegCfg != NULL)
			{
				// Clear and copy new config
				memset(pRegCfg, 0x00, sizeof(RegCfgInfo));
				memcpy(pRegCfg, (unsigned char*)pvParam, sizeof(RegCfgInfo));

				printf("\tSerial Number:%s\n", pRegCfg->m_SysBaseInfo.m_cSnNumber);

				// Convert port byte order
				unsigned short usValue = ((pRegCfg->m_EtherInfo.m_sDestUDPPort & 0xff) << 8) | ((pRegCfg->m_EtherInfo.m_sDestUDPPort >> 8) & 0xff);
				printf("\tSourceIP:%d.%d.%d.%d:%u\n",
					pRegCfg->m_EtherInfo.m_byDestIP[0],
					pRegCfg->m_EtherInfo.m_byDestIP[1],
					pRegCfg->m_EtherInfo.m_byDestIP[2],
					pRegCfg->m_EtherInfo.m_byDestIP[3],
					usValue);

				usValue = ((pRegCfg->m_EtherInfo.m_sSourceUDPPort & 0xff) << 8) | ((pRegCfg->m_EtherInfo.m_sSourceUDPPort >> 8) & 0xff);
				printf("\tDestIP:%d.%d.%d.%d:%u\n",
					pRegCfg->m_EtherInfo.m_bySourceIP[0],
					pRegCfg->m_EtherInfo.m_bySourceIP[1],
					pRegCfg->m_EtherInfo.m_bySourceIP[2],
					pRegCfg->m_EtherInfo.m_bySourceIP[3],
					usValue);

				////j = sprintf(pBaseCfg->SrcIP, "%d.%d.%d.%d",
				////	pRegCfg->m_EtherInfo.m_byDestIP[0],
				////	pRegCfg->m_EtherInfo.m_byDestIP[1],
				////	pRegCfg->m_EtherInfo.m_byDestIP[2],
				////	pRegCfg->m_EtherInfo.m_byDestIP[3]);
				////pBaseCfg->SrcIP[j] = '\0';
				////j = sprintf(pBaseCfg->DstIP, "%d.%d.%d.%d",
				////	pRegCfg->m_EtherInfo.m_bySourceIP[0],
				////	pRegCfg->m_EtherInfo.m_bySourceIP[1],
				////	pRegCfg->m_EtherInfo.m_bySourceIP[2],
				////	pRegCfg->m_EtherInfo.m_bySourceIP[3]);
				////pBaseCfg->DstIP[j] = '\0';
				////// 高低位需要转换
				////usValue = ((pRegCfg->m_EtherInfo.m_sDestUDPPort & 0xff) << 8) | ((pRegCfg->m_EtherInfo.m_sDestUDPPort >> 8) & 0xff);
				////pBaseCfg->DstPort = usValue;			
				////usValue = ((pRegCfg->m_EtherInfo.m_sSourceUDPPort & 0xff) << 8) | ((pRegCfg->m_EtherInfo.m_sSourceUDPPort >> 8) & 0xff);
				////pBaseCfg->SrcPort = usValue;
				
				// Process panel size and image configuration
			else if (pRegCfg->m_SysBaseInfo.m_byPanelSize == 0x03)
				printf("\tPanelSize:0x%02x,fpd type:1613-125um\n", pRegCfg->m_SysBaseInfo.m_byPanelSize);
			
			else if (pRegCfg->m_SysBaseInfo.m_byPanelSize == 0x04)
				printf("\tPanelSize:0x%02x,fpd type:3030-140um\n", pRegCfg->m_SysBaseInfo.m_byPanelSize);
			
			else if (pRegCfg->m_SysBaseInfo.m_byPanelSize == 0x05)
				printf("\tPanelSize:0x%02x,fpd type:2530-85um\n", pRegCfg->m_SysBaseInfo.m_byPanelSize);
			
			else if (pRegCfg->m_SysBaseInfo.m_byPanelSize == 0x06)
				printf("\tPanelSize:0x%02x,fpd type:3025-140um\n", pRegCfg->m_SysBaseInfo.m_byPanelSize);
			
			else if (pRegCfg->m_SysBaseInfo.m_byPanelSize == 0x07)
				printf("\tPanelSize:0x%02x,fpd type:4343-100um\n", pRegCfg->m_SysBaseInfo.m_byPanelSize);
			
			else if (pRegCfg->m_SysBaseInfo.m_byPanelSize == 0x08)
				printf("\tPanelSize:0x%02x,fpd type:2530-75um\n", pRegCfg->m_SysBaseInfo.m_byPanelSize);
			
			else if (pRegCfg->m_SysBaseInfo.m_byPanelSize == 0x09)
				printf("\tPanelSize:0x%02x,fpd type:2121-200um\n", pRegCfg->m_SysBaseInfo.m_byPanelSize);
			
			else if (pRegCfg->m_SysBaseInfo.m_byPanelSize == 0x0a)
				printf("\tPanelSize:0x%02x,fpd type:1412-50um\n", pRegCfg->m_SysBaseInfo.m_byPanelSize);
			
			else if (pRegCfg->m_SysBaseInfo.m_byPanelSize == 0x0b)
				printf("\tPanelSize:0x%02x,fpd type:0606-50um\n", pRegCfg->m_SysBaseInfo.m_byPanelSize);
			
			else
				printf("\tPanelSize:0x%02x,invalid fpd type\n", pRegCfg->m_SysBaseInfo.m_byPanelSize);

			// Update image dimensions
			if (theDemo->m_imgWA != pRegCfg->m_SysBaseInfo.m_sImageWidth)
				theDemo->m_imgWA = pRegCfg->m_SysBaseInfo.m_sImageWidth;
			if (theDemo->m_imgHA != pRegCfg->m_SysBaseInfo.m_sImageHeight)
				theDemo->m_imgHA = pRegCfg->m_SysBaseInfo.m_sImageHeight;
			printf("\twidth=%d,hight=%d\n", theDemo->m_imgWA, theDemo->m_imgHA);
			printf("\tdatatype is unsigned char.\n");
			printf("\tdatabit is 16bits.\n");
			printf("\tdata is little endian.\n");

				theDemo->AutoResize();

				// Process panel configuration if valid size
				if (pRegCfg->m_SysBaseInfo.m_byPanelSize >= 0x01 &&
					pRegCfg->m_SysBaseInfo.m_byPanelSize <= DETECTOR_TYPE_NUMBER)
				{
					/*
					* workmode
					* 01-Software Trigger;02-Clear;03-Static:Hvg Trigger Mode;04-Static:AED Trigger Mode;
					* 05-Dynamic:Hvg Sync Mode;06-Dynamic:Fpd Sync Mode;07-Dynamic:Fpd Continue;08-Static:SAECMode;
					*/
					if (pRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x01)
						printf("\tstatic software trigger.[0x%02x]\n", pRegCfg->m_SysCfgInfo.m_byTriggerMode);
					
					else if (pRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x03)
						printf("\tstatic hvg trigger.[0x%02x]\n", pRegCfg->m_SysCfgInfo.m_byTriggerMode);
					
					else if (pRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x04)
						printf("\tFree AED trigger mode.[0x%02x]\n", pRegCfg->m_SysCfgInfo.m_byTriggerMode);
					
					else if (pRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x05)
						printf("\tDynamic:Hvg Sync mode.[0x%02x]\n", pRegCfg->m_SysCfgInfo.m_byTriggerMode);
					
					else if (pRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x06)
						printf("\tDynamic:Fpd Sync mode.[0x%02x]\n", pRegCfg->m_SysCfgInfo.m_byTriggerMode);
					
					else if (pRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x07)
						printf("\tDynamic:Fpd Continue mode.[0x%02x]\n", pRegCfg->m_SysCfgInfo.m_byTriggerMode);
					
					else if (pRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x08)
						printf("\tStatic:SAEC mode.[0x%02x]\n", pRegCfg->m_SysCfgInfo.m_byTriggerMode);
					
					else
						printf("\tother trigger mode.[0x%02x]\n", pRegCfg->m_SysCfgInfo.m_byTriggerMode);

					//Process correction modes and timing settings
					theDemo->m_fpd_base->trigger_mode = pRegCfg->m_SysCfgInfo.m_byTriggerMode;

					printf("\tPre Acquisition Delay Time.%u\n", pRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime);
					
					// Update correction mode settings
					if (theDemo->m_fpd_base->offset_enable != pRegCfg->m_ImgCaliCfg.m_byOffsetCorrection)
						theDemo->m_fpd_base->offset_enable = pRegCfg->m_ImgCaliCfg.m_byOffsetCorrection;

					if (theDemo->m_fpd_base->gain_enable = pRegCfg->m_ImgCaliCfg.m_byGainCorrection)
						theDemo->m_fpd_base->gain_enable = pRegCfg->m_ImgCaliCfg.m_byGainCorrection;

					if (theDemo->m_fpd_base->defect_enable = pRegCfg->m_ImgCaliCfg.m_byDefectCorrection)
						theDemo->m_fpd_base->defect_enable = pRegCfg->m_ImgCaliCfg.m_byDefectCorrection;

					if (theDemo->m_fpd_base->dummy_enable = pRegCfg->m_ImgCaliCfg.m_byDummyCorrection)
						theDemo->m_fpd_base->dummy_enable = pRegCfg->m_ImgCaliCfg.m_byDummyCorrection;
					
					// Log correction mode states
					if (0x00 == theDemo->m_fpd_base->offset_enable)
						printf("\tNo Offset Correction\n");
					else if (0x01 == theDemo->m_fpd_base->offset_enable)
						printf("\tSoftware Offset Correction\n");
					else if (0x02 == theDemo->m_fpd_base->offset_enable)
						printf("\tFirmware PostOffset Correction\n");
					else if (0x03 == theDemo->m_fpd_base->offset_enable)
						printf("\tFirmware PreOffset Correction\n");
					else
						printf("\tInvalid Offset Correction\n");

					if (0x00 == theDemo->m_fpd_base->gain_enable)
						printf("\tNo Gain Correction\n");
					else if (0x01 == theDemo->m_fpd_base->gain_enable)
						printf("\tSoftware Gain Correction\n");
					else if (0x02 == theDemo->m_fpd_base->gain_enable)
						printf("\tFirmware Gain Correction\n");
					else
						printf("\tInvalid Gain Correction\n");

					if (0x00 == theDemo->m_fpd_base->defect_enable)
						printf("\tNo Defect Correction\n");
					else if (0x01 == theDemo->m_fpd_base->defect_enable)
						printf("\tSoftware Defect Correction\n");
					else if (0x02 == theDemo->m_fpd_base->defect_enable)
						printf("\tFirmware Defect Correction\n");
					else
						printf("\tInvalid Defect Correction\n");

					if (0x00 == theDemo->m_fpd_base->dummy_enable)
						printf("\tNo Dummy Correction\n");
					else if (0x01 == theDemo->m_fpd_base->dummy_enable)
						printf("\tSoftware Dummy Correction\n");
					else if (0x02 == theDemo->m_fpd_base->dummy_enable)
						printf("\tFirmware Dummy Correction\n");
					else
						printf("\tInvalid Dummy Correction\n");

					// PGA Gain Level Configuration
					theDemo->m_fpd_base->nPGALevel = theDemo->GetPGA(pRegCfg->m_TICOFCfg.m_sTICOFRegister[26]);

					// Configure Binning Type
					int iscale = pRegCfg->m_SysCfgInfo.m_byBinning;
					if (iscale < 1 || iscale > 4)
					{
						theDemo->m_fpd_base->nBinning = 1;
						iscale = 1;
					}

					else
						theDemo->m_fpd_base->nBinning = iscale;

					// Update Image Dimensions
					if (theDemo->m_imgWA != pRegCfg->m_SysBaseInfo.m_sImageWidth)  theDemo->m_imgWA = pRegCfg->m_SysBaseInfo.m_sImageWidth;
					if (theDemo->m_imgHA != pRegCfg->m_SysBaseInfo.m_sImageHeight) theDemo->m_imgHA = pRegCfg->m_SysBaseInfo.m_sImageHeight;

					// Matching Panel Size Configuration
					int j = (pRegCfg->m_SysBaseInfo.m_byPanelSize - 1);
					if (theDemo->m_imgApro.nFpdNum != HB_FPD_SIZE[j].fpd_num)   theDemo->m_imgApro.nFpdNum = HB_FPD_SIZE[j].fpd_num;
					
					// Scale the image properties based on binning
					theDemo->m_imgApro.nwidth = HB_FPD_SIZE[j].fpd_width / iscale;
					theDemo->m_imgApro.nheight = HB_FPD_SIZE[j].fpd_height / iscale;
					theDemo->m_imgApro.packet_size = HB_FPD_SIZE[j].fpd_packet_size / (iscale * iscale);
					
					// Calculate Frame Size (Assuming 16-bit data)
					int frame_size = theDemo->m_imgApro.nwidth * theDemo->m_imgApro.nheight * 2;
					
					// Ensure Data Properties are Set to Default
					if (theDemo->m_imgApro.frame_size != frame_size) theDemo->m_imgApro.frame_size = frame_size; // 目前默认为16bits数据
					if (theDemo->m_imgApro.datatype != 0) theDemo->m_imgApro.datatype = 0;
					if (theDemo->m_imgApro.ndatabit != 0) theDemo->m_imgApro.ndatabit = 0;
					if (theDemo->m_imgApro.nendian != 0)  theDemo->m_imgApro.nendian = 0;

					// Debug Logging
					printf("\tnBinning=%d\n", theDemo->m_fpd_base->nBinning);
					printf("\twidth=%d,hight=%d\n", theDemo->m_imgWA, theDemo->m_imgHA);
					printf("\tdatatype is unsigned char.\n");
					printf("\tdatabit is 16bits.\n");
					printf("\tdata is little endian.\n");
					printf("\tnPGA=%d\n", theDemo->m_fpd_base->nPGALevel);
					printf("\tnBinning=%d\n", theDemo->m_fpd_base->nBinning);
				}

				// Configure Packet Interval Time
				usValue = ((pRegCfg->m_SysCfgInfo.m_sPacketIntervalTm & 0xff) << 8) | ((pRegCfg->m_SysCfgInfo.m_sPacketIntervalTm >> 8) & 0xff);
				theDemo->m_fpd_base->packetInterval = usValue;

				// Configure Acquisition Delay Time
				BYTE byte1 = 0, byte2 = 0, byte3 = 0, byte4 = 0;
				byte1 = BREAK_UINT32(pRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime, 0);
				byte2 = BREAK_UINT32(pRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime, 1);
				byte3 = BREAK_UINT32(pRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime, 2);
				byte4 = BREAK_UINT32(pRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime, 3);

				unsigned int unValue = BUILD_UINT32(byte4, byte3, byte2, byte1);
				theDemo->m_fpd_base->prepareTime = unValue;

				// Configure Frame Rate Timing Based on Communication Type
				if (theDemo->m_fpd_base->commType == 0 || theDemo->m_fpd_base->commType == 3)
				{
					byte1 = BREAK_UINT32(pRegCfg->m_SysCfgInfo.m_unContinuousAcquisitionSpanTime, 0);
					byte2 = BREAK_UINT32(pRegCfg->m_SysCfgInfo.m_unContinuousAcquisitionSpanTime, 1);
					byte3 = BREAK_UINT32(pRegCfg->m_SysCfgInfo.m_unContinuousAcquisitionSpanTime, 2);
					byte4 = BREAK_UINT32(pRegCfg->m_SysCfgInfo.m_unContinuousAcquisitionSpanTime, 3);
				}

				else
				{
					byte1 = BREAK_UINT32(pRegCfg->m_SysCfgInfo.m_unSelfDumpingSpanTime, 0);
					byte2 = BREAK_UINT32(pRegCfg->m_SysCfgInfo.m_unSelfDumpingSpanTime, 1);
					byte3 = BREAK_UINT32(pRegCfg->m_SysCfgInfo.m_unSelfDumpingSpanTime, 2);
					byte4 = BREAK_UINT32(pRegCfg->m_SysCfgInfo.m_unSelfDumpingSpanTime, 3);
				}

				unValue = BUILD_UINT32(byte4, byte3, byte2, byte1);
				if (unValue <= 0)
				{
					printf("\tlive acquisition time=%u\n", unValue);
					unValue = 1000;
				}

				if (theDemo->m_fpd_base->liveAcqTime != unValue) 
					theDemo->m_fpd_base->liveAcqTime = unValue;

				unValue = (UINT)(1000 / unValue);

				if (unValue <= 0) 
					unValue = 1;

				if (theDemo->m_uMaxFps != unValue) 
					theDemo->m_uMaxFps = unValue;

				// Debug Logging
				printf("\tpacketInterval=%u\n", theDemo->m_fpd_base->packetInterval);
				printf("\tprepareTime=%u\n", theDemo->m_fpd_base->prepareTime);
				printf("\tm_uMaxFps=%u\n", theDemo->m_uMaxFps);
				printf("\tliveAcqTime=%u\n", theDemo->m_fpd_base->liveAcqTime);

				// Store Configuration
				theDemo->m_pLastRegCfg = pRegCfg;

				// Update FPD Base Data
				theDemo->PostMessage(WM_USER_CURR_CONTROL_DATA, (WPARAM)0, (LPARAM)0);

				//theDemo->UpdateCurControlData();
			}
		}
		break;

	case ECALLBACK_TYPE_SINGLE_IMAGE:   // Single frame image capture event
	case ECALLBACK_TYPE_MULTIPLE_IMAGE: // Continuous image capture event

		if (imagedata != NULL)
		{	
			// Update image width and height if changed
			if (theDemo->m_imgWA != imagedata->uwidth) theDemo->m_imgWA = imagedata->uwidth;
			if (theDemo->m_imgHA != imagedata->uheight) theDemo->m_imgHA = imagedata->uheight;
			
			// Retrieve last known configuration
			pRegCfg = theDemo->m_pLastRegCfg;
			if (pRegCfg != NULL)
			{
				if (pRegCfg->m_SysBaseInfo.m_sImageWidth != theDemo->m_imgWA)  pRegCfg->m_SysBaseInfo.m_sImageWidth = theDemo->m_imgWA;
				if (pRegCfg->m_SysBaseInfo.m_sImageHeight != theDemo->m_imgHA) pRegCfg->m_SysBaseInfo.m_sImageHeight = theDemo->m_imgHA;
			}

			// Save image if display mode is off
			if (!theDemo->m_bShowPic && (theDemo->m_uDefaultFpdid == 0))
			{
				ret = theDemo->SaveImage((unsigned char*)imagedata->databuff, imagedata->datalen);
				if (ret)
					printf("theDemo->SaveImage succss!Frame ID:%05d\n", imagedata->uframeid);
				else
					printf("theDemo->SaveImage failed!Frame ID:%05d\n", imagedata->uframeid);
			}

			else
			{
				// Display the image
				ret = theDemo->ShowImageA((unsigned short*)imagedata->databuff, (imagedata->datalen / sizeof(unsigned short)), imagedata->uframeid, param3);
				if (ret)
					printf("theDemo->ShowImageA succss!Frame ID:%05d\n", imagedata->uframeid);
				else
					printf("theDemo->ShowImageA failed!Frame ID:%05d\n", imagedata->uframeid);

				// Set event signal if necessary
				if (pDlg != NULL)
				{
					if (pDlg->m_hEventA != NULL)
					{
						::SetEvent(pDlg->m_hEventA);
					}
				}
			}
		}
		break;

	case ECALLBACK_TYPE_GENERATE_TEMPLATE: // Handling template generation events
	{
	{
		if (nlength == ECALLBACK_TEMPLATE_BEGIN) {
			printf("ECALLBACK_TEMPLATE_BEGIN\n");
		}

		else if (nlength == ECALLBACK_TEMPLATE_INVALVE_PARAM) {
			printf("ECALLBACK_TEMPLATE_INVALVE_PARAM:%d\n", param3);
		}

		else if (nlength == ECALLBACK_TEMPLATE_MALLOC_FAILED) {
			printf("ECALLBACK_TEMPLATE_MALLOC_FAILED:%d\n", param3);
		}

		else if (nlength == ECALLBACK_TEMPLATE_SEND_FAILED) {
			printf("ECALLBACK_TEMPLATE_SEND_FAILED:%d\n", param3);
		}

		else if (nlength == ECALLBACK_TEMPLATE_STATUS_ABORMAL) {
			printf("ECALLBACK_TEMPLATE_STATUS_ABORMAL:%d\n", param3);
		}

		else if (nlength == ECALLBACK_TEMPLATE_FRAME_NUM) {
			printf("ECALLBACK_TEMPLATE_FRAME_NUM:%d\n", param3);
		}

		else if (nlength == ECALLBACK_TEMPLATE_TIMEOUT) {
			printf("ECALLBACK_TEMPLATE_TIMEOUT:%d\n", param3);
		}

		else if (nlength == ECALLBACK_TEMPLATE_MEAN) 
		{
			ECALLBACK_RAW_INFO *ptr = (ECALLBACK_RAW_INFO *)pvParam;
			if (ptr != NULL) 
			{
				printf("ECALLBACK_TEMPLATE_MEAN:%s,dMean=%0.2f\n", ptr->szRawName, ptr->dMean);
			}
		}

		else if (nlength == ECALLBACK_TEMPLATE_GENERATE)
		{
			if (param3 == OFFSET_TMP)
				printf("ECALLBACK_TEMPLATE_GENERATE:OFFSET_TMP\n");
			else if (param3 == GAIN_TMP)
				printf("ECALLBACK_TEMPLATE_GENERATE:GAIN_TMP\n");
			else if (param3 == DEFECT_TMP)
				printf("ECALLBACK_TEMPLATE_GENERATE:DEFECT_TMP,bad point=%d\n", nlength);
			else
				printf("ECALLBACK_TEMPLATE_GENERATE:nid=%d\n", param3);
		}

		else if (nlength == ECALLBACK_TEMPLATE_RESULT) {
			printf("ECALLBACK_TEMPLATE_RESULT:%d\n", param3);
		}

		else {
			printf("Other template event: Length = %d, ID = %d\n", nlength, param3);
		}
	}

	break;

	case ECALLBACK_OVERLAY_16BIT_IMAGE: // Processing 16-bit overlay images
		// Update image size if necessary
		if (theDemo->m_imgWA != imagedata->uwidth)
			theDemo->m_imgWA = imagedata->uwidth;
		if (theDemo->m_imgHA != imagedata->uheight)
			theDemo->m_imgHA = imagedata->uheight;

		// Update configuration
		pRegCfg = theDemo->m_pLastRegCfg;
		if (pRegCfg != NULL)
		{
			if (pRegCfg->m_SysBaseInfo.m_sImageWidth != theDemo->m_imgWA)  pRegCfg->m_SysBaseInfo.m_sImageWidth = theDemo->m_imgWA;
			if (pRegCfg->m_SysBaseInfo.m_sImageHeight != theDemo->m_imgHA) pRegCfg->m_SysBaseInfo.m_sImageHeight = theDemo->m_imgHA;
		}

		// Save or display image
		if (!theDemo->m_bShowPic && (theDemo->m_uDefaultFpdid == 0))
		{
			ret = theDemo->SaveImage((unsigned char *)imagedata->databuff, imagedata->datalen);
			if (ret)
				printf("theDemo->SaveImage succss!Frame ID:%05d\n", imagedata->uframeid);
			else
				printf("theDemo->SaveImage failed!Frame ID:%05d\n", imagedata->uframeid);
		}

		else 
		{
			ret = theDemo->ShowImageA((unsigned short *)imagedata->databuff, (imagedata->datalen / sizeof(unsigned short)), imagedata->uframeid, param3);
			if (ret)
				printf("theDemo->ShowImageA succss!Frame ID:%05d\n", param3);
			else
				printf("theDemo->ShowImageA failed!Frame ID:%05d\n", param3);

			// Set event signal
			if (pDlg != NULL) {
				if (pDlg->m_hEventA != NULL)
				{
					::SetEvent(pDlg->m_hEventA);
				}
			}
		}
		break;

	case ECALLBACK_OVERLAY_32BIT_IMAGE: // Processing 32-bit overlay images
		printf("pImgPage->UpdateOverlay32BitImage failed,frame_%d,eventid=%d", imagedata->uframeid, param4);
		break;

	case ECALLBACK_TYPE_THREAD_EVENT: 
	{
		if (nlength == 100)
			printf("ECALLBACK_TYPE_THREAD_EVENT,start recv data!\n");

		else if (nlength == 101)
			printf("ECALLBACK_TYPE_THREAD_EVENT,end recv data!\n");

		else if (nlength == 104)
			printf("ECALLBACK_TYPE_THREAD_EVENT,Packet Retransmission:start recv data!\n");

		else if (nlength == 105)
			printf("ECALLBACK_TYPE_THREAD_EVENT,Frame Retransmission:start recv data!\n");

		else if (nlength == 106)
			printf("ECALLBACK_TYPE_THREAD_EVENT,Frame loss retransmission over,end recv data!\n");
		
		else if (nlength == 107)
			printf("ECALLBACK_TYPE_THREAD_EVENT,image buff is null:end recv data!\n");
		
		else if (nlength == 108)
			printf("ECALLBACK_TYPE_THREAD_EVENT,Generate Offset Template:start thread!\n");
		
		else if (nlength == 109)
			printf("ECALLBACK_TYPE_THREAD_EVENT,Generate Offset Template:end thread!\n");
		
		else if (nlength == 110)
			printf("ECALLBACK_TYPE_THREAD_EVENT,Generate Gain Template:start thread!\n");
		
		else if (nlength == 111)
			printf("ECALLBACK_TYPE_THREAD_EVENT,Generate Gain Template:end thread!\n");
		
		else if (nlength == 112)
			printf("ECALLBACK_TYPE_THREAD_EVENT,offset calibrate:success!\n");
		
		else if (nlength == 113)
			printf("ECALLBACK_TYPE_THREAD_EVENT,offset calibrate:failed!\n");
		
		else if (nlength == 114)
			printf("ECALLBACK_TYPE_THREAD_EVENT,gain calibrate:success!\n");
		
		else if (nlength == 115)
			printf("ECALLBACK_TYPE_THREAD_EVENT,gain calibrate:failed!\n");
		
		else if (nlength == 116)
			printf("ECALLBACK_TYPE_THREAD_EVENT,defect calibrate:success!\n");
		
		else if (nlength == 117)
			printf("ECALLBACK_TYPE_THREAD_EVENT,defect calibrate:failed!\n");
		
		else if (nlength == 118)
			printf("ECALLBACK_TYPE_THREAD_EVENT,InitGainTemplate:failed!\n");
		
		else if (nlength == 119)
			printf("ECALLBACK_TYPE_THREAD_EVENT,firmare offset calibrate:success!\n");
		
		else if (nlength == 120)
			printf("ECALLBACK_TYPE_THREAD_EVENT,Generate Defect Template:start thread!\n");
		
		else if (nlength == 121)
			printf("ECALLBACK_TYPE_THREAD_EVENT,Generate Defect Template:end thread!\n");
		
		else
			printf("ECALLBACK_TYPE_THREAD_EVENT,Err:未知错误[%d]\n", nlength);
	}

	break;

	case ECALLBACK_TYPE_FILE_NOTEXIST: // Handling missing file errors
		if (pvParam != NULL)
			printf("err:%s not exist!\n", (char *)pvParam);
		break;

	default:
		printf("ECALLBACK_TYPE_INVALVE,command=%02x\n", byteEventId);
		break;
	}

	return ret;
}


// Handles the event when "Show Picture" radio button is clicked
void CHB_SDK_DEMO2008Dlg::OnBnClickedRadioShowPic()
{
	// TODO: Add your control notification handler code here
	if (!m_bShowPic) m_bShowPic = true;
}

// Handles the event when "Save Picture" radio button is clicked
void CHB_SDK_DEMO2008Dlg::OnBnClickedRadioSavePic()
{
	// TODO: Add your control notification handler code here	
	if (m_bShowPic) m_bShowPic = false;

	// Create directory for saving images
	char filename[MAX_PATH];
	memset(filename, 0x00, MAX_PATH);
	sprintf(filename, "%s\\raw_dir", m_path);
	if (!PathFileExists(filename))
	{
		CreateDirectory(filename, NULL);
	}
}

// Opens the detector configuration file (DETECTOR_CFG.INI) if it exists
void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonOpenConfig()
{
	// TODO: Add control notification handler code here
	char szPath[MAX_PATH] = { 0 };
	memset(szPath, 0x00, MAX_PATH);
	sprintf(szPath, "%s\\DETECTOR_CFG.INI", m_path);

	// Check if configuration file exists
	if (!PathFileExists(szPath))
	{
		::AfxMessageBox(_T("Warning: The file DETECTOR_CFG.INI does not exist in the current directory. Please check!"));
		return;
	}

	// Open the configuration file
	ShellExecute(NULL, "open", szPath, NULL, NULL, SW_SHOW);
}


// Opens the detector configuration file (DETECTOR_CFG.INI) if it exists
void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonOpenConfig()
{
	// TODO: Add control notification handler code here
	char szPath[MAX_PATH] = { 0 };
	memset(szPath, 0x00, MAX_PATH);
	sprintf(szPath, "%s\\DETECTOR_CFG.INI", m_path);

	// Check if configuration file exists
	if (!PathFileExists(szPath))
	{
		::AfxMessageBox(_T("Warning: The file DETECTOR_CFG.INI does not exist in the current directory. Please check!"));
		return;
	}

	// Open the configuration file
	ShellExecute(NULL, "open", szPath, NULL, NULL, SW_SHOW);
}

// Displays an image based on buffer data and updates frame ID and FPS
int CHB_SDK_DEMO2008Dlg::ShowImageA(unsigned short* imgbuff, int nbufflen, int nframeid, int nfps)
{
	if (m_hEventA == NULL) {
		printf("Error: m_hEventA is NULL!\n");
		return 0;
	}

	if (m_ThreadhdlA == NULL)
	{
		printf("Error: m_ThreadhdlA is NULL!\n");
		return 0;
	}

	// Check image resolution
	int WIDTH = m_imgWA;
	int HEIGHT = m_imgHA;

	if ((imgbuff == NULL) || (nbufflen != (WIDTH * HEIGHT)))
	{
		printf("Data inconsistency detected!\n");
		return 0;
	}

	// Resize the image based on scaling factor
	CvSize sz;
	sz.width = (int)(WIDTH * picA_factor);
	sz.height = (int)(HEIGHT * picA_factor);

	// Allocate or reallocate image buffer if necessary
	if (pIplimageA != NULL)
	{
		if (pIplimageA->width != WIDTH || pIplimageA->height != HEIGHT)
		{
			cvReleaseImage(&pIplimageA);
			pIplimageA = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_16U, 1);
		}
	}

	else
	{
		pIplimageA = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_16U, 1);
	}

	if (pIplimageA == NULL) {
		printf("Error: pIplimageA is NULL!\n");
		return -1;
	}

	// Copy image data to buffer
	memcpy(pIplimageA->imageData, imgbuff, nbufflen * sizeof(unsigned short));

	// Update frame ID and FPS
	if (m_frameidA != nframeid) m_frameidA = nframeid;
	if (m_ufpsA != nfps) m_ufpsA = nfps;

	return 1;
}

// Closes the image display thread and releases resources
void CHB_SDK_DEMO2008Dlg::CloseShowThread()
{
	RunFlag = 0;
	if (m_ThreadhdlA != NULL)
	{
		// Notify the thread to terminate
		if (m_hEventA != NULL)
		{
			SetEvent(m_hEventA);
			printf("%s\n", "SetEvent(m_hEventA)!");
		}

		// Wait for the thread to terminate
		DWORD dw = WaitForSingleObject(m_ThreadhdlA, 5000);
		if (dw != WAIT_OBJECT_0)
		{
			printf("Error: %s\n", "TerminateThread(m_ThreadhdlA, 0)!");
		}
		else
		{
			printf("%s\n", "CloseHandle(m_ThreadhdlA, 0)!");
		}

		// Close thread handle
		CloseHandle(m_ThreadhdlA);
		m_ThreadhdlA = NULL;
		m_uThreadFunIDA = 0;

		// Close event handle
		if (m_hEventA != NULL)
		{
			::CloseHandle(m_hEventA);
			m_hEventA = NULL;
			printf("%s\n", "CloseHandle(m_hEventA)!");
		}
	}
#ifdef _DEBUG
	printf("CloseShowThread\n");

#endif
}

// Updates and displays the current image on the screen
void CHB_SDK_DEMO2008Dlg::UpdateImageA()
{
	if (pPicWndA == NULL) return;

	CDC* pDC = pPicWndA->GetDC();  // Get display component's DC
	if (pDC == NULL) return;

	if (pIplimageA == NULL) return;

	// Clear previous frame with a black background
	pDC->FillSolidRect(&m_PicRectA, RGB(0, 0, 0));

	HDC hDC = pDC->GetSafeHdc();  // Get device context (HDC) for drawing
	CvvImage cimg;
	cimg.CopyOf(pIplimageA);      // Copy the image
	cimg.DrawToHDC(hDC, &m_PicRectA);  // Render the image to the display component
	SetBkMode(hDC, TRANSPARENT);
	SetTextColor(hDC, RGB(255, 0, 0));

	// Display image metadata
	char buff[32];
	memset(buff, 0x00, 32);

	// Display current date and time
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	int j = sprintf(buff, "Date:%4d-%02d-%02d %02d:%02d:%02d.%03d",
		sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
	buff[j] = '\0';
	::TextOut(hDC, 10, 10, buff, j);

	// Display frame ID
	j = sprintf(buff, "Frame ID:%05d", m_frameidA);
	buff[j] = '\0';
	::TextOut(hDC, 10, 30, buff, j);

	// Display resolution
	j = sprintf(buff, "Pixel:%d * %d", m_imgWA, m_imgHA);
	buff[j] = '\0';
	::TextOut(hDC, 10, 50, buff, j);

	// Display frame rate (FPS)
	j = sprintf(buff, "Fps:%u", m_ufpsA);
	buff[j] = '\0';
	::TextOut(hDC, 10, 70, buff, j);

	ReleaseDC(pDC);
}

// Thread function for handling image display updates
unsigned int __stdcall CHB_SDK_DEMO2008Dlg::ShowThreadProcFunA(LPVOID pParam)
{
	printf("%s\n", "ShowThreadProcFunA start!");
	int ret = 0;
	int j = 0;
	DWORD dwResult = 0;

	// Validate input parameter
	CHB_SDK_DEMO2008Dlg* pPlugIn = (CHB_SDK_DEMO2008Dlg*)pParam;
	if (pPlugIn == NULL)
	{
		printf("%s\n", "Thread parameter is NULL, failed to create ShowThreadProcFunA!");
		goto EXIT_SHOW_IMAGE_THREAD;
	}

	// Check for valid event handle
	if (NULL == pPlugIn->m_hEventA)
	{
		printf("Error: ShowThreadProcFunA, %s\n", "pPlugIn->m_hEventA is NULL!");
		goto EXIT_SHOW_IMAGE_THREAD;
	}

	// Reset event to an inactive state
	::ResetEvent(pPlugIn->m_hEventA);

	// Main loop to wait for image update events
	while (pPlugIn->RunFlag)
	{
		dwResult = ::WaitForSingleObject(pPlugIn->m_hEventA, INFINITE);
		if (WAIT_OBJECT_0 == dwResult)
		{
			if (!pPlugIn->RunFlag)
			{
				printf("pPlugIn->RunFlag=%d, exiting!\n", pPlugIn->RunFlag);
				break;
			}

			// Update image display
			if (pPlugIn->pPicWndA != NULL)
			{
				pPlugIn->UpdateImageA();
			}
		}

		// Reset event status
		::ResetEvent(pPlugIn->m_hEventA);
	}

	// Exit thread
EXIT_SHOW_IMAGE_THREAD:
	printf("%s\n", "ShowThreadProcFunA end!");
	return 0;
}

// Retrieves and displays the firmware version
void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonFirmwareVer()
{
	printf("OnBnClickedButtonFirmwareVer\n");
	if (m_pFpd == NULL) {
		::AfxMessageBox("m_pFpd is NULL!");
		return;
	}
	int ret = HBI_GetFirmareVerion(m_pFpd, szFirmVer);
	if (0 != ret) {
		CString strStr = _T("");
		strStr.Format(_T("HBI_GetFirmareVerion Error, ret=%d"), ret);
		::AfxMessageBox(strStr);
		return;
	}
	printf("szFirmVer=%s\n", szFirmVer);
}

// Retrieves and displays the software version
void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSoftwareVer()
{
	printf("OnBnClickedButtonSoftwareVer\n");
	if (m_pFpd == NULL) {
		::AfxMessageBox("m_pFpd is NULL!");
		return;
	}

	int ret = HBI_GetSDKVerion(m_pFpd, szSdkVer);
	if (0 != ret) {
		CString strStr = _T("");
		strStr.Format(_T("HBI_GetSDKVerion Error, ret=%d"), ret);
		::AfxMessageBox(strStr);
		return;
	}
	printf("szSdkVer=%s\n", szSdkVer);
}

// Retrieves and displays the image properties
void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnGetImageProperty()
{
	printf("Get Image Property started!\n");
	if (m_pFpd == NULL)
	{
		printf("Detector not initialized!\n");
		return;
	}

	if (!this->m_IsOpen)
	{
		printf("Detector not connected!\n");
		return;
	}

	size_t size = sizeof(HB_FPD_SIZE) / sizeof(struct FpdPixelMatrixTable);

	int ret = HBI_GetImageProperty(m_pFpd, &m_imgApro);

	if (ret == 0)
	{
		printf("HBI_GetImageProperty:\n\tnnFpdNum=%u\n", m_imgApro.nFpdNum);
		if (m_imgWA != m_imgApro.nwidth) m_imgWA = m_imgApro.nwidth;
		if (m_imgHA != m_imgApro.nheight) m_imgHA = m_imgApro.nheight;
		printf("\twidth=%d,hight=%d\n", m_imgWA, m_imgHA);

		// Display image data type
		if (m_imgApro.datatype == 0) 
			printf("\tdatatype is unsigned char.\n");

		else if (m_imgApro.datatype == 1) 
			printf("\tdatatype is char.\n");

		else if (m_imgApro.datatype == 2) 
			printf("\tdatatype is unsigned short.\n");

		else if (m_imgApro.datatype == 3) 
			printf("\tdatatype is float.\n");

		else if (m_imgApro.datatype == 4) 
			printf("\tdatatype is double.\n");

		else 
			printf("\tdatatype is not support.\n");

		// Display data bit-depth
		if (m_imgApro.ndatabit == 0) 
			printf("\tdatabit is 16bits.\n");

		else if (m_imgApro.ndatabit == 1) 
			printf("\tdatabit is 14bits.\n");

		else if (m_imgApro.ndatabit == 2)
			printf("\tdatabit is 12bits.\n");

		else if (m_imgApro.ndatabit == 3) 
			printf("\tdatabit is 8bits.\n");

		else
			printf("\tdatatype is unsigned char.\n");

		// Display data endianness
		if (m_imgApro.nendian == 0) 
			printf("\tdata is little endian.\n");

		else 
			printf("\tdata is bigger endian.\n");

		// Display additional image properties
		printf("\tpacket_size=%u\n", m_imgApro.packet_size);
		printf("\tframe_size=%d\n\n", m_imgApro.frame_size);
	}

	else
	{
		::AfxMessageBox("HBI_GetImageProperty failed!");
	}
}

// Retrieves and displays the firmware configuration
void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonGetConfig()
{
	bool b = GetFirmwareConfig();
	if (b)
		printf("GetFirmwareConfig success\n");
	else
		printf("Error: GetFirmwareConfig failed\n");
}

// Prepares the detector for a single acquisition
void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSinglePrepare()
{
	printf("OnBnClickedButtonSinglePrepare!\n");
	if (m_pFpd == NULL)
	{
		::AfxMessageBox(_T("Error: m_pFpd is NULL!"));
		return;
	}

	int ret = HBI_SinglePrepare(m_pFpd);
	if (ret == 0)
		printf("HBI_SinglePrepare success!\n");
	else
		printf("HBI_SinglePrepare failed!\n");
}

// Performs a single image acquisition
void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSingleShot()
{
	UpdateData(TRUE);
	printf("OnBnClickedButtonSingleShot!\n");
	if (m_pFpd == NULL)
	{
		::AfxMessageBox(_T("Error: m_pFpd is NULL!"));
		return;
	}

	// Configure acquisition mode parameters
	m_aqc_mode.eAqccmd = SINGLE_ACQ_DEFAULT_TYPE;
	m_aqc_mode.nAcqnumber = 0;  // Capture only one frame
	m_aqc_mode.nframeid = 0;
	m_aqc_mode.ngroupno = 0;
	m_aqc_mode.ndiscard = 0;
	m_aqc_mode.eLivetype = ONLY_IMAGE;  // Only capture the image

	printf("HBI_SingleAcquisition:[_curmode]=%d,[_acqMaxNum]=%d,[_discardNum]=%d,[_groupNum]=%d\n"
		, m_aqc_mode.eAqccmd, m_aqc_mode.nAcqnumber, m_aqc_mode.ndiscard, m_aqc_mode.ngroupno);
	
	int ret = HBI_SingleAcquisition(m_pFpd, m_aqc_mode);
	
	if (ret == 0)
		printf("mode=%d,sum=%d,num=%d,group=%d,HBI_SingleAcquisition succss!\n", m_aqc_mode.eAqccmd, m_aqc_mode.nAcqnumber, m_aqc_mode.ndiscard, m_aqc_mode.ngroupno);
	
	else {
		printf("mode=%d,sum=%d,num=%d,group=%d,HBI_SingleAcquisition failed!\n", m_aqc_mode.eAqccmd, m_aqc_mode.nAcqnumber, m_aqc_mode.ndiscard, m_aqc_mode.ngroupno);
	}
}


// Configures trigger mode and correction settings
void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnSetTriggerCorrection()
{
	UpdateData(TRUE);
	printf("OnBnClickedBtnSetTriggerAndCorrect begin!\n");

	if (m_pFpd == NULL) {
		printf("Error: Detector not connected!\n");
		return;
	}

	// Select trigger mode
	int _triggerMode = STATIC_SOFTWARE_TRIGGER_MODE;  // Default: software trigger
	int select = ((CComboBox*)GetDlgItem(IDC_COMBO_TRIGGER_MODE))->GetCurSel();
	if (select <= 0 || select >= 7) _triggerMode = DYNAMIC_FPD_CONTINUE_MODE;
	else _triggerMode = select;

	// Configure correction settings
	IMAGE_CORRECT_ENABLE* pcorrect = new IMAGE_CORRECT_ENABLE;
	if (pcorrect != NULL) {		
#if 0
		pcorrect->bFeedbackCfg = false; // false-ECALLBACK_TYPE_SET_CFG_OK Event
#else
		pcorrect->bFeedbackCfg = true;  // //true-ECALLBACK_TYPE_ROM_UPLOAD Event,
#endif
		pcorrect->ucOffsetCorrection = ((CComboBox *)GetDlgItem(IDC_COMBO_ENABLE_OFFSET))->GetCurSel();
		pcorrect->ucGainCorrection   = ((CComboBox *)GetDlgItem(IDC_COMBO_ENABLE_GAIN))->GetCurSel();
		pcorrect->ucDefectCorrection = ((CComboBox *)GetDlgItem(IDC_COMBO_ENABLE_DEFECT))->GetCurSel();
		pcorrect->ucDummyCorrection  = 0;

		int ret = HBI_TriggerAndCorrectApplay(m_pFpd, _triggerMode, pcorrect);
		
		if (ret == 0)
			printf("OnBnClickedBtnSetTriggerAndCorrect:\n\tHBI_TriggerAndCorrectApplay success!\n");
		
		else
			::AfxMessageBox("HBI_TriggerAndCorrectApplay failed!");

		delete pcorrect;
		pcorrect = NULL;
	}
	else {
		::AfxMessageBox("malloc IMAGE_CORRECT_ENABLE failed!");
	}
}


// Updates the detector's trigger mode
void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnSetTriggerMode()
{
	UpdateData(TRUE);
	printf("Set firmware trigger mode begin!\n");

	if (m_pFpd == NULL) {
		printf("Error: Detector not connected!\n");
		return;
	}

	/*
	* 除触发模式发生修改外，其他值不变
	typedef enum
	{
		INVALID_TRIGGER_MODE         = 0x00,
		STATIC_SOFTWARE_TRIGGER_MODE = 0x01,         // 
		STATIC_PREP_TRIGGER_MODE     = 0x02,         //
		STATIC_HVG_TRIGGER_MODE      = 0x03,         //
		STATIC_AED_TRIGGER_MODE      = 0x04,
		DYNAMIC_HVG_TRIGGER_MODE     = 0x05,
		DYNAMIC_FPD_TRIGGER_MODE     = 0x06,
		DYNAMIC_FPD_CONTINUE_MODE    = 0x07
	}EnumTRIGGER_MODE;
	*/

	// Select trigger mode
	int _triggerMode = STATIC_SOFTWARE_TRIGGER_MODE;
	int select = ((CComboBox *)GetDlgItem(IDC_COMBO_TRIGGER_MODE))->GetCurSel();
	if (select <= 0 || select >= 7) _triggerMode = DYNAMIC_FPD_CONTINUE_MODE;
	else _triggerMode = select;

	// Apply the new trigger mode
	int ret = HBI_UpdateTriggerMode(m_pFpd, _triggerMode);
	if (ret == 0) {
		printf("OnBnClickedBtnSetFirmwareWorkmode:\n\tHBI_UpdateTriggerMode success!\n");
	}

	else {
		::AfxMessageBox("HBI_UpdateTriggerMode failed!");
	}
}


// Enables or disables image correction settings
void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnCorrectEnable()
{
	UpdateData(TRUE);
	printf("Set correction enable begin!\n");

	if (m_pFpd == NULL) {
		printf("Error: Detector not connected!\n");
		return;
	}

	// Configure correction settings
	IMAGE_CORRECT_ENABLE *pcorrect = new IMAGE_CORRECT_ENABLE;

	if (pcorrect != NULL) {
#if 0
		pcorrect->bFeedbackCfg = false; // false-ECALLBACK_TYPE_SET_CFG_OK Event
#else
		pcorrect->bFeedbackCfg = true;  // //true-ECALLBACK_TYPE_ROM_UPLOAD Event,
#endif
		pcorrect->ucOffsetCorrection = ((CComboBox *)GetDlgItem(IDC_COMBO_ENABLE_OFFSET))->GetCurSel();
		pcorrect->ucGainCorrection = ((CComboBox *)GetDlgItem(IDC_COMBO_ENABLE_GAIN))->GetCurSel();
		pcorrect->ucDefectCorrection = ((CComboBox *)GetDlgItem(IDC_COMBO_ENABLE_DEFECT))->GetCurSel();
		pcorrect->ucDummyCorrection = 0;

		// Apply correction settings
		int ret = HBI_UpdateCorrectEnable(m_pFpd, pcorrect);
		if (ret == 0)
			printf("OnBnClickedBtnCorrectEnable:\n\tHBI_UpdateCorrectEnable success!\n");
		else
			::AfxMessageBox("HBI_QuckInitDllCfg failed!");

		delete pcorrect;
		pcorrect = NULL;
	}

	else {
		::AfxMessageBox("malloc IMAGE_CORRECT_ENABLE failed!");
	}
}

// Sets the gain mode for the firmware
void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSetGainMode()
{
	UpdateData(TRUE);
	printf("Setting firmware gain mode...\n");

	if (m_pFpd == NULL) {
		printf("Error: Detector not connected!\n");
		return;
	}

	/*
	[n] - Invalid
	[1] - 0.6pC
	[2] - 1.2pC
	[3] - 2.4pC
	[4] - 3.6pC
	[5] - 4.8pC
	[6] - 7.2pC
	[7] - 9.6pC
	*/
	int nGainMode = ((CComboBox*)GetDlgItem(IDC_COMBO_PGA_LEVEL))->GetCurSel();
	int ret = HBI_SetPGALevel(m_pFpd, nGainMode);
	if (ret == 0) {
		printf("HBI_SetPGALevel: Success!\n");
	}
	else {
		::AfxMessageBox("HBI_SetPGALevel failed!");
	}
}

// Retrieves and displays the current gain mode
void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonGetGainMode()
{
	printf("Retrieving firmware gain mode...\n");

	if (m_pFpd == NULL) {
		::AfxMessageBox("Error: m_pFpd is NULL!");
		return;
	}

	/*
	* 除触发模式发生修改外，其他值不变
	[n]-Invalid
	[1]-0.6pC
	[2]-1.2pC
	[3]-2.4pC
	[4]-3.6pC
	[5]-4.8pC
	[6]-7.2pC
	[7]-9.6pC
	*/

	int gainmode = HBI_GetPGALevel(m_pFpd);
	
	if (gainmode == 1)
		printf("HBI_GetPGALevel [1]-0.6pC\n");
	
	else if (gainmode == 2)
		printf("HBI_GetPGALevel [2]-1.2pC\n");
	
	else if (gainmode == 3)
		printf("HBI_GetPGALevel [3]-2.4pC\n");
	
	else if (gainmode == 4)
		printf("HBI_GetPGALevel [4]-3.6pC\n");
	
	else if (gainmode == 5)
		printf("HBI_GetPGALevel [5]-4.8pC\n");
	
	else if (gainmode == 6)
		printf("HBI_GetPGALevel [6]-7.2pC\n");
	
	else if (gainmode == 7)
		printf("HBI_GetPGALevel [7]-9.6pC\n");
	
	else {
		printf("HBI_GetPGALevel Invalid.\n\tgainmode=%d\n", gainmode);
	}

	if (m_fpd_base != NULL)
	{
		if (m_fpd_base->nPGALevel != gainmode) m_fpd_base->nPGALevel = gainmode;

		if (m_fpd_base->nPGALevel >= 1 && m_fpd_base->nPGALevel <= 7)
			((CComboBox *)GetDlgItem(IDC_COMBO_PGA_LEVEL))->SetCurSel(m_fpd_base->nPGALevel);
		
		else
			((CComboBox *)GetDlgItem(IDC_COMBO_PGA_LEVEL))->SetCurSel(0);
	}
}

// Sets the binning mode
void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSetBinning()
{
	this->UpdateData(TRUE);

	if (m_pFpd == NULL) {
		::AfxMessageBox("m_pFpd is NULL!");
		return;
	}

	int binning = ((CComboBox *)GetDlgItem(IDC_COMBO_BINNING))->GetCurSel();

	if (binning == 0) binning = 1;

	printf("HBI_SetBinning:bin=%d\n", binning);

	int ret = HBI_SetBinning(m_pFpd, binning);

	if (ret)
		::AfxMessageBox("HBI_SetBinning failed!");

	/*
	else
{
	// 重新计算分辨率
	if (m_pLastRegCfg != NULL)
	{
		if (m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize >= 0x01 && m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize <= 0x0b)
		{
		// 重新计算分辨率
		int j = (m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize - 1);// 匹配静态数组
		int ww = HB_FPD_SIZE[j].fpd_width;
			int hh = HB_FPD_SIZE[j].fpd_height;
		// Bining:1-1x1,2-2x2,3-3x3,4-4x4
			int iscale = m_pLastRegCfg->m_SysCfgInfo.m_byBinning;
			if (iscale <= 0 || iscale > 4) iscale = 1;

		ww /= iscale;
		hh /= iscale;

			if (m_pLastRegCfg->m_SysBaseInfo.m_sImageWidth != ww)  m_pLastRegCfg->m_SysBaseInfo.m_sImageWidth = ww;
			if (m_pLastRegCfg->m_SysBaseInfo.m_sImageHeight != hh) m_pLastRegCfg->m_SysBaseInfo.m_sImageHeight = hh;
			//
			if (theDemo->m_imgWA != m_pLastRegCfg->m_SysBaseInfo.m_sImageWidth)  theDemo->m_imgWA = m_pLastRegCfg->m_SysBaseInfo.m_sImageWidth;
			if (theDemo->m_imgHA != m_pLastRegCfg->m_SysBaseInfo.m_sImageHeight) theDemo->m_imgHA = m_pLastRegCfg->m_SysBaseInfo.m_sImageHeight;
		}
	}
}
*/

}

// Retrieves and updates the binning mode
void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonGetBinning()
{
	if (m_pFpd == NULL)
	{
		::AfxMessageBox("m_pFpd is NULL!");
		return;
	}

	unsigned int binning = 1;
	int ret = HBI_GetBinning(m_pFpd, &binning);

	if (ret)
	{
		::AfxMessageBox("HBI_SetBinning failed!");
	}

	else
	{
		if (m_fpd_base != NULL)
		{
			if (m_fpd_base->nBinning != binning) m_fpd_base->nBinning = binning;

			if (m_fpd_base->nBinning >= 1 && m_fpd_base->nBinning <= 4)
				((CComboBox *)GetDlgItem(IDC_COMBO_BINNING))->SetCurSel(m_fpd_base->nBinning);

			else
				((CComboBox *)GetDlgItem(IDC_COMBO_BINNING))->SetCurSel(0);
		}
		printf("HBI_SetBinning:bin=%d\n", binning);
	}

	//this->UpdateData(FALSE);
}

// Sets the live acquisition time for the detector
void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSetLiveAcquisitionTm()
{
    this->UpdateData(TRUE);

	int time = m_uLiveTime;

	if (time <= 0)
	{
		::AfxMessageBox("Warning: Time must be greater than 0!");
		return;
	}

	if (m_pFpd == NULL)
	{
		::AfxMessageBox("m_pFpd is NULL!");
		return;
	}

	if (!m_IsOpen)
	{
		::AfxMessageBox("warnning:detector is disconnect!");
		return;
	}

	if (m_fpd_base == NULL)
	{
		::AfxMessageBox(_T("err:m_fpdbasecfg error!"));
		return;
	}

	// Determine if the detector is static or dynamic
	bool bStaticDetector = false;
	COMM_CFG commCfg;
	if (m_fpd_base->commType == 0 || m_fpd_base->commType == 3) 
	{
		bStaticDetector = true;
	}

	int ret = 0;
	if (bStaticDetector)
	{
		printf("HBI_SetLiveAcquisitionTime:time=%d\n", time);
		ret = HBI_SetLiveAcquisitionTime(m_pFpd, time);
		if (ret)
			::AfxMessageBox("HBI_SetLiveAcquisitionTime failed!");

		else
		{
			if (m_fpd_base != NULL)
			{
				unsigned int unValue = (UINT)(1000 / time);
				if (unValue <= 0) unValue = 1;
				if (m_uMaxFps != unValue) m_uMaxFps = unValue;

				CString strstr = _T("");
				strstr.Format(_T("%u"), theDemo->m_uMaxFps);

				((CEdit *)GetDlgItem(IDC_EDIT_MAX_FPS))->SetWindowTextA(strstr);
				strstr.ReleaseBuffer();
			}
		}
	}
	else
	{
		printf("HBI_SetSelfDumpingTime:time=%d\n", time);
		ret = HBI_SetSelfDumpingTime(m_pFpd, time);
		if (ret)
			::AfxMessageBox("HBI_SetSelfDumpingTime failed!");
		else
		{
			if (m_fpd_base != NULL)
			{
				unsigned int unValue = (UINT)(1000 / time);
				if (unValue <= 0) unValue = 1;
				if (m_uMaxFps != unValue) m_uMaxFps = unValue;

				CString strstr = _T("");
				strstr.Format(_T("%u"), theDemo->m_uMaxFps);
				((CEdit *)GetDlgItem(IDC_EDIT_MAX_FPS))->SetWindowTextA(strstr);
				strstr.ReleaseBuffer();
			}
		}
	}
}

// Retrieves the live acquisition time from the detector
void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonGetLiveAcqTime()
{

	if (m_pFpd == NULL) {
		::AfxMessageBox("m_pFpd is NULL!");
		return;
	}

	if (!m_IsOpen)
	{
		::AfxMessageBox("warnning:detector is disconnect!");
		return;
	}

	if (m_fpd_base == NULL)
	{
		::AfxMessageBox(_T("err:m_fpdbasecfg error!"));
		return;
	}

	// Check if the detector is static or wireless
	bool bStaticDetector = false;
	COMM_CFG commCfg;
	if (m_fpd_base->commType == 0 || m_fpd_base->commType == 3) // Static or wireless detector
	{
		bStaticDetector = true;
	}

	int time = 0;
	int ret = 0;

	if (bStaticDetector)
	{
		ret = HBI_GetLiveAcquisitionTime(m_pFpd, &time);
		if (ret)
			::AfxMessageBox("HBI_GetLiveAcquisitionTime failed!");
		else
		{
			m_uLiveTime = time;

			if (m_fpd_base != NULL)
			{
				if (m_fpd_base->liveAcqTime != m_uLiveTime) 
					m_fpd_base->liveAcqTime = m_uLiveTime;
			}

			if (m_uLiveTime <= 0) m_uLiveTime = 1000;
			unsigned int unValue = (UINT)(1000 / m_uLiveTime);

			if (unValue <= 0) unValue = 1;
			if (theDemo->m_uMaxFps != unValue) 
				theDemo->m_uMaxFps = unValue;

			CString strstr = _T("");
			strstr.Format(_T("%u"), theDemo->m_uMaxFps);
			((CEdit *)GetDlgItem(IDC_EDIT_MAX_FPS))->SetWindowTextA(strstr);
			strstr.ReleaseBuffer();

			this->UpdateData(FALSE);
			printf("HBI_GetLiveAcquisitionTime:time=%d\n", time);
		}
	}
	else
	{
		ret = HBI_GetSelfDumpingTime(m_pFpd, &time);
		if (ret)
			::AfxMessageBox("HBI_GetSelfDumpingTime failed!");
		else
		{
			m_uLiveTime = time;

			if (m_fpd_base != NULL)
			{
				if (m_fpd_base->liveAcqTime != m_uLiveTime) 
					m_fpd_base->liveAcqTime = m_uLiveTime;
			}

			if (m_uLiveTime <= 0) m_uLiveTime = 1000;
			unsigned int unValue = (UINT)(1000 / m_uLiveTime);
			if (unValue <= 0) unValue = 1;
			if (theDemo->m_uMaxFps != unValue) 
				theDemo->m_uMaxFps = unValue;

			// Update UI with new FPS value
			CString strstr = _T("");
			strstr.Format(_T("%u"), theDemo->m_uMaxFps);
			((CEdit *)GetDlgItem(IDC_EDIT_MAX_FPS))->SetWindowTextA(strstr);
			strstr.ReleaseBuffer();

			this->UpdateData(FALSE);
			printf("HBI_GetSelfDumpingTime:time=%d\n", time);
		}
	}
}

// Retrieves and displays the FPD serial number
void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonLiveAcquisitionTm()
{
	if (m_pFpd == NULL) {
		::AfxMessageBox("m_pFpd is NULL!");
		return;
	}

	memset(szSeiralNum, 0x00, 16);
	int ret = HBI_GetFPDSerialNumber(m_pFpd, szSeiralNum);
	if (ret)
		::AfxMessageBox("HBI_GetFPDSerialNumber failed!");

	printf("HBI_GetFPDSerialNumber:szSeiralNum=%s\n", szSeiralNum);
}


bool CHB_SDK_DEMO2008Dlg::GetFirmwareConfig()
{
	printf("Retrieving all firmware configuration information...\n");

	if (m_pFpd == NULL)
	{
		printf("Error: Detector is not connected!\n");
		return false;
	}

	//if (m_pLastRegCfg == NULL)
	//	m_pLastRegCfg = new RegCfgInfo;

	if (m_pLastRegCfg == NULL)
	{
		printf("m_pLastRegCfg is NULL!\n");
		return false;
	}

	// Reset the configuration structure
	memset(m_pLastRegCfg, 0x00, sizeof(RegCfgInfo));

	 /*
	 * Retrieve firmware parameters. Once connected, parameters can be obtained.
	 */

	int ret = HBI_GetFpdCfgInfo(m_pFpd, m_pLastRegCfg);

	if (!ret) 
	{
		printf("HBI_GetFpdCfgInfo:width=%d,height=%d\n", m_imgApro.nwidth, m_imgApro.nheight);
		
		/*
		* Retrieve IP and Port information (Endianness conversion required)
		*/

		unsigned short usValue = ((m_pLastRegCfg->m_EtherInfo.m_sDestUDPPort & 0xff) << 8) | 
			((m_pLastRegCfg->m_EtherInfo.m_sDestUDPPort >> 8) & 0xff);

		printf("\tSourceIP:%d.%d.%d.%d:%u\n",
			m_pLastRegCfg->m_EtherInfo.m_byDestIP[0],
			m_pLastRegCfg->m_EtherInfo.m_byDestIP[1],
			m_pLastRegCfg->m_EtherInfo.m_byDestIP[2],
			m_pLastRegCfg->m_EtherInfo.m_byDestIP[3],
			usValue);

		usValue = ((m_pLastRegCfg->m_EtherInfo.m_sSourceUDPPort & 0xff) << 8) | 
			((m_pLastRegCfg->m_EtherInfo.m_sSourceUDPPort >> 8) & 0xff);
		
		printf("\tDestIP:%d.%d.%d.%d:%u\n",
			m_pLastRegCfg->m_EtherInfo.m_bySourceIP[0],
			m_pLastRegCfg->m_EtherInfo.m_bySourceIP[1],
			m_pLastRegCfg->m_EtherInfo.m_bySourceIP[2],
			m_pLastRegCfg->m_EtherInfo.m_bySourceIP[3],
			usValue);

		/*
	   * Retrieve and print the panel size and type
	   */

		if (m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize == 0x01)
			printf("\tPanelSize:0x%02x,fpd type:4343-140um\n", m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
		
		else if (m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize == 0x02)
			printf("\tPanelSize:0x%02x,fpd type:3543-140um\n", m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
		
		else if (m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize == 0x03)
			printf("\tPanelSize:0x%02x,fpd type:1613-125um\n", m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
		
		else if (m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize == 0x04)
			printf("\tPanelSize:0x%02x,fpd type:3030-140um\n", m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
		
		else if (m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize == 0x05)
			printf("\tPanelSize:0x%02x,fpd type:2530-85um\n",  m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
		
		else if (m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize == 0x06)
			printf("\tPanelSize:0x%02x,fpd type:3025-140um\n", m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
		
		else if (m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize == 0x07)
			printf("\tPanelSize:0x%02x,fpd type:4343-100um\n", m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
		
		else if (m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize == 0x08)
			printf("\tPanelSize:0x%02x,fpd type:2530-75um\n",  m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
		
		else if (m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize == 0x09)
			printf("\tPanelSize:0x%02x,fpd type:2121-200um\n", m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
		
		else if (m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize == 0x0a)
			printf("\tPanelSize:0x%02x,fpd type:1412-50um\n",  m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
		
		else if (m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize == 0x0b)
			printf("\tPanelSize:0x%02x,fpd type:0606-50um\n",  m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
		
		else
			printf("\tPanelSize:0x%02x,invalid fpd type\n", m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
		
		/*
		 * Update image dimensions
		 */

		if (m_uDefaultFpdid == 0)
		{
			if (m_imgWA != m_pLastRegCfg->m_SysBaseInfo.m_sImageWidth) 
				m_imgWA = m_pLastRegCfg->m_SysBaseInfo.m_sImageWidth;
			if (m_imgHA != m_pLastRegCfg->m_SysBaseInfo.m_sImageHeight) 
				m_imgHA = m_pLastRegCfg->m_SysBaseInfo.m_sImageHeight;

			printf("\twidth=%d,hight=%d\n", m_imgWA, m_imgHA);
		}

		else
		{
		}

		printf("\tdatatype is unsigned char.\n");
		printf("\tdatabit is 16bits.\n");
		printf("\tdata is little endian.\n");

		/*
		 * Retrieve and print the current trigger mode
		 */

		if (m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x01)
			printf("\tstatic software trigger.[0x%02x]\n", m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);
		
		else if (m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x03)
			printf("\tstatic hvg trigger.[0x%02x]\n", m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);
		
		else if (m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x04)
			printf("\tFree AED trigger mode.[0x%02x]\n", m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);
		
		else if (m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x05)
			printf("\tDynamic:Hvg Sync mode.[0x%02x]\n", m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);
		
		else if (m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x06)
			printf("\tDynamic:Fpd Sync mode.[0x%02x]\n", m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);
		
		else if (m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x07)
			printf("\tDynamic:Fpd Continue mode.[0x%02x]\n", m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);
		
		else if (m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x08)
			printf("\tStatic:SAEC mode.[0x%02x]\n", m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);
		
		else
			printf("\tother trigger mode.[0x%02x]\n", m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);

		printf("\tPre Acquisition Delay Time.%u\n", m_pLastRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime);
		
		/*
		* correction enable
		*/

		if (m_pLastRegCfg->m_ImgCaliCfg.m_byOffsetCorrection == 0x01)
			printf("\tFirmware offset correction disenable.[0x%02x]\n", m_pLastRegCfg->m_ImgCaliCfg.m_byOffsetCorrection);
		
		else if (m_pLastRegCfg->m_ImgCaliCfg.m_byOffsetCorrection == 0x02)
			printf("\tFirmware offset correction enable.[0x%02x]\n", m_pLastRegCfg->m_ImgCaliCfg.m_byOffsetCorrection);
		
		else
			printf("\tFirmware other offset correction enable.[0x%02x]\n", m_pLastRegCfg->m_ImgCaliCfg.m_byOffsetCorrection);
		
		if (m_pLastRegCfg->m_ImgCaliCfg.m_byGainCorrection == 0x01)
			printf("\tFirmware gain correction disenable.[0x%02x]\n", m_pLastRegCfg->m_ImgCaliCfg.m_byGainCorrection);
		
		else if (m_pLastRegCfg->m_ImgCaliCfg.m_byGainCorrection == 0x02)
			printf("\tFirmware gain correction enable.[0x%02x]\n", m_pLastRegCfg->m_ImgCaliCfg.m_byGainCorrection);
		
		else
			printf("\tFirmware gain offset correction enable.[0x%02x]\n", m_pLastRegCfg->m_ImgCaliCfg.m_byGainCorrection);
		
		if (m_pLastRegCfg->m_ImgCaliCfg.m_byDefectCorrection == 0x01)
			printf("\tFirmware defect correction disenable.[0x%02x]\n", m_pLastRegCfg->m_ImgCaliCfg.m_byDefectCorrection);
		
		else if (m_pLastRegCfg->m_ImgCaliCfg.m_byDefectCorrection == 0x02)
			printf("\tFirmware defect correction enable.[0x%02x]\n", m_pLastRegCfg->m_ImgCaliCfg.m_byDefectCorrection);
		
		else
			printf("\tFirmware defect offset correction enable.[0x%02x]\n", m_pLastRegCfg->m_ImgCaliCfg.m_byDefectCorrection);
		
		if (m_pLastRegCfg->m_ImgCaliCfg.m_byDummyCorrection == 0x01)
			printf("\tFirmware Dummy correction disenable.[0x%02x]\n", m_pLastRegCfg->m_ImgCaliCfg.m_byDummyCorrection);
		
		else if (m_pLastRegCfg->m_ImgCaliCfg.m_byDummyCorrection == 0x02)
			printf("\tFirmware Dummy correction enable.[0x%02x]\n", m_pLastRegCfg->m_ImgCaliCfg.m_byDummyCorrection);
		
		else
			printf("\tFirmware Dummy offset correction enable.[0x%02x]\n", m_pLastRegCfg->m_ImgCaliCfg.m_byDummyCorrection);

		// Set PGA Level
		if (m_fpd_base != NULL)
		{
			// Set Network Configuration (IP and Port)
			unsigned short usValue = ((m_pLastRegCfg->m_EtherInfo.m_sDestUDPPort & 0xff) << 8) | ((m_pLastRegCfg->m_EtherInfo.m_sDestUDPPort >> 8) & 0xff);
			printf("\tSourceIP:%d.%d.%d.%d:%u\n",
				m_pLastRegCfg->m_EtherInfo.m_byDestIP[0],
				m_pLastRegCfg->m_EtherInfo.m_byDestIP[1],
				m_pLastRegCfg->m_EtherInfo.m_byDestIP[2],
				m_pLastRegCfg->m_EtherInfo.m_byDestIP[3],
				usValue/*m_pLastRegCfg->m_EtherInfo.m_sDestUDPPort*/);

			usValue = ((m_pLastRegCfg->m_EtherInfo.m_sSourceUDPPort & 0xff) << 8) | ((m_pLastRegCfg->m_EtherInfo.m_sSourceUDPPort >> 8) & 0xff);
			printf("\tDestIP:%d.%d.%d.%d:%u\n",
				m_pLastRegCfg->m_EtherInfo.m_bySourceIP[0],
				m_pLastRegCfg->m_EtherInfo.m_bySourceIP[1],
				m_pLastRegCfg->m_EtherInfo.m_bySourceIP[2],
				m_pLastRegCfg->m_EtherInfo.m_bySourceIP[3],
				usValue/*m_pLastRegCfg->m_EtherInfo.m_sSourceUDPPort*/);

			////int j = sprintf(m_fpd_base->SrcIP, "%d.%d.%d.%d",
			////	m_pLastRegCfg->m_EtherInfo.m_byDestIP[0],
			////	m_pLastRegCfg->m_EtherInfo.m_byDestIP[1],
			////	m_pLastRegCfg->m_EtherInfo.m_byDestIP[2],
			////	m_pLastRegCfg->m_EtherInfo.m_byDestIP[3]);
			////m_fpd_base->SrcIP[j] = '\0';
			////j = sprintf(m_fpd_base->DstIP, "%d.%d.%d.%d",
			////	m_pLastRegCfg->m_EtherInfo.m_bySourceIP[0],
			////	m_pLastRegCfg->m_EtherInfo.m_bySourceIP[1],
			////	m_pLastRegCfg->m_EtherInfo.m_bySourceIP[2],
			////	m_pLastRegCfg->m_EtherInfo.m_bySourceIP[3]);
			////m_fpd_base->DstIP[j] = '\0';
			////// 高低位需要转换
			////usValue = ((m_pLastRegCfg->m_EtherInfo.m_sDestUDPPort & 0xff) << 8) | ((m_pLastRegCfg->m_EtherInfo.m_sDestUDPPort >> 8) & 0xff);
			////m_fpd_base->SrcPort = usValue/*m_pLastRegCfg->m_EtherInfo.m_sDestUDPPort*/;
			////usValue = ((m_pLastRegCfg->m_EtherInfo.m_sSourceUDPPort & 0xff) << 8) | ((m_pLastRegCfg->m_EtherInfo.m_sSourceUDPPort >> 8) & 0xff);
			////m_fpd_base->DstPort = usValue/*m_pLastRegCfg->m_EtherInfo.m_sSourceUDPPort*/;
			
			// Set Work Mode (Trigger Mode)
			if (m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x01)
				printf("\tstatic software trigger.[0x%02x]\n", m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);
			
			else if (m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x03)
				printf("\tstatic hvg trigger.[0x%02x]\n", m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);
			
			else if (m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x04)
				printf("\tFree AED trigger mode.[0x%02x]\n", m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);
			
			else if (m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x05)
				printf("\tDynamic:Hvg Sync mode.[0x%02x]\n", m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);
			
			else if (m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x06)
				printf("\tDynamic:Fpd Sync mode.[0x%02x]\n", m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);
			
			else if (m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x07)
				printf("\tDynamic:Fpd Continue mode.[0x%02x]\n", m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);
			
			else if (m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x08)
				printf("\tStatic:SAEC mode.[0x%02x]\n", m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);
			
			else
				printf("\tother trigger mode.[0x%02x]\n", m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);

			m_fpd_base->trigger_mode = m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode;

			printf("\tPre Acquisition Delay Time.%u\n", m_pLastRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime);
			
			// Set Correction Configuration
			if (m_fpd_base->offset_enable != m_pLastRegCfg->m_ImgCaliCfg.m_byOffsetCorrection)
				m_fpd_base->offset_enable = m_pLastRegCfg->m_ImgCaliCfg.m_byOffsetCorrection;

			if (m_fpd_base->gain_enable = m_pLastRegCfg->m_ImgCaliCfg.m_byGainCorrection)
				m_fpd_base->gain_enable = m_pLastRegCfg->m_ImgCaliCfg.m_byGainCorrection;

			if (m_fpd_base->defect_enable = m_pLastRegCfg->m_ImgCaliCfg.m_byDefectCorrection)
				m_fpd_base->defect_enable = m_pLastRegCfg->m_ImgCaliCfg.m_byDefectCorrection;

			if (m_fpd_base->dummy_enable = m_pLastRegCfg->m_ImgCaliCfg.m_byDummyCorrection)
				m_fpd_base->dummy_enable = m_pLastRegCfg->m_ImgCaliCfg.m_byDummyCorrection;

			if (0x00 == m_fpd_base->offset_enable)
				printf("\tNo Offset Correction\n");
			
			else if (0x01 == m_fpd_base->offset_enable)
				printf("\tSoftware Offset Correction\n");
			
			else if (0x02 == m_fpd_base->offset_enable)
				printf("\tFirmware PostOffset Correction\n");
			
			else if (0x03 == m_fpd_base->offset_enable)
				printf("\tFirmware PreOffset Correction\n");
			
			else
				printf("\tInvalid Offset Correction\n");

			if (0x00 == m_fpd_base->gain_enable)
				printf("\tNo Gain Correction\n");
			else if (0x01 == m_fpd_base->gain_enable)
				printf("\tSoftware Gain Correction\n");
			else if (0x02 == m_fpd_base->gain_enable)
				printf("\tFirmware Gain Correction\n");
			else
				printf("\tInvalid Gain Correction\n");

			if (0x00 == m_fpd_base->defect_enable)
				printf("\tNo Defect Correction\n");
			else if (0x01 == m_fpd_base->defect_enable)
				printf("\tSoftware Defect Correction\n");
			else if (0x02 == m_fpd_base->defect_enable)
				printf("\tFirmware Defect Correction\n");
			else
				printf("\tInvalid Defect Correction\n");

			if (0x00 == m_fpd_base->dummy_enable)
				printf("\tNo Dummy Correction\n");
			else if (0x01 == m_fpd_base->dummy_enable)
				printf("\tSoftware Dummy Correction\n");
			else if (0x02 == m_fpd_base->dummy_enable)
				printf("\tFirmware Dummy Correction\n");
			else
				printf("\tInvalid Dummy Correction\n");

			// Set PGA Level
			m_fpd_base->nPGALevel = GetPGA(m_pLastRegCfg->m_TICOFCfg.m_sTICOFRegister[26]);

			// Set Binning Mode
			int iscale = m_pLastRegCfg->m_SysCfgInfo.m_byBinning;

			if (iscale < 1 || iscale > 4)
			{
				theDemo->m_fpd_base->nBinning = 1;
				iscale = 1;
			}

			else
				theDemo->m_fpd_base->nBinning = iscale;

			// Set Pre-Acquisition Delay Time
			BYTE byte1 = 0, byte2 = 0, byte3 = 0, byte4 = 0;
			byte1 = BREAK_UINT32(m_pLastRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime, 0);
			byte2 = BREAK_UINT32(m_pLastRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime, 1);
			byte3 = BREAK_UINT32(m_pLastRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime, 2);
			byte4 = BREAK_UINT32(m_pLastRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime, 3);
			unsigned int unValue = BUILD_UINT32(byte4, byte3, byte2, byte1);
			m_fpd_base->prepareTime = unValue;

			// Set Acquisition Time Interval
			if (m_fpd_base->commType == 0 || m_fpd_base->commType == 3)
			{
				byte1 = BREAK_UINT32(m_pLastRegCfg->m_SysCfgInfo.m_unContinuousAcquisitionSpanTime, 0);
				byte2 = BREAK_UINT32(m_pLastRegCfg->m_SysCfgInfo.m_unContinuousAcquisitionSpanTime, 1);
				byte3 = BREAK_UINT32(m_pLastRegCfg->m_SysCfgInfo.m_unContinuousAcquisitionSpanTime, 2);
				byte4 = BREAK_UINT32(m_pLastRegCfg->m_SysCfgInfo.m_unContinuousAcquisitionSpanTime, 3);
			}

			else
			{
				byte1 = BREAK_UINT32(m_pLastRegCfg->m_SysCfgInfo.m_unSelfDumpingSpanTime, 0);
				byte2 = BREAK_UINT32(m_pLastRegCfg->m_SysCfgInfo.m_unSelfDumpingSpanTime, 1);
				byte3 = BREAK_UINT32(m_pLastRegCfg->m_SysCfgInfo.m_unSelfDumpingSpanTime, 2);
				byte4 = BREAK_UINT32(m_pLastRegCfg->m_SysCfgInfo.m_unSelfDumpingSpanTime, 3);
			}

			// Compute Live Acquisition Time and Max FPS
			unValue = BUILD_UINT32(byte4, byte3, byte2, byte1);
			if (unValue <= 0)
			{
				printf("\tlive aqc time=%u\n", unValue);
				unValue = 1000;
			}

			if (m_fpd_base->liveAcqTime != unValue) 
				m_fpd_base->liveAcqTime = unValue;

			unValue = (UINT)(1000 / unValue);
			if (unValue <= 0) unValue = 1;
			if (m_uMaxFps != unValue) m_uMaxFps = unValue;

			// Print Updated Values
			printf("\tnPGA=%d\n", m_fpd_base->nPGALevel);
			printf("\tnBinning=%d\n", m_fpd_base->nBinning);
			printf("\tm_uMaxFps=%u\n",   m_uMaxFps);
			printf("\tm_uLiveTime=%u\n", m_uLiveTime);
		}

		//// Update Display Information
		//PostMessage(WM_USER_CURR_FPD_INFO, (WPARAM)0, (LPARAM)0);
	}

	else
	{
		printf("Failed to retrieve firmware configuration parameters!\n");
		return false;
	}

	return true;
}

//Generate the current template file path based on detector settings
char* CHB_SDK_DEMO2008Dlg::getCurTemplatePath(char *curdir, const char *datatype, const char *temptype)
{
	// TODO: Perform external initialization here
	if (m_pLastRegCfg == NULL) return NULL;

	static char temp[MAX_PATH] = { 0 };
	BYTE byteBIN = 1;
	if (1 <= m_pLastRegCfg->m_SysCfgInfo.m_byBinning && m_pLastRegCfg->m_SysCfgInfo.m_byBinning <= 4)
		byteBIN = m_pLastRegCfg->m_SysCfgInfo.m_byBinning;

	// Retrieve configured frame rate
	BYTE byte1 = 0, byte2 = 0, byte3 = 0, byte4 = 0;
	unsigned int bytePanelSize = m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize;
	unsigned int uPGALevel = 0;

	if (bytePanelSize == 0x0a || bytePanelSize == 0x0b)
	{
		unsigned short usValue = ((m_pLastRegCfg->m_CMOSCfg.m_sCMOSRegister[0] & 0xff) << 8) 
			| ((m_pLastRegCfg->m_CMOSCfg.m_sCMOSRegister[0] >> 8) & 0xff);
		
		// 1412 Panel

		if (bytePanelSize == 0x0a)
		{
			if (usValue == 0x8d8) uPGALevel = 9;      // 1412 HFW
			else if (usValue == 0x8c8) uPGALevel = 8; // 1412 HFW
			else uPGALevel = 0;
		}

		// 0606 Panel
		else if (bytePanelSize == 0x0b)
		{
			if (usValue == 0x8e8) uPGALevel = 9;      // 0606 HFW
			else if (usValue == 0x8c8) uPGALevel = 8; // 0606 LFW
			else uPGALevel = 0;
		}
	}

	else
	{
		unsigned short usValue = ((m_pLastRegCfg->m_TICOFCfg.m_sTICOFRegister[26] & 0xff) << 8) | ((m_pLastRegCfg->m_TICOFCfg.m_sTICOFRegister[26] >> 8) & 0xff);
		BYTE select = (usValue >> 10) & 0x3f;

		if (select == 0x02) uPGALevel = 1;
		else if (select == 0x04) uPGALevel = 2;
		else if (select == 0x08) uPGALevel = 3;
		else if (select == 0x0c) uPGALevel = 4;
		else if (select == 0x10) uPGALevel = 5;
		else if (select == 0x18) uPGALevel = 6;
		else if (select == 0x3e) uPGALevel = 7;
		else uPGALevel = 0;
	}

	// high Voltage modified by mhyang 20210918
	//byte1 = BREAK_UINT32(m_pLastRegCfg->m_SysCfgInfo.m_unVoltage, 0);
	//byte2 = BREAK_UINT32(m_pLastRegCfg->m_SysCfgInfo.m_unVoltage, 1);
	//byte3 = BREAK_UINT32(m_pLastRegCfg->m_SysCfgInfo.m_unVoltage, 2);
	//byte4 = BREAK_UINT32(m_pLastRegCfg->m_SysCfgInfo.m_unVoltage, 3);
	//unsigned int uiValue = BUILD_UINT32(byte4, byte3, byte2, byte1);
	//if (uiValue >= 1000) uiValue = 999;
	//unsigned int uVoltage = uiValue;

	unsigned short uVoltage = ((m_pLastRegCfg->m_SysCfgInfo.m_usVoltage & 0xff) << 8) | ((m_pLastRegCfg->m_SysCfgInfo.m_usVoltage >> 8) & 0xff);
	if (uVoltage >= 1000) uVoltage = 999;

	int j = 0;
	if (curdir == NULL)
	{
		j = sprintf_s(temp, "S%02dB%dP%dV%03d", bytePanelSize, byteBIN, uPGALevel, uVoltage);
	}

	else
	{
		if (temptype == NULL)
			j = sprintf_s(temp, "%s\\%s\\S%02dB%dP%dV%03d", curdir, datatype, bytePanelSize, byteBIN, uPGALevel, uVoltage);
		else
			j = sprintf_s(temp, "%s\\%s\\S%02dB%dP%dV%03d\\%s", curdir, datatype, bytePanelSize, byteBIN, uPGALevel, uVoltage, temptype);
	}

	temp[j] = '\0';

	return temp;
}


void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnDownloadTemplate()
{
	// Handle template file download process
	printf("OnBnClickedBtnDownloadTemplate!\n");

	if (m_pFpd == NULL) 
	{
		::AfxMessageBox(_T("err:m_pFpd is NULL!"));
		return;
	}

	if (!m_IsOpen) {
		::AfxMessageBox(_T("err:m_pFpd is disconnect!"));
		return;
	}

	// Allocate memory for the download file structure
	if (downloadfile == NULL)
		downloadfile = new DOWNLOAD_FILE;

	if (downloadfile == NULL) {
		::AfxMessageBox(_T("Error:downloadfile is NULL!"));
		return;
	}

	// Generate template filename based on detector ID
	char hbitemp[24] = { 0 };
	memset(hbitemp, 0x00, 24);
	sprintf(hbitemp, "hbi_id%u_template", m_uDefaultFpdid);

	CString strPath = _T("");

	// Get the template path based on the configuration
	char *ptr = getCurTemplatePath(m_path, hbitemp, NULL);

	if (ptr == NULL)
	{
		strPath.Format(_T("%s\\"), m_path);
	}

	else
	{
		if (!PathFileExists(ptr))
		{
			strPath.Format(_T("%s\\"), m_path);
		}
		else
		{
			strPath.Format(_T("%s\\"), ptr);
		}
	}

	strPath.ReleaseBuffer();

	// Open file selection dialog for choosing a template file
	CString szFilter = _T("Template File|*.raw||");
	CFileDialog dlg(TRUE, _T("txt"), strPath, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL);
	
	if (dlg.DoModal() == IDCANCEL)
	{
		return;
	}

	// Get file details
	CString strFileName = dlg.GetFileTitle();
	CString strFilePath = dlg.GetPathName();
	CString strFileExe  = dlg.GetFileExt();

	// Determine the type of template file selected

	//if ((0 == strFileName.Compare("offsettemp")) && (0 == strFileExe.Compare("raw")))

	if ((0 == strFileName.FindOneOf("offset_")) && (0 == strFileExe.Compare("raw")))
	{
		downloadfile->emfiletype = OFFSET_TMP;
		memset(downloadfile->filepath, 0x00, MAX_PATH);
		sprintf(downloadfile->filepath, "%s", strFilePath.GetBuffer());
	}

	else if ((0 == strFileName.Compare("gaina")) && (0 == strFileExe.Compare("raw")))
	{
		downloadfile->emfiletype = GAIN_TMP;
		memset(downloadfile->filepath, 0x00, MAX_PATH);
		sprintf(downloadfile->filepath, "%s", strFilePath.GetBuffer());
	}

	else if ((0 == strFileName.Compare("defect")) && (0 == strFileExe.Compare("raw")))
	{
		downloadfile->emfiletype = DEFECT_TMP;
		memset(downloadfile->filepath, 0x00, MAX_PATH);
		sprintf(downloadfile->filepath, "%s", strFilePath.GetBuffer());
	}

	else
	{
		::AfxMessageBox(_T("Error:The file is not template' file!"));
		return;
	}

	strFileName.ReleaseBuffer();
	strFilePath.ReleaseBuffer();
	strFileExe.ReleaseBuffer();

	// User selects the binning type
	BYTE byBinnig = 1;
	CDlgBinningType dlgBinning;
	if (IDOK == dlgBinning.DoModal())
	{
		byBinnig = dlgBinning.m_uBinningType;
	}

	else
	{
		::AfxMessageBox(_T("warnning:Please select binning type!"));
		return;
	}

	if (byBinnig < 1 || byBinnig > 4) byBinnig = 1;
	if (downloadfile->nBinningtype != byBinnig) 
		downloadfile->nBinningtype = byBinnig;

	// Register the download progress callback function
	if (0 != HBI_RegProgressCallBack(m_pFpd, DownloadTemplateCBFun, (void *)this))
	{
		::AfxMessageBox(_T("err:HBI_RegProgressCallBack failed!"));
		return;
	}

	// Disable the download button during the process
	((CButton *)GetDlgItem(IDC_BTN_DOWNLOAD_TEMPLATE))->EnableWindow(FALSE);
	
	// Start the template download
	int ret = HBI_DownloadTemplate(m_pFpd, downloadfile);
	if (ret != HBI_SUCCSS) {
		::AfxMessageBox(_T("err:Download template failed!"));
	}

	else
		::AfxMessageBox(_T("Download template success!"));

	// Restore button state after the download
	((CButton *)GetDlgItem(IDC_BTN_DOWNLOAD_TEMPLATE))->SetWindowTextA(_T("Download Template"));
	((CButton *)GetDlgItem(IDC_BTN_DOWNLOAD_TEMPLATE))->EnableWindow(TRUE);
}

//Set the preparation time for single-frame acquisition
void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSetPrepareTm()
{
	this->UpdateData(TRUE);

	// Check if the detector object is initialized
	if (m_pFpd == NULL) {
		::AfxMessageBox("Error: Detector (m_pFpd) is NULL!");
		return;
	}

	// Retrieve the preparation time from UI
	int time = m_uPrepareTime;
	printf("Setting single-frame preparation time: %d ms\n", time);

	// Apply the preparation time setting to the detector
	int ret = HBI_SetSinglePrepareTime(m_pFpd, time);
	if (ret)
		::AfxMessageBox("Error: Failed to set preparation time!");
}

//Get the preparation time for single-frame acquisition
void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonGetPrepareTm()
{
	if (m_pFpd == NULL)
	{
		::AfxMessageBox("Error: Detector (m_pFpd) is NULL!");
		return;
	}

	// Retrieve the preparation time from the detector
	int time = 0;
	int ret = HBI_GetSinglePrepareTime(m_pFpd, &time);
	if (ret)
	{
		::AfxMessageBox("Error: Failed to retrieve preparation time!");
	}
	else
	{
		// Update internal variable with the retrieved time
		m_uPrepareTime = time;

		// Sync with the base configuration if it exists
		if (m_fpd_base != NULL)
		{
			if (m_fpd_base->prepareTime != m_uPrepareTime)
				m_fpd_base->prepareTime = m_uPrepareTime;
		}

		// Update UI with new data
		this->UpdateData(FALSE);
	}

	printf("Retrieved single-frame preparation time: %d ms\n", time);
}

//Set the packet interval time
void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnUpdatePacketIntervalTime()
{
	this->UpdateData(TRUE);

	// Check if the detector object is initialized
	if (m_pFpd == NULL) {
		::AfxMessageBox("Error: Detector (m_pFpd) is NULL!");
		return;
	}

	// Retrieve packet interval time from UI
	int time = m_upacketInterval;
	printf("Setting packet interval time: %d ms\n", time);

	// Apply the packet interval setting to the detector
	int ret = HBI_SetPacketIntervalTime(m_pFpd, time);
	if (ret)
		::AfxMessageBox("Error: Failed to set packet interval time!");
}

//Retrieve the packet interval time from the detector
void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnGetPacketIntervalTime()
{
	if (m_pFpd == NULL)
	{
		::AfxMessageBox("Error: Detector (m_pFpd) is NULL!");
		return;
	}

	// Retrieve the packet interval time
	int time = 0;
	int ret = HBI_GetPacketIntervalTime(m_pFpd, &time);
	if (ret)
	{
		::AfxMessageBox("Error: Failed to retrieve packet interval time!");
	}
	else
	{
		// Update internal variable with the retrieved time
		m_upacketInterval = time;

		// Sync with the base configuration if it exists
		if (m_fpd_base != NULL)
		{
			if (m_fpd_base->packetInterval != m_upacketInterval)
				m_fpd_base->packetInterval = m_upacketInterval;
		}

		// Update UI with new data
		this->UpdateData(FALSE);
	}

	printf("Retrieved packet interval time: %d ms\n", time);
}

//Callback for Downloading Template
int CHB_SDK_DEMO2008Dlg::DownloadTemplateCBFun(unsigned char command, int code, void* inContext)
{
	// Ensure theDemo object exists
	if (theDemo == NULL) {
		printf("Error: theDemo is NULL!\n");
		return 0;
	}

	int len = 0;
	switch (command)
	{
	case ECALLBACK_UPDATE_STATUS_START:
		// Event triggered at the start of the download process
		printf("DownloadTemplateCBFun: Start downloading...\n");
		len = sprintf(theDemo->evnet_msg, "Starting download...");
		theDemo->evnet_msg[len] = '\0';
		theDemo->PostMessage(WM_DOWNLOAD_TEMPLATE_CB_MSG, (WPARAM)command, (LPARAM)theDemo->evnet_msg);
		break;

	case ECALLBACK_UPDATE_STATUS_PROGRESS:
		// Event triggered to update progress percentage
		len = sprintf(theDemo->evnet_msg, "Progress: %d%%", code);
		theDemo->evnet_msg[len] = '\0';
		theDemo->PostMessage(WM_DOWNLOAD_TEMPLATE_CB_MSG, (WPARAM)command, (LPARAM)theDemo->evnet_msg);
		break;

	case ECALLBACK_UPDATE_STATUS_RESULT: 
		// Event triggered upon completion or failure of the download process

		if ((0 <= code) && (code <= 6))
		{       
			if (code == 0) {       
				printf("DownloadOffsetCBFun:%s\n", "download offset template!");
				len = sprintf(theDemo->evnet_msg, "offset temp");
			}
			else if (code == 1) {  
				printf("DownloadOffsetCBFun:%s\n", "download gain template!");
				len = sprintf(theDemo->evnet_msg, "gain temp");
			}
			else if (code == 2) {  
				printf("DownloadOffsetCBFun:%s\n", "download defect template!");
				len = sprintf(theDemo->evnet_msg, "defect temp");
			}
			else if (code == 3) {  
				printf("DownloadOffsetCBFun:%s\n", "download offset finish!");
				len = sprintf(theDemo->evnet_msg, "offset finish");
			}
			else if (code == 4) {  
				printf("DownloadOffsetCBFun:%s\n", "download gain finish!");
				len = sprintf(theDemo->evnet_msg, "gain finish");
			}
			else if (code == 5) {  
				printf("DownloadOffsetCBFun:%s\n", "download defect finish!");
				len = sprintf(theDemo->evnet_msg, "defect finish");
			}
			else/* if (code == 6)*/ {  
				printf("DownloadOffsetCBFun:%s\n", "Download finish and sucess!");
				len = sprintf(theDemo->evnet_msg, "success");
			}
			theDemo->evnet_msg[len] = '\0';
		}
		else 
		{
			// Handle failure cases
			if (code == -1) {
				printf("err:DownloadOffsetCBFun:%s\n", "wait event other error!");
				len = sprintf(theDemo->evnet_msg, "other error");
			}

			else if (code == -2) {
				printf("err:DownloadOffsetCBFun:%s\n", "timeout!");
				len = sprintf(theDemo->evnet_msg, "timeout");
			}

			else if (code == -3) {
				printf("err:DownloadOffsetCBFun:%s\n", "downlod offset failed!");
				len = sprintf(theDemo->evnet_msg, "offset failed");
			}

			else if (code == -4) {
				printf("err:DownloadOffsetCBFun:%s\n", "downlod gain failed!");
				len = sprintf(theDemo->evnet_msg, "gain failed");
			}

			else if (code == -5) {
				printf("err:DownloadOffsetCBFun:%s\n", "downlod defect failed!");
				len = sprintf(theDemo->evnet_msg, "defect failed");
			}

			else if (code == -6) {
				printf("err:DownloadOffsetCBFun:%s\n", "Download failed");
				len = sprintf(theDemo->evnet_msg, "Download failed");
			}

			else if (code == -7) {
				printf("err:DownloadOffsetCBFun:%s\n", "read offset failed!");
				len = sprintf(theDemo->evnet_msg, "err:read failed");
			}

			else if (code == -8) {
				printf("err:DownloadOffsetCBFun:%s\n", "read gain failed!");
				len = sprintf(theDemo->evnet_msg, "err:read gain");
			}

			else if (code == -9) {
				printf("err:DownloadOffsetCBFun:%s\n", "read defect failed!");
				len = sprintf(theDemo->evnet_msg, "err:read defect");
			}
			else {
				printf("err:DownloadOffsetCBFun:unknow error,%d\n", code);
				len = sprintf(theDemo->evnet_msg, "unknow error");
			}

			theDemo->evnet_msg[len] = '\0';
		}

		theDemo->PostMessage(WM_DOWNLOAD_TEMPLATE_CB_MSG, (WPARAM)command, (LPARAM)theDemo->evnet_msg);
		break;

	default:                              // unusual
		// Handle any unexpected events
		printf("handleProgressFun:unusual,retcode=%d\n", code);
		break;
	}

	return 1;
}

//Handling the Template Download Callback Message
LRESULT CHB_SDK_DEMO2008Dlg::OnDownloadTemplateCBMessage(WPARAM wParam, LPARAM lParam)
{
	// Retrieve the message from the callback
	char* ptr = (char*)lParam;

	// Update the button text with the received message
	SetDlgItemText(IDC_BTN_DOWNLOAD_TEMPLATE, ptr);

	return 0;
}

//Initializing Base Configuration
int CHB_SDK_DEMO2008Dlg::init_base_cfg()
{
	// Set default detector ID to 0
	m_uDefaultFpdid = 0;

	// Create a new base configuration object
	CFPD_BASE_CFG* pcfg = new CFPD_BASE_CFG;
	if (pcfg != NULL)
	{
		pcfg->detectorId = 0; // Set default detector ID
		m_fpdbasecfg.push_back(pcfg); // Store in the configuration list
	}

	// Validate configuration size
	size_t nsize = m_fpdbasecfg.size();
	if (1 != nsize)
	{
		free_base_cfg(); // Cleanup if initialization fails
		return 0;
	}

	return 1;
}

//Freeing Base Configuration
void CHB_SDK_DEMO2008Dlg::free_base_cfg()
{
	CFPD_BASE_CFG* pcfg = NULL;
	size_t nsize = m_fpdbasecfg.size();

	// Iterate through stored configurations and free memory
	for (size_t i = 0; i < nsize; i++)
	{
		pcfg = m_fpdbasecfg[i];
		if (pcfg != NULL)
		{
			//pcfg->RESET();

			delete pcfg; // Free memory
			pcfg = NULL;
		}
	}

	// Clear the vector to release allocated memory
	if (!m_fpdbasecfg.empty())
		vector<CFPD_BASE_CFG*>().swap(m_fpdbasecfg);
}

//Reading Configuration from DETECTOR_CFG.INI
int CHB_SDK_DEMO2008Dlg::read_ini_cfg()
{
	// Ensure only one configuration exists
	unsigned int usize = theDemo->m_fpdbasecfg.size();
	if (usize != 1)
	{
		return 0;
	}

	// Define the configuration file path
	char szPath[MAX_PATH] = { 0 };
	memset(szPath, 0x00, MAX_PATH);
	sprintf(szPath, "%s\\DETECTOR_CFG.INI", theDemo->m_path);

	// Pointer to store the base configuration
	CFPD_BASE_CFG* pcfg = NULL;

	// Check if the configuration file exists
	if (!PathFileExists(szPath))
	{
		// Reset support for dual detectors and default detector ID
		if (m_bSupportDual) 
			m_bSupportDual  = false;
		if (m_uDefaultFpdid != 0) 
			m_uDefaultFpdid = 0;

		// Reset username and load default values
		memset(m_username, 0x00, 64);
		pcfg = m_fpd_base;
		if (pcfg != NULL)
		{
			strcpy(m_username, "Shanghai Hua Ying Image Technology Co., Ltd.");
			pcfg->detectorId = 0;
			pcfg->commType = 0;
			memset(pcfg->DstIP, 0x00, 16);

			strcpy(pcfg->DstIP, "192.168.10.40");
			pcfg->DstPort = 32897;
			memset(pcfg->SrcIP, 0x00, 16);

			strcpy(pcfg->SrcIP, "192.168.10.20");
			pcfg->DstPort = 32896;
		}
	}
	else
	{
		// Read configuration parameters from the `.ini` file
		GetPrivateProfileString("FPD_CFG", "USER_NAME", "上海昊博影像科技有限公司", m_username, 63, szPath);
		
		// Reset support for dual detectors and default detector ID
		if (m_bSupportDual) 
			m_bSupportDual = false;
		if (m_uDefaultFpdid != 0) 
			m_uDefaultFpdid = 0;

		// Load settings into the configuration structure
		pcfg = m_fpd_base;
		if (pcfg != NULL)
		{
			pcfg->detectorId = 0;
			pcfg->commType = GetPrivateProfileInt("DETECTOR_A", "COMM_TYPE", 0, szPath);
			memset(pcfg->DstIP, 0x00, 16);

			GetPrivateProfileString("DETECTOR_A", "DETECTOR_IP", "192.168.10.40", pcfg->DstIP, 16, szPath);
			pcfg->DstPort = GetPrivateProfileInt("DETECTOR_A", "DETECTOR_PORT", 32897, szPath);
			memset(pcfg->SrcIP, 0x00, 16);

			GetPrivateProfileString("DETECTOR_A", "LOCAL_IP", "192.168.10.20", pcfg->SrcIP, 16, szPath);
			pcfg->SrcPort = GetPrivateProfileInt("DETECTOR_A", "LOCAL_PORT", 32896, szPath);
		}

		// Update the window title with the username
		this->SetWindowTextA(m_username);
	}
	return 1;
}

//Getting Configuration Pointer by ID
CFPD_BASE_CFG *CHB_SDK_DEMO2008Dlg::get_base_cfg_ptr(int id)
{
	CFPD_BASE_CFG *pcfg = NULL;

	vector<CFPD_BASE_CFG *>::iterator iter = m_fpdbasecfg.begin();

	// Iterate through the stored configurations and return the matching one
	for (iter; iter != m_fpdbasecfg.end(); ++iter)
	{
		if (*iter != NULL)
		{
			if ((*iter)->detectorId == id)
			{
				pcfg = *iter;
				break;
			}
		}
	} 

	return pcfg;
}

// Getting Default Detector Configuration
CFPD_BASE_CFG *CHB_SDK_DEMO2008Dlg::get_default_cfg_ptr()
{
	CFPD_BASE_CFG *pcfg = NULL;

	// Ensure default detector ID is within valid range
	if (m_uDefaultFpdid >= DETECTOR_MAX_NUMBER)
	{
		return pcfg;
	}

	vector<CFPD_BASE_CFG *>::iterator iter = m_fpdbasecfg.begin();

	// Iterate through stored configurations and return the default one
	for (iter; iter != m_fpdbasecfg.end(); ++iter)
	{
		if (*iter != NULL)
		{
			if ((*iter)->detectorId == m_uDefaultFpdid)
			{
				pcfg = *iter;
				break;
			}
		}
	}
	return pcfg;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//   CFPD_BASE_CFG - Constructor
//////////////////////////////////////////////////////////////////////////////////////////////
CFPD_BASE_CFG::CFPD_BASE_CFG()
{
	detectorId = 0;          // Detector ID
	commType = 0;          // Communication type

	// Default IP addresses and ports
	memset(DstIP, 0x00, 16);
	strcpy(DstIP, "192.168.10.40");
	DstPort = 32897;

	memset(SrcIP, 0x00, 16);
	strcpy(SrcIP, "192.168.10.20");
	SrcPort = 32896;

	// Raw pixel dimensions
	m_rawpixel_x = 3072;
	m_rawpixel_y = 3072;

	// Data format settings
	databit = 0;
	datatype = 0;
	endian = 0;

	// Trigger and correction settings
	trigger_mode = 7;
	offset_enable = 0;
	gain_enable = 0;
	defect_enable = 0;
	dummy_enable = 0;

	// Image acquisition settings
	nBinning = 1;             // Binning mode
	nPGALevel = 6;            // PGA Level
	prepareTime = 1500;       // Preparation delay (in ms)
	selfDumpingTime = 1000;   // Self-dumping time (in ms)
	liveAcqTime = 1000;       // Continuous acquisition interval (in ms)

	// Device connection state
	m_bOpenOfFpd = false;     // true - connected, false - disconnected
	m_pFpdHand = NULL;        // Device handle
	m_pRegCfg = new RegCfgInfo;  // Configuration structure
}

//Destructor
CFPD_BASE_CFG::~CFPD_BASE_CFG()
{
	// Free allocated configuration structure
	if (m_pRegCfg != NULL)
	{
		delete m_pRegCfg;
		m_pRegCfg = NULL;
	}
}

//Reset Function
void CFPD_BASE_CFG::RESET()
{
	// Clear and reset configuration
	if (m_pRegCfg != NULL)
	{
		delete m_pRegCfg;
		m_pRegCfg = NULL;
	}
}

//Prints detailed information about the detector's configuration, 
// including IP addresses, image properties, 
// trigger mode, correction settings, 
// PGA level, and binning mode.

void CFPD_BASE_CFG::PRINT_FPD_INFO()
{
	printf("@=======================Detector:%d======================\n", detectorId);
	printf("Local  IP Addr:%s:%u\n", SrcIP, SrcPort);
	printf("Remote IP Addr:%s:%u\n", DstIP, DstPort);

	//Image Properties
	printf("Image property:\n\twidth=%d,hight=%d\n", m_rawpixel_x, m_rawpixel_y);

	//Data Type
	if (datatype == 0) printf("\tdatatype is unsigned char.\n");
	else if (datatype == 1) printf("\tdatatype is char.\n");
	else if (datatype == 2) printf("\tdatatype is unsigned short.\n");
	else if (datatype == 3) printf("\tdatatype is float.\n");
	else if (datatype == 4) printf("\tdatatype is double.\n");
	else printf("\tdatatype is not support.\n");\
		
	//Data Bit Depth
	if (databit == 0) printf("\tdatabit is 16bits.\n");
	else if (databit == 1) printf("\tdatabit is 14bits.\n");
	else if (databit == 2) printf("\tdatabit is 12bits.\n");
	else if (databit == 3) printf("\tdatabit is 8bits.\n");
	else printf("\tdatatype is unsigned char.\n");

	//Edianness
	if (endian == 0) printf("\tdata is little endian.\n");
	else printf("\tdata is bigger endian.\n");

	//Trigger Mode
	if (trigger_mode == 0x01)
		printf("trigger_mode[0x%02x]:\n\tstatic software trigger.\n", trigger_mode);
	else if (trigger_mode == 0x03)
		printf("trigger_mode[0x%02x]:\n\tstatic hvg trigger.\n", trigger_mode);
	else
		printf("trigger_mode[0x%02x]:\n\tother trigger mode.\n", trigger_mode);

	//Image Calibration Settings
	printf("Image Calibration:\n");
	if (0x00 == offset_enable)
		printf("\tNo Offset Correction\n");
	else if (0x01 == offset_enable)
		printf("\tSoftware Offset Correction\n");
	else if (0x02 == offset_enable)
		printf("\tFirmware PostOffset Correction\n");
	else if (0x03 == offset_enable)
		printf("\tFirmware PreOffset Correction\n");
	else
		printf("\tInvalid Offset Correction\n");

	//Gain Correction Mode
	if (0x00 == gain_enable)
		printf("\tNo Gain Correction\n");
	else if (0x01 == gain_enable)
		printf("\tSoftware Gain Correction\n");
	else if (0x02 == gain_enable)
		printf("\tFirmware Gain Correction\n");
	else
		printf("\tInvalid Gain Correction\n");

	//Defect Correction Mode
	if (0x00 == defect_enable)
		printf("\tNo Defect Correction\n");
	else if (0x01 == defect_enable)
		printf("\tSoftware Defect Correction\n");
	else if (0x02 == defect_enable)
		printf("\tFirmware Defect Correction\n");
	else
		printf("\tInvalid Defect Correction\n");

	//Dummy Correction Mode
	if (0x00 == dummy_enable)
		printf("\tNo Dummy Correction\n");
	else if (0x01 == dummy_enable)
		printf("\tSoftware Dummy Correction\n");
	else if (0x02 == dummy_enable)
		printf("\tFirmware Dummy Correction\n");
	else
		printf("\tInvalid Dummy Correction\n");
	
	//00-Invalid;
	// 01-0.6pC;
	// 02-1.2pC;
	// 03-2.4pC;
	// 04-3.6pC;
	// 05-4.8pC;
	// 06-7.2pC;
	// 07-9.6pC;
	// 08-LFW(CMOS);
	// 09-HFW(CMOS);

	//PGA Level
	if (0x00 == nPGALevel)
		printf("\t00-Invalid pC\n");
	else if (0x01 == nPGALevel)
		printf("\t01-0.6pC\n");
	else if (0x02 == nPGALevel)
		printf("\t02-1.2pC\n");
	else if (0x03 == nPGALevel)
		printf("\t03-2.4pC\n");
	else if (0x04 == nPGALevel)
		printf("\t04-3.6pC\n");
	else if (0x05 == nPGALevel)
		printf("\t05-4.8pC\n");
	else if (0x06 == nPGALevel)
		printf("\t06-7.2pC\n");
	else if (0x07 == nPGALevel)
		printf("\t07-9.6pC\n");
	else if (0x08 == nPGALevel)
		printf("\t08-LFW(CMOS)\n");
	else if (0x09 == nPGALevel)
		printf("\t09-HFW(CMOS)\n");
	else
		printf("\t0x%02x-Invalid pC\n", nPGALevel);

	// Binning Mode
	if (0x01 == nBinning)
		printf("\t0x01:1*1\n");
	else if (0x02 == nBinning)
		printf("\t0x02:2*2\n");
	else if (0x03 == nBinning)
		printf("\t0x03:3*3\n");
	else if (0x04 == nBinning)
		printf("\t0x04:4*4\n");
	else
		printf("\tInvalid Binning\n");

	printf("----------------------------------------------\n");
}

//Updates the trigger mode, binning mode, and acquisition time (FPS settings) for the detector.
void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnUpdateTriggerBinningFps()
{
	printf("OnBnClickedBtnUpdateTriggerBinningFps!\n");

	this->UpdateData(TRUE);

	if (m_pFpd == NULL) {
		::AfxMessageBox(_T("err:m_pFpd is NULL!"));
		return;
	}

	if (!m_IsOpen)
	{
		::AfxMessageBox(_T("err:disconnect!"));
		return;
	}

	// Get Trigger Mode
	int _triggerMode = STATIC_SOFTWARE_TRIGGER_MODE;
	int select = ((CComboBox *)GetDlgItem(IDC_COMBO_TRIGGER_MODE))->GetCurSel();

	if (select <= 0 || select >= 7) 
		_triggerMode = DYNAMIC_FPD_CONTINUE_MODE;
	else 
		_triggerMode = select;

	// Get Binning Mode
	int binning = ((CComboBox *)GetDlgItem(IDC_COMBO_BINNING))->GetCurSel();
	if (binning == 0) binning = 1;

	// Get Frame Interval (FPS)
	int time = m_uLiveTime;

	// Apply Settings
	int ret = HBI_TriggerBinningAcqTime(m_pFpd, _triggerMode, binning, time);

	if (ret == 0)
		printf("id:%u,HBI_TriggerBinningAcqTime succss!\n", m_uDefaultFpdid);
	else 
		printf("id:%u,HBI_TriggerBinningAcqTime failed!\n", m_uDefaultFpdid);
}

//Updates the trigger mode, PGA level, binning type, and frame acquisition time.
void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnUpdatePgaBinningFps()
{

	printf("OnBnClickedBtnUpdatePgaBinningFps!\n");

	this->UpdateData(TRUE);

	if (m_pFpd == NULL) {
		::AfxMessageBox(_T("err:m_pFpd is NULL!"));
		return;
	}

	if (!m_IsOpen)
	{
		::AfxMessageBox(_T("err:disconnect!"));
		return;
	}

	// Get PGA Level
	int nGainMode = ((CComboBox *)GetDlgItem(IDC_COMBO_PGA_LEVEL))->GetCurSel();

	// Get Binning Mode
	int binning = ((CComboBox *)GetDlgItem(IDC_COMBO_BINNING))->GetCurSel();
	if (binning == 0) binning = 1;

	// Get Frame Interval (FPS)
	int time = m_uLiveTime;

	// Apply Settings
	int ret = HBI_PgaBinningAcqTime(m_pFpd, nGainMode, binning, time);
	if (ret == 0)
		printf("HBI_PgaBinningAcqTime succss!\n");
	else
		printf("err:HBI_PgaBinningAcqTime failed!\n");
}

// Opens the Template Wizard, which manages calibration templates.
void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnUpdateTriggerPgaBinningFps()
{
	printf("OnBnClickedBtnUpdateTriggerPgaBinningFps!\n");

	this->UpdateData(TRUE);

	if (m_pFpd == NULL) {
		::AfxMessageBox(_T("err:m_pFpd is NULL!"));
		return;
	}

	if (!m_IsOpen)
	{
		::AfxMessageBox(_T("err:disconnect!"));
		return;
	}

	// Get Trigger Mode
	int _triggerMode = STATIC_SOFTWARE_TRIGGER_MODE;
	int select = ((CComboBox *)GetDlgItem(IDC_COMBO_TRIGGER_MODE))->GetCurSel();

	if (select <= 0 || select >= 7) 
		_triggerMode = DYNAMIC_FPD_CONTINUE_MODE;
	else 
		_triggerMode = select;

	// Get PGA Level
	int nGainMode = ((CComboBox *)GetDlgItem(IDC_COMBO_PGA_LEVEL))->GetCurSel();

	// Get Binning Mode
	int binning = ((CComboBox *)GetDlgItem(IDC_COMBO_BINNING))->GetCurSel();
	if (binning == 0) binning = 1;

	// Get Frame Interval (FPS)
	int time = m_uLiveTime;

	// Apply Settings
	int ret = HBI_TriggerPgaBinningAcqTime(m_pFpd, _triggerMode, nGainMode, binning, time);
	if (ret == 0)
		printf("HBI_TriggerPgaBinningAcqTime succss!\n");
	else {
		printf("err:HBI_TriggerPgaBinningAcqTime failed!\n");
	}
}

// Opens the Template Wizard, which is used for managing calibration templates.
void CHB_SDK_DEMO2008Dlg::OnFileOpenTemplateWizard()
{
	if (m_pFpd == NULL) {
		::AfxMessageBox(_T("err:m_pFpd is NULL!"));
		return;
	}
	if (!m_IsOpen)
	{
		::AfxMessageBox(_T("err:disconnect!"));
		return;
	}

	int ret = HBI_OpenTemplateWizard(m_pFpd);
	if (ret != HBI_SUCCSS)
	{
		if (HBI_ERR_NODEVICE == ret)
		{
			::AfxMessageBox(_T("warnning:Dll not initialized!"));
		}

		else if (HBI_ERR_DISCONNECT == ret)
		{
			::AfxMessageBox(_T("warnning:disconnect!"));
		}

		else if (HBI_ERR_INVALID_PARAMS == ret)
		{
			::AfxMessageBox(_T("err:pRomCfg is NULL!"));
		}

		else if (HBI_ERR_OPEN_WIZARD_FAILED == ret)
		{
			::AfxMessageBox(_T("err:open template wizard failed!"));
		}

		else if (HBI_ERR_WIZARD_ALREADY_EXIST == ret)
		{
			::AfxMessageBox(_T("warnning:template wizard already exist!"));
		}

		else
		{
			::AfxMessageBox(_T("err:other error!"));
		}
	}
}

//Displays a message that firmware update is not supported.
void CHB_SDK_DEMO2008Dlg::OnFirmwareUpdateTool()
{
	::AfxMessageBox(_T("warnning:Not support yet!"));
}

//Registers a callback function for processing updates.
int CHB_SDK_DEMO2008Dlg::DoRegUpdateCallbackFun(USER_CALLBACK_HANDLE_PROCESS handFun, void* pContext)
{
	if (handFun == NULL || pContext == NULL)
	{
		::AfxMessageBox(_T("err:handFun or pContext is NULL!"));
		return -1;
	}
	return HBI_RegProgressCallBack(m_pFpd, handFun, pContext);
}

