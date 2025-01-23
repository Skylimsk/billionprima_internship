
// HB_SDK_DEMO2008Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "HB_SDK_DEMO2008.h"
#include "HB_SDK_DEMO2008Dlg.h"
#include "CDlgBinningType.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CHB_SDK_DEMO2008Dlg* theDemo = NULL;
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedButtonGetFirmwareVer();
	afx_msg void OnBnClickedButtonGetSdkVer();
	afx_msg void OnBnClickedButtonGetFpdSn();

public:
	void updateFirmwareVersionInfo();
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

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


void CAboutDlg::updateFirmwareVersionInfo()
{
	if (theDemo != NULL)
	{
		if (theDemo->m_pFpd == NULL)
		{
			::AfxMessageBox("m_pFpd is NULL!");
			return;
		}
		//
		char buff[128];
		memset(buff, 0x00, 128);
		int ret = HBI_GetSDKVerion(theDemo->m_pFpd, buff);
		if (0 == ret)
		{
			((CStatic *)GetDlgItem(IDC_STATIC_SDK_VERSION))->SetWindowTextA(buff);
		}
		//
		if (!theDemo->m_IsOpen)
		{
			return;
		}
		//
		ret = HBI_GetFirmareVerion(theDemo->m_pFpd, buff);
		if (0 == ret)
		{
			((CStatic *)GetDlgItem(IDC_STATIC_FIRMWARE_VERSION))->SetWindowTextA(buff);
		}
		//
		ret = HBI_GetFPDSerialNumber(theDemo->m_pFpd, buff);
		if (0 == ret)
		{
			((CStatic *)GetDlgItem(IDC_STATIC_SERIAL_NUMBER))->SetWindowTextA(buff);
		}
	}
}


void CAboutDlg::OnBnClickedButtonGetFirmwareVer()
{
	// TODO: 在此添加控件通知处理程序代码
	if (theDemo != NULL)
	{
		if (theDemo->m_pFpd == NULL) 
		{
			::AfxMessageBox("m_pFpd is NULL!");
			return;
		}
		int ret = HBI_GetFirmareVerion(theDemo->m_pFpd, theDemo->szFirmVer);
		if (0 != ret) 
		{
			CString strStr = _T("");
			strStr.Format(_T("HBI_GetFirmareVerion Err,ret=%d"), ret);
			::AfxMessageBox(strStr);
			return;
		}
		((CStatic *)GetDlgItem(IDC_STATIC_FIRMWARE_VERSION))->SetWindowTextA(theDemo->szFirmVer);
	}
}


void CAboutDlg::OnBnClickedButtonGetSdkVer()
{
	// TODO: 在此添加控件通知处理程序代码
	if (theDemo != NULL)
	{
		if (theDemo->m_pFpd == NULL) {
			::AfxMessageBox("m_pFpd is NULL!");
			return;
		}

		int ret = HBI_GetSDKVerion(theDemo->m_pFpd, theDemo->szSdkVer);
		if (0 != ret)
		{
			CString strStr = _T("");
			strStr.Format(_T("HBI_GetSDKVerion Err,ret=%d"), ret);
			::AfxMessageBox(strStr);
			return;
		}
		((CStatic *)GetDlgItem(IDC_STATIC_SDK_VERSION))->SetWindowTextA(theDemo->szFirmVer);
	}
}


void CAboutDlg::OnBnClickedButtonGetFpdSn()
{
	// TODO: 在此添加控件通知处理程序代码
	if (theDemo != NULL)
	{
		if (theDemo->m_pFpd == NULL) {
			::AfxMessageBox("m_pFpd is NULL!");
			return;
		}
		memset(theDemo->szSeiralNum, 0x00, 16);
		int ret = HBI_GetFPDSerialNumber(theDemo->m_pFpd, theDemo->szSeiralNum);
		if (0 != ret)
		{
			CString strStr = _T("");
			strStr.Format(_T("HBI_GetFPDSerialNumber Err,ret=%d"), ret);
			::AfxMessageBox(strStr);
			return;
		}
		((CStatic *)GetDlgItem(IDC_STATIC_SERIAL_NUMBER))->SetWindowTextA(theDemo->szFirmVer);
	}
}

// CHB_SDK_DEMO2008Dlg dialog




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
	//
	theDemo = this;
	m_pFpd  = NULL;
	m_pLastRegCfg = NULL;
	m_fpd_base = NULL;
	m_imgWA = m_imgHA = 3072;
	m_ufpsA = 0;
	pIplimageA = NULL;
	m_nDetectorAStatus = FPD_DISCONN_STATUS;
	m_bSupportDual = false;
	m_uDefaultFpdid = 0;
	memset(m_username, 0x00, 64);
	//
	init_base_cfg();
	//
	picA_factor = 1.0;
	m_bShowPic = true;
	pPicWndA = NULL;
	//
	m_hEventA = NULL;
	m_hEventA = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hEventA == NULL)
	{
		printf("err:%s", "CreateEventA failed!");
	}
	//
	RunFlag = 1;
	m_ThreadhdlA = (HANDLE)_beginthreadex(NULL, 0, &ShowThreadProcFunA, (void *)this, CREATE_SUSPENDED, &m_uThreadFunIDA);
	//
	m_IsOpen = false;
	downloadfile = NULL;
	m_bIsDetectA = true;
	//获取系统默认背景颜色
	COLORREF dd = GetSysColor(COLOR_3DFACE);
	m_r = GetRValue(dd);
	m_g = GetGValue(dd);
	m_b = GetBValue(dd);
	////////////////////////////////////////////////////////////////////////////////
	// 初始化sdk实例并注册回调函数
	////////////////////////////////////////////////////////////////////////////////
	CFPD_BASE_CFG *pBaseCfg = get_base_cfg_ptr(m_uDefaultFpdid);
	if (pBaseCfg != NULL)
	{
		m_fpd_base = pBaseCfg;
		m_fpd_base->m_pFpdHand = HBI_Init(0);
	}
	m_pFpd = m_fpd_base->m_pFpdHand;
	//注册回调函数
	if (m_pFpd != NULL)
	{
		HBI_RegEventCallBackFun(m_pFpd, handleCommandEventA, (void *)this);
	}
}

CHB_SDK_DEMO2008Dlg::~CHB_SDK_DEMO2008Dlg()
{
	// 释放DLL资源
	if (m_pFpd != NULL)
	{
		HBI_Destroy(m_pFpd);
	}
	//HBI_DestroyEx();

	// 关闭显示线程
	CloseShowThread();
	//
	if (pIplimageA != NULL) 
	{
		cvReleaseImage(&pIplimageA);
		pIplimageA = NULL;
	}
	//
	if (downloadfile != NULL)
	{
		delete downloadfile;
		downloadfile = NULL;
	}
	// 释放变量
	free_base_cfg();
	m_pFpd = NULL;
	m_IsOpen = false;
	m_pLastRegCfg = NULL;
	m_fpd_base = NULL;

	////if (m_hBrush != NULL)
	////	DeleteObject(m_hBrush);
	////m_hBrush = NULL;
}

void CHB_SDK_DEMO2008Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SPIN_MAX_FRAME_SUM, m_ctlSpinAqcSum);
	DDX_Control(pDX, IDC_SPIN_DISCARD_FRAME_NUM, m_ctlSpinDiscardNum);
	DDX_Control(pDX, IDC_SPIN_MAX_FPS, m_ctlSpinMaxFps);
	DDX_Text(pDX, IDC_EDIT_MAX_FRAME_SUM, m_uFrameNum);
	DDX_Text(pDX, IDC_EDIT_DISCARD_FRAME_NUM, m_uDiscardNum);
	DDX_Text(pDX, IDC_EDIT_MAX_FPS, m_uMaxFps);
	DDX_Control(pDX, IDI_ICON_DISCONNECT, m_ctlConnStatus);
	DDX_Text(pDX, IDC_EDIT_PREPARE_TIME, m_uPrepareTime);
	DDX_Text(pDX, IDC_EDIT_LIVE_ACQ_TIME, m_uLiveTime);
	DDX_Text(pDX, IDC_EDIT_PACKET_INTERVAL_TIME, m_upacketInterval);
}

BEGIN_MESSAGE_MAP(CHB_SDK_DEMO2008Dlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_APP_ABOUT, &CHB_SDK_DEMO2008Dlg::OnAppAbout)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_USER_CURR_CONTROL_DATA, &CHB_SDK_DEMO2008Dlg::OnUpdateCurControlData)
	ON_COMMAND(ID_FILE_GENERATETEMPLATE, &CHB_SDK_DEMO2008Dlg::OnFileTemplate)
	ON_COMMAND(ID_FILE_SETTING, &CHB_SDK_DEMO2008Dlg::OnFileDetectorSetting)
	ON_BN_CLICKED(IDC_BTN_CONN, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnConn)
	ON_BN_CLICKED(IDC_BTN_DISCONN, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnDisconn)
	ON_BN_CLICKED(IDC_BTN_LIVE_ACQ, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnLiveAcq)
	ON_BN_CLICKED(IDC_BTN_STOP_LIVE_ACQ, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnStopLiveAcq)
	ON_BN_CLICKED(IDC_RADIO_SHOW_PIC, &CHB_SDK_DEMO2008Dlg::OnBnClickedRadioShowPic)
	ON_BN_CLICKED(IDC_RADIO_SAVE_PIC, &CHB_SDK_DEMO2008Dlg::OnBnClickedRadioSavePic)
	ON_BN_CLICKED(IDC_BUTTON_SINGLE_SHOT, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSingleShot)
	ON_BN_CLICKED(IDC_BUTTON_FIRMWARE_VER, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonFirmwareVer)
	ON_BN_CLICKED(IDC_BUTTON_SOFTWARE_VER, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSoftwareVer)
	ON_BN_CLICKED(IDC_BTN_GET_IMAGE_PROPERTY, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnGetImageProperty)
	ON_BN_CLICKED(IDC_BUTTON_GET_CONFIG, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonGetConfig)
	ON_BN_CLICKED(IDC_BTN_SET_TRIGGER_CORRECTION, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnSetTriggerCorrection)
	ON_BN_CLICKED(IDC_BTN_SET_TRIGGER_MODE, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnSetTriggerMode)
	ON_BN_CLICKED(IDC_BTN_FIRMWARE_CORRECT_ENABLE, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnCorrectEnable)
	ON_BN_CLICKED(IDC_BUTTON_SET_GAIN_MODE, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSetGainMode)
	ON_BN_CLICKED(IDC_BUTTON_GET_GAIN_MODE, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonGetGainMode)
	ON_BN_CLICKED(IDC_BUTTON_SET_PGA_LEVEL, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonGetLiveAcqTime)
	ON_BN_CLICKED(IDC_BUTTON_SET_BINNING, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSetBinning)
	ON_BN_CLICKED(IDC_BUTTON_SET_LIVE_ACQUISITION_TM, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSetLiveAcquisitionTm)
	ON_BN_CLICKED(IDC_BUTTON_GET_SERIAL_NUMBER, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonLiveAcquisitionTm)
	ON_EN_CHANGE(IDC_EDIT_MAX_FPS, &CHB_SDK_DEMO2008Dlg::OnEnChangeEditMaxFps)
	ON_BN_CLICKED(IDC_BTN_DOWNLOAD_TEMPLATE, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnDownloadTemplate)
	ON_BN_CLICKED(IDC_BUTTON_SET_PREPARE_TM, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSetPrepareTm)
	ON_BN_CLICKED(IDC_BUTTON_GET_PREPARE_TM, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonGetPrepareTm)
	ON_BN_CLICKED(IDC_BUTTON_GET_BINNING, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonGetBinning)
	ON_BN_CLICKED(IDC_BUTTON_SINGLE_PREPARE, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSinglePrepare)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_CONFIG, &CHB_SDK_DEMO2008Dlg::OnBnClickedButtonOpenConfig)
	ON_MESSAGE(WM_DETECTORA_CONNECT_STATUS, &CHB_SDK_DEMO2008Dlg::OnUpdateDetectorAStatus)
	ON_MESSAGE(WM_DOWNLOAD_TEMPLATE_CB_MSG, &CHB_SDK_DEMO2008Dlg::OnDownloadTemplateCBMessage)
	ON_BN_CLICKED(IDC_BTN_UPDATE_TRIGGER_BINNING_FPS, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnUpdateTriggerBinningFps)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE_PGA_BINNING_FPS, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnUpdatePgaBinningFps)
	ON_BN_CLICKED(IDC_BTN_UPDATE_TRIGGER_PGA_BINNING_FPS, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnUpdateTriggerPgaBinningFps)
	ON_COMMAND(ID_FILE_OPEN_TEMPLATE_WIZARD, &CHB_SDK_DEMO2008Dlg::OnFileOpenTemplateWizard)
	ON_COMMAND(ID_FIRMWARE_UPDATE_TOOL, &CHB_SDK_DEMO2008Dlg::OnFirmwareUpdateTool)
	ON_BN_CLICKED(IDC_BTN_UPDATE_PACKET_INTERVAL_TIME, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnUpdatePacketIntervalTime)
	ON_BN_CLICKED(IDC_BTN_GET_PACKET_INTERVAL_TIME, &CHB_SDK_DEMO2008Dlg::OnBnClickedBtnGetPacketIntervalTime)
