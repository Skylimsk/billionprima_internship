// TemplateTool.cpp : implementation file
//

#include "stdafx.h"
#include "HB_SDK_DEMO2008.h"
#include "TemplateTool.h"
#include "HB_SDK_DEMO2008Dlg.h"

#define WAITTING_TIME 5

extern CHB_SDK_DEMO2008Dlg* theDemo;

#ifndef _XSLEEP_H_
#define _XSLEEP_H_

// This structure is used internally by the XSleep function 
struct XSleep_Structure {
	XSleep_Structure() {
		duration = 1000;
		eventHandle = NULL;
	}
	int duration;
	HANDLE eventHandle;
};

void XSleep(int nWaitInMSecs);

#endif // _XSLEEP_H_

DWORD WINAPI XSleepThread(LPVOID pWaitTime)
{
	XSleep_Structure *sleep = (XSleep_Structure *)pWaitTime;

	Sleep(sleep->duration);
	SetEvent(sleep->eventHandle);

	return 0;
}

//////////////////////////////////////////////////////////////////////
// Function  : XSleep()
// Purpose   : To make the application sleep for the specified time
//			   duration.
//			   Duration the entire time duration XSleep sleeps, it
//			   keeps processing the message pump, to ensure that all
//			   messages are posted and that the calling thread does
//			   not appear to block all threads!
// Returns   : none
// Parameters:       
//  1. nWaitInMSecs - Duration to sleep specified in miliseconds.
//////////////////////////////////////////////////////////////////////
void XSleep(int nWaitInMSecs)
{
	XSleep_Structure sleep;
	sleep.duration = nWaitInMSecs;
	sleep.eventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);

	HANDLE getHandle = (HANDLE)_beginthreadex(NULL, 0, (unsigned int(__stdcall*)(void *))XSleepThread, &sleep, 0, NULL);
	if (getHandle == NULL)
		return;

	MSG msg;
	while (::WaitForSingleObject(sleep.eventHandle, 0) == WAIT_TIMEOUT)
	{
		//get and dispatch messages
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	CloseHandle(sleep.eventHandle);
	CloseHandle(getHandle);
}

static int DownloadCallBackFun(unsigned char command, int code, void *inContext)
{
	CTemplateTool *ptr = (CTemplateTool *)inContext;
	if (ptr == NULL) {
		printf("err:inContext is NULL!\n");
		return 0;
	}
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
		ptr->PostMessage(WM_DOWNLOAD_CB_MESSAGE, (WPARAM)command, (LPARAM)theDemo->evnet_msg);
		break;
	case ECALLBACK_UPDATE_STATUS_PROGRESS:// 进度 code：百分比（0~100）	
		len = sprintf(theDemo->evnet_msg, "%% %d", code);
		theDemo->evnet_msg[len] = '\0';
		ptr->PostMessage(WM_DOWNLOAD_CB_MESSAGE, (WPARAM)command, (LPARAM)theDemo->evnet_msg);
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
		ptr->PostMessage(WM_DOWNLOAD_CB_MESSAGE, (WPARAM)command, (LPARAM)theDemo->evnet_msg);
		break;
	default:                              // unusual
		printf("DownloadCallBackFun:unusual,retcode=%d\n", code);
		break;
	}
	return 1;
}

// CTemplateTool dialog

IMPLEMENT_DYNAMIC(CTemplateTool, CDialog)

CTemplateTool::CTemplateTool(CWnd* pParent /*=NULL*/)
	: CDialog(CTemplateTool::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_TEMPLATE);//修改对话框的图标
	enumTemplateType = FPD_OFFSET_TEMPLATE;
	m_ungroupid = 0;
}

CTemplateTool::~CTemplateTool()
{
}

void CTemplateTool::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CTemplateTool, CDialog)
	ON_BN_CLICKED(IDC_RADIO_OFFSET, &CTemplateTool::OnBnClickedRadioOffset)
	ON_BN_CLICKED(IDC_RADIO_GAIN, &CTemplateTool::OnBnClickedRadioGain)
	ON_BN_CLICKED(IDC_RADIO_DEFECT, &CTemplateTool::OnBnClickedRadioDefect)
	ON_BN_CLICKED(IDC_BTN_TEMPLATE_GENERATE, &CTemplateTool::OnBnClickedBtnTemplateGenerate)
	ON_MESSAGE(WM_USER_NOTICE_TEMPLATE_TOOL, &CTemplateTool::OnGenerateTemplateResult)
	ON_MESSAGE(WM_DOWNLOAD_CB_MESSAGE, &CTemplateTool::OnUpdateBtnTitle)
