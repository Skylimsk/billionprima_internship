// HB_SDK_DEMO2008Dlg.h : header file
//

#pragma once

#include "TemplateTool.h"
#include "DetectorSettingDlg.h"
#include "Cvvimage\\CvvImage.h"

#define WM_DETECTORA_CONNECT_STATUS   (WM_USER + 99)   // Connection status indicator
#define WM_USER_CURR_CONTROL_DATA     (WM_USER + 101)  // Update control information
#define WM_DOWNLOAD_TEMPLATE_CB_MSG   (WM_USER + 102)  // Download offset callback

// Macros for 32-bit integer manipulation
#define BREAK_UINT32(var, ByteNum) \
		(unsigned char)((unsigned int)(((var) >> ((ByteNum)* 8)) & 0x00FF))

#define BUILD_UINT32(Byte0, Byte1, Byte2, Byte3) \
	((unsigned int)((unsigned int)((Byte0)& 0x00FF) \
	+ ((unsigned int)((Byte1)& 0x00FF) << 8) \
	+ ((unsigned int)((Byte2)& 0x00FF) << 16) \
	+ ((unsigned int)((Byte3)& 0x00FF) << 24)))

#define USER_WEI_MING 0

// CHB_SDK_DEMO2008Dlg dialog
class CFPD_BASE_CFG;
class CHB_SDK_DEMO2008Dlg : public CDialog
{
	// Construction
public:
	CHB_SDK_DEMO2008Dlg(CWnd* pParent = NULL);	// standard constructor
	~CHB_SDK_DEMO2008Dlg();

	// Dialog Data
	enum { IDD = IDD_HB_SDK_DEMO2008_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	//HBRUSH m_hBrush;

// Implementation
protected:
	HICON m_hIcon;
	HICON m_hConnIco;            // Connected icon
	HICON m_hDisConnIco;         // Disconnected icon
	HICON m_hReadyIco;           // Ready state icon
	HICON m_hbusyIco;            // Busy state icon
	HICON m_hprepareIco;         // Prepare state icon
	HICON m_hExposeIco;          // Expose state icon
	HICON m_hOffsetIco;          // Offset icon
	HICON m_hConReadyIco;        // Connection ready icon
	HICON m_hGainAckIco;         // Gain acknowledge icon
	HICON m_hDefectAckIco;       // Defect acknowledge icon
	HICON m_hOffsetAckIco;       // Offset acknowledge icon
	HICON m_hUpdateFirmIco;      // Firmware update icon
	HICON m_hRetransIco;         // Retransmission icon

	// Core MFC message handlers for dialog functionality
	virtual BOOL OnInitDialog();							// Called when dialog starts - setup initialization
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);		// Handles system menu commands like minimize/close
	afx_msg void OnPaint();									// Called when dialog needs redrawing
	afx_msg HCURSOR OnQueryDragIcon();						// Sets cursor icon when dragging dialog
	DECLARE_MESSAGE_MAP()									// Maps Windows messages to these handler functions

public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnAppAbout();
	afx_msg void OnBnClickedBtnConn();           // Connect button handler
	afx_msg void OnBnClickedBtnDisconn();        // Disconnect button handler
	afx_msg void OnBnClickedBtnLiveAcq();        // Live acquisition button handler
	afx_msg void OnBnClickedBtnStopLiveAcq();    // Stop live acquisition handler
	afx_msg void OnBnClickedRadioShowPic();      // Show picture radio button handler
	afx_msg void OnBnClickedRadioSavePic();      // Save picture radio button handler
	afx_msg void OnFileTemplate();               // File template handler
	afx_msg void OnBnClickedButtonSingleShot();  // Single shot button handler
	afx_msg void OnBnClickedButtonFirmwareVer(); // Get firmware version handler
	afx_msg void OnBnClickedButtonSoftwareVer(); // Get software version handler
	afx_msg void OnBnClickedBtnGetImageProperty(); // Get image property handler
	afx_msg void OnBnClickedButtonGetConfig();    // Get configuration handler
	afx_msg void OnBnClickedBtnSetTriggerCorrection(); // Set trigger correction
	afx_msg void OnBnClickedBtnSetTriggerMode();  // Set trigger mode handler
	afx_msg void OnBnClickedBtnCorrectEnable();   // Enable correction handler
	afx_msg void OnBnClickedButtonSetGainMode();  // Set gain mode handler
	afx_msg void OnBnClickedButtonGetGainMode();  // Get gain mode handler
	afx_msg void OnBnClickedButtonGetLiveAcqTime(); // Get live acquisition time
	afx_msg void OnBnClickedButtonSetBinning();   // Set binning handler
	afx_msg void OnBnClickedButtonSetLiveAcquisitionTm(); // Set live acquisition time
	afx_msg void OnBnClickedButtonLiveAcquisitionTm();    // Live acquisition time handler
	afx_msg void OnEnChangeEditMaxFps();          // Change max FPS handler
	afx_msg void OnBnClickedBtnDownloadTemplate(); // Download template handler
	afx_msg void OnBnClickedButtonSetPrepareTm(); // Set prepare time handler
	afx_msg void OnBnClickedButtonGetPrepareTm(); // Get prepare time handler
	afx_msg void OnBnClickedButtonGetBinning();   // Get binning handler
	afx_msg void OnBnClickedButtonSinglePrepare(); // Single prepare handler
	afx_msg void OnBnClickedButtonOpenConfig();    // Open config handler
	afx_msg void OnFileDetectorSetting();         // Detector settings handler