END_MESSAGE_MAP()


// CHB_SDK_DEMO2008Dlg message handlers

BOOL CHB_SDK_DEMO2008Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	InitCtrlData(); // 初始化控件

	// 获取group-box A和B的区域
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
	
	// 获取当前模块的路径
	memset(m_path, 0x00, MAX_PATH);
	GetModuleFileName(NULL, m_path, MAX_PATH);

	// 
	((CButton *)GetDlgItem(IDC_RADIO_SHOW_PIC))->SetCheck(TRUE);

	// 当前执行文件目录
	PathRemoveFileSpec(m_path);

	// 读配置
	if (read_ini_cfg())
	{
		printf("err:read_ini_cfg failed!\n");
	}
	//// 初始化
	if (m_fpd_base != NULL)
	{
		m_pLastRegCfg = m_fpd_base->m_pRegCfg;

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

	//获取显示区域
	pPicWndA = GetDlgItem(IDC_STATIC_PICTURE_ZONE);
	if (pPicWndA != NULL)
	{
		pPicWndA->GetClientRect(&m_PicRectA);

		picctl_width  = m_PicRectA.Width();
		picctl_height = m_PicRectA.Height();
	}
    // 启动线程
	if (m_ThreadhdlA != NULL)  
	{
		ResumeThread(m_ThreadhdlA);
	}
	else 
	{
		printf("start ShowThreadProcFunA failed!\n");
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CHB_SDK_DEMO2008Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
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


// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CHB_SDK_DEMO2008Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


HBRUSH CHB_SDK_DEMO2008Dlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	//static CBrush brh(RGB(255, 0, 0));//静态画刷资源

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
			if ((pWnd->GetDlgCtrlID() == IDC_GROUP_BOX_CONN) || (pWnd->GetDlgCtrlID() == IDC_GROUP_BOX_CORRECTION) ||
				(pWnd->GetDlgCtrlID() == IDC_GROUP_BOX_BINNING) || (pWnd->GetDlgCtrlID() == IDC_GROUP_BOX_LIVE_ACQ) || 
				(pWnd->GetDlgCtrlID() == IDC_GROUP_BOX_SIGLE_ACQ) || (pWnd->GetDlgCtrlID() == IDC_GROUP_BOX_CUSTOM))
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
	//
	if (pWnd->GetDlgCtrlID() == IDC_RADIO_SHOW_PIC || pWnd->GetDlgCtrlID() == IDC_RADIO_SAVE_PIC)
	{
		pDC->SetBkColor(RGB(0, 0, 0));
		pDC->SetTextColor(RGB(0, 255, 0));
	}

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}


void CHB_SDK_DEMO2008Dlg::OnAppAbout()
{
	// TODO: Add your command handler code here
	CAboutDlg AboutDlg;
	AboutDlg.DoModal();
}


void CHB_SDK_DEMO2008Dlg::OnFileDetectorSetting()
{
	// TODO: 在此添加命令处理程序代码
	CDetectorSettingDlg dlg;
	dlg.DoModal();
}


void CHB_SDK_DEMO2008Dlg::OnFileTemplate()
{
	// TODO: 在此添加命令处理程序代码
	if (m_pFpd == NULL) {
		::AfxMessageBox(_T("HBI_Init failed!"));
		return;
	}
	//
	if (!m_IsOpen)
	{
		::AfxMessageBox(_T("warning:fpd is disconnect!"));
		return;
	}
	//
	CTemplateTool dlg;
	dlg.DoModal();
}


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


void CHB_SDK_DEMO2008Dlg::InitCtrlData()
{
	// 
	((CComboBox *)GetDlgItem(IDC_COMBO_CONN_OFFSETTEMP))->SetCurSel(0);

	//设置旋转按钮的位置范围
	m_ctlSpinAqcSum.SetRange(0, 3000);
	//设置旋转按钮的当前位置
	m_ctlSpinAqcSum.SetPos(0);
	//设置旋转按钮的当前基数
	m_ctlSpinAqcSum.SetBase(1);
	//设置旋转按钮的伙伴窗口
	m_ctlSpinAqcSum.SetBuddy(GetDlgItem(IDC_EDIT_MAX_FRAME_SUM));

	//设置旋转按钮的位置范围
	m_ctlSpinDiscardNum.SetRange(0, 10);
	//设置旋转按钮的当前位置
	m_ctlSpinDiscardNum.SetPos(0);
	//设置旋转按钮的当前基数
	m_ctlSpinDiscardNum.SetBase(1);
	//设置旋转按钮的伙伴窗口
	m_ctlSpinDiscardNum.SetBuddy(GetDlgItem(IDC_EDIT_DISCARD_FRAME_NUM));

	//设置旋转按钮的位置范围
	m_ctlSpinMaxFps.SetRange(1, 30);
	//设置旋转按钮的当前位置
	m_ctlSpinMaxFps.SetPos(0);
	//设置旋转按钮的当前基数
	m_ctlSpinMaxFps.SetBase(1);
	//设置旋转按钮的伙伴窗口
	m_ctlSpinMaxFps.SetBuddy(GetDlgItem(IDC_EDIT_MAX_FPS));

	//
	((CComboBox *)GetDlgItem(IDC_COMBO_LIVE_ACQ_MODE))->SetCurSel(1);

	m_hConnIco    = ::AfxGetApp()->LoadIconA(IDI_ICON_CONNECT);
	m_hDisConnIco = ::AfxGetApp()->LoadIconA(IDI_ICON_DISCONNECT);
	m_hReadyIco   = ::AfxGetApp()->LoadIconA(IDI_ICON_READY);
	m_hbusyIco    = ::AfxGetApp()->LoadIconA(IDI_ICON_BUSY);
	m_hprepareIco = ::AfxGetApp()->LoadIconA(IDI_ICON_PREPARE);
	//
	m_hExposeIco     = ::AfxGetApp()->LoadIconA(IDI_ICON_EXPOSE);
	m_hOffsetIco     = ::AfxGetApp()->LoadIconA(IDI_ICON_CONTINUE_OFFSET);
	m_hConReadyIco   = ::AfxGetApp()->LoadIconA(IDI_ICON_CONTINUE_READY);
	m_hGainAckIco    = ::AfxGetApp()->LoadIconA(IDI_ICON_GAIN_ACK);
	m_hDefectAckIco  = ::AfxGetApp()->LoadIconA(IDI_ICON_DEFECT_ACK);
	m_hOffsetAckIco  = ::AfxGetApp()->LoadIconA(IDI_ICON_OFFSET_ACK);
	m_hUpdateFirmIco = ::AfxGetApp()->LoadIconA(IDI_ICON_UPDATE_FIRMWARE);
	m_hRetransIco    = ::AfxGetApp()->LoadIconA(IDI_ICON_RETRANS_MISS);
}


int CHB_SDK_DEMO2008Dlg::GetPGA(unsigned short usValue)
{
	unsigned short gainMode = ((usValue & 0xff) << 8) | ((usValue >> 8) & 0xff);
	int nPGA = (gainMode >> 10) & 0x3f;
	if (nPGA == 0x02) return 1;
	else if (nPGA == 0x04) return 2;
	else if (nPGA == 0x08) return 3;
	else if (nPGA == 0x0c) return 4;
	else if (nPGA == 0x10) return 5;
	else if (nPGA == 0x18) return 6;
	else if (nPGA == 0x3e) return 7;
	else return 0;
}

// 缩放倍数
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


void CHB_SDK_DEMO2008Dlg::UpdateUI(int type)
{

}

// 方法一
void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnConn()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	int offset_tmp = ((CComboBox *)GetDlgItem(IDC_COMBO_CONN_OFFSETTEMP))->GetCurSel();
	//
	size_t nsize = m_fpdbasecfg.size();
	if (nsize == 0)
	{
		::AfxMessageBox(_T("err:m_fpdbasecfg error!"));
		return;
	}
	//
	if (m_uDefaultFpdid != 0) m_uDefaultFpdid = 0;
	//
	CFPD_BASE_CFG *pBaseCfg = m_fpd_base;
	if (pBaseCfg == NULL)
	{
		::AfxMessageBox(_T("err:m_fpdbasecfg error!"));
		return;
	}
	//
	if (m_pFpd == NULL)
	{
		if (m_IsOpen) m_IsOpen = false;
		::AfxMessageBox(_T("HBI_Init failed!"));
		return;
	}
	/*
	* 连接平板探测器
	*/
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
	//
	int ret = HBI_ConnectDetector(m_pFpd, commCfg, offset_tmp);
	if (ret != 0)
	{
		if (m_IsOpen) m_IsOpen = false;
		::AfxMessageBox(_T("err:HBI_ConnectDetector failed!"));
		return;
	}
	//
	conn_button_status();
	printf("HBI_ConnectDetector success!\n");
}

void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnDisconn()
{
	// TODO: Add your control notification handler code here
	if (m_pFpd != NULL)
	{
		HBI_DisConnectDetector(m_pFpd);
		//
		printf("HBI_DisConnectDetector DetectorA success!\n");
	}
	//
	CFPD_BASE_CFG *pBaseCfg = m_fpd_base;
	if (pBaseCfg != NULL)
	{
		if (pBaseCfg->m_bOpenOfFpd) pBaseCfg->m_bOpenOfFpd = false;
	}
	//
	disconn_button_status();
	//
	PostMessage(WM_DETECTORA_CONNECT_STATUS, (WPARAM)FPD_DISCONN_STATUS, (LPARAM)0);
}


void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnLiveAcq()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	printf("OnBnClickedBtnLiveAcq!\n");
	if (m_pFpd == NULL) {
		::AfxMessageBox(_T("err:m_pFpd is NULL!"));
		return;
	}
	//
	int nMode = ((CComboBox *)GetDlgItem(IDC_COMBO_LIVE_ACQ_MODE))->GetCurSel();
	////aqc_mode.eLivetype = PREOFFSET_IMAGE; //Offsete Template+Only Image
	////aqc_mode.eLivetype = ONLY_IMAGE; //Only Image
	////aqc_mode.eLivetype = ONLY_PREOFFSET; //Only Template
	if (nMode == 0) m_aqc_mode.eLivetype = PREOFFSET_IMAGE;
	else if (nMode == 1) m_aqc_mode.eLivetype = ONLY_IMAGE;
	else if (nMode == 2) m_aqc_mode.eLivetype = ONLY_PREOFFSET;    // overlap result is 16bit'image
	else if (nMode == 3) m_aqc_mode.eLivetype = OVERLAY_16BIT_IMG;
	else if (nMode == 4) m_aqc_mode.eLivetype = OVERLAY_32BIT_IMG; // overlap result is 32bit'image
	else 
		m_aqc_mode.eLivetype = ONLY_IMAGE;
	//
	m_aqc_mode.eAqccmd    = LIVE_ACQ_DEFAULT_TYPE;
	m_aqc_mode.nAcqnumber = m_uFrameNum;
	m_aqc_mode.nframeid   = 0;
	m_aqc_mode.ngroupno   = 0;
	m_aqc_mode.ndiscard   = m_uDiscardNum;

	printf("Do Live Acquisition:[cmd]=%d,[_acqMaxNum]=%d,[_discardNum]=%d,[_groupNum]=%d\n", m_aqc_mode.eAqccmd, m_aqc_mode.nAcqnumber, m_aqc_mode.ndiscard, m_aqc_mode.ngroupno);
	int ret = HBI_LiveAcquisition(m_pFpd, m_aqc_mode);
	if (ret == 0)
		printf("cmd=%d,sum=%d,num=%d,group=%d,Do Live Acquisition succss!\n", m_aqc_mode.eAqccmd, m_aqc_mode.nAcqnumber, m_aqc_mode.ndiscard, m_aqc_mode.ngroupno);
	else {
		printf("cmd=%d,sum=%d,num=%d,group=%d,Do Live Acquisition failed!\n", m_aqc_mode.eAqccmd, m_aqc_mode.nAcqnumber, m_aqc_mode.ndiscard, m_aqc_mode.ngroupno);
	}
}

void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnStopLiveAcq()
{
	// TODO: Add your control notification handler code here
	printf("OnBnClickedBtnStopLiveAcq!\n");
	if (m_pFpd == NULL) {
		::AfxMessageBox(_T("err:m_pFpd is NULL!"));
		return;
	}
	//
	int ret = HBI_StopAcquisition(m_pFpd);
	if (ret == 0)
		printf("Stop Live Acquisition succss!\n");
	else {
		printf("Stop Live Acquisition failed!\n");
	}
}