END_MESSAGE_MAP()


// CTemplateTool message handlers
BOOL CTemplateTool::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化
	// 初始化参数
	CheckRadioButton(IDC_RADIO_GAIN, IDC_RADIO_DEFECT, IDC_RADIO_OFFSET);
	if (m_ungroupid != 0) m_ungroupid = 0;

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}



void CTemplateTool::OnBnClickedRadioOffset()
{
	// TODO: 在此添加控件通知处理程序代码
	if (enumTemplateType != FPD_OFFSET_TEMPLATE) {
		enumTemplateType = FPD_OFFSET_TEMPLATE;
		((CButton *)GetDlgItem(IDC_BTN_TEMPLATE_GENERATE))->SetWindowTextA("Generate Offset Template");
	}
}


void CTemplateTool::OnBnClickedRadioGain()
{
	// TODO: 在此添加控件通知处理程序代码
	if (enumTemplateType != FPD_GAIN_TEMPLATE) {
		enumTemplateType = FPD_GAIN_TEMPLATE;
		((CButton *)GetDlgItem(IDC_BTN_TEMPLATE_GENERATE))->SetWindowTextA("Generate Gain Template");
	}
}


void CTemplateTool::OnBnClickedRadioDefect()
{
	// TODO: 在此添加控件通知处理程序代码
	if (enumTemplateType != FPD_DEFECT_ACQ_GROUP1) {
		enumTemplateType = FPD_DEFECT_ACQ_GROUP1;
		if (m_ungroupid != 0) m_ungroupid = 0;
		((CButton *)GetDlgItem(IDC_BTN_TEMPLATE_GENERATE))->SetWindowTextA("Defect Template-Collection Group1");
	}
}


LRESULT CTemplateTool::OnGenerateTemplateResult(WPARAM wparam, LPARAM lparam)
{
	int result = (int)wparam;
	if (1 == result)
		::AfxMessageBox("generate offset template success!");
	else if (-1 == result)
		::AfxMessageBox("generate offset template failed!");
	else if (2 == result) {
	//	::AfxMessageBox("generate gain template success!");
		printf("generate gain template success!\n");
	}
	else if (-2 == result)
		::AfxMessageBox("generate gain template failed!");
	else if (3 == result) {
	//	::AfxMessageBox("generate defect template success!");
		printf("generate defect template success!\n");
	}
	else if (-3 == result)
		::AfxMessageBox("generate defect template failed!");
	else if (4 == result)
		::AfxMessageBox("defect acq light image success!");
	else if (-4 == result)
		::AfxMessageBox("defect acq light image failed!");
	else
		::AfxMessageBox("generate template other error!");
	return S_OK;
}

