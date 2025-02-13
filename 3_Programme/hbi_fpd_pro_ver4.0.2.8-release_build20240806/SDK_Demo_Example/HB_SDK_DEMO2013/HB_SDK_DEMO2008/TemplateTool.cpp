// TemplateTool.cpp : implementation file
//

#include "stdafx.h"
#include "HB_SDK_DEMO2008.h"
#include "TemplateTool.h"
#include "HB_SDK_DEMO2008Dlg.h"

#define WAITTING_TIME 5 // Wait time duration in seconds

extern CHB_SDK_DEMO2008Dlg* theDemo;

#ifndef _XSLEEP_H_
#define _XSLEEP_H_

// Structure used internally by the XSleep function
struct XSleep_Structure {
	XSleep_Structure() {
		duration = 1000; // Default duration to 1 second
		eventHandle = NULL;
	}
	int duration; // Sleep duration in milliseconds
	HANDLE eventHandle; // Event handle for signaling sleep completion
};

// Function to make the application sleep while processing messages
void XSleep(int nWaitInMSecs);

#endif // _XSLEEP_H_

//////////////////////////////////////////////////////////////////////
// Function  : XSleepThread()
// Purpose   : Thread function to handle sleep duration and signal completion
// Returns   : DWORD (Thread exit code)
// Parameters: 
//  1. LPVOID pWaitTime - Pointer to XSleep_Structure containing sleep duration
//////////////////////////////////////////////////////////////////////
DWORD WINAPI XSleepThread(LPVOID pWaitTime)
{
	XSleep_Structure* sleep = (XSleep_Structure*)pWaitTime;
	Sleep(sleep->duration);
	SetEvent(sleep->eventHandle); // Signal that sleep duration has ended
	return 0;
}

//////////////////////////////////////////////////////////////////////
// Function  : XSleep()
// Purpose   : Makes the application sleep for a specified duration while
//             processing the message pump to prevent UI freezing.
// Returns   : none
// Parameters:       
//  1. nWaitInMSecs - Duration to sleep specified in miliseconds.
//////////////////////////////////////////////////////////////////////
void XSleep(int nWaitInMSecs)
{
	XSleep_Structure sleep;
	sleep.duration = nWaitInMSecs;
	sleep.eventHandle = CreateEvent(NULL, TRUE, FALSE, NULL); // Create event for signaling

	// Create a new thread for sleeping
	HANDLE getHandle = (HANDLE)_beginthreadex(NULL, 0, (unsigned int(__stdcall*)(void*))XSleepThread, &sleep, 0, NULL);
	if (getHandle == NULL)
		return;

	MSG msg;
	while (::WaitForSingleObject(sleep.eventHandle, 0) == WAIT_TIMEOUT)
	{
		// Process messages while waiting
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	// Cleanup handles
	CloseHandle(sleep.eventHandle);
	CloseHandle(getHandle);
}

// Callback function to handle the download progress and update the UI accordingly.
static int DownloadCallBackFun(unsigned char command, int code, void* inContext)
{
	CTemplateTool* ptr = (CTemplateTool*)inContext;
	if (ptr == NULL) {
		printf("Error: inContext is NULL!\n");
		return 0;
	}
	if (theDemo == NULL) {
		printf("Error: theDemo is NULL!\n");
		return 0;
	}

	int len = 0;
	switch (command)
	{
	case ECALLBACK_UPDATE_STATUS_START:
		// Initializing the progress bar	
		printf("handleProgressFun:ECALLBACK_UPDATE_STATUS_START\n");
		len = sprintf(theDemo->evnet_msg, "start download");
		theDemo->evnet_msg[len] = '\0';
		ptr->PostMessage(WM_DOWNLOAD_CB_MESSAGE, (WPARAM)command, (LPARAM)theDemo->evnet_msg);
		break;

	case ECALLBACK_UPDATE_STATUS_PROGRESS:
		// Updating progress percentage (0 to 100)
		len = sprintf(theDemo->evnet_msg, "%% %d", code);
		theDemo->evnet_msg[len] = '\0';
		ptr->PostMessage(WM_DOWNLOAD_CB_MESSAGE, (WPARAM)command, (LPARAM)theDemo->evnet_msg);
		break;

	case ECALLBACK_UPDATE_STATUS_RESULT:
		// Handling download result based on the return code

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
			// Error handling

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

	default: 
		// Unknown event handling
		printf("DownloadCallBackFun:unusual,retcode=%d\n", code);
		break;
	}
	return 1;
}

// CTemplateTool dialog class implementation

IMPLEMENT_DYNAMIC(CTemplateTool, CDialog)


// Constructor: Initializes the dialog and sets the default template type.
CTemplateTool::CTemplateTool(CWnd* pParent /*=NULL*/)
	: CDialog(CTemplateTool::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_TEMPLATE); // Set the dialog icon
	enumTemplateType = FPD_OFFSET_TEMPLATE;
	m_ungroupid = 0;
}