void CHB_SDK_DEMO2008Dlg::conn_button_status()
{
	((CButton *)GetDlgItem(IDC_BTN_CONN))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BTN_DISCONN))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BTN_LIVE_ACQ))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BTN_STOP_LIVE_ACQ))->EnableWindow(TRUE);
	//
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
	//
	((CButton *)GetDlgItem(IDC_BUTTON_GET_BINNING))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BUTTON_SET_PREPARE_TM))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BUTTON_GET_PREPARE_TM))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BUTTON_SINGLE_PREPARE))->EnableWindow(TRUE);
	//
	((CButton*)GetDlgItem(IDC_BTN_UPDATE_PACKET_INTERVAL_TIME))->EnableWindow(TRUE);
	((CButton*)GetDlgItem(IDC_BTN_GET_PACKET_INTERVAL_TIME))->EnableWindow(TRUE);
	//
	((CButton *)GetDlgItem(IDC_BTN_UPDATE_TRIGGER_BINNING_FPS))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BUTTON_UPDATE_PGA_BINNING_FPS))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BTN_UPDATE_TRIGGER_PGA_BINNING_FPS))->EnableWindow(TRUE);
	//
	if (!m_IsOpen) m_IsOpen = true;
}


void CHB_SDK_DEMO2008Dlg::disconn_button_status()
{
	((CButton *)GetDlgItem(IDC_BTN_CONN))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BTN_DISCONN))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BTN_LIVE_ACQ))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BTN_STOP_LIVE_ACQ))->EnableWindow(FALSE);
	//
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
	((CButton *)GetDlgItem(IDC_BTN_DOWNLOAD_TEMPLATE))->EnableWindow(FALSE);
	//
	((CButton *)GetDlgItem(IDC_BUTTON_GET_BINNING))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BUTTON_SET_PREPARE_TM))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BUTTON_GET_PREPARE_TM))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BUTTON_SINGLE_PREPARE))->EnableWindow(FALSE);
	//
	((CButton*)GetDlgItem(IDC_BTN_UPDATE_PACKET_INTERVAL_TIME))->EnableWindow(FALSE);
	((CButton*)GetDlgItem(IDC_BTN_GET_PACKET_INTERVAL_TIME))->EnableWindow(FALSE);
	//
	((CButton *)GetDlgItem(IDC_BTN_UPDATE_TRIGGER_BINNING_FPS))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BUTTON_UPDATE_PGA_BINNING_FPS))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BTN_UPDATE_TRIGGER_PGA_BINNING_FPS))->EnableWindow(FALSE);
	//
	if (m_IsOpen) m_IsOpen = false;
}