void CTemplateTool::OnBnClickedBtnTemplateGenerate()
{
	// TODO: 在此添加控件通知处理程序代码
	if (theDemo == NULL) {
		::AfxMessageBox("err:theDoc is NULL!");
		return;
	}
	if (theDemo->m_pFpd == NULL) {
		::AfxMessageBox("err:theDoc->m_pFpd is NULL!");
		return;
	}
	if (!theDemo->m_IsOpen) {
		::AfxMessageBox("err:Fpd is not connected!");
		return;
	}
	// 获取一组单选框选中状态
	int nID = GetCheckedRadioButton(IDC_RADIO_GAIN, IDC_RADIO_DEFECT);
	if (nID == IDC_RADIO_OFFSET) {
		if (enumTemplateType != FPD_OFFSET_TEMPLATE) enumTemplateType = FPD_OFFSET_TEMPLATE;
		printf("OnBnClickedTemplateGenerate:OFFSET_TEMPLATE_MODE\n");
	}
	else if (nID == IDC_RADIO_GAIN) {
		if (enumTemplateType != FPD_GAIN_TEMPLATE) enumTemplateType = FPD_GAIN_TEMPLATE;
		printf("OnBnClickedTemplateGenerate:GAIN_TEMPLATE_MODE\n");
	}
	else if (nID == IDC_RADIO_DEFECT) {
		if (0 == m_ungroupid)
		{
			if (enumTemplateType != FPD_DEFECT_ACQ_GROUP1) enumTemplateType = FPD_DEFECT_ACQ_GROUP1;
			printf("OnBnClickedTemplateGenerate:DEFECT_ACQ_GROUP1\n");
		}
		else if (1 == m_ungroupid) {
			if (enumTemplateType != FPD_DEFECT_ACQ_GROUP2) enumTemplateType = FPD_DEFECT_ACQ_GROUP2;
			printf("OnBnClickedTemplateGenerate:DEFECT_ACQ_GROUP2\n");
		}
		else if (2 == m_ungroupid) {
			if (enumTemplateType != FPD_DEFECT_ACQ_AND_TEMPLATE) enumTemplateType = FPD_DEFECT_ACQ_AND_TEMPLATE;
			printf("OnBnClickedTemplateGenerate:DEFECT_ACQ_AND_TEMPLATE\n");
		}
		else {
			::AfxMessageBox("err:m_ungroupid is error!");
			return;
		}
	}
	else {
		::AfxMessageBox("err:Control nID is not Exits!");
		return;
	}
	///////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////
	// 开始采图并生成模板
	int ret = -1;
	if (enumTemplateType == OFFSET_TEMPLATE) {
		ret = DoOffsetTemp();
	}
	else if (enumTemplateType == GAIN_TEMPLATE) {
		// 调节电压
		if (MessageBoxEx(NULL, "请确定剂量:正常电压+正常毫安秒?", "亮场图", MB_ICONINFORMATION | MB_YESNO, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)) == IDNO)
		{
			return; //注意无返回值
		}
		ret = DoGainTemp();
	}
	else if (enumTemplateType == FPD_DEFECT_ACQ_GROUP1 ||
		enumTemplateType == FPD_DEFECT_ACQ_GROUP2 ||
		enumTemplateType == FPD_DEFECT_ACQ_AND_TEMPLATE)
	{
		if (enumTemplateType == FPD_DEFECT_ACQ_GROUP1) {
			if (MessageBoxEx(NULL, "请确定剂量:正常电压+正常毫安秒的10%?", "第一组亮场图", MB_ICONINFORMATION | MB_YESNO, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)) == IDNO)
			{
				return; //注意无返回值
			}
			ret =DoDefectGroup1();
		}
		else if (enumTemplateType == FPD_DEFECT_ACQ_GROUP2) {
			if (MessageBoxEx(NULL, "请确定剂量:正常电压+正常毫安秒的50%?", "第二组亮场图", MB_ICONINFORMATION | MB_YESNO, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)) == IDNO)
			{
				return; //注意无返回值
			}
			ret =DoDefectGroup2();
		}
		else /*if (enumTemplateType == FPD_DEFECT_ACQ_GROUP2) */{
			if (MessageBoxEx(NULL, "请确定剂量:正常电压+正常毫安秒?", "第三组亮场图&生成模板", MB_ICONINFORMATION | MB_YESNO, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)) == IDNO)
			{
				return; //注意无返回值
			}	
			ret =DoDefectGroup3();
		}
		/////ret = DoDefectTemp();
	}
	//
	if (ret != HBI_SUCCSS) 
	{
		printf("err:Generate Template failed!\n");
	}
	else
	{// 成功后,下一步
		if (enumTemplateType == OFFSET_TEMPLATE)
		{
			if (m_ungroupid != 0) m_ungroupid = 0; // 重置
			printf("DO OFFSET_TEMPLATE_MODE SUCCESS!\n");
			::AfxMessageBox(_T("DO OFFSET_TEMPLATE_TYPE SUCCESS!"));
		}
		else if (enumTemplateType == GAIN_TEMPLATE)
		{
			if (m_ungroupid != 0) m_ungroupid = 0; // 重置
			printf("DO GAIN_TEMPLATE_MODE SUCCESS!\n");
			// 开始下载Gain模板
			if (0 != HBI_RegProgressCallBack(theDemo->m_pFpd, DownloadCallBackFun, (void *)this))
			{
				printf("err:HBI_RegProgressCallBack failed!\n");
				return;
			}
			// int infiletype - 下载文件类型
			// 0-gain模板，1-defect模板，2-offset模板，其他-不支持
			int ret = HBI_DownloadTemplateByType(theDemo->m_pFpd, 0);
			if (ret != HBI_SUCCSS)
				::AfxMessageBox(_T("err:Download gain-template failed!"));
			else
				::AfxMessageBox(_T("Download gain-template success!"));	

			// 设置SDK参数包括校正使能
			IMAGE_CORRECT_ENABLE *pcorrect = new IMAGE_CORRECT_ENABLE;
			if (pcorrect != NULL)
			{
#if 0
				pcorrect->bFeedbackCfg = false; // false-ECALLBACK_TYPE_SET_CFG_OK Event
#else
				pcorrect->bFeedbackCfg = true;  // //true-ECALLBACK_TYPE_ROM_UPLOAD Event,
#endif
				pcorrect->ucOffsetCorrection = 0x03;
				pcorrect->ucGainCorrection   = 0x02;
				pcorrect->ucDefectCorrection = 0x00;
				pcorrect->ucDummyCorrection  = 0x00;
				ret = HBI_UpdateCorrectEnable(theDemo->m_pFpd, pcorrect);
				if (ret != HBI_SUCCSS)
					::AfxMessageBox("Gain:HBI_UpdateCorrectEnable failed!");
				else
					printf("Gain:HBI_UpdateCorrectEnable success!\n");
				delete pcorrect;
				pcorrect = NULL;
			}
			else {
				::AfxMessageBox("malloc IMAGE_CORRECT_ENABLE failed!");
			}
			((CButton *)GetDlgItem(IDC_BTN_TEMPLATE_GENERATE))->SetWindowTextA(_T("Generate Gain Template"));
		}
	else if (enumTemplateType == FPD_DEFECT_ACQ_GROUP1 ||
		enumTemplateType == FPD_DEFECT_ACQ_GROUP2 ||
		enumTemplateType == FPD_DEFECT_ACQ_AND_TEMPLATE)
		{
			if (enumTemplateType == FPD_DEFECT_ACQ_GROUP1)
			{
				printf("DO DEFECT_ACQ_GROUP1 SUCCESS!\n");
				((CButton *)GetDlgItem(IDC_BTN_TEMPLATE_GENERATE))->SetWindowTextA(_T("Defect Template-Collection Group2"));
				// ADD +1
				m_ungroupid++;
			}
			else if (enumTemplateType == FPD_DEFECT_ACQ_GROUP2)
			{
				printf("DO DEFECT_ACQ_GROUP2 SUCCESS!\n");
				((CButton *)GetDlgItem(IDC_BTN_TEMPLATE_GENERATE))->SetWindowTextA(_T("Defect Template-Collection Group3 And Generate"));
				// ADD +1
				m_ungroupid++;
			}
			else /*if (enumTemplateType == DEFECT_ACQ_AND_TEMPLATE)*/ 
			{
				printf("DO DEFECT_ACQ_AND_TEMPLATE SUCCESS!\n");
				m_ungroupid = 0; // 重置

				// 开始下载Defect模板
				if (0 != HBI_RegProgressCallBack(theDemo->m_pFpd, DownloadCallBackFun, (void *)this))
				{
					printf("err:HBI_RegProgressCallBack failed!\n");
					return;
				}
				// int infiletype - 下载文件类型
				// 0-gain模板，1-defect模板，2-offset模板，其他-不支持
				int ret = HBI_DownloadTemplateByType(theDemo->m_pFpd, 1);
				if (ret != HBI_SUCCSS) {
					::AfxMessageBox(_T("err:Download defect template failed!"));
				}
				else
				{
					::AfxMessageBox(_T("Download defect template success!"));

					// 设置SDK参数包括校正使能
					IMAGE_CORRECT_ENABLE *pcorrect = new IMAGE_CORRECT_ENABLE;
					if (pcorrect != NULL)
					{
#if 0
						pcorrect->bFeedbackCfg = false; // false-ECALLBACK_TYPE_SET_CFG_OK Event
#else
						pcorrect->bFeedbackCfg = true;  // //true-ECALLBACK_TYPE_ROM_UPLOAD Event,
#endif
						pcorrect->ucOffsetCorrection = 0x03;
						pcorrect->ucGainCorrection   = 0x02;
						pcorrect->ucDefectCorrection = 0x02;
						pcorrect->ucDummyCorrection  = 0x00;
						ret = HBI_UpdateCorrectEnable(theDemo->m_pFpd, pcorrect);
						if (ret != HBI_SUCCSS)
							::AfxMessageBox("Defect:HBI_UpdateCorrectEnable failed!");
						else
							printf("Defect:HBI_UpdateCorrectEnable success!\n");				
						delete pcorrect;
						pcorrect = NULL;
					}
					else {
						::AfxMessageBox("malloc IMAGE_CORRECT_ENABLE failed!");
					}
				}
				//
				((CButton *)GetDlgItem(IDC_BTN_TEMPLATE_GENERATE))->SetWindowTextA(_T("Defect Template-Collection Group1"));
			}
		}
	}
}

