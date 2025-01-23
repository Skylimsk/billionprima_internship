
// HB_SDK_DEMO2008Dlg.h : header file
//

#pragma once

#include "TemplateTool.h"
#include "DetectorSettingDlg.h"
#include "Cvvimage\\CvvImage.h"

#define WM_DETECTORA_CONNECT_STATUS   (WM_USER + 99)   // 连接状态图标
#define WM_USER_CURR_CONTROL_DATA     (WM_USER + 101)  // 更新控件信息
#define WM_DOWNLOAD_TEMPLATE_CB_MSG   (WM_USER + 102)  // 下载offset

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
	HICON m_hConnIco;
	HICON m_hDisConnIco;
	HICON m_hReadyIco;
	HICON m_hbusyIco;
	HICON m_hprepareIco;
	HICON m_hExposeIco;
	HICON m_hOffsetIco;
	HICON m_hConReadyIco;
	HICON m_hGainAckIco;
	HICON m_hDefectAckIco;
	HICON m_hOffsetAckIco;
	HICON m_hUpdateFirmIco;
	HICON m_hRetransIco;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnAppAbout();
	afx_msg void OnBnClickedBtnConn();
	afx_msg void OnBnClickedBtnDisconn();
	afx_msg void OnBnClickedBtnLiveAcq();
	afx_msg void OnBnClickedBtnStopLiveAcq();
	afx_msg void OnBnClickedRadioShowPic();
	afx_msg void OnBnClickedRadioSavePic();
	afx_msg void OnFileTemplate();
	afx_msg void OnBnClickedButtonSingleShot();
	afx_msg void OnBnClickedButtonFirmwareVer();
	afx_msg void OnBnClickedButtonSoftwareVer();
	afx_msg void OnBnClickedBtnGetImageProperty();
	afx_msg void OnBnClickedButtonGetConfig();
	afx_msg void OnBnClickedBtnSetTriggerCorrection();
	afx_msg void OnBnClickedBtnSetTriggerMode();
	afx_msg void OnBnClickedBtnCorrectEnable();
	afx_msg void OnBnClickedButtonSetGainMode();
	afx_msg void OnBnClickedButtonGetGainMode();
	afx_msg void OnBnClickedButtonGetLiveAcqTime();
	afx_msg void OnBnClickedButtonSetBinning();
	afx_msg void OnBnClickedButtonSetLiveAcquisitionTm();
	afx_msg void OnBnClickedButtonLiveAcquisitionTm();
	afx_msg void OnEnChangeEditMaxFps();
	afx_msg void OnBnClickedBtnDownloadTemplate();
	afx_msg void OnBnClickedButtonSetPrepareTm();
	afx_msg void OnBnClickedButtonGetPrepareTm();
	afx_msg void OnBnClickedButtonGetBinning();
	afx_msg void OnBnClickedButtonSinglePrepare();
	afx_msg void OnBnClickedButtonOpenConfig();
	afx_msg void OnFileDetectorSetting();
	//
	afx_msg void OnBnClickedBtnUpdateTriggerBinningFps();
	afx_msg void OnBnClickedBtnUpdatePgaBinningFps();
	afx_msg void OnBnClickedBtnUpdateTriggerPgaBinningFps();
	afx_msg void OnFileOpenTemplateWizard();
	// 自定义消息
	afx_msg LRESULT OnUpdateDetectorAStatus(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnUpdateCurControlData(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnDownloadTemplateCBMessage(WPARAM wParam, LPARAM lParam);
	//
	afx_msg void OnFirmwareUpdateTool();
	afx_msg void OnBnClickedBtnUpdatePacketIntervalTime();
	afx_msg void OnBnClickedBtnGetPacketIntervalTime();
	//
	void UpdateCurControlData();
	int DoRegUpdateCallbackFun(USER_CALLBACK_HANDLE_PROCESS handFun, void* pContext);

public:
	CSpinButtonCtrl m_ctlSpinAqcSum;
	CSpinButtonCtrl m_ctlSpinDiscardNum;
	CSpinButtonCtrl m_ctlSpinMaxFps;
	CStatic m_ctlConnStatus;
	UINT m_uFrameNum;
	UINT m_uDiscardNum;
	UINT m_uMaxFps;
	unsigned int m_upacketInterval;
	unsigned int m_uLiveTime;
	unsigned int m_uPrepareTime;
	char m_path[MAX_PATH];

public:
	// 回调函数以及显示线程
	static int handleCommandEventA(void* _contex, int nDevId, unsigned char byteEventId, void *buff, int param2, int param3, int param4);
	static unsigned int __stdcall ShowThreadProcFunA(LPVOID pParam);
	void CloseShowThread();
	//
	void conn_button_status();
	void disconn_button_status();
	bool GetFirmwareConfig();	
	void InitCtrlData();
	int GetPGA(unsigned short usValue);
	//
	int ShowImageA(unsigned short *imgbuff, int nbufflen, int nframeid, int nfps);
	int SaveImage(unsigned char *imgbuff, int nbufflen, int type = 0);
	void UpdateImageA();
	void AutoResize(int type=0);
	void UpdateUI(int type = 0);
	// 下载模板
	DOWNLOAD_FILE *downloadfile;
	static int DownloadTemplateCBFun(unsigned char command, int code, void *inContext);
	char evnet_msg[16];
	char* getCurTemplatePath(char *curdir, const char *datatype, const char *temptype);

//private:
public:
	bool m_bIsDetectA;
	BYTE m_r, m_g, m_b;
	bool m_bShowPic;
	char szSdkVer[128];
	char szFirmVer[128];
	char szSeiralNum[16];
	////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////
	void* m_pFpd;              // 当前DLL实例句柄
	bool m_IsOpen;             // 该平板是否处于连接状态
	CFPD_BASE_CFG *m_fpd_base; // 当前探测器基础数据
	RegCfgInfo* m_pLastRegCfg; // 固件配置
	FPD_AQC_MODE m_aqc_mode;   // 采集模式和参数，单帧采集、多帧采集、暗场图以及亮场图采集
	IMAGE_PROPERTY m_imgApro;

public:
	int m_nDetectorAStatus;
	CRect m_rectA;
	CWnd *pPicWndA;
	int m_imgWA, m_imgHA;        // 图像分辨率
	int m_frameidA;
	unsigned int m_ufpsA;
	// Add show window by mhyang 20201211
	CRect m_PicRectA;
	IplImage* pIplimageA;
	int picctl_width, picctl_height; // 图片控件的宽高
	double picA_factor; // 缩放倍数
	// 图像显示处理线程
	volatile int RunFlag;
	HANDLE m_hEventA;	
	HANDLE m_ThreadhdlA;
	unsigned int m_uThreadFunIDA;

public:
	char m_username[64];
	bool m_bSupportDual;            // false-不支持双板，true-支持双板 
	unsigned int m_uDefaultFpdid;   // 默认平板Id
	vector<CFPD_BASE_CFG *> m_fpdbasecfg;
	int init_base_cfg();
	void free_base_cfg();
	int read_ini_cfg();
	CFPD_BASE_CFG *get_base_cfg_ptr(int id);
	CFPD_BASE_CFG *get_default_cfg_ptr();
};

/* 平板基本信息 */
class CFPD_BASE_CFG
{
public:
	CFPD_BASE_CFG();
	~CFPD_BASE_CFG();

	void RESET();
	void PRINT_FPD_INFO();
public:
	int detectorId;               // 平板ID
	int commType;                 // 通讯类型
	char SrcIP[16];               // 本地IP
	char DstIP[16];               // 远程IP
	unsigned short SrcPort;       // 本地PORT
	unsigned short DstPort;       // 远程PORT
	unsigned int detectorSize;    // 平板型号
	unsigned int m_rawpixel_x;    // 平板分辨率width
	unsigned int m_rawpixel_y;    // 平板分辨率hight
	unsigned int databit;         // 图像数据位数
	unsigned int datatype;        // 图像数据类型
	unsigned int endian;          // 图像数据小端
	unsigned int trigger_mode;    // 触发模式
	unsigned int offset_enable;   // offset矫正使能
	unsigned int gain_enable;     // gain矫正使能
	unsigned int defect_enable;   // defect矫正使能
	unsigned int dummy_enable;    // dummy矫正使能
	unsigned int nBinning;        // binning 类型
	unsigned int nPGALevel;       // pga 档位
	unsigned int prepareTime;     // prepare延时时间
	unsigned int selfDumpingTime; // 自清空时间
	unsigned int liveAcqTime;     // 连续采集时间间隔
	unsigned int packetInterval  ;// 包间隔时间
	bool m_bOpenOfFpd;            // true-连接，false-断开
	void *m_pFpdHand;             // 句柄
	RegCfgInfo* m_pRegCfg;        // 参数
};