LRESULT CHB_SDK_DEMO2008Dlg::OnUpdateDetectorAStatus(WPARAM wparam, LPARAM lparam)
{
	int status = (int)wparam;
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


LRESULT CHB_SDK_DEMO2008Dlg::OnUpdateCurControlData(WPARAM wparam, LPARAM lparam)
{
	if (m_fpd_base != NULL)
	{
		////////////////////////////////////
		if (m_fpd_base->trigger_mode >= 1 && m_fpd_base->trigger_mode <= 7)
			((CComboBox *)GetDlgItem(IDC_COMBO_TRIGGER_MODE))->SetCurSel(m_fpd_base->trigger_mode);
		else
			((CComboBox *)GetDlgItem(IDC_COMBO_TRIGGER_MODE))->SetCurSel(0);
		////////////////////////////////////
	    if (m_fpd_base->offset_enable >= 0 && m_fpd_base->offset_enable <=3)
			((CComboBox *)GetDlgItem(IDC_COMBO_ENABLE_OFFSET))->SetCurSel(m_fpd_base->offset_enable);
		else
			((CComboBox *)GetDlgItem(IDC_COMBO_ENABLE_OFFSET))->SetCurSel(0);
		//
		if (m_fpd_base->gain_enable >= 0 && m_fpd_base->gain_enable <=2)
			((CComboBox *)GetDlgItem(IDC_COMBO_ENABLE_GAIN))->SetCurSel(m_fpd_base->gain_enable);
		else
			((CComboBox *)GetDlgItem(IDC_COMBO_ENABLE_GAIN))->SetCurSel(0);
		//
		if (m_fpd_base->gain_enable >= 0 && m_fpd_base->gain_enable <=2)
			((CComboBox *)GetDlgItem(IDC_COMBO_ENABLE_DEFECT))->SetCurSel(m_fpd_base->defect_enable);
		else
			((CComboBox *)GetDlgItem(IDC_COMBO_ENABLE_DEFECT))->SetCurSel(0);
		////////////////////////////////////
		CString strstr = _T("");
		// 
		if (m_uPrepareTime != m_fpd_base->prepareTime) m_uPrepareTime = m_fpd_base->prepareTime;
		if (m_uLiveTime != m_fpd_base->liveAcqTime) m_uLiveTime = m_fpd_base->liveAcqTime;
		if (m_upacketInterval != m_fpd_base->packetInterval) m_upacketInterval = m_fpd_base->packetInterval;
		//
		strstr.Format(_T("%u"), m_uLiveTime);
		((CEdit *)GetDlgItem(IDC_EDIT_LIVE_ACQ_TIME))->SetWindowTextA(strstr);
		strstr.ReleaseBuffer();
		//
		strstr.Format(_T("%u"), m_uPrepareTime);
		((CEdit *)GetDlgItem(IDC_EDIT_PREPARE_TIME))->SetWindowTextA(strstr);
		strstr.ReleaseBuffer();

		strstr.Format(_T("%u"), m_upacketInterval);
		((CEdit*)GetDlgItem(IDC_EDIT_PACKET_INTERVAL_TIME))->SetWindowTextA(strstr);
		strstr.ReleaseBuffer();

		//
		if (m_uLiveTime <= 0) m_uLiveTime = 1000;
		unsigned int unValue = (UINT)(1000 / m_uLiveTime);
		if (unValue <= 0) unValue = 1;
		if (theDemo->m_uMaxFps != unValue) theDemo->m_uMaxFps = unValue;
		//
		strstr.Format(_T("%u"), theDemo->m_uMaxFps);
		((CEdit *)GetDlgItem(IDC_EDIT_MAX_FPS))->SetWindowTextA(strstr);
		strstr.ReleaseBuffer();
		//
		if (m_fpd_base->nPGALevel >= 1 && m_fpd_base->nPGALevel <= 7)
			((CComboBox *)GetDlgItem(IDC_COMBO_PGA_LEVEL))->SetCurSel(m_fpd_base->nPGALevel);
		else
			((CComboBox *)GetDlgItem(IDC_COMBO_PGA_LEVEL))->SetCurSel(0);
		//
		if (m_fpd_base->nBinning >= 1 && m_fpd_base->nBinning <= 4)
			((CComboBox *)GetDlgItem(IDC_COMBO_BINNING))->SetCurSel(m_fpd_base->nBinning);
		else
			((CComboBox *)GetDlgItem(IDC_COMBO_BINNING))->SetCurSel(0);

	//	this->UpdateData(FALSE);
	}
	return S_OK;
}


void CHB_SDK_DEMO2008Dlg::UpdateCurControlData()
{
	if (m_fpd_base != NULL)
	{
		////////////////////////////////////
		if (m_fpd_base->trigger_mode >= 1 && m_fpd_base->trigger_mode <= 7)
			((CComboBox *)GetDlgItem(IDC_COMBO_TRIGGER_MODE))->SetCurSel(m_fpd_base->trigger_mode);
		else
			((CComboBox *)GetDlgItem(IDC_COMBO_TRIGGER_MODE))->SetCurSel(0);
		////////////////////////////////////
		if (m_fpd_base->offset_enable >= 0 && m_fpd_base->offset_enable <= 3)
			((CComboBox *)GetDlgItem(IDC_COMBO_ENABLE_OFFSET))->SetCurSel(m_fpd_base->offset_enable);
		else
			((CComboBox *)GetDlgItem(IDC_COMBO_ENABLE_OFFSET))->SetCurSel(0);
		//
		if (m_fpd_base->gain_enable >= 0 && m_fpd_base->gain_enable <= 2)
			((CComboBox *)GetDlgItem(IDC_COMBO_ENABLE_GAIN))->SetCurSel(m_fpd_base->gain_enable);
		else
			((CComboBox *)GetDlgItem(IDC_COMBO_ENABLE_GAIN))->SetCurSel(0);
		//
		if (m_fpd_base->gain_enable >= 0 && m_fpd_base->gain_enable <= 2)
			((CComboBox *)GetDlgItem(IDC_COMBO_ENABLE_DEFECT))->SetCurSel(m_fpd_base->defect_enable);
		else
			((CComboBox *)GetDlgItem(IDC_COMBO_ENABLE_DEFECT))->SetCurSel(0);
		////////////////////////////////////
		CString strstr = _T("");
		// 
		if (m_uPrepareTime != m_fpd_base->prepareTime) m_uPrepareTime = m_fpd_base->prepareTime;
		if (m_uLiveTime != m_fpd_base->liveAcqTime) m_uLiveTime = m_fpd_base->liveAcqTime;
		if (m_upacketInterval != m_fpd_base->packetInterval) m_upacketInterval = m_fpd_base->packetInterval;
		//
		strstr.Format(_T("%u"), theDemo->m_uLiveTime);
		((CEdit *)GetDlgItem(IDC_EDIT_LIVE_ACQ_TIME))->SetWindowTextA(strstr);
		strstr.ReleaseBuffer();
		//
		strstr.Format(_T("%u"), theDemo->m_uPrepareTime);
		((CEdit *)GetDlgItem(IDC_EDIT_PREPARE_TIME))->SetWindowTextA(strstr);
		strstr.ReleaseBuffer();
		//
		strstr.Format(_T("%u"), m_upacketInterval);
		((CEdit*)GetDlgItem(IDC_EDIT_PACKET_INTERVAL_TIME))->SetWindowTextA(strstr);
		strstr.ReleaseBuffer();
		//
		if (m_uLiveTime <= 0) m_uLiveTime = 1000;
		unsigned int unValue = (UINT)(1000 / m_uLiveTime);
		if (unValue <= 0) unValue = 1;
		if (theDemo->m_uMaxFps != unValue) theDemo->m_uMaxFps = unValue;
		//
		strstr.Format(_T("%u"), theDemo->m_uMaxFps);
		((CEdit *)GetDlgItem(IDC_EDIT_MAX_FPS))->SetWindowTextA(strstr);
		strstr.ReleaseBuffer();
		//
		if (m_fpd_base->nPGALevel >= 1 && m_fpd_base->nPGALevel <= 7)
			((CComboBox *)GetDlgItem(IDC_COMBO_PGA_LEVEL))->SetCurSel(m_fpd_base->nPGALevel);
		else
			((CComboBox *)GetDlgItem(IDC_COMBO_PGA_LEVEL))->SetCurSel(0);
		//
		if (m_fpd_base->nBinning >= 1 && m_fpd_base->nBinning <= 4)
			((CComboBox *)GetDlgItem(IDC_COMBO_BINNING))->SetCurSel(m_fpd_base->nBinning);
		else
			((CComboBox *)GetDlgItem(IDC_COMBO_BINNING))->SetCurSel(0);

		//	this->UpdateData(FALSE);
	}
}

int CHB_SDK_DEMO2008Dlg::SaveImage(unsigned char *imgbuff, int nbufflen, int type)
{
	/* 探测器分辨率大小 */
	int WIDTH  = m_imgWA;
	int HEIGHT = m_imgHA;
	//
	if ((imgbuff == NULL) || (nbufflen != (WIDTH * HEIGHT * 2)))
	{
		printf("数据异常!\n");
		return 0;
	}
	// 保存或显示
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
	//
	FILE *fp = fopen(filename, "wb");
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


/* 
* 注册回调函数
* 一般有两种：全局函数或静态成员函数。
* 本例使用了静态成员函数，全局函数也可以，用户根据实际情况使用。
* handleCommandEvent
*/
int CHB_SDK_DEMO2008Dlg::handleCommandEventA(void* _contex, int nDevId, unsigned char byteEventId, void *pvParam, int nlength, int param3, int param4)
{
	int ret = 1;
	if (theDemo == NULL) return ret;
	////////////////////////////////////////////////////////////////////////////////////////////////////	
	ImageData *imagedata    = NULL;
	RegCfgInfo *pRegCfg     = NULL;
	if ((byteEventId == ECALLBACK_TYPE_ROM_UPLOAD) || (byteEventId == ECALLBACK_TYPE_RAM_UPLOAD) ||
		(byteEventId == ECALLBACK_TYPE_FACTORY_UPLOAD))
	{
		if (pvParam == NULL || nlength == 0)
		{
			printf("注册回调函数参数异常!\n");
			return ret;
		}
		// 防止参数冲突
		if (0 != nDevId)
		{
			printf("warnning:handleCommandEventA:m_uDefaultFpdid=%u,nDevId=%d,eventid=0x%02x,nParam2=%d\n",theDemo->m_uDefaultFpdid, nDevId, byteEventId, nlength);
			return ret;
		}
		//
		if (theDemo->m_fpd_base != NULL)
		{
			pRegCfg = theDemo->m_fpd_base->m_pRegCfg;
		}	
	}
	else if ((byteEventId == ECALLBACK_TYPE_SINGLE_IMAGE) || (byteEventId == ECALLBACK_TYPE_MULTIPLE_IMAGE) ||
		(byteEventId == ECALLBACK_TYPE_PREVIEW_IMAGE) || (byteEventId == ECALLBACK_TYPE_OFFSET_TMP) ||
		(byteEventId == ECALLBACK_OVERLAY_16BIT_IMAGE) || (byteEventId == ECALLBACK_OVERLAY_32BIT_IMAGE))
	{
		if (pvParam == NULL || nlength == 0)
		{
			printf("注册回调函数参数异常!\n");
			return ret;
		}
		// 防止参数冲突
		if (0 != nDevId)
		{
			printf("warnning:handleCommandEventA:m_uDefaultFpdid=%u,nDevId=%d,eventid=0x%02x,nParam2=%d\n", theDemo->m_uDefaultFpdid, nDevId, byteEventId, nlength);
			return ret;
		}
		//
		imagedata = (ImageData *)pvParam;
	}
	else
	{}
	////////////////////////////////////////////////////////////////////////////////////////////////////	
	int status = -1;
	int j = 0;
	ret = 1;
	bool b = false;
	CHB_SDK_DEMO2008Dlg *pDlg = (CHB_SDK_DEMO2008Dlg *)_contex;
	switch (byteEventId)
	{
	case ECALLBACK_TYPE_FPD_STATUS: // 平板状态：连接/断开/ready/busy
		printf("ECALLBACK_TYPE_FPD_STATUS,recode=%d\n", nlength);
		if (theDemo != NULL)
		{
			CString strMsg = _T("");
			if (nlength <= 0 && nlength >= -11)
			{
				if (nlength == 0)
					printf("ECALLBACK_TYPE_FPD_STATUS,Err:网络未连接!\n");
				else if (nlength == -1)
					printf("ECALLBACK_TYPE_FPD_STATUS,Err:参数异常!\n");
				else if (nlength == -2)
					printf("ECALLBACK_TYPE_FPD_STATUS,Err:准备就绪的描述符数返回失败!\n");
				else if (nlength == -3)
					printf("ECALLBACK_TYPE_FPD_STATUS,Err:接收超时!\n");
				else if (nlength == -4)
					printf("ECALLBACK_TYPE_FPD_STATUS,Err:接收失败!\n");
				else if (nlength == -5)
					printf("ECALLBACK_TYPE_FPD_STATUS,Err:端口不可读!\n");
				else if (nlength == -6)
					printf("ECALLBACK_TYPE_FPD_STATUS,network card unusual!\n");
				else if (nlength == -7)
					printf("ECALLBACK_TYPE_FPD_STATUS,network card ok!\n");
				else if (nlength == -8)
					printf("ECALLBACK_TYPE_FPD_STATUS:update Firmware end!\n");
				else if (nlength == -9)
					printf("ECALLBACK_TYPE_FPD_STATUS:光纤已断开!!\n");
				else if (nlength == -10)
					printf("ECALLBACK_TYPE_FPD_STATUS:read ddr failed,try restarting the PCIe driver!\n");
				else /*if (nlength == -11)*/
					printf("ECALLBACK_TYPE_FPD_STATUS:ECALLBACK_TYPE_FPD_STATUS:is not jumb!\n");
				status = (int)FPD_DISCONN_STATUS;
			}
			else if (nlength == FPD_CONN_SUCCESS) { // connect
				printf("ECALLBACK_TYPE_FPD_STATUS,开始监听!\n");
				status = (int)FPD_CONN_SUCCESS;
			}
			else if (nlength == FPD_PREPARE_STATUS) { // ready
				printf("ECALLBACK_TYPE_FPD_STATUS,ready!\n");
				status = (int)FPD_PREPARE_STATUS;
			}
			else if (nlength == FPD_READY_STATUS) { // busy
				printf("ECALLBACK_TYPE_FPD_STATUS,busy!\n");
				status = (int)FPD_READY_STATUS;
			}
			else if (nlength == FPD_DOOFFSET_TEMPLATE) { // prepare
				printf("ECALLBACK_TYPE_FPD_STATUS,prepare!\n");
				status = (int)FPD_DOOFFSET_TEMPLATE;
			}
			else if (nlength == FPD_EXPOSE_STATUS) { // busy expose
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

			// ADD BY MH.YANG 2019/11/12
			if (status != -1)
			{
				// 更新图标
				theDemo->PostMessage(WM_DETECTORA_CONNECT_STATUS, (WPARAM)status, (LPARAM)0);

				// 触发断开消息
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
	case ECALLBACK_TYPE_SET_CFG_OK:
		printf("ECALLBACK_TYPE_SET_CFG_OK:Reedback set rom param succuss!\n");
		break;
	case ECALLBACK_TYPE_ROM_UPLOAD:/* 更新配置 */
		printf("ECALLBACK_TYPE_ROM_UPLOAD:\n");
		if (theDemo->m_fpd_base != NULL)
		{
			if (pRegCfg != NULL)
			{
				memset(pRegCfg, 0x00, sizeof(RegCfgInfo));
				memcpy(pRegCfg, (unsigned char*)pvParam, sizeof(RegCfgInfo));

				printf("\tSerial Number:%s\n", pRegCfg->m_SysBaseInfo.m_cSnNumber);

				// 高低位需要转换
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
				//
				if (pRegCfg->m_SysBaseInfo.m_byPanelSize == 0x01)
					printf("\tPanelSize:0x%02x,fpd type:4343-140um\n", pRegCfg->m_SysBaseInfo.m_byPanelSize);
				else if (pRegCfg->m_SysBaseInfo.m_byPanelSize == 0x02)
					printf("\tPanelSize:0x%02x,fpd type:3543-140um\n", pRegCfg->m_SysBaseInfo.m_byPanelSize);
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

				//
				if (theDemo->m_imgWA != pRegCfg->m_SysBaseInfo.m_sImageWidth)  theDemo->m_imgWA = pRegCfg->m_SysBaseInfo.m_sImageWidth;
				if (theDemo->m_imgHA != pRegCfg->m_SysBaseInfo.m_sImageHeight) theDemo->m_imgHA = pRegCfg->m_SysBaseInfo.m_sImageHeight;
				printf("\twidth=%d,hight=%d\n", theDemo->m_imgWA, theDemo->m_imgHA);
				printf("\tdatatype is unsigned char.\n");
				printf("\tdatabit is 16bits.\n");
				printf("\tdata is little endian.\n");

				theDemo->AutoResize();

				if (pRegCfg->m_SysBaseInfo.m_byPanelSize >= 0x01 && pRegCfg->m_SysBaseInfo.m_byPanelSize <= DETECTOR_TYPE_NUMBER)
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

					theDemo->m_fpd_base->trigger_mode = pRegCfg->m_SysCfgInfo.m_byTriggerMode;

					printf("\tPre Acquisition Delay Time.%u\n", pRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime);
					/*
					* correction enable
					*/
					if (theDemo->m_fpd_base->offset_enable != pRegCfg->m_ImgCaliCfg.m_byOffsetCorrection)
						theDemo->m_fpd_base->offset_enable = pRegCfg->m_ImgCaliCfg.m_byOffsetCorrection;

					if (theDemo->m_fpd_base->gain_enable = pRegCfg->m_ImgCaliCfg.m_byGainCorrection)
						theDemo->m_fpd_base->gain_enable = pRegCfg->m_ImgCaliCfg.m_byGainCorrection;

					if (theDemo->m_fpd_base->defect_enable = pRegCfg->m_ImgCaliCfg.m_byDefectCorrection)
						theDemo->m_fpd_base->defect_enable = pRegCfg->m_ImgCaliCfg.m_byDefectCorrection;

					if (theDemo->m_fpd_base->dummy_enable = pRegCfg->m_ImgCaliCfg.m_byDummyCorrection)
						theDemo->m_fpd_base->dummy_enable = pRegCfg->m_ImgCaliCfg.m_byDummyCorrection;

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
					// PGA档位
					theDemo->m_fpd_base->nPGALevel = theDemo->GetPGA(pRegCfg->m_TICOFCfg.m_sTICOFRegister[26]);

					// Binning类型
					int iscale = pRegCfg->m_SysCfgInfo.m_byBinning;
					if (iscale < 1 || iscale > 4)
					{
						theDemo->m_fpd_base->nBinning = 1;
						iscale = 1;
					}
					else
						theDemo->m_fpd_base->nBinning = iscale;
					//
					if (theDemo->m_imgWA != pRegCfg->m_SysBaseInfo.m_sImageWidth)  theDemo->m_imgWA = pRegCfg->m_SysBaseInfo.m_sImageWidth;
					if (theDemo->m_imgHA != pRegCfg->m_SysBaseInfo.m_sImageHeight) theDemo->m_imgHA = pRegCfg->m_SysBaseInfo.m_sImageHeight;

					//
					int j = (pRegCfg->m_SysBaseInfo.m_byPanelSize - 1);// 匹配静态数组
					if (theDemo->m_imgApro.nFpdNum != HB_FPD_SIZE[j].fpd_num)   theDemo->m_imgApro.nFpdNum = HB_FPD_SIZE[j].fpd_num;
					theDemo->m_imgApro.nwidth = HB_FPD_SIZE[j].fpd_width / iscale;
					theDemo->m_imgApro.nheight = HB_FPD_SIZE[j].fpd_height / iscale;
					theDemo->m_imgApro.packet_size = HB_FPD_SIZE[j].fpd_packet_size / (iscale * iscale);
					int frame_size = theDemo->m_imgApro.nwidth * theDemo->m_imgApro.nheight * 2;
					if (theDemo->m_imgApro.frame_size != frame_size) theDemo->m_imgApro.frame_size = frame_size; // 目前默认为16bits数据
					if (theDemo->m_imgApro.datatype != 0) theDemo->m_imgApro.datatype = 0;
					if (theDemo->m_imgApro.ndatabit != 0) theDemo->m_imgApro.ndatabit = 0;
					if (theDemo->m_imgApro.nendian != 0)  theDemo->m_imgApro.nendian = 0;

					printf("\tnBinning=%d\n", theDemo->m_fpd_base->nBinning);
					printf("\twidth=%d,hight=%d\n", theDemo->m_imgWA, theDemo->m_imgHA);
					printf("\tdatatype is unsigned char.\n");
					printf("\tdatabit is 16bits.\n");
					printf("\tdata is little endian.\n");
					printf("\tnPGA=%d\n", theDemo->m_fpd_base->nPGALevel);
					printf("\tnBinning=%d\n", theDemo->m_fpd_base->nBinning);
				}
				// packet interval time
				usValue = ((pRegCfg->m_SysCfgInfo.m_sPacketIntervalTm & 0xff) << 8) | ((pRegCfg->m_SysCfgInfo.m_sPacketIntervalTm >> 8) & 0xff);
				theDemo->m_fpd_base->packetInterval = usValue;
				// prepare 延时
				BYTE byte1 = 0, byte2 = 0, byte3 = 0, byte4 = 0;
				byte1 = BREAK_UINT32(pRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime, 0);
				byte2 = BREAK_UINT32(pRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime, 1);
				byte3 = BREAK_UINT32(pRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime, 2);
				byte4 = BREAK_UINT32(pRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime, 3);
				unsigned int unValue = BUILD_UINT32(byte4, byte3, byte2, byte1);
				theDemo->m_fpd_base->prepareTime = unValue;
				// 连续采集时间间隔和帧率计算
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
				//
				unValue = BUILD_UINT32(byte4, byte3, byte2, byte1);
				if (unValue <= 0)
				{
					printf("\tlive aqc time=%u\n", unValue);
					unValue = 1000;
				}

				if (theDemo->m_fpd_base->liveAcqTime != unValue) theDemo->m_fpd_base->liveAcqTime = unValue;

				unValue = (UINT)(1000 / unValue);
				if (unValue <= 0) unValue = 1;
				if (theDemo->m_uMaxFps != unValue) theDemo->m_uMaxFps = unValue;

				//
				printf("\tpacketInterval=%u\n", theDemo->m_fpd_base->packetInterval);
				printf("\tprepareTime=%u\n", theDemo->m_fpd_base->prepareTime);
				printf("\tm_uMaxFps=%u\n", theDemo->m_uMaxFps);
				printf("\tliveAcqTime=%u\n", theDemo->m_fpd_base->liveAcqTime);
				//
				theDemo->m_pLastRegCfg = pRegCfg;

				// 更新fpd_base
				theDemo->PostMessage(WM_USER_CURR_CONTROL_DATA, (WPARAM)0, (LPARAM)0);
				//theDemo->UpdateCurControlData();
			}
		}
		break;
	case ECALLBACK_TYPE_SINGLE_IMAGE:   // 单帧采集上图
	case ECALLBACK_TYPE_MULTIPLE_IMAGE: // 连续采集上图
		if (imagedata != NULL)
		{	
			if (theDemo->m_imgWA != imagedata->uwidth) theDemo->m_imgWA = imagedata->uwidth;
			if (theDemo->m_imgHA != imagedata->uheight) theDemo->m_imgHA = imagedata->uheight;
			//
			pRegCfg = theDemo->m_pLastRegCfg;
			if (pRegCfg != NULL)
			{
				if (pRegCfg->m_SysBaseInfo.m_sImageWidth != theDemo->m_imgWA)  pRegCfg->m_SysBaseInfo.m_sImageWidth = theDemo->m_imgWA;
				if (pRegCfg->m_SysBaseInfo.m_sImageHeight != theDemo->m_imgHA) pRegCfg->m_SysBaseInfo.m_sImageHeight = theDemo->m_imgHA;
			}
			//
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
				ret = theDemo->ShowImageA((unsigned short*)imagedata->databuff, (imagedata->datalen / sizeof(unsigned short)), imagedata->uframeid, param3);
				if (ret)
					printf("theDemo->ShowImageA succss!Frame ID:%05d\n", imagedata->uframeid);
				else
					printf("theDemo->ShowImageA failed!Frame ID:%05d\n", imagedata->uframeid);

				// 设置为信号状态
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
	case ECALLBACK_TYPE_GENERATE_TEMPLATE:
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
		else {// other
			printf("other:len=%d,nid=%d\n", nlength, param3);
		}
	}
	break;
	case ECALLBACK_OVERLAY_16BIT_IMAGE:
		if (theDemo->m_imgWA != imagedata->uwidth)  theDemo->m_imgWA = imagedata->uwidth;
		if (theDemo->m_imgHA != imagedata->uheight) theDemo->m_imgHA = imagedata->uheight;
		//
		pRegCfg = theDemo->m_pLastRegCfg;
		if (pRegCfg != NULL)
		{
			if (pRegCfg->m_SysBaseInfo.m_sImageWidth != theDemo->m_imgWA)  pRegCfg->m_SysBaseInfo.m_sImageWidth = theDemo->m_imgWA;
			if (pRegCfg->m_SysBaseInfo.m_sImageHeight != theDemo->m_imgHA) pRegCfg->m_SysBaseInfo.m_sImageHeight = theDemo->m_imgHA;
		}
		//
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

			// 设置为信号状态
			if (pDlg != NULL) {
				if (pDlg->m_hEventA != NULL)
				{
					::SetEvent(pDlg->m_hEventA);
				}
			}
		}
		break;
	case ECALLBACK_OVERLAY_32BIT_IMAGE:
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
	case ECALLBACK_TYPE_FILE_NOTEXIST:
		if (pvParam != NULL)
			printf("err:%s not exist!\n", (char *)pvParam);
		break;
	default:
		printf("ECALLBACK_TYPE_INVALVE,command=%02x\n", byteEventId);
		break;
	}

	return ret;
}


void CHB_SDK_DEMO2008Dlg::OnBnClickedRadioShowPic()
{
	// TODO: Add your control notification handler code here
	if (!m_bShowPic) m_bShowPic = true;
}


void CHB_SDK_DEMO2008Dlg::OnBnClickedRadioSavePic()
{
	// TODO: Add your control notification handler code here	
	if (m_bShowPic) m_bShowPic = false;

	// 创建保存目录
	char filename[MAX_PATH];
	memset(filename, 0x00, MAX_PATH);
	sprintf(filename, "%s\\raw_dir", m_path);
	if (!PathFileExists(filename))
	{
		CreateDirectory(filename, NULL);
	}
}


void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonOpenConfig()
{
	// TODO: 在此添加控件通知处理程序代码
	char szPath[MAX_PATH] = { 0 };
	memset(szPath, 0x00, MAX_PATH);
	sprintf(szPath, "%s\\DETECTOR_CFG.INI", m_path);
	//
	if (!PathFileExists(szPath))
	{
		::AfxMessageBox(_T("warnning:当前目录下,DETECTOR_CFG.INI不存在,请检查!"));
		return;
	}
	//
	ShellExecute(NULL, "open", szPath, NULL, NULL, SW_SHOW);
}


int CHB_SDK_DEMO2008Dlg::ShowImageA(unsigned short *imgbuff, int nbufflen, int nframeid, int nfps)
{
	if (m_hEventA == NULL) {
		printf("err:m_hEventA is NULL!\n");
		return 0;
	}
	if (m_ThreadhdlA == NULL)
	{
		printf("err:m_ThreadhdlA is NULL!\n");
		return 0;
	}

	/* 探测器分辨率大小 */
	int WIDTH  = m_imgWA;
	int HEIGHT = m_imgHA;
	if ((imgbuff == NULL) || (nbufflen != (WIDTH * HEIGHT))) 
	{
		printf("数据异常!\n");
		return 0;
	}
	// 
	CvSize sz;
	sz.width  = (int)(WIDTH  *  picA_factor);
	sz.height = (int)(HEIGHT *  picA_factor);
	//
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
		printf("err:pIplimageA or desIplimageA is NULL!");
		return -1;
	}
	// 数据拷贝
	memcpy(pIplimageA->imageData, imgbuff, nbufflen * sizeof(unsigned short));
	if (m_frameidA != nframeid) m_frameidA = nframeid;
	if (m_ufpsA != nfps) m_ufpsA = nfps;

	return 1;
}

void CHB_SDK_DEMO2008Dlg::CloseShowThread()
{
    RunFlag = 0;
	if (m_ThreadhdlA != NULL)
	{
		if (m_hEventA != NULL) 
		{
			SetEvent(m_hEventA);
			printf("%s\n", "SetEvent(m_hEventA)!");
		}
		//
		DWORD dw = WaitForSingleObject(m_ThreadhdlA, 5000);
		if (dw != WAIT_OBJECT_0)	
		{
			printf("err:%s\n", "TerminateThread(m_ThreadhdlA， 0）!");
		}
		else	
		{
			printf("%s\n", "CloseHandle(m_ThreadhdlA， 0）!");
		}
		//
		CloseHandle(m_ThreadhdlA);
		m_ThreadhdlA = NULL;
		m_uThreadFunIDA = 0;
		//
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

void CHB_SDK_DEMO2008Dlg::UpdateImageA()
{
	if (pPicWndA == NULL) return;

	CDC *pDC = pPicWndA->GetDC();        // 获得显示控件的 DC wishchin
	if (pDC == NULL) return;

	if (pIplimageA == NULL) return;

	// 清空历史
	pDC->FillSolidRect(&m_PicRectA, RGB(0, 0, 0));

	HDC hDC = pDC->GetSafeHdc();        // 获取 HDC(设备句柄) 来进行绘图操作
	CvvImage cimg;
	cimg.CopyOf(pIplimageA);            // 复制图片
	cimg.DrawToHDC(hDC, &m_PicRectA);   // 将图片绘制到显示控件的指定区域内  参数不对？
	SetBkMode( hDC,TRANSPARENT);
	SetTextColor( hDC,RGB(255, 0, 0));

	// 显示图像信息
	char buff[32];
	memset(buff, 0x00, 32);
	// 显示时间
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	int j = sprintf(buff, "Date:%4d-%02d-%02d %02d:%02d:%02d.%03d", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
	buff[j] = '\0';
	::TextOut(hDC, 10, 10, buff, j);
	// 显示帧号
	j = sprintf(buff, "Frame ID:%05d", m_frameidA);
	buff[j] = '\0';
	::TextOut(hDC, 10, 30, buff, j);
	// 分辨率
	j = sprintf(buff, "Pixel:%d * %d", m_imgWA, m_imgHA);
	buff[j] = '\0';
	::TextOut(hDC, 10, 50, buff, j);
	// 帧率
	j = sprintf(buff, "Fps:%u", m_ufpsA);
	buff[j] = '\0';
	::TextOut(hDC, 10, 70, buff, j);
	ReleaseDC( pDC );
}


unsigned int __stdcall CHB_SDK_DEMO2008Dlg::ShowThreadProcFunA(LPVOID pParam)
{
	printf("%s\n", "ShowThreadProcFunA start!");
	int ret = 0;
	int j = 0;
	DWORD dwResult = 0;
	
	///////////////////////////////////////////////////////////////////////////////////////
	CHB_SDK_DEMO2008Dlg* pPlugIn = (CHB_SDK_DEMO2008Dlg *)pParam;
	if (pPlugIn == NULL) 
	{
		printf("%s\n", "线程参数为空，创建 ShowThreadProcFunA 失败!");
		goto EXIT_SHOW_IMAGE_THREAD;
	}

	// 事件检查
	if (NULL == pPlugIn->m_hEventA)
	{
		printf("err:ShowThreadProcFunA,%s\n", "pPlugIn->m_hEventA is NULL!");
		goto EXIT_SHOW_IMAGE_THREAD;
	}
	// 设置为非信号状态
	::ResetEvent(pPlugIn->m_hEventA);

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	while (pPlugIn->RunFlag) 
	{
		dwResult = ::WaitForSingleObject(pPlugIn->m_hEventA, INFINITE);
		if (WAIT_OBJECT_0 == dwResult) 
		{
			if (!pPlugIn->RunFlag)
			{
				printf("pPlugIn->RunFlag=%dexit!\n", pPlugIn->RunFlag);
				break;
			}
			//
			if (pPlugIn->pPicWndA != NULL) 
			{
				//printf("ShowThreadProcFunA:frameid=%d\n", pPlugIn->m_frameid);
				pPlugIn->UpdateImageA();
			}
		}
		else /* timeout or other error */	
		{	
			if (WAIT_TIMEOUT == dwResult) 
			{}
		}
		// 设置为非信号状态
		::ResetEvent(pPlugIn->m_hEventA);
	}
//// 退出线程
EXIT_SHOW_IMAGE_THREAD:
	printf("%s\n", "ShowThreadProcFunA end!");
	return 0;
}

void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonFirmwareVer()
{
	// TODO: 在此添加控件通知处理程序代码
	printf("OnBnClickedButtonFirmwareVer\n");
	if (m_pFpd == NULL) {
		::AfxMessageBox("m_pFpd is NULL!");
		return;
	}
	int ret = HBI_GetFirmareVerion(m_pFpd, szFirmVer);
	if (0 != ret) {
		CString strStr = _T("");
		strStr.Format(_T("HBI_GetFirmareVerion Err,ret=%d"), ret);
		::AfxMessageBox(strStr);
		return;
	}
	printf("szFirmVer=%s\n", szFirmVer);
}


void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSoftwareVer()
{
	// TODO: 在此添加控件通知处理程序代码
	printf("OnBnClickedButtonSoftwareVer\n");
	if (m_pFpd == NULL) {
		::AfxMessageBox("m_pFpd is NULL!");
		return;
	}

	int ret = HBI_GetSDKVerion(m_pFpd, szSdkVer);
	if (0 != ret) {
		CString strStr = _T("");
		strStr.Format(_T("HBI_GetSDKVerion Err,ret=%d"), ret);
		::AfxMessageBox(strStr);
		return;
	}
	printf("szSdkVer=%s\n", szSdkVer);
}


void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnGetImageProperty()
{
	// TODO: 在此添加控件通知处理程序代码
	printf("get Image property begin!\n");
	if (m_pFpd == NULL)
	{
		printf("未初始化!\n");
		return;
	}
	//
	if (!this->m_IsOpen)
	{
		printf("未连接状态!\n");
		return;
	}
	//
	size_t size = sizeof(HB_FPD_SIZE) / sizeof(struct FpdPixelMatrixTable);
	//
	int ret = HBI_GetImageProperty(m_pFpd, &m_imgApro);
	if (ret == 0)
	{
		printf("HBI_GetImageProperty:\n\tnnFpdNum=%u\n", m_imgApro.nFpdNum);
		if (m_imgWA != m_imgApro.nwidth) m_imgWA = m_imgApro.nwidth;
		if (m_imgHA != m_imgApro.nheight) m_imgHA = m_imgApro.nheight;
		printf("\twidth=%d,hight=%d\n", m_imgWA, m_imgHA);

		//
		if (m_imgApro.datatype == 0) printf("\tdatatype is unsigned char.\n");
		else if (m_imgApro.datatype == 1) printf("\tdatatype is char.\n");
		else if (m_imgApro.datatype == 2) printf("\tdatatype is unsigned short.\n");
		else if (m_imgApro.datatype == 3) printf("\tdatatype is float.\n");
		else if (m_imgApro.datatype == 4) printf("\tdatatype is double.\n");
		else printf("\tdatatype is not support.\n");
		//
		if (m_imgApro.ndatabit == 0) printf("\tdatabit is 16bits.\n");
		else if (m_imgApro.ndatabit == 1) printf("\tdatabit is 14bits.\n");
		else if (m_imgApro.ndatabit == 2) printf("\tdatabit is 12bits.\n");
		else if (m_imgApro.ndatabit == 3) printf("\tdatabit is 8bits.\n");
		else printf("\tdatatype is unsigned char.\n");
		//
		if (m_imgApro.nendian == 0) printf("\tdata is little endian.\n");
		else printf("\tdata is bigger endian.\n");
		//
		printf("\tpacket_size=%u\n", m_imgApro.packet_size);
		printf("\tframe_size=%d\n\n", m_imgApro.frame_size);
	}
	else
	{
		::AfxMessageBox("HBI_GetImageProperty failed!");
	}
}


void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonGetConfig()
{
	// TODO: 在此添加控件通知处理程序代码
	bool b = GetFirmwareConfig();
	if (b)
		printf("GetFirmwareConfig success\n");
	else
		printf("err:GetFirmwareConfig failed\n");
}



void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSinglePrepare()
{
	// TODO: 在此添加控件通知处理程序代码
	printf("OnBnClickedButtonSinglePrepare!\n");
	if (m_pFpd == NULL)
	{
		::AfxMessageBox(_T("err:m_pFpd is NULL!"));
		return;
	}
	//
	int ret = HBI_SinglePrepare(m_pFpd);
	if (ret == 0)
		printf("HBI_SinglePrepare succss!\n");
	else
		printf("HBI_SinglePrepare failed!\n");
}



void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSingleShot()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	printf("OnBnClickedButtonSingleShot!\n");
	if (m_pFpd == NULL)
	{
		::AfxMessageBox(_T("err:m_pFpd is NULL!"));
		return;
	}
	//
	m_aqc_mode.eAqccmd    = SINGLE_ACQ_DEFAULT_TYPE;
	m_aqc_mode.nAcqnumber = 0; // 只采一帧
	m_aqc_mode.nframeid   = 0;
	m_aqc_mode.ngroupno   = 0;
	m_aqc_mode.ndiscard   = 0;
	m_aqc_mode.eLivetype  = ONLY_IMAGE; //Only Image
	printf("HBI_SingleAcquisition:[_curmode]=%d,[_acqMaxNum]=%d,[_discardNum]=%d,[_groupNum]=%d\n", m_aqc_mode.eAqccmd, m_aqc_mode.nAcqnumber, m_aqc_mode.ndiscard, m_aqc_mode.ngroupno);
	int ret = HBI_SingleAcquisition(m_pFpd, m_aqc_mode);
	if (ret == 0)
		printf("mode=%d,sum=%d,num=%d,group=%d,HBI_SingleAcquisition succss!\n", m_aqc_mode.eAqccmd, m_aqc_mode.nAcqnumber, m_aqc_mode.ndiscard, m_aqc_mode.ngroupno);
	else {
		printf("mode=%d,sum=%d,num=%d,group=%d,HBI_SingleAcquisition failed!\n", m_aqc_mode.eAqccmd, m_aqc_mode.nAcqnumber, m_aqc_mode.ndiscard, m_aqc_mode.ngroupno);
	}
}


void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnSetTriggerCorrection()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	printf("OnBnClickedBtnSetTriggerAndCorrect begin!\n");
	if (m_pFpd == NULL) {
		printf("未连接状态!\n");
		return;
	}

	// 触发模式
	int _triggerMode = STATIC_SOFTWARE_TRIGGER_MODE; //1 - 软触发，3 - 高压触发，4 - FreeAED。	
	int select = ((CComboBox *)GetDlgItem(IDC_COMBO_TRIGGER_MODE))->GetCurSel();
	if (select <= 0 || select >= 7) _triggerMode = DYNAMIC_FPD_CONTINUE_MODE;
	else _triggerMode = select;

	// 工作站设置SDK参数包括触发模式、校正
	IMAGE_CORRECT_ENABLE *pcorrect = new IMAGE_CORRECT_ENABLE;
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


void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnSetTriggerMode()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);

	printf("set Firmware trigger mode begin!\n");
	if (m_pFpd == NULL) {
		printf("未连接状态!\n");
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
	// 触发模式
	int _triggerMode = STATIC_SOFTWARE_TRIGGER_MODE;
	int select = ((CComboBox *)GetDlgItem(IDC_COMBO_TRIGGER_MODE))->GetCurSel();
	if (select <= 0 || select >= 7) _triggerMode = DYNAMIC_FPD_CONTINUE_MODE;
	else _triggerMode = select;
	//
	int ret = HBI_UpdateTriggerMode(m_pFpd, _triggerMode);
	if (ret == 0) {
		printf("OnBnClickedBtnSetFirmwareWorkmode:\n\tHBI_UpdateTriggerMode success!\n");
	}
	else {
		::AfxMessageBox("HBI_UpdateTriggerMode failed!");
	}
}


void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnCorrectEnable()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	printf("set correct enable begin!\n");

	if (m_pFpd == NULL) {
		printf("未连接状态!\n");
		return;
	}

	// 设置SDK参数包括校正使能
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


void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSetGainMode()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);

	printf("set Firmware gain mode begin!\n");
	if (m_pFpd == NULL) {
		printf("未连接状态!\n");
		return;
	}
	/*
	[n]-Invalid
	[1]-0.6pC
	[2]-1.2pC
	[3]-2.4pC
	[4]-3.6pC
	[5]-4.8pC
	[6]-7.2pC
	[7]-9.6pC
	*/
	int nGainMode = ((CComboBox *)GetDlgItem(IDC_COMBO_PGA_LEVEL))->GetCurSel();
	int ret = HBI_SetPGALevel(m_pFpd, nGainMode);
	if (ret == 0) {
		printf("HBI_SetPGALevel:\n\tHBI_SetPGALevel success!\n");
	}
	else {
		::AfxMessageBox("HBI_SetPGALevel failed!");
	}
}


void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonGetGainMode()
{
	// TODO: 在此添加控件通知处理程序代码
	printf("get Firmware gain mode begin!\n");
	if (m_pFpd == NULL) {
		::AfxMessageBox("m_pFpd is NULL!");
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
	//
	if (m_fpd_base != NULL)
	{
		if (m_fpd_base->nPGALevel != gainmode) m_fpd_base->nPGALevel = gainmode;
		if (m_fpd_base->nPGALevel >= 1 && m_fpd_base->nPGALevel <= 7)
			((CComboBox *)GetDlgItem(IDC_COMBO_PGA_LEVEL))->SetCurSel(m_fpd_base->nPGALevel);
		else
			((CComboBox *)GetDlgItem(IDC_COMBO_PGA_LEVEL))->SetCurSel(0);
	}
}


void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSetBinning()
{
	// TODO: 在此添加控件通知处理程序代码
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
	////else
	////{
	////	// 重新计算分辨率
	////	if (m_pLastRegCfg != NULL)
	////	{
	////		if (m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize >= 0x01 && m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize <= 0x0b)
	////		{
	////			// 重新计算分辨率
	////			int j = (m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize - 1);// 匹配静态数组
	////			int ww = HB_FPD_SIZE[j].fpd_width;
	////			int hh = HB_FPD_SIZE[j].fpd_height;
	////			// Bining:1-1x1,2-2x2,3-3x3,4-4x4
	////			int iscale = m_pLastRegCfg->m_SysCfgInfo.m_byBinning;
	////			if (iscale <= 0 || iscale > 4) iscale = 1;
	////			//
	////			ww /= iscale;
	////			hh /= iscale;
	////			//
	////			if (m_pLastRegCfg->m_SysBaseInfo.m_sImageWidth != ww)  m_pLastRegCfg->m_SysBaseInfo.m_sImageWidth = ww;
	////			if (m_pLastRegCfg->m_SysBaseInfo.m_sImageHeight != hh) m_pLastRegCfg->m_SysBaseInfo.m_sImageHeight = hh;
	////			//
	////			if (theDemo->m_imgWA != m_pLastRegCfg->m_SysBaseInfo.m_sImageWidth)  theDemo->m_imgWA = m_pLastRegCfg->m_SysBaseInfo.m_sImageWidth;
	////			if (theDemo->m_imgHA != m_pLastRegCfg->m_SysBaseInfo.m_sImageHeight) theDemo->m_imgHA = m_pLastRegCfg->m_SysBaseInfo.m_sImageHeight;
	////		}
	////	}
	////}
}



void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonGetBinning()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pFpd == NULL)
	{
		::AfxMessageBox("m_pFpd is NULL!");
		return;
	}
	//
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


void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSetLiveAcquisitionTm()
{
	// TODO: 在此添加控件通知处理程序代码
    this->UpdateData(TRUE);
	//
	int time = m_uLiveTime;
	if (time <= 0)
	{
		::AfxMessageBox("warnning:time > 0!");
		return;
	}
	//
	if (m_pFpd == NULL)
	{
		::AfxMessageBox("m_pFpd is NULL!");
		return;
	}
	//
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
	// 平板类型
	bool bStaticDetector = false;
	COMM_CFG commCfg;
	if (m_fpd_base->commType == 0 || m_fpd_base->commType == 3) // 静态平板或无线平板
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


void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonGetLiveAcqTime()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pFpd == NULL) {
		::AfxMessageBox("m_pFpd is NULL!");
		return;
	}
	//
	if (!m_IsOpen)
	{
		::AfxMessageBox("warnning:detector is disconnect!");
		return;
	}
	//
	if (m_fpd_base == NULL)
	{
		::AfxMessageBox(_T("err:m_fpdbasecfg error!"));
		return;
	}
	// 平板类型
	bool bStaticDetector = false;
	COMM_CFG commCfg;
	if (m_fpd_base->commType == 0 || m_fpd_base->commType == 3) // 静态平板或无线平板
	{
		bStaticDetector = true;
	}
	//
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
				if (m_fpd_base->liveAcqTime != m_uLiveTime) m_fpd_base->liveAcqTime = m_uLiveTime;
			}
			//
			if (m_uLiveTime <= 0) m_uLiveTime = 1000;
			unsigned int unValue = (UINT)(1000 / m_uLiveTime);
			if (unValue <= 0) unValue = 1;
			if (theDemo->m_uMaxFps != unValue) theDemo->m_uMaxFps = unValue;

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
				if (m_fpd_base->liveAcqTime != m_uLiveTime) m_fpd_base->liveAcqTime = m_uLiveTime;
			}
			//
			if (m_uLiveTime <= 0) m_uLiveTime = 1000;
			unsigned int unValue = (UINT)(1000 / m_uLiveTime);
			if (unValue <= 0) unValue = 1;
			if (theDemo->m_uMaxFps != unValue) theDemo->m_uMaxFps = unValue;

			CString strstr = _T("");
			strstr.Format(_T("%u"), theDemo->m_uMaxFps);
			((CEdit *)GetDlgItem(IDC_EDIT_MAX_FPS))->SetWindowTextA(strstr);
			strstr.ReleaseBuffer();

			this->UpdateData(FALSE);
			printf("HBI_GetSelfDumpingTime:time=%d\n", time);
		}
	}
}


