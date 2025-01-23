/*---------------------------------------------------------------------------
* Copyright (c) 2019 DT Image Technology (Shanghai) Co., Ltd
* All rights reserved.
*
* File name: HbiError. h
* Document ID:
* Summary: flat-panel detector error message header file
*
* Current version: 3.0
* Author: mhyang
* Date: November 20, 2020
* Copyright (c) 2019 地太科特影像科技有限公司
* All rights reserved.
*
* 文件名称: HbiError.h
* 文件标识:
* 摘    要: 平板探测器错误信息头文件
*
* 当前版本: 3.0
* 作    者: mhyang
* 完成日期: 2020/11/20
----------------------------------------------------------------------------*/
#ifndef __HB_DLL_ERROR_H_
#define __HB_DLL_ERROR_H_

enum HBIRETCODE {
	HBI_SUCCSS                    = 0,
	HBI_ERR_ININT_FAILED          = 8000,
	HBI_ERR_OPEN_DETECTOR_FAILED  = 8001,
	HBI_ERR_INVALID_PARAMS        = 8002,
	HBI_ERR_CONNECT_FAILED        = 8003,
	HBI_ERR_MALLOC_FAILED         = 8004,
	HBI_ERR_RELIMGMEM_FAILED      = 8005,
	HBI_ERR_RETIMGMEM_FAILED      = 8006,
	HBI_ERR_NODEVICE              = 8007,
	HBI_ERR_DISCONNECT            = 8008,
	HBI_ERR_DEVICE_BUSY           = 8009,
	HBI_ERR_SENDDATA_FAILED       = 8010,
	HBI_ERR_RECEIVE_DATA_FAILED   = 8011,
	HBI_ERR_COMMAND_DISMATCH      = 8012,
	HBI_ERR_NO_IMAGE_RAW          = 8013,
	HBI_ERR_PTHREAD_ACTIVE_FAILED = 8014,
	HBI_ERR_STOP_ACQUISITION      = 8015,
	HBI_ERR_INSERT_FAILED         = 8016,
	HBI_ERR_GET_CFG_FAILED        = 8017,
	HBI_NOT_SUPPORT               = 8018,
	HBI_REGISTER_CALLBACK_FAILED  = 8019,
	HBI_SEND_MESSAGE_FAILD        = 8020,
	HBI_ERR_WORKMODE              = 8021,
	HBI_FAILED                    = 8022,
	HBI_FILE_NOT_EXISTS           = 8023,
	HBI_COMM_TYPE_ERR             = 8024,
	HBI_TYPE_IS_NOT_EXISTS        = 8025,
	HBI_SAVE_FILE_FAILED          = 8026,
	HBI_INIT_PARAM_FAILED         = 8027,
	HBI_INIT_FILE_NOT_EXIST       = 8028,
	HBI_INVALID_FLAT_PANEL        = 8029,
	HBI_INVALID_DLL_HANDLE        = 8031,
	HBI_FPD_IS_DISCONNECT         = 8032,
	HBI_ERR_DETECTOR_NUMBER       = 8033,
	HBI_ERR_DATA_CHECK_FAILED     = 8034,
	HBI_ERR_CFG_ISEMPTY           = 8035,
	HBI_ERR_OPEN_WIZARD_FAILED    = 8036,
	HBI_ERR_WIZARD_ALREADY_EXIST  = 8037,
	HBI_DIRECTORY_NOT_EXISTS      = 8038,
	HBI_UNREGISTER_CALLBACK       = 8039,
	HBI_ERR_IMAGE_ISEMPTY         = 8040,
	HBI_ERR_ABNORMMAL_IMAGE       = 8041,
	HBI_PREPARATION_COMPLETE      = 8042,
	HBI_ERR_FILE_EXCEPTION        = 8043,
	HBI_ERR_COMM_TYPE             = 8044,
	HBI_ERR_INVALID_MODE          = 8045,
	HBI_ERR_FIRM_CONFIG_DATA      = 8046,
	HBI_ERR_IS_NULL               = 8047,
	HBI_ERR_ROM_PARAMETER         = 8048,
	HBI_ERR_FILE_TYPE             = 8049,
	HBI_ERR_PCIE_DEVICE_USED      = 8050,
	HBI_END                       = 8051
};