// Destructor: Cleans up any allocated resources.
CTemplateTool::~CTemplateTool()
{
}

// Data exchange function for dialog controls.
void CTemplateTool::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

// Message map for handling UI events.
BEGIN_MESSAGE_MAP(CTemplateTool, CDialog)
	ON_BN_CLICKED(IDC_RADIO_OFFSET, &CTemplateTool::OnBnClickedRadioOffset)
	ON_BN_CLICKED(IDC_RADIO_GAIN, &CTemplateTool::OnBnClickedRadioGain)
	ON_BN_CLICKED(IDC_RADIO_DEFECT, &CTemplateTool::OnBnClickedRadioDefect)
	ON_BN_CLICKED(IDC_BTN_TEMPLATE_GENERATE, &CTemplateTool::OnBnClickedBtnTemplateGenerate)
	ON_MESSAGE(WM_USER_NOTICE_TEMPLATE_TOOL, &CTemplateTool::OnGenerateTemplateResult)
	ON_MESSAGE(WM_DOWNLOAD_CB_MESSAGE, &CTemplateTool::OnUpdateBtnTitle)
END_MESSAGE_MAP()

// Initializes the dialog box.
BOOL CTemplateTool::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);   // Set large icon
	SetIcon(m_hIcon, FALSE);  // Set small icon

	// Initialize parameters
	CheckRadioButton(IDC_RADIO_GAIN, IDC_RADIO_DEFECT, IDC_RADIO_OFFSET);
	if (m_ungroupid != 0) m_ungroupid = 0;

	return TRUE;  // Return TRUE unless you set the focus to a control
}

// Handles the selection of the "Offset Template" radio button.
void CTemplateTool::OnBnClickedRadioOffset()
{
	if (enumTemplateType != FPD_OFFSET_TEMPLATE) {
		enumTemplateType = FPD_OFFSET_TEMPLATE;
		((CButton*)GetDlgItem(IDC_BTN_TEMPLATE_GENERATE))->SetWindowTextA("Generate Offset Template");
	}
}

// Handles the selection of the "Gain Template" radio button.
void CTemplateTool::OnBnClickedRadioGain()
{
	if (enumTemplateType != FPD_GAIN_TEMPLATE) {
		enumTemplateType = FPD_GAIN_TEMPLATE;
		((CButton*)GetDlgItem(IDC_BTN_TEMPLATE_GENERATE))->SetWindowTextA("Generate Gain Template");
	}
}

// Handles the selection of the "Defect Template" radio button.
void CTemplateTool::OnBnClickedRadioDefect()
{
	// Check if the selected template type is different from "Defect Collection Group 1"
	if (enumTemplateType != FPD_DEFECT_ACQ_GROUP1) {
		enumTemplateType = FPD_DEFECT_ACQ_GROUP1;

		// Reset group ID if previously set
		if (m_ungroupid != 0)
			m_ungroupid = 0;

		// Update button text to reflect the selected defect collection group
		((CButton*)GetDlgItem(IDC_BTN_TEMPLATE_GENERATE))->SetWindowTextA("Defect Template - Collection Group 1");
	}
}

