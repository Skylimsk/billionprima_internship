// DemoDll.cpp : 定义控制台应用程序的入口点。
//
/*---------------------------------------------------------------------------
* Copyright (c) 2019 上海昊博影像科技有限公司
* All rights reserved.
*
* 文件名称: DemoDll.cpp
* 文件标识:
* 摘    要: DLL 显示（动态）调用Demo
*
* 当前版本: 1.0
* 作    者: mhyang
* 完成日期: 2019/09/27
----------------------------------------------------------------------------*/
#include "stdafx.h"
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define BREAK_UINT32(var, ByteNum) \
		(unsigned char)((unsigned int)(((var) >> ((ByteNum)* 8)) & 0x00FF))
#define BUILD_UINT32(Byte0, Byte1, Byte2, Byte3) \
	((unsigned int)((unsigned int)((Byte0)& 0x00FF) \
	+ ((unsigned int)((Byte1)& 0x00FF) << 8) \
	+ ((unsigned int)((Byte2)& 0x00FF) << 16) \
	+ ((unsigned int)((Byte3)& 0x00FF) << 24)))

// Header file
#define _DLL_EX_IM 0
#include "HbiFpd.h"

// application program interface
typedef void *(__stdcall *DLL_INIT_INSTANCE)(int); // init dll instance
typedef int(__stdcall *DLL_REGCALLBACK)(void *, USER_CALLBACK_HANDLE_ENVENT, void *); // regeidt callback func
typedef int(__stdcall *DLL_CONN_DETECTOR)(void *, COMM_CFG, int);     // connect detector
typedef int(__stdcall *DLL_DISCONN_DETECTOR)(void *);  // disconnect detector
typedef int(__stdcall *DLL_UPDATE_TRIGGER_CORRECTENABLE)(void *, int, IMAGE_CORRECT_ENABLE*);  // Update trigger mode and correction enable status.
typedef int(__stdcall *DLL_GETCFG)(void *, RegCfgInfo*); // get the parameters of the detector firmware.
typedef int(__stdcall *DLL_GET_IMAGING_PROPERTY)(void *, IMAGE_PROPERTY *); // Get image properties
typedef int(__stdcall *DLL_DESTROOY)(void *);  // Release dynamic library
typedef int(__stdcall *DLL_SINGLE_PREPARE)(void *);            // single shot mode1
typedef int(__stdcall *DLL_SINGLE_SHOT)(void *, FPD_AQC_MODE); // single shot mode2
typedef int(__stdcall *DLL_LIVE_ACQ)(void *, FPD_AQC_MODE);    // live acquisition
typedef int(__stdcall *DLL_STOP_LIVE_ACQ)(void *);  // stop live acquisition
// ADD BY MHYANG 20220616
typedef int(__stdcall *DLL_GENERATE_TEMPLATE)(void *, EnumIMAGE_ACQ_CMD, int); // Generate Template
typedef int(__stdcall *DLL_REG_PROGRESS_FUNC)(void *, USER_CALLBACK_HANDLE_PROCESS, void *);  // regedit download progress callback func
typedef int(__stdcall *DLL_DOWNLOAD_TEMPLATE)(void *, int);  // download template
typedef int(__stdcall *DLL_UPDATE_CORRECTENABLE)(void *, IMAGE_CORRECT_ENABLE*);  // Update correction enable status.

// 
DLL_INIT_INSTANCE InitDll = NULL;
DLL_REGCALLBACK RegEventCallBackFun = NULL;
DLL_CONN_DETECTOR ConnectDetector = NULL;
DLL_DISCONN_DETECTOR DisConnectDetector = NULL;
DLL_UPDATE_TRIGGER_CORRECTENABLE TriggerCorrectEnable = NULL;
DLL_GETCFG GetFpdCfgInfo = NULL;
DLL_GET_IMAGING_PROPERTY GetImagingProperty = NULL;
DLL_DESTROOY DestroyDll = NULL;
DLL_SINGLE_PREPARE SinglePrepare = NULL;
DLL_SINGLE_SHOT SingleShot = NULL;
DLL_LIVE_ACQ LiveAcq = NULL;
DLL_STOP_LIVE_ACQ StopLiveAcq = NULL;
// ADD BY MHYANG 20220616
DLL_GENERATE_TEMPLATE GenerateTemplate = NULL;
DLL_REG_PROGRESS_FUNC RegProgressCallBackFun = NULL;
DLL_DOWNLOAD_TEMPLATE DownloadTemplate = NULL;
DLL_UPDATE_CORRECTENABLE UpdateCorrect = NULL;

// Global function
void Usage();
int handleCommandEventA(void* _contex, int nDevId, unsigned char byteEventId, void *buff, int param2, int param3, int param4);
int DownloadCallBackFun(unsigned char command, int code, void *inContext);
int GetPGA(unsigned short usValue);
void PrintDetectorCfg(RegCfgInfo *pCfg);
int SaveImage(unsigned char *imgbuff, int nbufflen, unsigned int uframeid);

// global variable
HINSTANCE hInst = NULL;
void *pFpd = NULL;
COMM_CFG commCfg;
int dooffsetTemplate = 0;
RegCfgInfo *pLastRegCfg = NULL;
char szPath[MAX_PATH];
int m_imgW = 0;
int m_imgH = 0;
IMAGE_PROPERTY img_pro;
FPD_AQC_MODE aqc_mode;
IMAGE_CORRECT_ENABLE *pcorrect = NULL;
int triggermode = 7;
unsigned int prepareTime;
unsigned int liveAcqTime;
int nBinning;
unsigned int uMaxFps;
int nPGALevel = 0;
EnumIMAGE_ACQ_CMD enumTemplateType = OFFSET_TEMPLATE_TYPE;
int infiletype = 0;// Template file type