int CTemplateTool::DoOffsetTemp()
{
	if (theDemo == NULL) {
		::AfxMessageBox("err:theDemo is NULL!");
		return -1;
	}
	if (!theDemo->m_pFpd) {
		::AfxMessageBox("warning:请初始化动态库!");
		return -1;
	}
	if (!theDemo->m_IsOpen) {
		::AfxMessageBox("warning:请连接平板!");
		return -1;
	}

	printf("DoOffsetTemp!\n");
	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(false);
	EnumIMAGE_ACQ_CMD enumTemplateType = OFFSET_TEMPLATE_TYPE;
	int ret = HBI_GenerateTemplate(theDemo->m_pFpd, enumTemplateType);
	if (ret == HBI_SUCCSS)
		printf("HBI_GenerateTemplate succss!\n");
	else 
		printf("err:HBI_GenerateTemplate failed!\n");
	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(true);
	return ret;
}

/*
    分3步：
	1、采集亮场图；
	2、生成模板；
	3、下载到固件；
*/
int CTemplateTool::DoGainTemp()
{
	if (theDemo == NULL) {
		::AfxMessageBox("err:theDemo is NULL!");
		return -1;
	}
	if (!theDemo->m_pFpd) {
		::AfxMessageBox("warning:请初始化动态库!");
		return -1;
	}
	if (!theDemo->m_IsOpen) {
		::AfxMessageBox("warning:请连接平板!");
		return -1;
	}
	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(false);
	EnumIMAGE_ACQ_CMD enumTemplateType = EnumIMAGE_ACQ_CMD::GAIN_TEMPLATE_TYPE;
	int ret = HBI_GenerateTemplate(theDemo->m_pFpd, enumTemplateType);
	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(true);
	return ret;
}