// Handles the results of template generation and provides user feedback.
LRESULT CTemplateTool::OnGenerateTemplateResult(WPARAM wparam, LPARAM lparam)
{
	int result = (int)wparam;

	// Handle different outcomes of template generation
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


// Handles the process of generating various types of templates (Offset, Gain, Defect).
// This function verifies the detector's connection, identifies the selected template type, 
// and executes the corresponding generation process. It also includes confirmation prompts 
// for Gain and Defect templates and initiates the download of Gain and Defect templates after generation.

void CTemplateTool::OnBnClickedBtnTemplateGenerate()
{
	// Validate Detector Conenction
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

	// Determine the selected template type from radio buttons
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

	// Start template generation
	int ret = -1;
	if (enumTemplateType == OFFSET_TEMPLATE) {
		ret = DoOffsetTemp();
	}

	else if (enumTemplateType == GAIN_TEMPLATE) {
		// Confirm the gain template acquisition settings
		if (MessageBoxEx(NULL, "Confirm Gain Settings: Normal Bias + Normal Exposure Time?",
			"Light Field Image", MB_ICONINFORMATION | MB_YESNO, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)) == IDNO)
		{
			return;
		}
		ret = DoGainTemp();
	}

	else if (enumTemplateType == FPD_DEFECT_ACQ_GROUP1 ||
		enumTemplateType == FPD_DEFECT_ACQ_GROUP2 ||
		enumTemplateType == FPD_DEFECT_ACQ_AND_TEMPLATE)
	{
		// Confirm the defect template acquisition settings
		if (enumTemplateType == FPD_DEFECT_ACQ_GROUP1) {
			if (MessageBoxEx(NULL, "Confirm Gain Settings: Normal Bias + 10% Normal Exposure Time?",
				"First Group Light Field Image", MB_ICONINFORMATION | MB_YESNO,
				MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)) == IDNO) {
				return;
			}
			ret = DoDefectGroup1();
		}

		else if (enumTemplateType == FPD_DEFECT_ACQ_GROUP2) {
			if (MessageBoxEx(NULL, "Confirm Gain Settings: Normal Bias + 50% Normal Exposure Time?",
				"Second Group Light Field Image", MB_ICONINFORMATION | MB_YESNO,
				MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)) == IDNO) {
				return;
			}
			ret = DoDefectGroup2();
		}

		else {
			if (MessageBoxEx(NULL, "Confirm Gain Settings: Normal Bias + Normal Exposure Time?",
				"Third Group Light Field Image & Template Generation", MB_ICONINFORMATION | MB_YESNO,
				MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)) == IDNO) {
				return;
			}
			ret = DoDefectGroup3();
		}
		//ret = DoDefectTemp();
	}

	// Check if the template generation was successful
	if (ret != HBI_SUCCSS) {
		printf("Error: Template generation failed!\n");
	}

	else
	{
		// Success case: Perform next steps based on template type
		if (enumTemplateType == OFFSET_TEMPLATE)
		{
			if (m_ungroupid != 0) 
				m_ungroupid = 0; // Reset group ID

			printf("DO OFFSET_TEMPLATE_MODE SUCCESS!\n");
			::AfxMessageBox(_T("DO OFFSET_TEMPLATE_TYPE SUCCESS!"));
		}

		else if (enumTemplateType == GAIN_TEMPLATE)
		{
			if (m_ungroupid != 0) 
				m_ungroupid = 0; // Reset group ID

			printf("DO GAIN_TEMPLATE_MODE SUCCESS!\n");

			// Start downloading the Gain template
			if (0 != HBI_RegProgressCallBack(theDemo->m_pFpd, DownloadCallBackFun, (void *)this))
			{
				printf("err:HBI_RegProgressCallBack failed!\n");

				return;
			}

			int ret = HBI_DownloadTemplateByType(theDemo->m_pFpd, 0); // 0: Gain template

			if (ret != HBI_SUCCSS)
				::AfxMessageBox(_T("err:Download gain-template failed!"));

			else
				::AfxMessageBox(_T("Download gain-template success!"));	

			// Configure SDK parameters for correction settings
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

				// Move to next defect acquisition group
				m_ungroupid++;
			}

			else if (enumTemplateType == FPD_DEFECT_ACQ_GROUP2)
			{
				printf("DO DEFECT_ACQ_GROUP2 SUCCESS!\n");
				((CButton *)GetDlgItem(IDC_BTN_TEMPLATE_GENERATE))->SetWindowTextA(_T("Defect Template-Collection Group3 And Generate"));
				
				// Move to next defect acquisition group
				m_ungroupid++;
			}

			else /*if (enumTemplateType == DEFECT_ACQ_AND_TEMPLATE)*/ 
			{
				printf("DO DEFECT_ACQ_AND_TEMPLATE SUCCESS!\n");
				m_ungroupid = 0; // Reset group ID for next session

				// Start downloading the defect template
				if (0 != HBI_RegProgressCallBack(theDemo->m_pFpd, DownloadCallBackFun, (void *)this))
				{
					printf("err:HBI_RegProgressCallBack failed!\n");
					return;
				}

				// Download the defect template (Type 1: Defect template)
				int ret = HBI_DownloadTemplateByType(theDemo->m_pFpd, 1);

				if (ret != HBI_SUCCSS) {
					::AfxMessageBox(_T("err:Download defect template failed!"));
				}
				else
				{
					::AfxMessageBox(_T("Download defect template success!"));

					// Configure SDK correction parameters including defect correction enablement
					IMAGE_CORRECT_ENABLE *pcorrect = new IMAGE_CORRECT_ENABLE;
					if (pcorrect != NULL)
					{
#if 0
						pcorrect->bFeedbackCfg = false; // false-ECALLBACK_TYPE_SET_CFG_OK Event
#else
						pcorrect->bFeedbackCfg = true;  // //true-ECALLBACK_TYPE_ROM_UPLOAD Event,
#endif
						pcorrect->ucOffsetCorrection = 0x03; // Firmware PreOffset Correction
						pcorrect->ucGainCorrection = 0x02; // Firmware Gain Correction
						pcorrect->ucDefectCorrection = 0x02; // Firmware Defect Correction
						pcorrect->ucDummyCorrection = 0x00; // No Dummy Correction

						ret = HBI_UpdateCorrectEnable(theDemo->m_pFpd, pcorrect);

						if (ret != HBI_SUCCSS)
							::AfxMessageBox("Defect:HBI_UpdateCorrectEnable failed!");

						else
							printf("Defect:HBI_UpdateCorrectEnable success!\n");		

						// Free allocated memory
						delete pcorrect;

						pcorrect = NULL;
					}

					else {
						::AfxMessageBox("malloc IMAGE_CORRECT_ENABLE failed!");
					}
				}

				// Reset UI to first defect acquisition group
				((CButton *)GetDlgItem(IDC_BTN_TEMPLATE_GENERATE))->SetWindowTextA(_T("Defect Template-Collection Group1"));
			}
		}
	}
}