	// FPS and binning update handlers
	afx_msg void OnBnClickedBtnUpdateTriggerBinningFps();
	afx_msg void OnBnClickedBtnUpdatePgaBinningFps();
	afx_msg void OnBnClickedBtnUpdateTriggerPgaBinningFps();
	afx_msg void OnFileOpenTemplateWizard();

	// Custom message handlers
	afx_msg LRESULT OnUpdateDetectorAStatus(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnUpdateCurControlData(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnDownloadTemplateCBMessage(WPARAM wParam, LPARAM lParam);

	afx_msg void OnFirmwareUpdateTool();
	afx_msg void OnBnClickedBtnUpdatePacketIntervalTime();
	afx_msg void OnBnClickedBtnGetPacketIntervalTime();

	void UpdateCurControlData();
	int DoRegUpdateCallbackFun(USER_CALLBACK_HANDLE_PROCESS handFun, void* pContext);

public:
	CSpinButtonCtrl m_ctlSpinAqcSum;      // Acquisition sum spin control
	CSpinButtonCtrl m_ctlSpinDiscardNum;  // Discard number spin control
	CSpinButtonCtrl m_ctlSpinMaxFps;      // Max FPS spin control
	CStatic m_ctlConnStatus;              // Connection status control
	UINT m_uFrameNum;                     // Frame number
	UINT m_uDiscardNum;                   // Discard number
	UINT m_uMaxFps;                       // Maximum FPS
	unsigned int m_upacketInterval;        // Packet interval
	unsigned int m_uLiveTime;             // Live acquisition time
	unsigned int m_uPrepareTime;          // Prepare time
	char m_path[MAX_PATH];                // Path buffer

public:
	// Callback functions and display thread
	static int handleCommandEventA(void* _contex, int nDevId, unsigned char byteEventId, void* buff, int param2, int param3, int param4);
	static unsigned int __stdcall ShowThreadProcFunA(LPVOID pParam);

	void CloseShowThread();

	// Utility functions
	void conn_button_status();            // Update connect button status
	void disconn_button_status();         // Update disconnect button status
	bool GetFirmwareConfig();             // Get firmware configuration
	void InitCtrlData();                  // Initialize control data
	int GetPGA(unsigned short usValue);   // Get PGA value

	// Image handling functions
	int ShowImageA(unsigned short* imgbuff, int nbufflen, int nframeid, int nfps);
	int SaveImage(unsigned char* imgbuff, int nbufflen, int type = 0);
	void UpdateImageA();                  // Update image A
	void AutoResize(int type = 0);          // Auto resize handler
	void UpdateUI(int type = 0);          // Update UI handler

	// Template download
	DOWNLOAD_FILE* downloadfile;          // Download file pointer
	static int DownloadTemplateCBFun(unsigned char command, int code, void* inContext);
	char evnet_msg[16];                  // Event message buffer
	char* getCurTemplatePath(char* curdir, const char* datatype, const char* temptype);

public:
	bool m_bIsDetectA;                   // Detector A status
	BYTE m_r, m_g, m_b;                  // RGB values
	bool m_bShowPic;                     // Show picture flag
	char szSdkVer[128];                  // SDK version
	char szFirmVer[128];                 // Firmware version
	char szSeiralNum[16];                // Serial number

	// Device handles and configurations
	void* m_pFpd;                        // Current DLL instance pointer
	bool m_IsOpen;                       // Connection status flag
	CFPD_BASE_CFG* m_fpd_base;          // Current detector base configuration
	RegCfgInfo* m_pLastRegCfg;          // Firmware configuration
	FPD_AQC_MODE m_aqc_mode;            // Acquisition mode and parameters
	IMAGE_PROPERTY m_imgApro;            // Image properties

public:
	int m_nDetectorAStatus;              // Detector A status
	CRect m_rectA;                       // Rectangle A
	CWnd* pPicWndA;                     // Picture window A
	int m_imgWA, m_imgHA;               // Image width and height
	int m_frameidA;                      // Frame ID A
	unsigned int m_ufpsA;                // FPS A

	// Picture display properties
	CRect m_PicRectA;                    // Picture rectangle A
	IplImage* pIplimageA;                // OpenCV image A
	int picctl_width, picctl_height;     // Picture control dimensions
	double picA_factor;                  // Picture zoom factor

	// Image display thread variables
	volatile int RunFlag;                // Thread run flag
	HANDLE m_hEventA;                    // Event handle A
	HANDLE m_ThreadhdlA;                 // Thread handle A
	unsigned int m_uThreadFunIDA;        // Thread function ID A

public:
	char m_username[64];                 // Username
	bool m_bSupportDual;                 // Dual panel support flag
	unsigned int m_uDefaultFpdid;        // Default panel ID
	vector<CFPD_BASE_CFG*> m_fpdbasecfg; // Panel configuration vector

	// Configuration management functions
	int init_base_cfg();                 // Initialize base configuration
	void free_base_cfg();                // Free base configuration
	int read_ini_cfg();                  // Read INI configuration
	CFPD_BASE_CFG* get_base_cfg_ptr(int id); // Get base configuration pointer
	CFPD_BASE_CFG* get_default_cfg_ptr();    // Get default configuration pointer
};

/* Panel Basic Information Class */
class CFPD_BASE_CFG
{
public:
	CFPD_BASE_CFG();
	~CFPD_BASE_CFG();

	void RESET();                        // Reset configuration
	void PRINT_FPD_INFO();               // Print panel information

public:
	int detectorId;                      // Panel ID
	int commType;                        // Communication type
	char SrcIP[16];                      // Local IP
	char DstIP[16];                      // Remote IP
	unsigned short SrcPort;              // Local port
	unsigned short DstPort;              // Remote port
	unsigned int detectorSize;           // Panel model
	unsigned int m_rawpixel_x;           // Panel resolution width
	unsigned int m_rawpixel_y;           // Panel resolution height
	unsigned int databit;                // Image data bits
	unsigned int datatype;               // Image data type
	unsigned int endian;                 // Image data endianness
	unsigned int trigger_mode;           // Trigger mode
	unsigned int offset_enable;          // Offset correction enable
	unsigned int gain_enable;            // Gain correction enable
	unsigned int defect_enable;          // Defect correction enable
	unsigned int dummy_enable;           // Dummy correction enable
	unsigned int nBinning;               // Binning type
	unsigned int nPGALevel;              // PGA level
	unsigned int prepareTime;            // Prepare delay time
	unsigned int selfDumpingTime;        // Self-dumping time
	unsigned int liveAcqTime;            // Continuous acquisition interval
	unsigned int packetInterval;         // Packet interval time
	bool m_bOpenOfFpd;                   // Connection status
	void* m_pFpdHand;                    // Handle pointer
	RegCfgInfo* m_pRegCfg;               // Parameters
};