int CTemplateTool::DoDefectTemp()
{
	if (theDemo == NULL) {
		::AfxMessageBox("err:theDemo is NULL!");
		return -1;
	}
	if (!theDemo->m_pFpd) {
		::AfxMessageBox("warning:请初始化动态库!");
		return -1;
	}
	if (!theDemo->m_IsOpen) {
		::AfxMessageBox("warning:请连接平板!");
		return -1;
	}
	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(false);
	int result = -1;
	EnumIMAGE_ACQ_CMD enumTemplateType = EnumIMAGE_ACQ_CMD::DEFECT_TEMPLATE_GROUP1;
	for (int i = 0; i < 3; i++)
	{
		if (i == 0)
		{
			enumTemplateType = DEFECT_TEMPLATE_GROUP1;
			if (MessageBoxEx(NULL, "请确定剂量:正常电压+正常毫安秒的10%?", "第一组亮场图",
				MB_ICONINFORMATION | MB_YESNO,
				MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)) == IDNO)
			{
				printf("取消采集第一组亮场!\n");
				break;
			}
		}
		else if (i == 1) {
			enumTemplateType = DEFECT_TEMPLATE_GROUP2;
			if (MessageBoxEx(NULL, "请确定剂量:正常电压+正常毫安秒的50%?", "第二组亮场图",
				MB_ICONINFORMATION | MB_YESNO,
				MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)) == IDNO)
			{
				printf("取消采集第二组亮场!\n");
				break;
			}
		}
		else {
			enumTemplateType = DEFECT_TEMPLATE_GROUP3;
			if (MessageBoxEx(NULL, "请确定剂量:正常电压+正常毫安秒?", "第三组亮场图&生成模板",
				MB_ICONINFORMATION | MB_YESNO,
				MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)) == IDNO)
			{
				printf("取消采集第三组亮场!\n");
				break;
			}
		}
		// 调用接口
		result = HBI_GenerateTemplate(theDemo->m_pFpd, enumTemplateType);
		if (result != HBI_SUCCSS) {
			if (i == 0)
				printf("采集第一组亮场失败!\n");
			else if (i == 1)
				printf("采集第二组亮场失败!\n");
			else
				printf("采集第三组亮场并生成模板失败!\n");
			break;
		}
	}
	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(true);
	return result;
}