// Generates the offset template by capturing images and processing them.
// Ensures the detector is initialized and connected before proceeding.、
int CTemplateTool::DoOffsetTemp()
{
	if (theDemo == NULL) {
		::AfxMessageBox("err:theDemo is NULL!");
		return -1;
	}

	if (!theDemo->m_pFpd) {
		::AfxMessageBox("warning:Çë³õÊ¼»¯¶¯Ì¬¿â!");
		return -1;
	}

	if (!theDemo->m_IsOpen) {
		::AfxMessageBox("warning:ÇëÁ¬½ÓÆ½°å!");
		return -1;
	}

	printf("DoOffsetTemp!\n");

	// Disable the "Generate Template" button while processing
	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(false);

	// Set the template type to Offset Template
	EnumIMAGE_ACQ_CMD enumTemplateType = OFFSET_TEMPLATE_TYPE;

	// Generate the offset template
	int ret = HBI_GenerateTemplate(theDemo->m_pFpd, enumTemplateType);

	if (ret == HBI_SUCCSS)
		printf("HBI_GenerateTemplate succss!\n");
	else 
		printf("err:HBI_GenerateTemplate failed!\n");

	// Re-enable the button after processing
	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(true);

	return ret;
}

// Generates the gain template by capturing images, processing them, 
// and downloading the template to hardware. The process consists of:
// 1. Capturing bright-field images
// 2. Generating the gain template
// 3. Downloading the template to the hardware

int CTemplateTool::DoGainTemp()
{
	if (theDemo == NULL) {
		::AfxMessageBox("err:theDemo is NULL!");
		return -1;
	}

	if (!theDemo->m_pFpd) {
		::AfxMessageBox("Warning: Please initialize the dynamic library!");
		return -1;
	}

	if (!theDemo->m_IsOpen) {
		::AfxMessageBox("Warning: Please connect the detector!");
		return -1;
	}

	// Disable the "Generate Template" button while processing
	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(false);

	// Set the template type to Gain Template
	EnumIMAGE_ACQ_CMD enumTemplateType = EnumIMAGE_ACQ_CMD::GAIN_TEMPLATE_TYPE;

	// Generate the gain template
	int ret = HBI_GenerateTemplate(theDemo->m_pFpd, enumTemplateType);

	// Re-enable the button after processing
	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(true);

	return ret;
}