void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonLiveAcquisitionTm()
{
	// TODO: 在此添加控件通知处理程序代码
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
	printf("get all firmware config info!\n");
	if (m_pFpd == NULL) 
	{
		printf("未连接状态!\n");
		return false;
	}
	////if (m_pLastRegCfg == NULL)
	////	m_pLastRegCfg = new RegCfgInfo;
	if (m_pLastRegCfg == NULL)
	{
		printf("m_pLastRegCfg is NULL!\n");
		return false;
	}
	// 测试，置零，检查结果
	memset(m_pLastRegCfg, 0x00, sizeof(RegCfgInfo));
	/*
	* 获取固件参数，连接后即可获取参数
	*/
	int ret = HBI_GetFpdCfgInfo(m_pFpd, m_pLastRegCfg);
	if (!ret) 
	{
		printf("HBI_GetFpdCfgInfo:width=%d,height=%d\n", m_imgApro.nwidth, m_imgApro.nheight);
		/*
		* ip and port
		*/
		// 高低位需要转换
		unsigned short usValue = ((m_pLastRegCfg->m_EtherInfo.m_sDestUDPPort & 0xff) << 8) | ((m_pLastRegCfg->m_EtherInfo.m_sDestUDPPort >> 8) & 0xff);
		printf("\tSourceIP:%d.%d.%d.%d:%u\n",
			m_pLastRegCfg->m_EtherInfo.m_byDestIP[0],
			m_pLastRegCfg->m_EtherInfo.m_byDestIP[1],
			m_pLastRegCfg->m_EtherInfo.m_byDestIP[2],
			m_pLastRegCfg->m_EtherInfo.m_byDestIP[3],
			usValue);
		usValue = ((m_pLastRegCfg->m_EtherInfo.m_sSourceUDPPort & 0xff) << 8) | ((m_pLastRegCfg->m_EtherInfo.m_sSourceUDPPort >> 8) & 0xff);
		printf("\tDestIP:%d.%d.%d.%d:%u\n",
			m_pLastRegCfg->m_EtherInfo.m_bySourceIP[0],
			m_pLastRegCfg->m_EtherInfo.m_bySourceIP[1],
			m_pLastRegCfg->m_EtherInfo.m_bySourceIP[2],
			m_pLastRegCfg->m_EtherInfo.m_bySourceIP[3],
			usValue);
		/*
		* width and hight
		* modified by mhyang 20191224
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
		//

		if (m_uDefaultFpdid == 0)
		{
			if (m_imgWA != m_pLastRegCfg->m_SysBaseInfo.m_sImageWidth) m_imgWA = m_pLastRegCfg->m_SysBaseInfo.m_sImageWidth;
			if (m_imgHA != m_pLastRegCfg->m_SysBaseInfo.m_sImageHeight) m_imgHA = m_pLastRegCfg->m_SysBaseInfo.m_sImageHeight;
			printf("\twidth=%d,hight=%d\n", m_imgWA, m_imgHA);
		}
		else
		{
		}
		printf("\tdatatype is unsigned char.\n");
		printf("\tdatabit is 16bits.\n");
		printf("\tdata is little endian.\n");
		/*
		* workmode
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
		//
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

		if (m_fpd_base != NULL)
		{
			// 高低位需要转换
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
			/*
			* workmode
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

			m_fpd_base->trigger_mode = m_pLastRegCfg->m_SysCfgInfo.m_byTriggerMode;

			printf("\tPre Acquisition Delay Time.%u\n", m_pLastRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime);
			/*
			* correction enable
			*/
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

			// PGA档位
			m_fpd_base->nPGALevel = GetPGA(m_pLastRegCfg->m_TICOFCfg.m_sTICOFRegister[26]);

			// Binning类型
			int iscale = m_pLastRegCfg->m_SysCfgInfo.m_byBinning;
			if (iscale < 1 || iscale > 4)
			{
				theDemo->m_fpd_base->nBinning = 1;
				iscale = 1;
			}
			else
				theDemo->m_fpd_base->nBinning = iscale;

			// prepare 延时
			BYTE byte1 = 0, byte2 = 0, byte3 = 0, byte4 = 0;
			byte1 = BREAK_UINT32(m_pLastRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime, 0);
			byte2 = BREAK_UINT32(m_pLastRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime, 1);
			byte3 = BREAK_UINT32(m_pLastRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime, 2);
			byte4 = BREAK_UINT32(m_pLastRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime, 3);
			unsigned int unValue = BUILD_UINT32(byte4, byte3, byte2, byte1);
			m_fpd_base->prepareTime = unValue;
			// 连续采集时间间隔和帧率计算
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
			//
			unValue = BUILD_UINT32(byte4, byte3, byte2, byte1);
			if (unValue <= 0)
			{
				printf("\tlive aqc time=%u\n", unValue);
				unValue = 1000;
			}

			if (m_fpd_base->liveAcqTime != unValue) m_fpd_base->liveAcqTime = unValue;

			unValue = (UINT)(1000 / unValue);
			if (unValue <= 0) unValue = 1;
			if (m_uMaxFps != unValue) m_uMaxFps = unValue;

			printf("\tnPGA=%d\n", m_fpd_base->nPGALevel);
			printf("\tnBinning=%d\n", m_fpd_base->nBinning);
			printf("\tm_uMaxFps=%u\n",   m_uMaxFps);
			printf("\tm_uLiveTime=%u\n", m_uLiveTime);
		}
		//// 更新显示信息
		//PostMessage(WM_USER_CURR_FPD_INFO, (WPARAM)0, (LPARAM)0);
	}
	else
	{
		printf("获取固件参数失败!\n");
		return false;
	}

	return true;
}