int _tmain(int argc, _TCHAR* argv[])
{
	printf("----------------------s-t-a-r-t------------------\n");
	
	/*
	* Get project path
	*/	
	memset(szPath, 0x00, MAX_PATH);
	::GetModuleFileName(NULL, szPath, MAX_PATH);
	(_tcsrchr(szPath, _T('\\')))[1] = 0;

	/*
	* Find dynamic library files
	*/ 
	char dllname[MAX_PATH];
	memset(dllname, 0x00, MAX_PATH);
	sprintf(dllname, "%sHBISDKApi.dll", szPath);
	if ((_access(dllname, 0)) == -1){
		printf("err:Dynamic library file does not exist!{%s}\n", dllname);
		return 0;
	}

	// the config of detector firmware	
	pLastRegCfg = new RegCfgInfo;
	if (pLastRegCfg == NULL) 
	{
		printf("err:pLastRegCfg-malloc failed\n");
		return 0;
	}

	//
	triggermode = 7;
	pcorrect = new IMAGE_CORRECT_ENABLE;
	if (pcorrect == NULL)
	{
		printf("err:pcorrect-malloc failed!\n");
		return 0;
	}

	// Communication parameters
//	commCfg._type = FPD_COMM_TYPE::UDP_COMM_TYPE;
//	commCfg._type = FPD_COMM_TYPE::PCIE_COMM_TYPE;
//	commCfg._type = FPD_COMM_TYPE::WALN_COMM_TYPE;
	commCfg._type = FPD_COMM_TYPE::UDP_JUMBO_COMM_TYPE;// 3025FQI-10FPS
	commCfg._loacalPort = 32896;
	commCfg._remotePort = 32897;
	int j = sprintf(commCfg._localip, "%s", "192.168.10.20");
	commCfg._localip[j] = '\0';
	j = sprintf(commCfg._remoteip, "%s", "192.168.10.40");
	commCfg._remoteip[j] = '\0';

	/*
	* Load dynamic library
	*/ 
	hInst = LoadLibrary(dllname);
	if (hInst == NULL) 
	{
		printf("Failed to load dynamic library!{%s}\n", dllname);
		return 0;
	}

	// 
	InitDll              = (DLL_INIT_INSTANCE)GetProcAddress(hInst, "HBI_Init");                  // init dll instance
	RegEventCallBackFun  = (DLL_REGCALLBACK)GetProcAddress(hInst, "HBI_RegEventCallBackFun");     // regeidt callback func
	ConnectDetector      = (DLL_CONN_DETECTOR)GetProcAddress(hInst, "HBI_ConnectDetector");       // connect detector
	DisConnectDetector   = (DLL_DISCONN_DETECTOR)GetProcAddress(hInst, "HBI_DisConnectDetector"); // disconnect detector
	TriggerCorrectEnable = (DLL_UPDATE_TRIGGER_CORRECTENABLE)GetProcAddress(hInst, "HBI_TriggerAndCorrectApplay");// Update trigger mode and correction enable status.
	GetFpdCfgInfo        = (DLL_GETCFG)GetProcAddress(hInst, "HBI_GetFpdCfgInfo");                  // get the parameters of the detector firmware.
	GetImagingProperty   = (DLL_GET_IMAGING_PROPERTY)GetProcAddress(hInst, "HBI_GetImageProperty"); // Get image properties
	DestroyDll           = (DLL_DESTROOY)GetProcAddress(hInst, "HBI_Destroy");             // Release dynamic library
	SinglePrepare        = (DLL_SINGLE_PREPARE)GetProcAddress(hInst, "HBI_SinglePrepare"); // single shot mode1
	SingleShot           = (DLL_SINGLE_SHOT)GetProcAddress(hInst, "HBI_SingleAcquisition");// single shot mode2
	LiveAcq              = (DLL_LIVE_ACQ)GetProcAddress(hInst, "HBI_LiveAcquisition");     // live acquisition
	StopLiveAcq          = (DLL_STOP_LIVE_ACQ)GetProcAddress(hInst, "HBI_StopAcquisition");// stop live acquisition
	// ADD BY MHYANG 20220616
	GenerateTemplate       = (DLL_GENERATE_TEMPLATE)GetProcAddress(hInst, "HBI_GenerateTemplate");      // Generate Template
	RegProgressCallBackFun = (DLL_REG_PROGRESS_FUNC)GetProcAddress(hInst, "HBI_RegProgressCallBack");   // regedit download progress callback func
	DownloadTemplate       = (DLL_DOWNLOAD_TEMPLATE)GetProcAddress(hInst, "HBI_DownloadTemplateByType");// download template
	UpdateCorrect          = (DLL_UPDATE_CORRECTENABLE)GetProcAddress(hInst, "HBI_UpdateCorrectEnable");// update correct enable status
	//
	if (InitDll == NULL || RegEventCallBackFun == NULL || TriggerCorrectEnable == NULL
		|| ConnectDetector == NULL || GetFpdCfgInfo == NULL || GetImagingProperty == NULL
		|| DisConnectDetector == NULL || DestroyDll == NULL || SinglePrepare == NULL
		|| SingleShot == NULL || LiveAcq == NULL || StopLiveAcq == NULL
		|| GenerateTemplate == NULL || RegProgressCallBackFun == NULL || DownloadTemplate == NULL
		|| UpdateCorrect == NULL)
	{
		FreeLibrary(hInst);
		hInst = NULL;
		delete pLastRegCfg;
		pLastRegCfg = NULL;
		printf("err:Failed to retrieve the address of the output library function in the specified dynamic link library (DLL)!{%s}\n", dllname);
		return 0;
	}

	/*
	* init dll
	*/
	printf("1、InitDll\n");
	pFpd = InitDll(0); // default detector id is 0
	if (pFpd == NULL) 
	{
		printf("init dll failed!\n");
		goto EXIT_EXIT_MAIN;
	}

	/*
	* regeidt callback func
	*/
	printf("2、RegEventCallBackFun\n");
	int ret = RegEventCallBackFun(pFpd, handleCommandEventA, NULL/*(void *)this*/); // the pointer of position machine object,Can be null ptr
	if (0 != ret) 
	{
		printf("err:regeidt callback func!Return code=%d\n", ret);
		goto EXIT_EXIT_MAIN;
	}

	/*
	* connect detector
	*/
	printf("3、ConnectDetector\n");
	ret = ConnectDetector(pFpd, commCfg, dooffsetTemplate);
	if (0 != ret) 
	{
		printf("err:connect faield!Return code=%d\n", ret);
		goto EXIT_EXIT_MAIN;
	}

	/*
	* Update trigger mode and correction enable status
	*/
	printf("4、TriggerCorrectEnable\n");
#if 0
	pcorrect->bFeedbackCfg = false; // false-ECALLBACK_TYPE_SET_CFG_OK Event
#else
	pcorrect->bFeedbackCfg = true;  // true-ECALLBACK_TYPE_ROM_UPLOAD Event,
#endif
	pcorrect->ucOffsetCorrection = 0X03;
	pcorrect->ucGainCorrection   = 0X02;
	pcorrect->ucDefectCorrection = 0X02;
	pcorrect->ucDummyCorrection  = 0X00;
	ret = TriggerCorrectEnable(pFpd, triggermode, pcorrect);
	if (0 != ret)
	{
		printf("err:Update trigger mode and correction enable status!Return code=%d\n", ret);
		goto EXIT_EXIT_MAIN;
	}

	Usage();
	printf("\nPlease enter:");

	// Enter action options
	char ch1 = '\0';
	while ((ch1 = getchar()) != 'q')
	{
		switch (ch1)
		{
		case '0':// single shot mode1-clear up: 
			ret = SinglePrepare(pFpd);
			if (ret != 0)
				printf("SinglePrepare failed!\n");
			else
				printf("SinglePrepare success!\n");
		//	Sleep(2000);
			break;
		case '1':// single shot mode2 
			aqc_mode.eAqccmd = SINGLE_ACQ_DEFAULT_TYPE;
			aqc_mode.nAcqnumber = 0; // default 0.
			aqc_mode.nframeid   = 0;
			aqc_mode.ngroupno   = 0;
			aqc_mode.ndiscard   = 0;
			aqc_mode.eLivetype  = ONLY_IMAGE; //Only Image
			ret = SingleShot(pFpd, aqc_mode);
			if (ret != 0)
				printf("SingleShot failed!\n");
			else
				printf("SingleShot success!\n");
			break;
		case '2':// live acquisition	
			aqc_mode.eLivetype  = ONLY_IMAGE;//Only Image
			aqc_mode.eAqccmd    = LIVE_ACQ_DEFAULT_TYPE; // command
			aqc_mode.nAcqnumber = 0;// Number of acquisition frames,when 0,To end, you need to send the stop command.
			aqc_mode.nframeid   = 0;
			aqc_mode.ngroupno   = 0;
			aqc_mode.ndiscard   = 0;
			ret = LiveAcq(pFpd, aqc_mode);
			if (ret != 0)
				printf("LiveAcq failed!\n");
			else
				printf("LiveAcq success!\n");
			break;
		case '3':// stop live acquisition	
			ret = StopLiveAcq(pFpd);
			if (ret != 0)
				printf("StopLiveAcq failed!\n");
			else
				printf("StopLiveAcq success!\n");
			printf("\nPlease enter:");
			break;
		case '4': // get the parameters of the detector firmware
			ret = GetFpdCfgInfo(pFpd, pLastRegCfg);
			if (0 != ret)
			{
				printf("err:get the parameters of the detector firmware failed!\n");
				goto EXIT_EXIT_MAIN;
			}
			// Print parameters
			PrintDetectorCfg(pLastRegCfg);
			printf("\nPlease enter:");
			break;
		case '5': // Get image properties
			ret = GetImagingProperty(pFpd, &img_pro);
			if (ret == 0)
			{
				printf("HBI_GetImageProperty:\n\tnnFpdNum=%u\n", img_pro.nFpdNum);
				if (m_imgW != img_pro.nwidth) m_imgW = img_pro.nwidth;
				if (m_imgH != img_pro.nheight) m_imgH = img_pro.nheight;
				printf("\twidth=%d,hight=%d\n", m_imgW, m_imgH);

				//
				if (img_pro.datatype == 0) printf("\tdatatype is unsigned char.\n");
				else if (img_pro.datatype == 1) printf("\tdatatype is char.\n");
				else if (img_pro.datatype == 2) printf("\tdatatype is unsigned short.\n");
				else if (img_pro.datatype == 3) printf("\tdatatype is float.\n");
				else if (img_pro.datatype == 4) printf("\tdatatype is double.\n");
				else printf("\tdatatype is not support.\n");
				//
				if (img_pro.ndatabit == 0) printf("\tdatabit is 16bits.\n");
				else if (img_pro.ndatabit == 1) printf("\tdatabit is 14bits.\n");
				else if (img_pro.ndatabit == 2) printf("\tdatabit is 12bits.\n");
				else if (img_pro.ndatabit == 3) printf("\tdatabit is 8bits.\n");
				else printf("\tdatatype is unsigned char.\n");
				//
				if (img_pro.nendian == 0) printf("\tdata is little endian.\n");
				else printf("\tdata is bigger endian.\n");
				//
				printf("\tpacket_size=%u\n", img_pro.packet_size);
				printf("\tframe_size=%d\n\n", img_pro.frame_size);
			}
			else
			{
				printf("HBI_GetImageProperty failed!\n");
			}
			printf("\nPlease enter:");
			break;
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Generate calibration template
			// Please update the calibration enabling status after the template generation. very important. ???
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		case '6': // Generate offset template.
			printf("First, please close the X-ray tube.\n");
			enumTemplateType = OFFSET_TEMPLATE_TYPE;
			ret = GenerateTemplate(pFpd, enumTemplateType, 0);// The last parameter defaults to 0
			if (ret == HBI_SUCCSS)
				printf("Offset-GenerateTemplate succss!\n");
			else
				printf("err:Offset-GenerateTemplate failed!\n");
			break;
			// X-ray tube coordination is required below.
			// Tube dose control needs user code control.
			// The following is for reference dose value only, subject to the actual gray value.	
		case '7': // Generate gain template.Under normal dose, collect a group of bright field images.
			printf("First, open the X-ray tube and adjust the voltage and milliampere second to normal values.\n");
			// TODO:user codes,Adjust the high pressure value.Call interface after stabilization.
			enumTemplateType = EnumIMAGE_ACQ_CMD::GAIN_TEMPLATE_TYPE;
			ret = GenerateTemplate(pFpd, enumTemplateType, 0);// The last parameter defaults to 0
			if (ret == HBI_SUCCSS)
			{
				printf("Gain-GenerateTemplate succss!\n");

				// Upload template file to firmware
				if (0 != RegProgressCallBackFun(pFpd, DownloadCallBackFun, NULL/*(void *)this*/)) //    
				{
					printf("err:Gain-RegProgressCallBackFun failed!\n");
					break;
				}
				else
					printf("Gain-RegProgressCallBackFun success!\n");

				// int infiletype - Template file type
				// 0-gain template，1-defect template，2-offset template，others-invalid.
				infiletype = 0;
				int ret = DownloadTemplate(pFpd, /*0*/infiletype);
				if (ret != HBI_SUCCSS)
					printf("err:Download gain-template failed!\n");
				else
					printf("Download gain-template success!\n");
			}
			else
				printf("err:Gain-GenerateTemplate failed!\n");
			//printf("Close the X-ray tube after completion.\n");
			break;
		case '8': // Generate defect template-Collect the first group of bright field images.
			printf("First, open the X-ray tube and adjust the voltage to normal value.\n");
			printf("And adjust the milliampere second to 10%% of normal values.\n");
			// TODO:user codes,Adjust the high pressure value.Call interface after stabilization.
			enumTemplateType = EnumIMAGE_ACQ_CMD::DEFECT_TEMPLATE_GROUP1;
			ret = GenerateTemplate(pFpd, enumTemplateType, 0);// The last parameter defaults to 0
			if (ret == HBI_SUCCSS)
				printf("Defect group1-HBI_GenerateTemplate succss!\n");
			else
				printf("err:Defect group1-HBI_GenerateTemplate failed!\n");
			//printf("Close the X-ray tube after completion.\n");
			break;
		case '9': // Generate defect template-Collect the second group of bright field images.
			printf("Secondly, open the X-ray tube and adjust the voltage to normal value.\n");
			printf("And adjust the milliampere second to 50%% of normal values.\n");
			// TODO:user codes,Adjust the high pressure value.Call interface after stabilization.
			enumTemplateType = DEFECT_TEMPLATE_GROUP2;
			ret = GenerateTemplate(pFpd, enumTemplateType, 0);// The last parameter defaults to 0
			if (ret == HBI_SUCCSS)
				printf("Defect group2-HBI_GenerateTemplate succss!\n");
			else
				printf("err:Defect group2-HBI_GenerateTemplate failed!\n");
			//printf("Close the X-ray tube after completion.\n");
			break;
		case 'a': // Generate defect template-Collect the third group of bright field images and generate templates.
			printf("Finally, open the X-ray tube and adjust the voltage to normal value and adjust the milliampere second to normal values.\n");
			// TODO:user codes,Adjust the high pressure value.Call interface after stabilization.
			enumTemplateType = DEFECT_TEMPLATE_GROUP3;
			ret = GenerateTemplate(pFpd, enumTemplateType, 0);// The last parameter defaults to 0
			if (ret == HBI_SUCCSS)
			{
				printf("Defect group3-GenerateTemplate succss!\n");

				// Upload template file to firmware
				if (0 != RegProgressCallBackFun(pFpd, DownloadCallBackFun, NULL/*(void *)this*/)) //    
				{
					printf("err:RegProgressCallBackFun failed!\n");
					break;
				}
				else
					printf("defect-RegProgressCallBackFun!\n");

				// int infiletype - Template file type
				// 0-gain template，1-defect template，2-offset template，others-invalid
				infiletype = 1;
				int ret = DownloadTemplate(pFpd, /*1*/infiletype);
				if (ret != HBI_SUCCSS)
					printf("err:Download defect-template failed!\n");
				else
					printf("Download defect-template success!\n");
			}
			else
				printf("err:Defect group3-GenerateTemplate failed!\n");
			//printf("Close the X-ray tube after completion.\n");
			break;
		case 'b': // Update calibration enable status.
			if (pcorrect != NULL)
			{
#if 0
				pcorrect->bFeedbackCfg = false; // false-ECALLBACK_TYPE_SET_CFG_OK Event
#else
				pcorrect->bFeedbackCfg = true;  // //true-ECALLBACK_TYPE_ROM_UPLOAD Event,
#endif
				pcorrect->ucOffsetCorrection = 0x03;
				pcorrect->ucGainCorrection = 0x02;
				pcorrect->ucDefectCorrection = 0x02;
				pcorrect->ucDummyCorrection = 0x00;
				ret = UpdateCorrect(pFpd, pcorrect);
				if (ret != HBI_SUCCSS)
					printf("err:UpdateCorrect failed!\n");
				else
					printf("UpdateCorrect success!\n");
			}
			break;
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Generate calibration template
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		case 'q': // exit loop
			printf("warnning:exit loop!\n");
			goto EXIT_EXIT_MAIN;
		default:
		//	Usage();
			break;
		}
		//	printf("\nPlease enter:");
	}
EXIT_EXIT_MAIN:	
	//Release dynamic library
	if (pFpd != NULL) 
	{
		// DisConnectDetector(pFpd);/* disconnect detector */
		DestroyDll(pFpd);    // include DisConnectDetector and free dll resource
		pFpd = NULL;
	}
	if (hInst != NULL) {
		FreeLibrary(hInst);
		hInst = NULL;
	}
	if (pLastRegCfg != NULL) {
		delete pLastRegCfg;
		pLastRegCfg = NULL;
	}
	if (pcorrect != NULL)
	{
		delete pcorrect;
		pcorrect = NULL;
	}
	printf("----------------------e-n-d------------------\n");
	return 0;
}