// Executes the defect template generation process by capturing multiple 
// groups of bright-field images.
int CTemplateTool::DoDefectTemp()
{
	if (theDemo == NULL) {
		::AfxMessageBox("err:theDemo is NULL!");
		return -1;
	}

	if (!theDemo->m_pFpd) {
		::AfxMessageBox("Warning: Please initialize the dynamic library!");
		return -1;
	}

	if (!theDemo->m_IsOpen) {
		::AfxMessageBox("Warning: Please connect the detector!");
		return -1;
	}

	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(false);

	int result = -1;
	EnumIMAGE_ACQ_CMD enumTemplateType = EnumIMAGE_ACQ_CMD::DEFECT_TEMPLATE_GROUP1;

	// Iterate through the three defect groups for capturing and processing images
	for (int i = 0; i < 3; i++)
	{
		if (i == 0)
		{
			enumTemplateType = DEFECT_TEMPLATE_GROUP1;
			if (MessageBoxEx(NULL,
				"Confirm the exposure: Normal voltage + 10% of normal brightness?",
				"Defect Template - Group 1",
				MB_ICONINFORMATION | MB_YESNO,
				MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)) == IDNO)
			{
				printf("Cancelled capturing for Group 1!\n");
				break;
			}
		}

		else if (i == 1) {
			enumTemplateType = DEFECT_TEMPLATE_GROUP2;
			if (MessageBoxEx(NULL,
				"Confirm the exposure: Normal voltage + 50% of normal brightness?",
				"Defect Template - Group 2",
				MB_ICONINFORMATION | MB_YESNO,
				MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)) == IDNO)
			{
				printf("Cancelled capturing for Group 2!\n");
				break;
			}
		}

		else {
			enumTemplateType = DEFECT_TEMPLATE_GROUP3;
			if (MessageBoxEx(NULL,
				"Confirm the exposure: Normal voltage + Normal brightness?",
				"Defect Template - Group 3 & Generate",
				MB_ICONINFORMATION | MB_YESNO,
				MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)) == IDNO)
			{
				printf("Cancelled capturing for Group 3!\n");
				break;
			}
		}

		// Call the API to generate the defect template
		result = HBI_GenerateTemplate(theDemo->m_pFpd, enumTemplateType);

		if (result != HBI_SUCCSS) {
			if (i == 0)
				printf("Failed to capture Group 1 bright-field image!\n");
			else if (i == 1)
				printf("Failed to capture Group 2 bright-field image!\n");
			else
				printf("Failed to capture Group 3 bright-field image and generate template!\n");
			break;
		}
	}
	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(true);

	return result;
}

// Captures and processes images for Defect Group 1 template.
int CTemplateTool::DoDefectGroup1()
{
	if (theDemo == NULL) {
		::AfxMessageBox("Error: theDemo instance is NULL!");
		return -1;
	}
	if (!theDemo->m_pFpd) {
		::AfxMessageBox("Warning: Please initialize the dynamic library!");
		return -1;
	}
	if (!theDemo->m_IsOpen) {
		::AfxMessageBox("Warning: Please connect the detector!");
		return -1;
	}

	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(false);

	EnumIMAGE_ACQ_CMD enumTemplateType = EnumIMAGE_ACQ_CMD::DEFECT_TEMPLATE_GROUP1;
	int ret = HBI_GenerateTemplate(theDemo->m_pFpd, enumTemplateType);

	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(true);
	return ret;
}

// Captures and processes images for Defect Group 2 template.
int CTemplateTool::DoDefectGroup2()
{
	if (theDemo == NULL) {
		::AfxMessageBox("Error: theDemo instance is NULL!");
		return -1;
	}
	if (!theDemo->m_pFpd) {
		::AfxMessageBox("Warning: Please initialize the dynamic library!");
		return -1;
	}
	if (!theDemo->m_IsOpen) {
		::AfxMessageBox("Warning: Please connect the detector!");
		return -1;
	}

	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(false);

	EnumIMAGE_ACQ_CMD enumTemplateType = EnumIMAGE_ACQ_CMD::DEFECT_TEMPLATE_GROUP2;
	int ret = HBI_GenerateTemplate(theDemo->m_pFpd, enumTemplateType);

	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(true);
	return ret;
}