// add by mhyang 20210909
char* CHB_SDK_DEMO2008Dlg::getCurTemplatePath(char *curdir, const char *datatype, const char *temptype)
{
	// TODO: 在此添加额外的初始化
	if (m_pLastRegCfg == NULL) return NULL;
	//
	static char temp[MAX_PATH] = { 0 };
	BYTE byteBIN = 1;
	if (1 <= m_pLastRegCfg->m_SysCfgInfo.m_byBinning && m_pLastRegCfg->m_SysCfgInfo.m_byBinning <= 4)
		byteBIN = m_pLastRegCfg->m_SysCfgInfo.m_byBinning;
	////获取设置的帧率
	BYTE byte1 = 0, byte2 = 0, byte3 = 0, byte4 = 0;
	unsigned int bytePanelSize = m_pLastRegCfg->m_SysBaseInfo.m_byPanelSize;
	unsigned int uPGALevel = 0;
	if (bytePanelSize == 0x0a || bytePanelSize == 0x0b)
	{
		unsigned short usValue = ((m_pLastRegCfg->m_CMOSCfg.m_sCMOSRegister[0] & 0xff) << 8) | ((m_pLastRegCfg->m_CMOSCfg.m_sCMOSRegister[0] >> 8) & 0xff);
		// 1412
		if (bytePanelSize == 0x0a)
		{
			if (usValue == 0x8d8) uPGALevel = 9;      // 1412 HFW
			else if (usValue == 0x8c8) uPGALevel = 8; // 1412 HFW
			else uPGALevel = 0;
		}
		// 0606
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
	//
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
	// TODO: Add your control notification handler code here
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
	if (downloadfile == NULL)
		downloadfile = new DOWNLOAD_FILE;
	if (downloadfile == NULL) {
		::AfxMessageBox(_T("Error:downloadfile is NULL!"));
		return;
	}
	// add by mhyang 20211022
	char hbitemp[24] = { 0 };
	memset(hbitemp, 0x00, 24);
	sprintf(hbitemp, "hbi_id%u_template", m_uDefaultFpdid);
	CString strPath = _T("");
	//
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
	//
	CString szFilter = _T("Template File|*.raw||");
	CFileDialog dlg(TRUE, _T("txt"), strPath, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL);
	if (dlg.DoModal() == IDCANCEL)
	{
		return;
	}
	//
	CString strFileName = dlg.GetFileTitle();
	CString strFilePath = dlg.GetPathName();
	CString strFileExe  = dlg.GetFileExt();
	//
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

	// 用户选择Binnig类型
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
	//
	if (downloadfile->nBinningtype != byBinnig) downloadfile->nBinningtype = byBinnig;
	//
	if (0 != HBI_RegProgressCallBack(m_pFpd, DownloadTemplateCBFun, (void *)this))
	{
		::AfxMessageBox(_T("err:HBI_RegProgressCallBack failed!"));
		return;
	}
	((CButton *)GetDlgItem(IDC_BTN_DOWNLOAD_TEMPLATE))->EnableWindow(FALSE);
	int ret = HBI_DownloadTemplate(m_pFpd, downloadfile);
	if (ret != HBI_SUCCSS) {
		::AfxMessageBox(_T("err:Download template failed!"));
	}
	else
		::AfxMessageBox(_T("Download template success!"));
	((CButton *)GetDlgItem(IDC_BTN_DOWNLOAD_TEMPLATE))->SetWindowTextA(_T("Download Template"));
	((CButton *)GetDlgItem(IDC_BTN_DOWNLOAD_TEMPLATE))->EnableWindow(TRUE);
}


void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonSetPrepareTm()
{
	// TODO: 在此添加控件通知处理程序代码
	this->UpdateData(TRUE);

	if (m_pFpd == NULL) {
		::AfxMessageBox("m_pFpd is NULL!");
		return;
	}
	int time = m_uPrepareTime;
	printf("HBI_SetSinglePrepareTime:time=%d\n", time);
	int ret = HBI_SetSinglePrepareTime(m_pFpd, time);
	if (ret)
		::AfxMessageBox("HBI_SetSinglePrepareTime failed!");
}


void CHB_SDK_DEMO2008Dlg::OnBnClickedButtonGetPrepareTm()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pFpd == NULL) 
	{
		::AfxMessageBox("m_pFpd is NULL!");
		return;
	}
	int time = 0;
	int ret = HBI_GetSinglePrepareTime(m_pFpd, &time);
	if (ret)
		::AfxMessageBox("HBI_GetSinglePrepareTime failed!");
	else
	{
		m_uPrepareTime = time;

		if (m_fpd_base != NULL)
		{
			if (m_fpd_base->prepareTime != m_uPrepareTime) m_fpd_base->prepareTime = m_uPrepareTime;
		}

		this->UpdateData(FALSE);
	}
	printf("HBI_GetSinglePrepareTime:time=%d\n", time);
}