// callback func:Static member function or global function.
int handleCommandEventA(void* _contex, int nDevId, unsigned char byteEventId, void *pvParam, int nlength, int param3, int param4)
{
	int ret = 1;

	////////////////////////////////////////////////////////////////////////////////////////////////////	
	ImageData *imagedata = NULL;
	RegCfgInfo *pRegCfg = NULL;
	if ((byteEventId == ECALLBACK_TYPE_ROM_UPLOAD) || (byteEventId == ECALLBACK_TYPE_RAM_UPLOAD) ||
		(byteEventId == ECALLBACK_TYPE_FACTORY_UPLOAD))
	{
		if (pvParam == NULL || nlength == 0)
		{
			printf("err:Parameter exception!\n");
			return ret;
		}
		// Prevent parameter conflicts
		if (0 != nDevId) // HBI_Init(0);
		{
			printf("warnning:handleCommandEventA:nDevId=%d,eventid=0x%02x,nParam2=%d\n", nDevId, byteEventId, nlength);
			return ret;
		}
	}
	else if ((byteEventId == ECALLBACK_TYPE_SINGLE_IMAGE) || (byteEventId == ECALLBACK_TYPE_MULTIPLE_IMAGE) ||
		(byteEventId == ECALLBACK_TYPE_PREVIEW_IMAGE) || (byteEventId == ECALLBACK_TYPE_OFFSET_TMP) ||
		(byteEventId == ECALLBACK_OVERLAY_16BIT_IMAGE) || (byteEventId == ECALLBACK_OVERLAY_32BIT_IMAGE))
	{
		if (pvParam == NULL || nlength == 0)
		{
			printf("err:Parameter exception!\n");
			return ret;
		}
		// Prevent parameter conflicts
		if (0 != nDevId) // HBI_Init(0);
		{
			printf("warnning:handleCommandEventA:nDevId=%d,eventid=0x%02x,nParam2=%d\n", nDevId, byteEventId, nlength);
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

	switch (byteEventId)
	{
	case ECALLBACK_TYPE_FPD_STATUS: // detetor status：connect/disconnect/ready/busy and so on
		if (nlength <= 0 && nlength >= -11)
		{
			if (nlength == 0)
				printf("ECALLBACK_TYPE_FPD_STATUS,Err:Network not connected!\n");
			else if (nlength == -1)
				printf("ECALLBACK_TYPE_FPD_STATUS,Err:Parameter exception!\n");
			else if (nlength == -2)
				printf("ECALLBACK_TYPE_FPD_STATUS,Err:Failed to return the number of ready descriptors!\n");
			else if (nlength == -3)
				printf("ECALLBACK_TYPE_FPD_STATUS,Err:Receive timeout!\n");
			else if (nlength == -4)
				printf("ECALLBACK_TYPE_FPD_STATUS,Err:Receive failed!\n");
			else if (nlength == -5)
				printf("ECALLBACK_TYPE_FPD_STATUS,Err:socket-Port unreadable!\n");
			else if (nlength == -6)
				printf("ECALLBACK_TYPE_FPD_STATUS,network card unusual!\n");
			else if (nlength == -7)
				printf("ECALLBACK_TYPE_FPD_STATUS,network card ok!\n");
			else if (nlength == -8)
				printf("ECALLBACK_TYPE_FPD_STATUS:update Firmware end!\n");
			else if (nlength == -9)
				printf("ECALLBACK_TYPE_FPD_STATUS:光Fiber disconnected!\n");
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
			// error,diconnect detector
			if (nlength <= 0 && nlength >= -10)
			{
				DisConnectDetector(pFpd);
			}
		}
		break;
	case ECALLBACK_TYPE_SET_CFG_OK: // Parameter setting succeeded,when bFeedbackCfg is 0.
		printf("ECALLBACK_TYPE_SET_CFG_OK:Reedback set rom param succuss!\n");
		break;
	case ECALLBACK_TYPE_ROM_UPLOAD:/* the config of detector firmware,when bFeedbackCfg is 1.*/
		printf("ECALLBACK_TYPE_ROM_UPLOAD:\n");
		{			
			pRegCfg = pLastRegCfg;
			if (pRegCfg != NULL)
			{
				memset(pRegCfg, 0x00, sizeof(RegCfgInfo));
				memcpy(pRegCfg, (unsigned char *)pvParam, sizeof(RegCfgInfo));

				// print detector config
				PrintDetectorCfg(pRegCfg);
			}
		}
		break;
	////case ECALLBACK_TYPE_SINGLE_IMAGE:   // Single image
	////	printf("handleCommandEventA:ECALLBACK_TYPE_SINGLE_IMAGE\n");
	////	if (m_imgW != imagedata->uwidth) m_imgW = imagedata->uwidth;  // image width
	////	if (m_imgH != imagedata->uheight) m_imgH = imagedata->uheight; // image height
	////	// save image
	////	ret = SaveImage((unsigned char *)imagedata->databuff, imagedata->datalen, imagedata->uframeid);
	////	if (ret)
	////		printf("theDemo->SaveImage succss!Frame ID:%05d\n", imagedata->uframeid);
	////	else
	////		printf("theDemo->SaveImage failed!Frame ID:%05d\n", imagedata->uframeid);
	////	printf("\nPlease enter:");
	////	break;
	case ECALLBACK_TYPE_SINGLE_IMAGE:   // Single image
	case ECALLBACK_TYPE_MULTIPLE_IMAGE: // Live image
		printf("handleCommandEventA:ECALLBACK_TYPE_MULTIPLE_IMAGE\n");
		if (m_imgW != imagedata->uwidth) m_imgW = imagedata->uwidth;  // image width
		if (m_imgH != imagedata->uheight) m_imgH = imagedata->uheight; // image height
		// save image
		ret = SaveImage((unsigned char *)imagedata->databuff, imagedata->datalen, imagedata->uframeid);
		if (ret)
			printf("theDemo->SaveImage succss!Frame ID:%05d\n", imagedata->uframeid);
		else
			printf("theDemo->SaveImage failed!Frame ID:%05d\n", imagedata->uframeid);

		printf("\nPlease enter:");
		break;
	case ECALLBACK_TYPE_GENERATE_TEMPLATE: // generate template
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
	case ECALLBACK_OVERLAY_16BIT_IMAGE: // bit-16 Overlay Image
		if (m_imgW != imagedata->uwidth) m_imgW = imagedata->uwidth;
		if (m_imgH != imagedata->uheight) m_imgH = imagedata->uheight;
		//
		ret = SaveImage((unsigned char *)imagedata->databuff, imagedata->datalen, imagedata->uframeid);
		if (ret)
			printf("theDemo->SaveImage succss!Frame ID:%05d\n", imagedata->uframeid);
		else
			printf("theDemo->SaveImage failed!Frame ID:%05d\n", imagedata->uframeid);
		break;
	case ECALLBACK_OVERLAY_32BIT_IMAGE: // bit-32 Overlay Image
		printf("pImgPage->UpdateOverlay32BitImage failed,frame_%d,eventid=%d", imagedata->uframeid, param4);
		break;
	case ECALLBACK_TYPE_THREAD_EVENT: // Listening thread exception 
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
			printf("ECALLBACK_TYPE_THREAD_EVENT,Err:other error,[%d]\n", nlength);
	}
	break;
	case ECALLBACK_TYPE_FILE_NOTEXIST: // file is not exist
		if (pvParam != NULL)
			printf("err:%s not exist!\n", (char *)pvParam);
		break;
	default:
		printf("ECALLBACK_TYPE_INVALVE,command=%02x\n", byteEventId);
		break;
	}

	return ret;
}

// Upload correction template callback function:Static member function or global function.
// inContext:position machine object pointer,Can be NULL.
static int DownloadCallBackFun(unsigned char command, int code, void *inContext)
{
	////CTemplateTool *ptr = (CTemplateTool *)inContext;
	////if (ptr == NULL) {
	////	printf("err:inContext is NULL!\n");
	////	return 0;
	////}
	////if (theDemo == NULL) {
	////	printf("err:theDemo is NULL!\n");
	////	return 0;
	////}
	int len = 0;
	switch (command)
	{
	case ECALLBACK_UPDATE_STATUS_START:   // Download start
		printf("DownloadCallBackFun:ECALLBACK_UPDATE_STATUS_START:Download start\n");
		break;
	case ECALLBACK_UPDATE_STATUS_PROGRESS:// speed of progress.code：percentage（0~100）	
		printf("DownloadCallBackFun:ECALLBACK_UPDATE_STATUS_PROGRESS:%%%d\n", code);
		break;
	case ECALLBACK_UPDATE_STATUS_RESULT:  // update result and error
		if ((0 <= code) && (code <= 6)) //  success
		{
			if (code == 0) {       // download Offset template
				printf("DownloadOffsetCBFun:%s\n", "download Offset template!\n");
			}
			else if (code == 1) {  // download Gain template
				printf("DownloadOffsetCBFun:%s\n", "download Gain template!\n");
			}
			else if (code == 2) {  // download Defect template
				printf("DownloadOffsetCBFun:%s\n", "download Defect template!\n");
			}
			else if (code == 3) {  // offset template download completed
				printf("DownloadOffsetCBFun:%s\n", "Offset template download completed!\n");
			}
			else if (code == 4) {  // Gain template download completed
				printf("DownloadOffsetCBFun:%s\n", "download gain finish!\n");
			}
			else if (code == 5) {  // Defect template download completed
				printf("DownloadOffsetCBFun:%s\n", "Defect template download completed!\n");
			}
			else/* if (code == 6)*/ {  // download success and exit thread
				printf("DownloadOffsetCBFun:%s\n", "Download success and exit thread!\n");
			}
		}
		else // failed
		{
			if (code == -1) {
				printf("err:DownloadOffsetCBFun:%s\n", "wait event other error!\n");
			}
			else if (code == -2) {
				printf("err:DownloadOffsetCBFun:%s\n", "timeout!\n");
			}
			else if (code == -3) {
				printf("err:DownloadOffsetCBFun:%s\n", "downlod offset failed!\n");
			}
			else if (code == -4) {
				printf("err:DownloadOffsetCBFun:%s\n", "downlod gain failed!\n");
			}
			else if (code == -5) {
				printf("err:DownloadOffsetCBFun:%s\n", "downlod defect failed!\n");
			}
			else if (code == -6) {
				printf("err:DownloadOffsetCBFun:%s\n", "Download failed and exit thread!\n");
			}
			else if (code == -7) {
				printf("err:DownloadOffsetCBFun:%s\n", "read offset failed!\n");
			}
			else if (code == -8) {
				printf("err:DownloadOffsetCBFun:%s\n", "read gain failed!\n");
			}
			else if (code == -9) {
				printf("err:DownloadOffsetCBFun:%s\n", "read defect failed!\n");
			}
			else {
				printf("err:DownloadOffsetCBFun:unknow error,%d\n", code);
			}
		}
		break;
	default: // unusual
		printf("DownloadCallBackFun:unusual,retcode=%d\n", code);
		break;
	}
	return 1;
}

int GetPGA(unsigned short usValue)
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

void PrintDetectorCfg(RegCfgInfo *pCfg)
{
	// TODO:  在此添加控件通知处理程序代码
	printf("\n***PrintDetectorCfg***\n");
	if (pCfg == NULL)
	{
		printf("pCfg is NULL!");
		return;
	}

	printf("\tSerial Number:%s\n", pLastRegCfg->m_SysBaseInfo.m_cSnNumber);

	/*
	* detector type,width and hight
	*/
	if (pLastRegCfg->m_SysBaseInfo.m_byPanelSize == 0x01)
		printf("\tPanelSize:0x%02x,fpd type:4343-140um\n", pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
	else if (pLastRegCfg->m_SysBaseInfo.m_byPanelSize == 0x02)
		printf("\tPanelSize:0x%02x,fpd type:3543-140um\n", pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
	else if (pLastRegCfg->m_SysBaseInfo.m_byPanelSize == 0x03)
		printf("\tPanelSize:0x%02x,fpd type:1613-125um\n", pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
	else if (pLastRegCfg->m_SysBaseInfo.m_byPanelSize == 0x04)
		printf("\tPanelSize:0x%02x,fpd type:3030-140um\n", pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
	else if (pLastRegCfg->m_SysBaseInfo.m_byPanelSize == 0x05)
		printf("\tPanelSize:0x%02x,fpd type:2530-85um\n", pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
	else if (pLastRegCfg->m_SysBaseInfo.m_byPanelSize == 0x06)
		printf("\tPanelSize:0x%02x,fpd type:3025-140um\n", pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
	else if (pLastRegCfg->m_SysBaseInfo.m_byPanelSize == 0x07)
		printf("\tPanelSize:0x%02x,fpd type:4343-100um\n", pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
	else if (pLastRegCfg->m_SysBaseInfo.m_byPanelSize == 0x08)
		printf("\tPanelSize:0x%02x,fpd type:2530-75um\n", pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
	else if (pLastRegCfg->m_SysBaseInfo.m_byPanelSize == 0x09)
		printf("\tPanelSize:0x%02x,fpd type:2121-200um\n", pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
	else if (pLastRegCfg->m_SysBaseInfo.m_byPanelSize == 0x0a)
		printf("\tPanelSize:0x%02x,fpd type:1412-50um\n", pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
	else if (pLastRegCfg->m_SysBaseInfo.m_byPanelSize == 0x0b)
		printf("\tPanelSize:0x%02x,fpd type:0606-50um\n", pLastRegCfg->m_SysBaseInfo.m_byPanelSize);
	else
		printf("\tPanelSize:0x%02x,invalid fpd type\n", pLastRegCfg->m_SysBaseInfo.m_byPanelSize);

	if (m_imgW != pLastRegCfg->m_SysBaseInfo.m_sImageWidth) m_imgW = pLastRegCfg->m_SysBaseInfo.m_sImageWidth;
	if (m_imgH != pLastRegCfg->m_SysBaseInfo.m_sImageHeight) m_imgH = pLastRegCfg->m_SysBaseInfo.m_sImageHeight;
	printf("\twidth=%d,hight=%d\n", m_imgW, m_imgH);
	printf("\tdatatype is unsigned char.\n");
	printf("\tdatabit is 16bits.\n");
	printf("\tdata is little endian.\n");
	//
	printf("m_sImageWidth=%u,m_sImageHeight=%u\n", pCfg->m_SysBaseInfo.m_sImageWidth, pCfg->m_SysBaseInfo.m_sImageHeight);
	/*
	* ip and port
	*/
	unsigned short usValue = ((pLastRegCfg->m_EtherInfo.m_sDestUDPPort & 0xff) << 8) | ((pLastRegCfg->m_EtherInfo.m_sDestUDPPort >> 8) & 0xff);
	printf("\tSourceIP:%d.%d.%d.%d:%u\n",
		pLastRegCfg->m_EtherInfo.m_byDestIP[0],
		pLastRegCfg->m_EtherInfo.m_byDestIP[1],
		pLastRegCfg->m_EtherInfo.m_byDestIP[2],
		pLastRegCfg->m_EtherInfo.m_byDestIP[3],
		usValue);

	usValue = ((pLastRegCfg->m_EtherInfo.m_sSourceUDPPort & 0xff) << 8) | ((pLastRegCfg->m_EtherInfo.m_sSourceUDPPort >> 8) & 0xff);
	printf("\tDestIP:%d.%d.%d.%d:%u\n",
		pLastRegCfg->m_EtherInfo.m_bySourceIP[0],
		pLastRegCfg->m_EtherInfo.m_bySourceIP[1],
		pLastRegCfg->m_EtherInfo.m_bySourceIP[2],
		pLastRegCfg->m_EtherInfo.m_bySourceIP[3],
		usValue);

	/*
	* trigger mode
	*/
	if (pLastRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x01)
		printf("\tstatic software trigger.[0x%02x]\n", pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);
	else if (pLastRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x03)
		printf("\tstatic hvg trigger.[0x%02x]\n", pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);
	else if (pLastRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x04)
		printf("\tFree AED trigger mode.[0x%02x]\n", pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);
	else if (pLastRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x05)
		printf("\tDynamic:Hvg Sync mode.[0x%02x]\n", pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);
	else if (pLastRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x06)
		printf("\tDynamic:Fpd Sync mode.[0x%02x]\n", pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);
	else if (pLastRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x07)
		printf("\tDynamic:Fpd Continue mode.[0x%02x]\n", pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);
	else if (pLastRegCfg->m_SysCfgInfo.m_byTriggerMode == 0x08)
		printf("\tStatic:SAEC mode.[0x%02x]\n", pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);
	else
		printf("\tother trigger mode.[0x%02x]\n", pLastRegCfg->m_SysCfgInfo.m_byTriggerMode);
	//
	printf("\tPre Acquisition Delay Time.%u\n", pLastRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime);
	/*
	* correction enable
	*/
	if (pLastRegCfg->m_ImgCaliCfg.m_byOffsetCorrection == 0x01)
		printf("\tFirmware offset correction disenable.[0x%02x]\n", pLastRegCfg->m_ImgCaliCfg.m_byOffsetCorrection);
	else if (pLastRegCfg->m_ImgCaliCfg.m_byOffsetCorrection == 0x02)
		printf("\tFirmware offset correction enable.[0x%02x]\n", pLastRegCfg->m_ImgCaliCfg.m_byOffsetCorrection);
	else
		printf("\tFirmware other offset correction enable.[0x%02x]\n", pLastRegCfg->m_ImgCaliCfg.m_byOffsetCorrection);
	if (pLastRegCfg->m_ImgCaliCfg.m_byGainCorrection == 0x01)
		printf("\tFirmware gain correction disenable.[0x%02x]\n", pLastRegCfg->m_ImgCaliCfg.m_byGainCorrection);
	else if (pLastRegCfg->m_ImgCaliCfg.m_byGainCorrection == 0x02)
		printf("\tFirmware gain correction enable.[0x%02x]\n", pLastRegCfg->m_ImgCaliCfg.m_byGainCorrection);
	else
		printf("\tFirmware gain offset correction enable.[0x%02x]\n", pLastRegCfg->m_ImgCaliCfg.m_byGainCorrection);
	if (pLastRegCfg->m_ImgCaliCfg.m_byDefectCorrection == 0x01)
		printf("\tFirmware defect correction disenable.[0x%02x]\n", pLastRegCfg->m_ImgCaliCfg.m_byDefectCorrection);
	else if (pLastRegCfg->m_ImgCaliCfg.m_byDefectCorrection == 0x02)
		printf("\tFirmware defect correction enable.[0x%02x]\n", pLastRegCfg->m_ImgCaliCfg.m_byDefectCorrection);
	else
		printf("\tFirmware defect offset correction enable.[0x%02x]\n", pLastRegCfg->m_ImgCaliCfg.m_byDefectCorrection);
	if (pLastRegCfg->m_ImgCaliCfg.m_byDummyCorrection == 0x01)
		printf("\tFirmware Dummy correction disenable.[0x%02x]\n", pLastRegCfg->m_ImgCaliCfg.m_byDummyCorrection);
	else if (pLastRegCfg->m_ImgCaliCfg.m_byDummyCorrection == 0x02)
		printf("\tFirmware Dummy correction enable.[0x%02x]\n", pLastRegCfg->m_ImgCaliCfg.m_byDummyCorrection);
	else
		printf("\tFirmware Dummy offset correction enable.[0x%02x]\n", pLastRegCfg->m_ImgCaliCfg.m_byDummyCorrection);

	// PGA level
	nPGALevel = GetPGA(pLastRegCfg->m_TICOFCfg.m_sTICOFRegister[26]);

	// Binning type
	nBinning = pLastRegCfg->m_SysCfgInfo.m_byBinning;
	if (nBinning < 1 || nBinning > 4)
	{
		nBinning = 1;
	}

	// prepare delayed
	BYTE byte1 = 0, byte2 = 0, byte3 = 0, byte4 = 0;
	byte1 = BREAK_UINT32(pLastRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime, 0);
	byte2 = BREAK_UINT32(pLastRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime, 1);
	byte3 = BREAK_UINT32(pLastRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime, 2);
	byte4 = BREAK_UINT32(pLastRegCfg->m_SysCfgInfo.m_unPreAcquisitionDelayTime, 3);
	prepareTime = BUILD_UINT32(byte4, byte3, byte2, byte1);
	// Continuous acquisition time interval and frame rate calculation
	if (commCfg._type == 0 || commCfg._type == 3)
	{
		byte1 = BREAK_UINT32(pLastRegCfg->m_SysCfgInfo.m_unContinuousAcquisitionSpanTime, 0);
		byte2 = BREAK_UINT32(pLastRegCfg->m_SysCfgInfo.m_unContinuousAcquisitionSpanTime, 1);
		byte3 = BREAK_UINT32(pLastRegCfg->m_SysCfgInfo.m_unContinuousAcquisitionSpanTime, 2);
		byte4 = BREAK_UINT32(pLastRegCfg->m_SysCfgInfo.m_unContinuousAcquisitionSpanTime, 3);
	}
	else
	{
		byte1 = BREAK_UINT32(pLastRegCfg->m_SysCfgInfo.m_unSelfDumpingSpanTime, 0);
		byte2 = BREAK_UINT32(pLastRegCfg->m_SysCfgInfo.m_unSelfDumpingSpanTime, 1);
		byte3 = BREAK_UINT32(pLastRegCfg->m_SysCfgInfo.m_unSelfDumpingSpanTime, 2);
		byte4 = BREAK_UINT32(pLastRegCfg->m_SysCfgInfo.m_unSelfDumpingSpanTime, 3);
	}
	//
	liveAcqTime = BUILD_UINT32(byte4, byte3, byte2, byte1);
	if (liveAcqTime <= 0)
	{
		printf("\tlive aqc time=%u\n", liveAcqTime);
		liveAcqTime = 1000;
	}
	//
	unsigned int uMaxFps = (unsigned int)(1000 / liveAcqTime);
	if (uMaxFps <= 0) uMaxFps = 1;

	//
	printf("\tprepareTime=%u\n", prepareTime);
	printf("\tuLiveTime=%u\n", liveAcqTime);
	printf("\tnPGA=%d\n", nPGALevel);
	printf("\tnBinning=%d\n", nBinning);
	printf("\tuMaxFps=%u\n", uMaxFps);
}

int SaveImage(unsigned char *imgbuff, int nbufflen, unsigned int uframeid)
{
	printf("SaveImage begin!\n");
	/* Detector resolution size */
	int WIDTH  = m_imgW;
	int HEIGHT = m_imgH;

	if ((imgbuff == NULL) || (nbufflen != (WIDTH * HEIGHT * 2))) {
		printf("err:date error!\n");
		return 0;
	}

	// save file
	char filename[MAX_PATH];
	int j = sprintf(filename, "%s\\%u.raw",szPath, uframeid);
	filename[j] = '\0';
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
		printf("err:SaveImage:%s failed!\n", filename);
		return 0;
	}
}

void Usage()
{
	printf("***********************Demo Usage*********************\n");
	printf(" Usage  :\n");
	printf(" ./DemoDll.exe\n");
	printf("q-Exit App.\n");
	printf("0-single shot(prepareTime>0).\n");
	printf("1-single shot(prepareTime=0).\n");
	printf("2-live acquisition.\n");
	printf("3-stop live acquisition.\n");
	printf("4-get the parameters of the detector firmware.\n");
	printf("5-Get image properties.\n");
	printf("6-Generate offset template.\n");
	printf("7-Generate gain template.\n");
	printf("8-Generate defect template-Collect the first group of bright field images.\n");
	printf("9-Generate defect template-Collect the second group of bright field images.\n");
	printf("a-Generate defect template-Collect the third group of bright field images and generate templates.\n");
	printf("b-Update calibration enable status.\n");
	printf("******************************************************\n");
}