// Captures and processes images for Defect Group 3 template.
int CTemplateTool::DoDefectGroup3()
{
	if (theDemo == NULL) {
		::AfxMessageBox("Error: theDemo instance is NULL!");
		return -1;
	}
	if (!theDemo->m_pFpd) {
		::AfxMessageBox("Warning: Please initialize the dynamic library!");
		return -1;
	}
	if (!theDemo->m_IsOpen) {
		::AfxMessageBox("Warning: Please connect the detector!");
		return -1;
	}

	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(false);

	EnumIMAGE_ACQ_CMD enumTemplateType = EnumIMAGE_ACQ_CMD::DEFECT_TEMPLATE_GROUP3;
	int ret = HBI_GenerateTemplate(theDemo->m_pFpd, enumTemplateType);

	GetDlgItem(IDC_BTN_TEMPLATE_GENERATE)->EnableWindow(true);
	return ret;
}

// Updates the button text on the UI with the current download status message.
LRESULT CTemplateTool::OnUpdateBtnTitle(WPARAM wParam, LPARAM lParam)
{
    char *ptr = (char *)lParam;
    SetDlgItemText(IDC_BTN_TEMPLATE_GENERATE, ptr);
    return 0;
}

// Callback function for handling template download status updates.
// It processes different stages of downloading offset, gain, and defect templates
// and updates the UI accordingly.
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

	int len = 0;

	switch (command)
	{
	case ECALLBACK_UPDATE_STATUS_START:   // Start download, initialize progress bar
		printf("handleProgressFun:ECALLBACK_UPDATE_STATUS_START\n");
		len = sprintf(theDemo->evnet_msg, "start download");
		theDemo->evnet_msg[len] = '\0';
		pObject->PostMessage(WM_DOWNLOAD_CB_MESSAGE, (WPARAM)command, (LPARAM)theDemo->evnet_msg);
		break;

	case ECALLBACK_UPDATE_STATUS_PROGRESS:// Download progress update (percentage 0-100)
		len = sprintf(theDemo->evnet_msg, "%% %d\n", code);
		theDemo->evnet_msg[len] = '\0';
		pObject->PostMessage(WM_DOWNLOAD_CB_MESSAGE, (WPARAM)command, (LPARAM)theDemo->evnet_msg);
		break;

	case ECALLBACK_UPDATE_STATUS_RESULT:  // Download result and error handling
		if ((0 <= code) && (code <= 6))      // Successful status updates
		{ 	
			if (code == 0) {     
				printf("DownloadOffsetCBFun:start download offset template!\n");
				len = sprintf(theDemo->evnet_msg, "offset temp");
			}

			else if (code == 1) {  
				printf("DownloadOffsetCBFun:start download gain template!\n");
				len = sprintf(theDemo->evnet_msg, "gain temp");
			}

			else if (code == 2) {  
				printf("DownloadOffsetCBFun:start download defect template!\n");
				len = sprintf(theDemo->evnet_msg, "defect temp");
			}
			else if (code == 3) {  
				printf("DownloadOffsetCBFun:download offset finish!\n");
				len = sprintf(theDemo->evnet_msg, "offset finish");
			}

			else if (code == 4) { 
				printf("DownloadOffsetCBFun:download gain finish!\n");
				len = sprintf(theDemo->evnet_msg, "gain finish");
			}

			else if (code == 5) { 
				printf("DownloadOffsetCBFun:download defect finish!\n");
				len = sprintf(theDemo->evnet_msg, "defect finish");
			}

			else/* if (code == 6)*/ { 
				printf("DownloadOffsetCBFun:Download sucess and exit download thread!\n");
				len = sprintf(theDemo->evnet_msg, "success");
			}

			theDemo->evnet_msg[len] = '\0';
		}

		else // Error handling cases
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

			else if (code == -6) { 
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

	default: // Unhandled Cases

		printf("handleProgressFun:unusual,retcode=%d\n", code);
		break;

	}

	return 1;
}