void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnUpdatePacketIntervalTime()
{
	// TODO: 在此添加控件通知处理程序代码
	this->UpdateData(TRUE);

	if (m_pFpd == NULL) {
		::AfxMessageBox("m_pFpd is NULL!");
		return;
	}
	int time = m_upacketInterval;
	printf("HBI_SetPacketIntervalTime:time=%d\n", time);
	int ret = HBI_SetPacketIntervalTime(m_pFpd, time);
	if (ret)
		::AfxMessageBox("HBI_SetPacketIntervalTime failed!");
}


void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnGetPacketIntervalTime()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pFpd == NULL)
	{
		::AfxMessageBox("m_pFpd is NULL!");
		return;
	}
	int time = 0;
	int ret = HBI_GetPacketIntervalTime(m_pFpd, &time);
	if (ret)
		::AfxMessageBox("HBI_GetPacketIntervalTime failed!");
	else
	{
		m_upacketInterval = time;

		if (m_fpd_base != NULL)
		{
			if (m_fpd_base->packetInterval != m_upacketInterval) m_fpd_base->packetInterval = m_upacketInterval;
		}

		this->UpdateData(FALSE);
	}
	printf("HBI_GetPacketIntervalTime:time=%d\n", time);
}


int CHB_SDK_DEMO2008Dlg::DownloadTemplateCBFun(unsigned char command, int code, void *inContext)
{
	if (theDemo == NULL) {
		printf("err:theDemo is NULL!\n");
		return 0;
	}
	//
	int len = 0;
	switch (command)
	{
	case ECALLBACK_UPDATE_STATUS_START:   // 初始化进度条	
		printf("handleProgressFun:ECALLBACK_UPDATE_STATUS_START\n");
		len = sprintf(theDemo->evnet_msg, "start download");
		theDemo->evnet_msg[len] = '\0';
		theDemo->PostMessage(WM_DOWNLOAD_TEMPLATE_CB_MSG, (WPARAM)command, (LPARAM)theDemo->evnet_msg);
		break;
	case ECALLBACK_UPDATE_STATUS_PROGRESS:// 进度 code：百分比（0~100）	
		len = sprintf(theDemo->evnet_msg, "%% %d", code);
		theDemo->evnet_msg[len] = '\0';
		theDemo->PostMessage(WM_DOWNLOAD_TEMPLATE_CB_MSG, (WPARAM)command, (LPARAM)theDemo->evnet_msg);
		break;
	case ECALLBACK_UPDATE_STATUS_RESULT:  // update result and error
		if ((0 <= code) && (code <= 6))
		{       // 显示Uploading gain template!
			if (code == 0) {       // offset模板上传完成
				printf("DownloadOffsetCBFun:%s\n", "download offset template!");
				len = sprintf(theDemo->evnet_msg, "offset temp");
			}
			else if (code == 1) {  // offset模板上传完成
				printf("DownloadOffsetCBFun:%s\n", "download gain template!");
				len = sprintf(theDemo->evnet_msg, "gain temp");
			}
			else if (code == 2) {  // gain模板上传完成
				printf("DownloadOffsetCBFun:%s\n", "download defect template!");
				len = sprintf(theDemo->evnet_msg, "defect temp");
			}
			else if (code == 3) {  // defect模板上传完成
				printf("DownloadOffsetCBFun:%s\n", "download offset finish!");
				len = sprintf(theDemo->evnet_msg, "offset finish");
			}
			else if (code == 4) {  // defect模板上传完成
				printf("DownloadOffsetCBFun:%s\n", "download gain finish!");
				len = sprintf(theDemo->evnet_msg, "gain finish");
			}
			else if (code == 5) {  // defect模板上传完成
				printf("DownloadOffsetCBFun:%s\n", "download defect finish!");
				len = sprintf(theDemo->evnet_msg, "defect finish");
			}
			else/* if (code == 6)*/ {  // defect模板上传完成
				printf("DownloadOffsetCBFun:%s\n", "Download finish and sucess!");
				len = sprintf(theDemo->evnet_msg, "success");
			}
			theDemo->evnet_msg[len] = '\0';
		}
		else // 失败
		{
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
		printf("handleProgressFun:unusual,retcode=%d\n", code);
		break;
	}
	return 1;
}


LRESULT CHB_SDK_DEMO2008Dlg::OnDownloadTemplateCBMessage(WPARAM wParam, LPARAM lParam)
{
	char *ptr = (char *)lParam;
	SetDlgItemText(IDC_BTN_DOWNLOAD_TEMPLATE, ptr);
	return 0;
}


int CHB_SDK_DEMO2008Dlg::init_base_cfg()
{
	m_uDefaultFpdid = 0;
	CFPD_BASE_CFG *pcfg = new CFPD_BASE_CFG;
	if (pcfg != NULL)
	{
		pcfg->detectorId = 0;
		m_fpdbasecfg.push_back(pcfg);
	}
	//
	size_t nsize = m_fpdbasecfg.size();
	if (1 != nsize)
	{
		free_base_cfg();
		return 0;
	}
	else
		return 1;
}


void CHB_SDK_DEMO2008Dlg::free_base_cfg()
{
	CFPD_BASE_CFG *pcfg = NULL;
	size_t nsize = m_fpdbasecfg.size();
	for (size_t i = 0; i < nsize; i++)
	{
		pcfg = m_fpdbasecfg[i];
		if (pcfg != NULL)
		{
			//pcfg->RESET();
			//
			delete pcfg;
			pcfg = NULL;
		}
	}
	if (m_fpdbasecfg.capacity() > 0)
		vector<CFPD_BASE_CFG *>().swap(m_fpdbasecfg);
}


int CHB_SDK_DEMO2008Dlg::read_ini_cfg()
{
	unsigned int usize = theDemo->m_fpdbasecfg.size();
	if (usize != 1)
	{
		return 0;
	}
	// 更新变量
	char szPath[MAX_PATH] = { 0 };
	memset(szPath, 0x00, MAX_PATH);
	sprintf(szPath, "%s\\DETECTOR_CFG.INI", theDemo->m_path);
	//
	CFPD_BASE_CFG *pcfg = NULL;
	if (!PathFileExists(szPath))
	{
		if (m_bSupportDual) m_bSupportDual  = false;
		if (m_uDefaultFpdid != 0) m_uDefaultFpdid = 0;
		//
		memset(m_username, 0x00, 64);
		pcfg = m_fpd_base;
		if (pcfg != NULL)
		{
			strcpy(m_username, "上海昊博影像科技有限公司");
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
		GetPrivateProfileString("FPD_CFG", "USER_NAME", "上海昊博影像科技有限公司", m_username, 63, szPath);
		//
		if (m_bSupportDual) m_bSupportDual = false;
		if (m_uDefaultFpdid != 0) m_uDefaultFpdid = 0;
		//
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

		this->SetWindowTextA(m_username);
	}
	return 1;
}


CFPD_BASE_CFG *CHB_SDK_DEMO2008Dlg::get_base_cfg_ptr(int id)
{
	CFPD_BASE_CFG *pcfg = NULL;
	vector<CFPD_BASE_CFG *>::iterator iter = m_fpdbasecfg.begin();
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
	} // FOR
	return pcfg;
}


CFPD_BASE_CFG *CHB_SDK_DEMO2008Dlg::get_default_cfg_ptr()
{
	CFPD_BASE_CFG *pcfg = NULL;
	if (m_uDefaultFpdid >= DETECTOR_MAX_NUMBER)
	{
		return pcfg;
	}
	//
	vector<CFPD_BASE_CFG *>::iterator iter = m_fpdbasecfg.begin();
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
	} // FOR
	return pcfg;
}
//////////////////////////////////////////////////////////////////////////////////////////////
//   CFPD_BASE_CFG
//////////////////////////////////////////////////////////////////////////////////////////////
CFPD_BASE_CFG::CFPD_BASE_CFG()
{
	detectorId = 0;          // 平板ID
	commType   = 0;          // 通讯类型
	//
	memset(DstIP, 0x00, 16);
	strcpy(DstIP, "192.168.10.40");
	DstPort = 32897;
	//
	memset(SrcIP, 0x00, 16);
	strcpy(SrcIP, "192.168.10.20");
	SrcPort = 32896;
	//
	m_rawpixel_x = 3072;
	m_rawpixel_y = 3072;
	databit = 0;
	datatype = 0;
	endian = 0;
	//
	trigger_mode = 7;
	offset_enable = 0;
	gain_enable = 0;
	defect_enable = 0;
	dummy_enable = 0;
	//
	nBinning = 1;
	nPGALevel = 6;
	prepareTime = 1500;      // prepare延时时间
	selfDumpingTime = 1000;  // 自清空时间
	liveAcqTime = 1000;      // 连续采集时间间隔
	// 句柄和连接状态
	m_bOpenOfFpd = false;            // true-连接，false-断开
	m_pFpdHand   = NULL;             // 句柄
	m_pRegCfg    = new RegCfgInfo;   // 参数
}