struct CodeStringTable
{
	int		     num;
	int        HBIRETCODE;
	const char *errString;
};
static const CodeStringTable CrErrStrList[] = {
	{ 0,  HBI_SUCCSS, "Success" },
	{ 1,  HBI_ERR_ININT_FAILED, "Init dll failed" },
	{ 2,  HBI_ERR_OPEN_DETECTOR_FAILED, "Open device driver failed" },
	{ 3,  HBI_ERR_INVALID_PARAMS, "Parameter error" },
	{ 4,  HBI_ERR_CONNECT_FAILED, "Connect failed" },
	{ 5,  HBI_ERR_MALLOC_FAILED, "Malloc memory failed" },
	{ 6,  HBI_ERR_RELIMGMEM_FAILED, "Releaseimagemem fail" },
	{ 7,  HBI_ERR_RETIMGMEM_FAILED, "ReturnImageMem fail" },
	{ 8,  HBI_ERR_NODEVICE, "No Init DLL Instance" },
	{ 9,  HBI_ERR_DISCONNECT, "Is Disconnect" },
	{ 10, HBI_ERR_DEVICE_BUSY, "Fpd is busy" },
	{ 11, HBI_ERR_SENDDATA_FAILED, "SendData failed" },
	{ 12, HBI_ERR_RECEIVE_DATA_FAILED, "Receive Data failed" },
	{ 13, HBI_ERR_COMMAND_DISMATCH, "Command dismatch" },
	{ 14, HBI_ERR_NO_IMAGE_RAW, "No Image raw" },
	{ 15, HBI_ERR_PTHREAD_ACTIVE_FAILED, "Pthread active failed" },
	{ 16, HBI_ERR_STOP_ACQUISITION, "Pthread stop data acquisition failed" },
	{ 17, HBI_ERR_INSERT_FAILED, "insert calibrate mode failed" },
	{ 18, HBI_ERR_GET_CFG_FAILED, "get Fpd config failed" },
	{ 19, HBI_NOT_SUPPORT, "not surport yet" },
	{ 20, HBI_REGISTER_CALLBACK_FAILED, "failed to register callback function" },
	{ 21, HBI_SEND_MESSAGE_FAILD, "send message failed" },
	{ 22, HBI_ERR_WORKMODE, "switch work mode failed" },
	{ 23, HBI_FAILED, "operation failed" },
	{ 24, HBI_FILE_NOT_EXISTS, "file does not exist" },
	{ 25, HBI_COMM_TYPE_ERR, "communication is not exist"},
	{ 26, HBI_TYPE_IS_NOT_EXISTS, "this type is not exists"},
	{ 27, HBI_SAVE_FILE_FAILED, "save file failed"},
	{ 28, HBI_INIT_PARAM_FAILED, "Init dll param failed"},
	{ 29, HBI_INIT_FILE_NOT_EXIST, "Init dll config file is not exist"},
	{ 30, HBI_INVALID_FLAT_PANEL, "Invalid flat panel detector"},
	{ 31, HBI_INVALID_DLL_HANDLE, "Invalid dll instance handle"},
	{ 32, HBI_FPD_IS_DISCONNECT, "Fpd is disconnect"},
	{ 33, HBI_ERR_DETECTOR_NUMBER, "Detector number error"},
	{ 34, HBI_ERR_DATA_CHECK_FAILED, "Data Check failed"},
    { 35, HBI_ERR_CFG_ISEMPTY, "The configuration variable is empty"},
	{ 36, HBI_ERR_OPEN_WIZARD_FAILED, "Open template wizard failed"},
	{ 37, HBI_ERR_WIZARD_ALREADY_EXIST, "template wizard already exist"},
	{ 38, HBI_DIRECTORY_NOT_EXISTS, "File directory is not exists"},
    { 39, HBI_UNREGISTER_CALLBACK, "Unregistered callbacke function"},
	{ 40, HBI_ERR_IMAGE_ISEMPTY, "Image is empty"},
	{ 41, HBI_ERR_ABNORMMAL_IMAGE, "Abnormal image"},
	{ 42, HBI_PREPARATION_COMPLETE, "Preparation complete"},
	{ 43, HBI_ERR_FILE_EXCEPTION, "File exception"},
	{ 44, HBI_ERR_COMM_TYPE, "Wrong communication type"},
	{ 45, HBI_ERR_INVALID_MODE, "This mode contains incorrect parameters"},
	{ 46, HBI_ERR_FIRM_CONFIG_DATA, "Firmware parameter data is abnormal"},
	{ 47, HBI_ERR_IS_NULL, "The pointer is NULL"},
	{ 48, HBI_ERR_ROM_PARAMETER, "ROM includes incorrect parameters"},
	{ 49, HBI_ERR_FILE_TYPE, "File type exception"},
	{ 50, HBI_ERR_PCIE_DEVICE_USED, "PCIE device has been used"},
	{ 51, HBI_END, "Exit monitoring"}
};
///*********************************************
//*Description: Get the current error information through the return code
//*Function: GetHbiErrorString
//*Parameter: CodeStringTable * inTable returns the code table
//*Int count Number of return codes
//*Int recode return code
//*Return value:
//*Const char * Returns a string
//*Remarks: BY MH.YANG 2019/12/09
//*********************************************/
//const char *GetHbiErrorString(CodeStringTable* inTable, int count, int recode)
//{
//	const char *retString = "Unknown";
//	for(int i = 0; i < count; i++) {
//		if(inTable[i].HBIRETCODE == recode) {
//			retString = inTable[i].errString;
//			break;
//		}
//	}
//	return retString;
//}
//int main()
//{
//		static const CodeStringTable CrErrStrList[] = {
//			{ 0,  HBI_SUCCSS, "Success" },
//			{ 1,  HBI_ERR_OPEN_DETECTOR_FAILED, "Open device driver failed" },
//			{ 2,  HBI_ERR_INVALID_PARAMS, "Parameter error" },
//			{ 3,  HBI_ERR_CONNECT_FAILED, "Connect failed" },
//			{ 4,  HBI_ERR_MALLOC_FAILED, "Malloc memory failed" },
//			{ 5,  HBI_ERR_RELIMGMEM_FAILED, "Releaseimagemem fail" },
//			{ 6,  HBI_ERR_RETIMGMEM_FAILED, "ReturnImageMem fail" },
//			{ 7,  HBI_ERR_NODEVICE, "No Device" },
//			{ 8,  HBI_ERR_DISCONNECT, "No Device,Try again" },
//			{ 9,  HBI_ERR_DEVICE_BUSY, "Device busy" },
//			{ 10, HBI_ERR_SENDDATA_FAILED, "SendData failed" },
//			{ 11, HBI_ERR_RECEIVE_DATA_FAILED, "Receive Data failed" },
//			{ 12, HBI_ERR_COMMAND_DISMATCH, "Command dismatch" },
//			{ 13, HBI_ERR_NO_IMAGE_RAW, "No Image raw" },
//			{ 14, HBI_ERR_PTHREAD_ACTIVE_FAILED, "Pthread active failed" },
//			{ 15, HBI_ERR_STOP_ACQUISITION, "Pthread stop data acquisition failed" },
//			{ 16, HBI_ERR_INSERT_FAILED, "insert calibrate mode failed" },
//			{ 17, HBI_ERR_GET_CFG_FAILED, "get device config failed" },
//			{ 16, HBI_NOT_SUPPORT, "not surport yet" },
//		};
//	int nCode = 8001;
//	const char *tmpString = GetErrString(CodeStringTable, sizeof(CrErrStrList)/sizeof(CodeStringTable), uValue);
//	printf("nCode=%d,tmpString=%s\n", nCode, tmpString);
//	return 0;
//}
#endif // __HB_DLL_ERROR_H_