int CTemplateTool::DoDefectGroup1()
{
	if (theDemo == NULL) {
		::AfxMessageBox("err:theDemo is NULL!");
		return -1;
	}
	if (!theDemo->m_pFpd) {
		::AfxMessageBox("warning:请初始化动态库!");
		return -1;
	}
	if (!theDemo->m_IsOpen) {
		::AfxMessageBox("warning:请连接平板!");
		return -1;
	}
	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(false);
	EnumIMAGE_ACQ_CMD enumTemplateType = EnumIMAGE_ACQ_CMD::DEFECT_TEMPLATE_GROUP1;
	int ret = HBI_GenerateTemplate(theDemo->m_pFpd, enumTemplateType);
	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(true);
	return ret;
}

int CTemplateTool::DoDefectGroup2()
{
	if (theDemo == NULL) {
		::AfxMessageBox("err:theDemo is NULL!");
		return -1;
	}
	if (!theDemo->m_pFpd) {
		::AfxMessageBox("warning:请初始化动态库!");
		return -1;
	}
	if (!theDemo->m_IsOpen) {
		::AfxMessageBox("warning:请连接平板!");
		return -1;
	}
	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(false);
	EnumIMAGE_ACQ_CMD enumTemplateType = EnumIMAGE_ACQ_CMD::DEFECT_TEMPLATE_GROUP2;
	int ret = HBI_GenerateTemplate(theDemo->m_pFpd, enumTemplateType);
	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(true);
	return ret;
}

int CTemplateTool::DoDefectGroup3()
{
	if (theDemo == NULL) {
		::AfxMessageBox("err:theDemo is NULL!");
		return -1;
	}
	if (!theDemo->m_pFpd) {
		::AfxMessageBox("warning:请初始化动态库!");
		return -1;
	}
	if (!theDemo->m_IsOpen) {
		::AfxMessageBox("warning:请连接平板!");
		return -1;
	}
	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(false);
	EnumIMAGE_ACQ_CMD enumTemplateType = EnumIMAGE_ACQ_CMD::DEFECT_TEMPLATE_GROUP3;
	int ret = HBI_GenerateTemplate(theDemo->m_pFpd, enumTemplateType);
	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(true);
	return ret;
}

LRESULT CTemplateTool::OnUpdateBtnTitle(WPARAM wParam, LPARAM lParam)
{
	char *ptr = (char *)lParam;
	SetDlgItemText(IDC_BTN_TEMPLATE_GENERATE, ptr);
	return 0;
}