CFPD_BASE_CFG::~CFPD_BASE_CFG()
{
	if (m_pRegCfg != NULL)
	{
		delete m_pRegCfg;
		m_pRegCfg = NULL;
	}
}

void CFPD_BASE_CFG::RESET()
{
	if (m_pRegCfg != NULL)
	{
		delete m_pRegCfg;
		m_pRegCfg = NULL;
	}
}

void CFPD_BASE_CFG::PRINT_FPD_INFO()
{
	printf("@=======================Detector:%d======================\n", detectorId);
	printf("Local  IP Addr:%s:%u\n", SrcIP, SrcPort);
	printf("Remote IP Addr:%s:%u\n", DstIP, DstPort);
	//
	printf("Image property:\n\twidth=%d,hight=%d\n", m_rawpixel_x, m_rawpixel_y);
	//
	if (datatype == 0) printf("\tdatatype is unsigned char.\n");
	else if (datatype == 1) printf("\tdatatype is char.\n");
	else if (datatype == 2) printf("\tdatatype is unsigned short.\n");
	else if (datatype == 3) printf("\tdatatype is float.\n");
	else if (datatype == 4) printf("\tdatatype is double.\n");
	else printf("\tdatatype is not support.\n");
	//
	if (databit == 0) printf("\tdatabit is 16bits.\n");
	else if (databit == 1) printf("\tdatabit is 14bits.\n");
	else if (databit == 2) printf("\tdatabit is 12bits.\n");
	else if (databit == 3) printf("\tdatabit is 8bits.\n");
	else printf("\tdatatype is unsigned char.\n");
	//
	if (endian == 0) printf("\tdata is little endian.\n");
	else printf("\tdata is bigger endian.\n");
	//
	if (trigger_mode == 0x01)
		printf("trigger_mode[0x%02x]:\n\tstatic software trigger.\n", trigger_mode);
	else if (trigger_mode == 0x03)
		printf("trigger_mode[0x%02x]:\n\tstatic hvg trigger.\n", trigger_mode);
	else
		printf("trigger_mode[0x%02x]:\n\tother trigger mode.\n", trigger_mode);
	//
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

	if (0x00 == gain_enable)
		printf("\tNo Gain Correction\n");
	else if (0x01 == gain_enable)
		printf("\tSoftware Gain Correction\n");
	else if (0x02 == gain_enable)
		printf("\tFirmware Gain Correction\n");
	else
		printf("\tInvalid Gain Correction\n");

	if (0x00 == defect_enable)
		printf("\tNo Defect Correction\n");
	else if (0x01 == defect_enable)
		printf("\tSoftware Defect Correction\n");
	else if (0x02 == defect_enable)
		printf("\tFirmware Defect Correction\n");
	else
		printf("\tInvalid Defect Correction\n");

	if (0x00 == dummy_enable)
		printf("\tNo Dummy Correction\n");
	else if (0x01 == dummy_enable)
		printf("\tSoftware Dummy Correction\n");
	else if (0x02 == dummy_enable)
		printf("\tFirmware Dummy Correction\n");
	else
		printf("\tInvalid Dummy Correction\n");
	//00-Invalid;01-0.6pC;02-1.2pC;03-2.4pC;04-3.6pC;05-4.8pC;06-7.2pC;07-9.6pC;08-LFW(CMOS);09-HFW(CMOS);
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
	//
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


void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnUpdateTriggerBinningFps()
{
	// TODO: 在此添加控件通知处理程序代码
	printf("OnBnClickedBtnUpdateTriggerBinningFps!\n");

	this->UpdateData(TRUE);
	//
	if (m_pFpd == NULL) {
		::AfxMessageBox(_T("err:m_pFpd is NULL!"));
		return;
	}
	if (!m_IsOpen)
	{
		::AfxMessageBox(_T("err:disconnect!"));
		return;
	}
	// 触发模式
	int _triggerMode = STATIC_SOFTWARE_TRIGGER_MODE;
	int select = ((CComboBox *)GetDlgItem(IDC_COMBO_TRIGGER_MODE))->GetCurSel();
	if (select <= 0 || select >= 7) _triggerMode = DYNAMIC_FPD_CONTINUE_MODE;
	else _triggerMode = select;
	// Binning 类型
	int binning = ((CComboBox *)GetDlgItem(IDC_COMBO_BINNING))->GetCurSel();
	if (binning == 0) binning = 1;
	// 连续采集时间间隔（帧率）
	int time = m_uLiveTime;
	//
	int ret = HBI_TriggerBinningAcqTime(m_pFpd, _triggerMode, binning, time);
	if (ret == 0)
		printf("id:%u,HBI_TriggerBinningAcqTime succss!\n", m_uDefaultFpdid);
	else 
		printf("id:%u,HBI_TriggerBinningAcqTime failed!\n", m_uDefaultFpdid);
}


void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnUpdatePgaBinningFps()
{
	// TODO: 在此添加控件通知处理程序代码
	printf("OnBnClickedBtnUpdatePgaBinningFps!\n");
	this->UpdateData(TRUE);
	//
	if (m_pFpd == NULL) {
		::AfxMessageBox(_T("err:m_pFpd is NULL!"));
		return;
	}
	if (!m_IsOpen)
	{
		::AfxMessageBox(_T("err:disconnect!"));
		return;
	}
	// PGA 档位
	int nGainMode = ((CComboBox *)GetDlgItem(IDC_COMBO_PGA_LEVEL))->GetCurSel();
	// Binning 类型
	int binning = ((CComboBox *)GetDlgItem(IDC_COMBO_BINNING))->GetCurSel();
	if (binning == 0) binning = 1;
	// 连续采集时间间隔（帧率）
	int time = m_uLiveTime;
	//
	int ret = HBI_PgaBinningAcqTime(m_pFpd, nGainMode, binning, time);
	if (ret == 0)
		printf("HBI_PgaBinningAcqTime succss!\n");
	else
		printf("err:HBI_PgaBinningAcqTime failed!\n");
}


void CHB_SDK_DEMO2008Dlg::OnBnClickedBtnUpdateTriggerPgaBinningFps()
{
	// TODO: 在此添加控件通知处理程序代码
	printf("OnBnClickedBtnUpdateTriggerPgaBinningFps!\n");
	this->UpdateData(TRUE);
	//
	if (m_pFpd == NULL) {
		::AfxMessageBox(_T("err:m_pFpd is NULL!"));
		return;
	}
	if (!m_IsOpen)
	{
		::AfxMessageBox(_T("err:disconnect!"));
		return;
	}
	// 触发模式
	int _triggerMode = STATIC_SOFTWARE_TRIGGER_MODE;
	int select = ((CComboBox *)GetDlgItem(IDC_COMBO_TRIGGER_MODE))->GetCurSel();
	if (select <= 0 || select >= 7) _triggerMode = DYNAMIC_FPD_CONTINUE_MODE;
	else _triggerMode = select;
	// PGA 档位
	int nGainMode = ((CComboBox *)GetDlgItem(IDC_COMBO_PGA_LEVEL))->GetCurSel();
	// Binning 类型
	int binning = ((CComboBox *)GetDlgItem(IDC_COMBO_BINNING))->GetCurSel();
	if (binning == 0) binning = 1;
	// 连续采集时间间隔（帧率）
	int time = m_uLiveTime;
	//
	int ret = HBI_TriggerPgaBinningAcqTime(m_pFpd, _triggerMode, nGainMode, binning, time);
	if (ret == 0)
		printf("HBI_TriggerPgaBinningAcqTime succss!\n");
	else {
		printf("err:HBI_TriggerPgaBinningAcqTime failed!\n");
	}
}


void CHB_SDK_DEMO2008Dlg::OnFileOpenTemplateWizard()
{
	// TODO: 在此添加命令处理程序代码
	if (m_pFpd == NULL) {
		::AfxMessageBox(_T("err:m_pFpd is NULL!"));
		return;
	}
	if (!m_IsOpen)
	{
		::AfxMessageBox(_T("err:disconnect!"));
		return;
	}
	//
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


void CHB_SDK_DEMO2008Dlg::OnFirmwareUpdateTool()
{
	// TODO: 在此添加命令处理程序代码
	::AfxMessageBox(_T("warnning:Not support yet!"));
}


int CHB_SDK_DEMO2008Dlg::DoRegUpdateCallbackFun(USER_CALLBACK_HANDLE_PROCESS handFun, void* pContext)
{
	if (handFun == NULL || pContext == NULL)
	{
		::AfxMessageBox(_T("err:handFun or pContext is NULL!"));
		return -1;
	}
	return HBI_RegProgressCallBack(m_pFpd, handFun, pContext);
}