int CTemplateTool::DownloadTemplateCBFun(unsigned char command, int code, void *inContext)
{
	if (theDemo == NULL) {
		printf("err:theDoc is NULL!\n");
		return 0;
	}
	CTemplateTool *pObject = (CTemplateTool *)inContext;
	if (pObject == NULL) {
		printf("err:DownloadTemplateCBFun:inContext is NULL!!\n");
		return 0;
	}
	//
	int len = 0;
	switch (command)
	{
	case ECALLBACK_UPDATE_STATUS_START:   // 开始下载，初始化进度条	
		printf("handleProgressFun:ECALLBACK_UPDATE_STATUS_START\n");
		len = sprintf(theDemo->evnet_msg, "start download");
		theDemo->evnet_msg[len] = '\0';
		pObject->PostMessage(WM_DOWNLOAD_CB_MESSAGE, (WPARAM)command, (LPARAM)theDemo->evnet_msg);
		break;
	case ECALLBACK_UPDATE_STATUS_PROGRESS:// 进度 code：百分比（0~100）	
		len = sprintf(theDemo->evnet_msg, "%% %d\n", code);
		theDemo->evnet_msg[len] = '\0';
		pObject->PostMessage(WM_DOWNLOAD_CB_MESSAGE, (WPARAM)command, (LPARAM)theDemo->evnet_msg);
		break;
	case ECALLBACK_UPDATE_STATUS_RESULT:  // update result and error
		if ((0 <= code) && (code <= 6))      // 显示Uploading gain template!
		{ 	
			if (code == 0) {       // 下载offset模板
				printf("DownloadOffsetCBFun:start download offset template!\n");
				len = sprintf(theDemo->evnet_msg, "offset temp");
			}
			else if (code == 1) {  // 下载gain模板
				printf("DownloadOffsetCBFun:start download gain template!\n");
				len = sprintf(theDemo->evnet_msg, "gain temp");
			}
			else if (code == 2) {  // 下载defect模板
				printf("DownloadOffsetCBFun:start download defect template!\n");
				len = sprintf(theDemo->evnet_msg, "defect temp");
			}
			else if (code == 3) {  // offset模板上传完成
				printf("DownloadOffsetCBFun:download offset finish!\n");
				len = sprintf(theDemo->evnet_msg, "offset finish");
			}
			else if (code == 4) {  // gain模板上传完成
				printf("DownloadOffsetCBFun:download gain finish!\n");
				len = sprintf(theDemo->evnet_msg, "gain finish");
			}
			else if (code == 5) {  // defect模板上传完成
				printf("DownloadOffsetCBFun:download defect finish!\n");
				len = sprintf(theDemo->evnet_msg, "defect finish");
			}
			else/* if (code == 6)*/ {  // 模板上传成功并退出线程
				printf("DownloadOffsetCBFun:Download sucess and exit download thread!\n");
				len = sprintf(theDemo->evnet_msg, "success");
			}
			theDemo->evnet_msg[len] = '\0';
		}
		else // 失败
		{               
			if (code == -1) {
				printf("err:DownloadOffsetCBFun:wait event other error!\n");
				len = sprintf(theDemo->evnet_msg, "other error");
			}
			else if (code == -2) {
				printf("err:DownloadOffsetCBFun:timeout!\n");
				len = sprintf(theDemo->evnet_msg, "timeout");
			}
			else if (code == -3) {
				printf("err:DownloadOffsetCBFun:downlod offset failed!\n");
				len = sprintf(theDemo->evnet_msg, "offset failed");
			}
			else if (code == -4) {
				printf("err:DownloadOffsetCBFun:downlod gain failed!\n");
				len = sprintf(theDemo->evnet_msg, "gain failed");
			}
			else if (code == -5) {
				printf("err:DownloadOffsetCBFun:downlod defect failed!\n");
				len = sprintf(theDemo->evnet_msg, "defect failed");
			}
			else if (code == -6) { // 模板上传失败并退出线程
				printf("err:DownloadOffsetCBFun:Download failed and exit download thread\n");
				len = sprintf(theDemo->evnet_msg, "Download failed");
			}
			else if (code == -7) {
				printf("err:DownloadOffsetCBFun:read offset failed!\n");
				len = sprintf(theDemo->evnet_msg, "err:read failed");
			}
			else if (code == -8) {
				printf("err:DownloadOffsetCBFun:read gain failed!\n");
				len = sprintf(theDemo->evnet_msg, "err:read gain");
			}
			else if (code == -9) {
				printf("err:DownloadOffsetCBFun:read defect failed!\n");
				len = sprintf(theDemo->evnet_msg, "err:read defect");
			}
			else {
				printf("err:DownloadOffsetCBFun:unknow error,%d\n", code);
				len = sprintf(theDemo->evnet_msg, "unknow error");
			}
			theDemo->evnet_msg[len] = '\0';
		}
		pObject->PostMessage(WM_DOWNLOAD_CB_MESSAGE, (WPARAM)command, (LPARAM)theDemo->evnet_msg);
		break;
	default: // unusual
		printf("handleProgressFun:unusual,retcode=%d\n", code);
		break;
	}
	return 1;
}