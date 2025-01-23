/*---------------------------------------------------------------------------
* Copyright (c) 2019 DT Image Technology (Shanghai) Co., Ltd
* All rights reserved.
*
* File name: HbiFpd. h
* Document ID:
* Abstract: flat-panel detector export function header file
*
* Current version: 3.0
* Author: mhyang
* Date: November 20, 2020
* Revision date: November 5, 2021
* Copyright (c) 2019 地太科特影像科技有限公司
* All rights reserved.
*
* 文件名称: HbiFpd.h
* 文件标识:
* 摘    要: 平板探测器导出函数头文件
*
* 当前版本: 3.0
* 作    者: mhyang
* 完成日期: 2020/11/20
* 修改日期: 2021/11/05
----------------------------------------------------------------------------*/
#pragma once
#ifndef __HBI_FPD_H_
#define __HBI_FPD_H_
#include "HbiType.h"
#include "HbiError.h"

#ifdef _DLL_EX_IM
#define HBI_FPD_DLL __declspec(dllexport)
#else
#define HBI_FPD_DLL __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////////////
// Dual panel solution #“双板”或“多板”解决方案接口 ADD BY MHYANG 20211009
////////////////////////////////////////////////////////////////////////////////////////
/*********************************************************
* No: No0001
* Function Name: HBI_InitDual
* Function Description: 		// Initialize double detector dynamic library
* Parameter Description:		// Callback parameter, that are two FPD handle instance
	In/Out:	void **handle1	    // Detector A¡¯s unsigned handle pointer instance
	In/Out:	void **handle1 	    // Detector B¡¯s unsigned handle pointer instance
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No0001
* 函 数 名: HBI_InitDual
* 功能描述: 初始化动态库
* 参数说明: 参数为回参，即两个平板的句柄
			In/Out:void **handle1 - 平板A的实例无符号对象指针
			In/Out:void **handle1 - 平板B的实例无符号对象指针
* 返 回 值：
			0   - 成功
			非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_InitDual(void **handle1, void **handle2);

/*********************************************************
* No: No0002
* Function Name: HBI_InitEx
* Function Description:	    // Initialize double or multi detector dynamic library instance
* Parameter Description:	// Callback parameter, FPD instance and ID number
	In/Out:	HBI_HANDLE *pHandArr	// Array object, refer to <<HbiType.h>>
	In:		int nArrSize 	        // Array number, equal to DETECTOR_MAX_NUMBER
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No0002
* 函 数 名: HBI_InitEx
* 功能描述: 初始化双板或多板动态库实例，参数为回参，即平板的句柄和ID
* 参数说明: 参数为回参，即平板的句柄和ID
			In/Out:HBI_HANDLE *pHandArr - 数组对象，见《HbiType.h》
			In:int nArrSize - 数组的个数，《= DETECTOR_MAX_NUMBER
* 返 回 值：
			0   - 成功
			非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_InitEx(HBI_HANDLE *pHandArr, int nArrSize);

/*********************************************************
* No: No0003
* Function Name: HBI_DestroyEx
* Function Description: 	// Destroy all dll resources
* Parameter Description:
	Null
* Return Value:
	Null
* Comment:
* 编    号: No0003
* 函 数 名: HBI_DestroyEx
* 功能描述: 销毁所有DLL资源
* 参数说明: 无
* 返 回 值：
			无
* 备    注:
*********************************************************/
HBI_FPD_DLL void __stdcall HBI_DestroyEx();

/*********************************************************
* No: No0004
* Function Name: HBI_GetDetectorHandle
* Function Description: 	// Get handle through FPD ID number
* Parameter Description:
	In：int fpdid 			// FPD ID number,0 《 panedl id 《 DETECTOR_MAX_NUMBER
* Return Value:
	void * 				    // unsigned object pointer
* Comment:
* 编    号: No0004
* 函 数 名: HBI_GetDetectorHandle
* 功能描述: 依据平板id获取获取句柄
* 参数说明:
			In：int fpdid - 平板id，0 《 平板id 《 DETECTOR_MAX_NUMBER
* 返 回 值：
			void * - 无符号对象指针
* 备    注:
*********************************************************/
HBI_FPD_DLL void *__stdcall HBI_GetDetectorHandle(int fpdid = 0);

/*********************************************************
* No: No0005
* Function Name: HBI_GetDetectorId
* Function Description: 	// Get FPD ID number through handle
* Parameter Description:
	In：void *handle 		// handle pointer
* Return Value:
	int					    // FPD ID number
* Comment:
* 编    号: No0005
* 函 数 名: HBI_GetDetectorId
* 功能描述: 依据句柄获取获取平板id
* 参数说明:
			In：void *handle - 平板句柄
* 返 回 值：
			int - 平板id
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetDetectorId(void *handle);

/*********************************************************
* No: No0006
* Function Name: HBI_ActivateDetector
* Function Description: 	// Activate detector, current SDK version supports multi FPD
* Parameter Description:
	In：void *handle 		// handle pointer
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No0006
* 函 数 名: HBI_ActivateDetector
* 功能描述: 激活平板，目前SDK支持双板同时上图，不需要切换
* 参数说明:
			In：void *handle - 平板句柄
* 返 回 值：
			0   - 成功，回调函数反馈固件参数
			非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_ActivateDetector(void *handle);
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

/*********************************************************
* No: No007
* Function Name: HBI_Init
* Function Description		// Initialize dynamic dll, nonvolatile parameters and thread message parameters, establish SDK object pointer.
* Parameter Description:
	int fpdid				// Flat panel detector ID, default is 0.
* Return Value:
	void *	// Null: failure;
			// Non-null: success.
* Comment:
* 编    号: No007
* 函 数 名: HBI_Init
* 功能描述: 初始化动态库
* 参数说明: int fpdid - 平板id，默认为0
* 返 回 值：void * - 无符号对象指针
			失败：NULL
			成功：非空
* 备    注:
*********************************************************/
HBI_FPD_DLL void* __stdcall HBI_Init(int fpdid=0);

/*********************************************************
* No: No008
* Function Name: HBI_Destroy
* Function Description: 	// Release dynamic library resources
* Parameter Description:
	In: void *handle		// Handle, dynamic SDK object pointer
	Out: Null
* Return Value:  void
* Comment:
* 编    号: No008
* 函 数 名: HBI_Destroy
* 功能描述: 释放动态库资源
* 参数说明:
		In: void *handle - 句柄
		Out: 无
* 返 回 值：void
* 备    注:
*********************************************************/
HBI_FPD_DLL void __stdcall HBI_Destroy(void *handle);

/*********************************************************
* No: No009
* Function Name: HBI_ConnectDetector
* Function Description: 	// Establish connection
* Parameter Description：
	In: void *handle 	    // unsigned handle pointer
		COMM_CFG commCfg	// refer to <<HbiType.h>> COMM_CFG
				UDP_COMM_TYPE = 0, Standard UDP communication method
				UDP_JUMBO_COMM_TYPE = 1, Standard UDP communication method，Support for jumbo
				PCIE_COMM_TYPE = 2, Fiber type
				WALN_COMM_TYPE = 3，Wireless type
			// UDP communication requires IP and port settings,PCIe only requires setting the communication type and deviceID
			char _remoteip[MAX_IP_LEN]- Panel IP Address,eg:192.168.10.40
			char _localip[MAX_IP_LEN] - loacal IP Address,eg:192.168.10.20
			unsigned short _loacalPort-local port口,如32896(0x8080)
			unsigned short _remotePort- Panel port,如32897(0x8081)
			unsigned short _pcieId; - PCIe device ID,default 7024
		Int doOffsetTemp  // Non-1: don’t update pre-offset template when connection is successful
						  // 1: update pre-offset template when connection is successful
	Out:	Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No009
* 函 数 名: HBI_ConnectDetector
* 功能描述: 建立连接
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			COMM_CFG commCfg - 连接配置参数，详细见《HbiType.h》
			    UDP_COMM_TYPE = 0, 标准网口
				UDP_JUMBO_COMM_TYPE = 1,标准网口，支持jumbo类型
				PCIE_COMM_TYPE = 2,光纤类型
				WALN_COMM_TYPE = 3，无线类型
			// 网口通讯需要设置,PCIe只要设置类型和设备ID即可
			char _remoteip[MAX_IP_LEN]- 平板IP地址,如192.168.10.40
			char _localip[MAX_IP_LEN] - 上位机地址,如192.168.10.20
			unsigned short _loacalPort-上位机端口,如32896(0x8080)
			unsigned short _remotePort- 平板端口,如32897(0x8081)
			unsigned short _pcieId; - PCIe设备ID,default 7024
			int doOffsetTemp - 非1:连接成功后固件不重新做offset模板
							   1:连接成功后固件重新做offset模板
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_ConnectDetector(void *handle, COMM_CFG commCfg, int doOffsetTemp = 0);

/*********************************************************
* No: No010
* Function Name: HBI_ConnectDetectorUdp    - Standard UDP communication method
				 HBI_ConnectDetectorJumbo  - Standard UDP communication method，Support for jumbo
				 HBI_ConnectDetectorWlan   - Fiber type
* Function Description: 	// Establish connection (gigabit Ethernet interface with typical UDP packet)
* Parameter Description:
	In: 	void *handle 	// unsigned handle pointer
		char *szDetectorIp 	            // FPD IP address: 192.168.10.40
		unsigned short usDetectorPort	// FPD port: 32897(0x8081)
		char *szlocalIp 				// PC IP address: 192.168.10.20
		unsigned short usLocalPort 	    // PC port: 32896(0x8080)
		int doOffsetTemp 	            // Non-1: don’t update pre-offset template when connection is successful
										// 1: update pre-offset template when connection is successful
	Out:	Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No010
* 函 数 名: HBI_ConnectDetectorUdp    - 有线标准UDP
            HBI_ConnectDetectorJumbo  - 有线标准UDP JUMBO
			HBI_ConnectDetectorWlan   - 有线标准UDP wireless
* 功能描述: 建立连接（支持以太网UDP协议）
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			char *szDetectorIp - 平板IP地址,如192.168.10.40
			unsigned short usDetectorPort - 平板端口,如32897(0x8081)
			char *szlocalIp - 上位机地址,如192.168.10.20
			unsigned short usLocalPort -上位机端口,如32896(0x8080)
			int doOffsetTemp - 非1:连接成功后固件不重新做offset模板
							   1:连接成功后固件重新做pre-offset模板，矫正offset使能03即可生效
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_ConnectDetectorUdp(void *handle, char *szDetectorIp, unsigned short usDetectorPort, char *szlocalIp, unsigned short usLocalPort, int doOffsetTemp = 0);
HBI_FPD_DLL int __stdcall HBI_ConnectDetectorJumbo(void *handle, char *szDetectorIp, unsigned short usDetectorPort, char *szlocalIp, unsigned short usLocalPort, int doOffsetTemp = 0);
HBI_FPD_DLL int __stdcall HBI_ConnectDetectorWlan(void *handle, char *szDetectorIp, unsigned short usDetectorPort, char *szlocalIp, unsigned short usLocalPort, int doOffsetTemp = 0);
/*********************************************************
* No: No011
* Function Name: HBI_ConnectDetectorPcie
* Function Description: 	// Establish connection (optical fiber interface)
* Parameter Description:
	In:	void *handle 	    // unsigned handle pointer
		int doOffsetTemp	// 1: don’t update pre-offset template when connection is successful
							// Non-1: update pre-offset template when connection is successful
	Out:	Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No011
* 函 数 名: HBI_ConnectDetectorPcie
* 功能描述: 建立连接（支持光口通讯协议）
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int doOffsetTemp - 非1:连接成功后固件不重新做offset模板
							   1:连接成功后固件重新做pre-offset模板，矫正offset使能03即可生效
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_ConnectDetectorPcie(void *handle, int doOffsetTemp = 0);
/******************************************************************************
* No: No012
* Function Name: HBI_ConnectDetectorEx
* Function Description: 	// Establish connection
* Parameter Description:
	In:	void *handle 	    // unsigned handle pointer, refer to <<hbi_init.ini>>
	Out:	Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment: <<hbi_fpd_init.ini>>
* FPD IP Address: 		FPD_IP = 192.168.10.40
* FPD Port: 				FPD_PORT = 32897
* Local IP Address: 		LOCAL_IP = 192.168.10.20
* Local Port: 				LOCAL_PORT = 32896
* TRIGGER_MODE:		0-Invalid Trigger;
						1: Static: Software Trigger;
						2: Clear;
						3: Static: Hvg Trigger Mode;
						4: Static: AED Trigger Mode;
						5: Dynamic: Hvg Sync Mode;
						6: Dynamic: Fpd Sync Mode;
						7: Dynamic: Free Run Mode;
						8: Static: SAEC Mode;
* OFFSET_ENABEL:		0: No Offset Correction;
						1: Software Preoffset Correction;
						2: Firmware Postoffset Correction;
						3: Firmware Preoffset Correction;
* GAIN_ENABLE:	  		0: No Gain Correction;
						1: Software Gain Correction;
						2: Firmware Gain Correction;
* DEFECT_ENABLE:		0: No Defect Correction;
						1: Software Defect Correction;
						2: Firmware Defect Correction;
* DUMMY_ENABLE:		0: No Dummy Correction;
						1: Software Dummy Correction;
						2: Firmware Dummy Correction;
* LIVE_ACQ_TIME:		1000~1000ms
* SELF_DUMPING_TIME:	500~500ms
* PREPARE_TIME:		    1500~1500ms
* DO_OFFSET:			0: No action
						1: Do offset template
* 编    号: No012
* 函 数 名: HBI_ConnectDetectorEx
* 功能描述: 建立连接(通过配置文件获取参数)
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			基本通讯控制参数，见《HBI_FPD_CONFIG.ini》
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:HBI_FPD_CONFIG.ini存放目录与HBISDKApi.dll是同目录。
* 说    明:
* 平板地址:FPD_IP=192.168.10.40
* 平板端口:FPD_PORT=32897
* 本地地址:LOCAL_IP=192.168.10.20
* 本地端口:LOCAL_PORT=32896
* 触发模式:TRIGGER_MODE=1           #0-Invalid Trigger;1-Software Trigger;2-Clear;3-Static:Hvg Trigger Mode;
									#4-Static:AED Trigger Mode;5-Dynamic:Hvg Sync Mode;
									#6-Dynamic:Fpd Sync Mode;7-Dynamic:Continue Mode;8-Static:SAECMode;
* 偏置矫正使能:OFFSET_ENABEL=3      #0-No Offset Correction;1-Software PreOffset Correction;2-Firmware PostOffset Correction;3-Firmware PreOffset Correction;
* 增益矫正使能:GAIN_ENABLE  =2      #0-No Gain Correction;1-Software Gain Correction;2-Firmware Gain Correction;
* 坏点矫正使能:DEFECT_ENABLE=2      #0-No Defect Correction;1-Software Defect Correction;2-Firmware Defect Correction;
* Dummy矫正使能:DUMMY_ENABLE=0      #0-No Dummy Correction;1-Software Dummy Correction;2-Firmware Dummy Correction;
* 连续采集时间:LIVE_ACQ_TIME=1000   #1000ms
* 自清空时间:SELF_DUMPING_TIME=500  #500ms
* Prepare延时:PREPARE_TIME=1500     #1500ms
* 是否做固件Offset模板:DO_OFFSET=1  #1-做固件Offset模板，0-不做固件Offset模板
******************************************************************************/
HBI_FPD_DLL int __stdcall HBI_ConnectDetectorEx(void *handle);
/*********************************************************
* No: No013
* Function Name: HBI_DisConnectDetector
* Function Description: 	// Disconnect detector
* Parameter Description：
	In: 	void *handle 	// unsigned handle pointer
	Out: Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No013
* 函 数 名: HBI_DisConnectDetector
* 功能描述: 断开连接
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		SOFTWARE_CALIBRATE_ENABLE inEnable - 校正使能状态,见HbiType.h中FPD_SOFTWARE_CALIBRATE_ENABLE
		Out: 无
* 返 回 值：int
			0   - 成功
			非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_DisConnectDetector(void *handle);

/*********************************************************
* No: No014
* Function Name: HBI_RegEventCallBackFun
* Function Description: 	// Register callback function
* Parameter Description：
	In: 	void *handle 	// unsigned handle pointer
		USER_CALLBACK_HANDLE_ENVENT callbackfun	// Register callback function
		void *pContext	    // Object pointer, application software transfer object pointer to SDK in order to access static and global variable
	Out: Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: D014
* 函 数 名：HBI_RegEventCallBackFun
* 功能描述: 注册回调函数
* 参数说明：
		In: void *handle - 句柄(无符号指针)
			USER_CALLBACK_HANDLE_ENVENT callbackfun - 注册回调函数
			void *pContext- 对象指针，一般上位机软件将对象指针传入SDK，回调函数反馈给上位机，以便静态或全局函数访问其他对象，亦可为空指针。
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_RegEventCallBackFun(void *handle, USER_CALLBACK_HANDLE_ENVENT handleEventfun, void *pContext);

/*********************************************************
* No: No015
* Function Name: HBI_ResetDetector
* Function Description: 	// Reset detector
* Parameter Description：
	In: 	void *handle 	// unsigned handle pointer
	Out:	Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No015
* 函 数 名: HBI_ResetDetector
* 功能描述: 重置探测器
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_ResetDetector(void *handle);

/*********************************************************
* No: No016
* Function Name: HBI_ResetFirmware
* Function Description: 	// Restore factory setting
* Parameter Description:
	In: 	void *handle 	// unsigned handle pointer
	Out:	Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No016
* 函 数 名: HBI_ResetFirmware
* 功能描述: 重置固件出厂设置
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_ResetFirmware(void *handle);

/*********************************************************
* No: No017
* Function Name: HBI_GetFPDSerialNumber
* Function Description: 	// Get FPD serial number
* Parameter Description:
	In: void *handle 		// unsigned handle pointer
	In/Out: char *szSn, 	// Serial number length 14bits
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No017
* 函 数 名: HBI_GetFPDSerialNumber
* 功能描述: 获取产品序列号
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		In/Out: char *szSn,长度14位
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetFPDSerialNumber(void *handle, char *szSn);

/*********************************************************
* No: No018
* Function Name: HBI_GetSDKVerion
* Function Description: 	// Get Dll version
* Parameter Description:
	In: 	void *handle 	// unsigned handle pointer
		char *szVer  	    // Dll version information string
	Out:	Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No018
* 函 数 名: HBI_GetSDKVerion
* 功能描述: 获取SDK版本号
* 参数说明:
	In: void *handle - 句柄(无符号指针)
	Out: char *szVer,数组长度至少》=64
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetSDKVerion(void *handle, char *szVer);

/*********************************************************
* No: No019
* Function Name: HBI_GetFirmareVerion
* Function Description: 	// Get firmware version
* Parameter Description:
	In: void *handle 		// unsigned handle pointer
		char *szVer  	    // Firmware version information string
	Out:	Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No019
* 函 数 名: HBI_GetFirmareVerion
* 功能描述: 获取固件版本号
* 参数说明:
	In: void *handle - 句柄(无符号指针)
	Out: char *szVer,数组长度至少》=64
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetFirmareVerion(void *handle, char *szFirmwareVer);

/*********************************************************
* No: No020
* Function Name: HBI_GetError
* Function Description: 	// Get error information, unavailable at present
* Parameter Description:
	In:	const CodeStringTable* inTable 	// Errata
		int count 		// Number of table
		int recode 		// Error code
	Out:	Null
* Return Value: const char *
			// Null: Failure
			// Non-null: Success
* Comment:
* 编    号: No020
* 函 数 名: HBI_GetError
* 功能描述: 获取错误信息
* 参数说明:
	In:  CodeStringTable* inTable - 错误信息信息列表
		 int count  - 信息列表个数
		 int recode - 错误码
	Out:无
* 返 回 值：const char *
	非NULL - 成功，错误信息
	NULL   - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL const char * __stdcall HBI_GetError(const CodeStringTable* inTable, int count, int recode);

/*********************************************************
* No: No021
* Function Name: HBI_GetFpdCfgInfo/HBI_GetDevCfgInfo(old version)
* Function Description: 	   // Get nonvolatile config parameter(ROM)
* Parameter Description:
	In: void *handle 		    // unsigned handle pointer
	Out:RegCfgInfo* pRegCfg		// Refer to <<HbiType.h>>
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No021
* 函 数 名: HBI_GetFpdCfgInfo/HBI_GetDevCfgInfo(old版本)
* 功能描述: 获取ROM参数
* 参数说明:
	In: void *handle - 句柄(无符号指针)
	Out:RegCfgInfo* pRegCfg,详细见《HbiType.h》。
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetFpdCfgInfo(void *handle, RegCfgInfo* pRegCfg);
HBI_FPD_DLL int __stdcall HBI_GetDevCfgInfo(void *handle, RegCfgInfo* pRegCfg);

/*********************************************************
* No: No022
* Function Name: HBI_GetImageProperty
* Function Description: 	    // Get image property
* Parameter Description:
	In: 	void *handle 	    // unsigned handle pointer
		IMAGE_PROPERTY *img_pro // Image property, refer to<<HbiType.h>> IMAGE_PROPERTY
	Out:	Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No022
* 函 数 名: HBI_GetImageProperty
* 功能描述: 获取图像属性
* 参数说明:
	In: void *handle - 句柄(无符号指针)
	Out: IMAGE_PROPERTY *img_pro,见HbiType.h。
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetImageProperty(void *handle, IMAGE_PROPERTY *img_pro);

/*********************************************************
* No: No023
* Function Name: HBI_GetPreviewImageProperty
* Function Description: 	    // Get preview image property
* Parameter Description:
	In: 	void *handle 	    // unsigned handle pointer
		IMAGE_PROPERTY *img_pro // Image property, refer to<<HbiType.h>> IMAGE_PROPERTY
	Out:	Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No023
* 函 数 名: HBI_GetPreviewImageProperty
* 功能描述: 获取图像属性
* 参数说明:
	In: void *handle - 句柄(无符号指针)
	Out: IMAGE_PROPERTY *img_pro,见HbiType.h。
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetPreviewImageProperty(void *handle, IMAGE_PROPERTY *img_pro);

/*********************************************************
* No: No024
* Function Name: HBI_SinglePrepare
* Function Description: Single image acquisition(combine with “prepare delay time” parameter)
* Parameter Description:
	In: void *handle 		// unsigned handle pointer
	Out:	Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:In the application, there will be a problem that the acquisition command type cannot be synchronized, resulting in the failure of the above figure. Use HBI_Prepare replacement
*Prepare delay=0 - indicates soft trigger single frame acquisition first HBI_Prepare after HBI_ SingleAcquisition completes single frame acquisition
*Prepare delay > "0 - means soft trigger single frame acquisition, only HBI_ Prepare is required can complete single frame acquisition according to the delay time
* 编    号: No024
* 函 数 名: HBI_SinglePrepare
* 功能描述: 单帧采集准备指令
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
0   成功
* 备    注:应用中会出现因为采集命令类型未能同步导致不上图的问题，使用HBI_Prepare替换
* prepare延时=0-表示软触发单帧采集先HBI_SinglePrepare后HBI_SingleAcquisition完成单帧采集
* prepare延时》0-表示软触发单帧采集，只需HBI_SinglePrepare即可按照延时时间完成单帧采集
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SinglePrepare(void *handle);

/*********************************************************
* No: No025
* Function Name: HBI_Prepare (Add interface)
* Function Description: Single image acquisition(combine with “prepare delay time” parameter)
* Parameter Description：
	In: void *handle 		// unsigned handle pointer
EnumIMAGE_ACQ_CMD //Current acquisition command type (image above or saved template image data)
		eg：
		SINGLE_ACQ_DEFAULT_TYPE // Single acquisition.
		SINGLE_ACQ_GAIN_IMAGE   // Generate gain template with static single acquisition.
		SINGLE_ACQ_DEFECT_IMAGE // Generate defect template with static single acquisition.
		SINGLE_GENERATE_OFFSET, // Quickly generate template collection types, collect a group of dark field images in a single frame - SDK generates offset template.
		SINGLE_GENERATE_GAIN,   // Quickly generate template collection types, and collect N groups of single frame (at least 3 groups by default) bright field images - SDK generates gain templates		SINGLE_GENERATE_DEFECT, // Quickly generate template collection types, and collect N groups of single frame (at least 3 groups by default) bright field images - SDK generates defect templates
	SINGLE_GENERATE_GAIN_DEFECT // Quickly generate template collection types, and collect N groups of single frame (at least 3 groups by default) bright field images - SDK generates gain and defect templates
	Out:	Null
* Return Value: int
			// 0: Success
						// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
*Prepare delay=0 - indicates soft trigger single frame acquisition first HBI_Prepare after HBI_ SingleAcquisition completes single frame acquisition
*Prepare delay > "0 - means soft trigger single frame acquisition, only HBI_ Prepare is required can complete single frame acquisition according to the delay time
* 编    号: No025
* 函 数 名: HBI_Prepare(新增)
* 功能描述: 单帧采集准备指令
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		    EnumIMAGE_ACQ_CMD - 当前采集命令类型（上图或保存模板图像数据）
			例如：
				SINGLE_ACQ_DEFAULT_TYPE         // 默认单帧采集
				SINGLE_ACQ_GAIN_IMAGE           // 创建Gain模板-单帧采集亮场图
				SINGLE_ACQ_DEFECT_IMAGE         // 创建Defect模板-单帧采集亮场图
				SINGLE_GENERATE_OFFSET,         // 快速生成模板采集类型,单帧采集1组暗场图 - SDK生成offset模板
				SINGLE_GENERATE_GAIN,           // 快速生成模板采集类型,单帧采集N组(默认至少3组)亮场图 - SDK生成gain模板
				SINGLE_GENERATE_DEFECT,         // 快速生成模板采集类型,单帧采集N组(默认至少3组)亮场图 - SDK生成defect模板
				SINGLE_GENERATE_GAIN_DEFECT     // 快速生成模板采集类型,单帧采集N组(默认至少3组)亮场图 - SDK生成gain和defect模板
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
0   成功
* 备    注:是对HBI_SinglePrepare接口做了必要的补充和优化
* prepare延时=0-表示软触发单帧采集先HBI_Prepare后HBI_SingleAcquisition完成单帧采集
* prepare延时》0-表示软触发单帧采集，只需HBI_Prepare即可按照延时时间完成单帧采集
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_Prepare(void* handle, EnumIMAGE_ACQ_CMD acqcmd = EnumIMAGE_ACQ_CMD::SINGLE_ACQ_DEFAULT_TYPE);

/*********************************************************
* No: No026
* Function Name: HBI_SingleAcquisition
* Function Description: 	    // Single image acquisition
* Parameter Description：
	In:	void *handle 	        // unsigned handle pointer
		FPD_AQC_MODE _mode		// Acquisition mode and related parameters
		 EnumIMAGE_ACQ_MODE 	// Image acquisition mode
					SINGLE_ACQ_DEFAULT_TYPE,  // Single frame acquisition
					SINGLE_ACQ_GAIN_IMAGE,    // Distributed generation correction template for template validation, creating Gain template - single frame acquisition of bright field images
					SINGLE_ACQ_DEFECT_IMAGE,  // Distributed generation of correction templates for template validation and creation of Defect templates - single frame acquisition of bright field images
		EnumLIVE_ACQUISITION eLivetype;       // Only for ONLY_ IMAGE
		int  ngroupno			// Group number
		int  nframesum			// Acsuisition image number
		int  ndiscard			// Discard image number
		int  nframeid			// Frame ID number
	Out: Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No026
* 函 数 名: HBI_SingleAcquisition
* 功能描述: 单帧采集
* 参数说明:
		In: void *handle - 句柄(无符号指针)
				FPD_AQC_MODE _mode - 结构体类型，参数包含采集模式以及帧数等其他参数
				其中结构体包含字段：
				EnumIMAGE_ACQ_CMD eAqccmd;      // 采集命令，参考文件《HbiType.h》,默认采图方式为SINGLE_ACQ_DEFAULT_TYPE，注意区分单帧采集和连续采集命令
					SINGLE_ACQ_DEFAULT_TYPE       , // 默认单帧采集
					SINGLE_ACQ_GAIN_IMAGE,          // 分布生成矫正模板，用于验证模板，创建Gain模板-单帧采集亮场图
					SINGLE_ACQ_DEFECT_IMAGE,        // 分布生成矫正模板，用于验证模板，创建Defect模板-单帧采集亮场图
				EnumLIVE_ACQUISITION eLivetype; // 只限于ONLY_IMAGE；
				int  ngroupno;                  // 默认为0
				int  nAcqnumber;                // 默认为0
				int  ndiscard;                  // 默认为0
				int  nframeid;                  // 默认为0
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SingleAcquisition(void *handle, FPD_AQC_MODE _mode);

/*********************************************************
* No: No027
* Function Name: HBI_LiveAcquisition
* Function Description: 	// Live acquisition
* Parameter Description：
	In: 	void *handle 	// unsigned handle pointer
		FPD_AQC_MODE _mode 	// Work mode and related parameter
	  The structure contains fields：
		EnumIMAGE_ACQ_CMD eAqccmd;           // Acquisition command，Refer to the document "HbiType. h", the default drawing method is LIVE_ ACQ_ DEFAULT_ TYPE, pay attention to distinguish between single-frame acquisition and continuous acquisition commands
			LIVE_ACQ_DEFAULT_TYPE,           // Acquisition command
					// Distribute and generate correction templates for template validation
					LIVE_ACQ_OFFSET_IMAGE,          // Create Offset template - continuously acquire dark field image
					LIVE_ACQ_GAIN_IMAGE,            // Create Gain template - continuous acquisition of bright field image
					LIVE_ACQ_DEFECT_IMAGE,          // Create Defect template - continuous acquisition of bright field image
					// Quickly generate correction templates for system integration
					OFFSET_TEMPLATE_TYPE,           // Quickly generate template collection types, continuously collect a group of dark field images and generate offset templates, and firmware generates templates
					GAIN_TEMPLATE_TYPE,             // Quickly generate template collection types, continuously collect a group of bright field images and generate gain templates
					DEFECT_TEMPLATE_GROUP1,         // Quickly generate template collection types and continuously collect a group of bright field images - defect group1
					DEFECT_TEMPLATE_GROUP2,         // Quickly generate template collection types and continuously collect a group of bright field images - defect group2
					DEFECT_TEMPLATE_GROUP3,         // Quickly generate template collection types and continuously collect a group of bright field images - defect group3 and generate template
					SOFTWARE_OFFSET_TEMPLATE        // Quickly generate template acquisition types and continuously collect a group of dark field images - SDK generates offset template
	Enum LIVE_ACQUISITION eLivetype;	// Live acquisition only, detail refer to LIVE_ACQ_DEFAULT_TYPE.
									// 1 - generate the firmware Preoffset template and then continue to 										// acquire image.
									// 2 - image acquisition only.
									// 3 - generate firmware Preoffset template only.
	int  ngroupno;                  // Group number.
	int  nAcqnumber;               	// Reserved image number.
	int  ndiscard;                  // Discard image number.
	int  nframeid;                  // Counter.
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No027
* 函 数 名: HBI_LiveAcquisition
* 功能描述: 连续采集
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			FPD_AQC_MODE _mode - 结构体类型，参数包含采集模式以及帧数等其他参数
			其中结构体包含字段：
				EnumIMAGE_ACQ_CMD eAqccmd;      // 采集命令，参考文件《HbiType.h》,默认采图方式为LIVE_ACQ_DEFAULT_TYPE，注意区分单帧采集和连续采集命令
					LIVE_ACQ_DEFAULT_TYPE,          // 默认连续采集
					// 分布生成矫正模板，用于验证模板
					LIVE_ACQ_OFFSET_IMAGE,          // 创建Offset模板-连续采集暗场图
					LIVE_ACQ_GAIN_IMAGE,            // 创建Gain模板-连续采集亮场图
					LIVE_ACQ_DEFECT_IMAGE,          // 创建Defect模板-连续采集亮场图
					// 快速生成矫正模板，用于系统集成
					OFFSET_TEMPLATE_TYPE,           // 快速生成模板采集类型,连续采集一组暗场图并生成offset模板，固件生成模板
					GAIN_TEMPLATE_TYPE,             // 快速生成模板采集类型,连续采集一组亮场图并生成gain模板
					DEFECT_TEMPLATE_GROUP1,         // 快速生成模板采集类型,连续采集一组亮场图 - defect group1
					DEFECT_TEMPLATE_GROUP2,         // 快速生成模板采集类型,连续采集一组亮场图 - defect group2
					DEFECT_TEMPLATE_GROUP3,         // 快速生成模板采集类型,连续采集一组亮场图 - defect group3 and generate template
					SOFTWARE_OFFSET_TEMPLATE        // 快速生成模板采集类型,连续采集一组暗场图 - SDK生成offset模板
				EnumLIVE_ACQUISITION eLivetype; // 只限于连续采集(LIVE_ACQ_DEFAULT_TYPE)详细任务, 1-固件做offset模板并上图；2-只上图；3-固件做只做offset模板;
												// 4 - 叠加：上传16位均图；5 -  叠加：上传32位均图；
				int  ngroupno;                  // 组号，默认为0
				int  nAcqnumber;                // 实际采集帧数（默认为0），nAcqnumber=0表示一直采图直到发送发送停止采集命令，nAcqnumber=n，n>0表示采集n帧图像。
				int  ndiscard;                  // 忽略的帧数，连续采集抛弃前几帧
				int  nframeid;                  // 目前用做计数器，默认为0
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_LiveAcquisition(void *handle, FPD_AQC_MODE _mode);

/*********************************************************
* No: No028
* Function Name: StopAcquisition
* Function Description: 	// Stop live acquisition
* Parameter Description：
	In: 	void *handle 	// unsigned handle pointer
	Out: Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No028
* 函 数 名: HBI_StopAcquisition
* 功能描述: 停止连续采集
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备     注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_StopAcquisition(void *handle);

/*********************************************************
* No: No029
* Function Description:     // Prepare delay time setting, which is also called exposure window
* Parameter Description:
	In: 	void *handle	// unsigned handle pointer
	Out:	int *out_itime 	// Time(0~10000), unit: ms
						// 0: “HBI_Prepare” and “HBI_SingleAcquisition” command are combined to realize single image acquisition
						// Non-0: FPD will delay “Prepare Delay Time” and automatically acquire one single image when the detector receives “HBI_Prepare” command.
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No029
* 函 数 名: HBI_SetSinglePrepareTime
* 功能描述: 设置软触发单帧采集清空和采集之间的时间间隔
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int *in_itime - 延时时间，单位:ms
				0-表示软触发单帧采集先HBI_SinglePrepare后HBI_SingleAcquisition完成单帧采集
				非0-表示软触发单帧采集，只需HBI_SinglePrepare即可按照延时时间完成单帧采集
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetSinglePrepareTime(void *handle, int in_itime);

/*********************************************************
* No: No030
* Function Name: HBI_GetSinglePrepareTime
* Function Description: 	// Get prepare delay time value, which is also called exposure window
* Parameter Description:
	In: 	void *handle    // unsigned handle pointer
	Out:	int *out_itime 	// Time(0~10000), unit: ms
						// 0: “HBI_Prepare” and “HBI_SingleAcquisition” command is combined to realize single image acquisition
						// Non-0: FPD will delay “Prepare Delay Time” and automatically acquire one single image when the detector receives “HBI_Prepare” command.
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No030
* 函 数 名: HBI_GetSinglePrepareTime
* 功能描述: 获取软触发单帧采集清空和采集之间的时间间隔
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		out:int *out_itime - 时间 [0~10000] 单位:mm
				0-表示软触发单帧采集先HBI_SinglePrepare后HBI_SingleAcquisition完成单帧采集
				非0-表示软触发单帧采集，只需HBI_SinglePrepare即可按照延时时间完成单帧采集
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetSinglePrepareTime(void *handle, int *out_itime);

/*********************************************************
* No: No031
* Function Name: HBI_SetLiveAcquisitionTime. (Static flat panel detector)
* Function Description: 	// Live acquisition span time setting (apply to static and wireless type FPD)
* Parameter Description:
	In: 	void *handle 	// unsigned handle pointer
		  int time			// Live acquisition span time, unit: ms
	Out: Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No031
* 函 数 名: HBI_SetLiveAcquisitionTime(静态平板)
* 功能描述: 设置采集时间间隔(即可设置帧率)
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int time - 间隔时间,单位是毫秒ms，> 0ms
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetLiveAcquisitionTime(void *handle, int time);

/*********************************************************
* No: No032
* Function Name: HBI_GetLiveAcquisitionTime. (Static flat panel detector)
* Function Description: 	// Get live acquisition span time (apply to static and wireless type FPD)
* Parameter Description:
	In: 	void *handle 	// unsigned handle pointer
	Out:	int *out_time 	// Live acquisition span time, unit: ms
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No032
* 函 数 名: HBI_GetLiveAcquisitionTime(静态平板)
* 功能描述: 获取采集时间间隔
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		out:int *out_time - 时间,单位是毫秒ms
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetLiveAcquisitionTime(void *handle, int *out_time);

/*********************************************************
* No: No033
* Function Name: HBI_SetSelfDumpingTime(Dynamic flat panel detector)
* Function Description: 	 // Self dumping span time setting
* Parameter Description:
	In: 		void *handle // unsigned handle pointer
			int time 		 // Self dumping span time, unit: ms
	Out: 	Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No033
* 函 数 名: HBI_SetSelfDumpingTime
* 功能描述: 设置采集时间间隔(动态平板)
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int time - 间隔时间,单位是毫秒ms，> 0ms
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetSelfDumpingTime(void *handle, int time);

/*********************************************************
* No: No034
* Function Name: HBI_GetSelfDumpingTime(Dynamic flat panel detector)
* Function Description: 	    // Get self dumping span time
* Parameter Description:
	In: 	 	void *handle	// unsigned handle pointer
	Out:		int *out_time	// Self dumping span time, unit: ms
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No034
* 函 数 名: HBI_GetSelfDumpingTime(动态平板)
* 功能描述: 获取采集时间间隔
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		out:int *out_time - 时间,单位是毫秒ms
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetSelfDumpingTime(void *handle, int *out_time);

/*********************************************************
* No: No035
* Function Name: HBI_TriggerAndCorrectApplay
* Function Description: 	// Work mode and image calibration setting
* Parameter Description:
	In: 	void *handle	// unsigned handle pointer
		int _triggerMode
						// 1: Software trigger mode
						// 3: High voltage generator trigger mode
						// 4: AED trigger mode
						// 5-Dynamic:Hvg Sync Mode
						// 6-Dynamic:Fpd Sync Mode
						// 7-Dynamic:Fpd Continue
						// 8-SAEC。
		IMAGE_CORRECT_ENABLE* pCorrect 	// refer to<< HbiType.h>>
	Out:	 Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment: Replace “HBI_WorkStationInitDllCfg” and “HBI_QuckInitDllCfg” API function
* 编    号: No035
* 函 数 名: HBI_TriggerAndCorrectApplay
* 功能描述: 设置触发模式和图像校正使能（工作站）新版本
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int _triggerMode,1-软触发，3-高压触发，4-FreeAED,
				5-Dynamic:Hvg Sync Mode
				6-Dynamic:Fpd Sync Mode
				7-Dynamic:Fpd Continue
				8-SAEC。
			IMAGE_CORRECT_ENABLE* pCorrect,见HbiType.h。
		Out:无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:替换HBI_WorkStationInitDllCfg和HBI_QuckInitDllCfg接口
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_TriggerAndCorrectApplay(void *handle, int _triggerMode, IMAGE_CORRECT_ENABLE* pCorrect);

/*********************************************************
* No: No036
* Function Name: HBI_UpdateTriggerMode
* Function Description: trigger mode setting
* Parameter Description:
	In: 	void *handle	// unsigned handle pointer
		int _triggerMode,
						// 1: Software trigger mode
						// 3: High voltage generator trigger mode
						// 4: AED trigger mode
						// 5-Dynamic:Hvg Sync Mode
						// 6-Dynamic:Fpd Sync Mode
						// 7-Dynamic:Fpd Continue
						// 8-SAEC。
	Out:	Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No036
* 函 数 名: HBI_UpdateTriggerMode
* 功能描述: 设置触发模式
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int _triggerMode,1-软触发，3-高压触发，4-FreeAED,
				5-Dynamic:Hvg Sync Mode
				6-Dynamic:Fpd Sync Mode
				7-Dynamic:Fpd Continue
				8-SAEC。
		Out:无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_UpdateTriggerMode(void *handle, int _triggerMode);

/*********************************************************
* No: No037
* Function Name: HBI_UpdateCorrectEnable
* Function Description: 	// Firmware calibration enable setting
* Parameter Description:
	In: 	void *handle 	// unsigned handle pointer
			IMAGE_CORRECT_ENABLE* pCorrect		// refer to<< HbiType.h>>
	Out:	Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No037
* 函 数 名: HBI_UpdateCorrectEnable
* 功能描述: 更新图像固件校正使能
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			IMAGE_CORRECT_ENABLE* pCorrect,见HbiType.h。
		Out:无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_UpdateCorrectEnable(void *handle, IMAGE_CORRECT_ENABLE* pCorrect);

/*********************************************************
* No: No038
* Function Name: HBI_GetCorrectEnable
* Function Description: 	// Get image calibration enable status
* Parameter Description:
	In:	void *handle 	    // unsigned handle pointer
		IMAGE_CORRECT_ENABLE* pCorrect		// refer to<< HbiType.h>>
	Out:	Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No038
* 函 数 名: HBI_GetCorrectEnable
* 功能描述: 获取图像固件校正使能
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			IMAGE_CORRECT_ENABLE* pCorrect,见HbiType.h。
		Out:无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetCorrectEnable(void *handle, IMAGE_CORRECT_ENABLE *pCorrect);

/*********************************************************
* No: No039
* Function Name: HBI_SetSoftwareCorrect
* Function Description: 	// Software calibration enable setting
* Parameter Description:
	In: 	void *handle 	// unsigned handle pointer
		SOFTWARE_CALIBRATE_ENABLE inEnable	// refer to<< HbiType.h>> SOFTWARE_CALIBRATE_ENABLE struct
	Out:	Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No039
* 函 数 名: HBI_SetSoftwareCorrect
* 功能描述: 设置校正使能，仅限于SDK校正，即软件矫正（相对于固件校正）
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			SOFTWARE_CALIBRATE_ENABLE inEnable - 校正使能状态,见HbiType.h中SOFTWARE_CALIBRATE_ENABLE struct
		Out: 无
* 返 回 值：int
		0   成功
		非0 失败，见HbiError.h错误码
* 备    注:新接口
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetSoftwareCorrect(void *handle, SOFTWARE_CALIBRATE_ENABLE inEnable);

/*********************************************************
* No: No040
* Function Name: HBI_GetSoftwareCorrect
* Function Description: 	// Get software calibration enable status
* Parameter Description:
	In: 	void *handle 	// unsigned handle pointer
		SOFTWARE_CALIBRATE_ENABLE inEnable	// refer to<<HbiType.h>>
		SOFTWARE_CALIBRATE_ENABLE struct
	Out: Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No040
* 函 数 名: HBI_GetSoftwareCorrect
* 功能描述: 获取校正使能参数，仅限于SDK校正，即软件矫正（相对于固件校正）
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			SOFTWARE_CALIBRATE_ENABLE inEnable - 校正使能状态,见HbiType.h中
			SOFTWARE_CALIBRATE_ENABLE struct
		Out: 无
* 返 回 值：int
		0   成功
		非0 失败，见HbiError.h错误码
* 备    注:新接口
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetSoftwareCorrect(void *handle, SOFTWARE_CALIBRATE_ENABLE *inEnable);

/*********************************************************
* No: No041
* Function Name: HBI_SetAedIntegrateTm
* Function Description: 	// AED integration time setting
* Parameter Description:
	In: 	void *handle 	// unsigned handle pointer
		int time 			// AED integration time, unit: ms, default = 500ms
	Out: Null
* Return Value: int
			// 0: Success
			// Non-0: Failure
* Comment:
* 编    号: No041
* 函 数 名: HBI_SetAedIntegrateTm
* 功能描述: 设置AED积分时间
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int time - 间隔时间,单位是毫秒ms，>= 500ms
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetAedIntegrateTm(void *handle, int time);

/*********************************************************
* No: No042
* Function Name: HBI_GetAedIntegrateTm
* Function Description: 	// Get AED integration time
* Parameter Description:
	In: 	void *handle    // unsigned handle pointer
	Out:	int *out_time	// AED integration time, unit: ms
* Return Value: int
			// 0: Success
			// Non-0: Failure
* Comment:
* 编    号: No042
* 函 数 名: HBI_GetAedIntegrateTm
* 功能描述: 获取AED积分时间
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		out:int *out_time - 时间,单位是毫秒ms
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetAedIntegrateTm(void *handle, int *out_time);

/*********************************************************
* No: No043
* Function Name: HBI_SetAedThreshold
* Function Description: 	// AED threshold value setting
* Parameter Description:
	In: 	void *handle	// unsigned handle pointer
		int out_ivalue   	// AED threshold (10,000~1,000,000)
	Out: Null
* Return Value: int
			// 0: Success
			// Non-0: Failure
* Comment:
* 编    号: No043
* 函 数 名: HBI_SetAedThreshold
* 功能描述: 设置AED阈值
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		int out_ivalue - 阈值 [25,000~1,000,000]
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetAedThreshold(void *handle, int in_ivalue);

/*********************************************************
* No: No044
* Function name: HBI_ GetAedThreshold
* Function description: Obtain AED threshold
* Parameter Description:
	In: void * handle - Handle (unsigned pointer)
	Out: int * out_ Ivalue - threshold [10000~1000000]
* Return value: int
	0- Success
	Non 0- Failed
* Remarks:
* 编    号: No044
* 函 数 名: HBI_GetAedThreshold
* 功能描述: 获取AED阈值
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		out:int *out_ivalue - 阈值 [10,000~1,000,000]
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetAedThreshold(void *handle, int *out_ivalue);

/*********************************************************
* No: No045
* Function Name: HBI_SetSaturationValue
* Function Description: 	// Saturation value setting
* Parameter Description:
	In: void *handle 		// unsigned handle pointer
	Out:int *out_ivalue 	// Saturation value (0 ~ 65,535)
* Return Value: int
			// 0: Success
			// Non-0: Failure
* Comment:
* 编    号: No045
* 函 数 名: HBI_SetSaturationValue
* 功能描述: 设置饱和值
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		out:int *out_ivalue - 饱和值 [0~65,535]
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetSaturationValue(void *handle, int in_ivalue);

/*********************************************************
* No: No046
* Function Name: HBI_GetSaturationValue
* Function Description: 	// Get saturation setting value
* Parameter Description:
	In: void *handle 		// unsigned handle pointer
	Out:int *out_ivalue	    // Saturation value (0 ~ 65,535)
* Return Value: int
			// 0: Success
			// Non-0: Failure
* Comment:
* 编    号: No046
* 函 数 名: HBI_GetSaturationValue
* 功能描述: 获取饱和值
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int *out_ivalue - 饱和值 [0~65,535]
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetSaturationValue(void *handle, int *out_ivalue);

/*********************************************************
* No: No047
* Function Name: HBI_SetClippingValue
* Function Description: Clipping value setting
* Parameter Description:
	In: void *handle 		// unsigned handle pointer
	Out:int *out_ivalue 	// Clipping value (0 ~ 65,535)
* Return Value: int
			// 0: Success
			// Non-0: Failure
* Comment:
* 编    号: No047
* 函 数 名: HBI_SetClippingValue
* 功能描述: 设置图像剪裁值
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		out:int *out_ivalue - 剪裁值 [0~65,535]
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetClippingValue(void *handle, int in_ivalue);

/*********************************************************
* No: No048
* Function Name: HBI_GetClippingValue
* Function Description: 	// Get clipping value
* Parameter Description:
	In: void *handle		// unsigned handle pointer
	Out:int *out_ivalue 	// Clipping value (0 ~ 65,535)
* Return Value: int
			// 0: Success
			// Non-0: Failure
* Comment:
* 编    号: No048
* 函 数 名: HBI_GetClippingValue
* 功能描述: 获取图像剪裁值
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		out:int *out_ivalue - 剪裁值 [0~65,535]
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetClippingValue(void *handle, int *out_ivalue);

/*********************************************************
* No: No049
* Function Name: HBI_SetPGALevel
* Function Description: 	// PGA gain stage setting
* Parameter Description:
	In: 	void *handle 	// unsigned handle pointer
		int mode 		// PGA gain stage
							// 1: 0.6 pC;
							// 2: 1.2 pC;
							// 3: 2.4 pC;
							// 4: 3.6 pC;
							// 5: 4.8 pC;
							// 6: 7.2 pC;
							// 7: 9.6 pC;
							// 8: LFW with CMOS detector
							// 9: HFW with CMOS detector
							// 10: 0.3 pC;
							// 11: 0.15 pC;
							// Default: 7.2pC.
	Out: Null
* Return Value: int
			// 0: Success
			// Non-0: Failure
* Comment:
* 编    号: No049
* 函 数 名: HBI_SetPGALevel
* 功能描述: 设置增益挡位,即设置可编程积分电容档位,提高灵敏度
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int mode - 模式
			[n]-Invalid
			[1]-0.6pC
			[2]-1.2pC
			[3]-2.4pC
			[4]-3.6pC
			[5]-4.8pC
			[6]-7.2pC,默认7.2pC
			[7]-9.6pC
			[8]-LFW (COMS)
			[9]-HFW (COMS)
			[10]-0.3pC
			[11]-0.15pC
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetPGALevel(void *handle, int nGainLevel);

/*********************************************************
* No: No050
* Function Name: HBI_GetPGALevel
* Function Description: 	// Get PGA gain stage setting value
* Parameter Description:
	In: 	void *handle 	// unsigned handle pointer
	Out: Null
* Return Value:
	int					// PGA gain stage
							// 1: 0.6 pC;
							// 2: 1.2 pC;
							// 3: 2.4 pC;
							// 4: 3.6 pC;
							// 5: 4.8 pC;
							// 6: 7.2 pC;
							// 7: 9.6 pC;
							// 8: LFW with CMOS detector
							// 9: HFW with CMOS detector
							// 10: 0.3 pC;
							// 11: 0.15 pC;
							// Default: 7.2pC.
* Comment:
* 编    号: No050
* 函 数 名: HBI_GetPGALevel
* 功能描述: 获取增益挡位,即获取可编程积分电容档位
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		Out: 无
* 返 回 值：int
		[n]-Invalid
		[1]-0.6pC
		[2]-1.2pC
		[3]-2.4pC
		[4]-3.6pC
		[5]-4.8pC
		[6]-7.2pC
		[7]-9.6pC
		[8]-LFW (COMS)
		[9]-HFW (COMS)
		[10]-0.3pC
		[11]-0.15pC
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetPGALevel(void *handle);

/*********************************************************
* No: No051
* Function Name: HBI_SetLPFLevel
* Function Description: 	// Low pass filter setting
* Parameter Description:
	In: 	void *handle	// unsigned handle pointer
		int nLPFLevel	// Low pass filter
							// 0: 210 kHz, default value is 210 kHz
							// 1: 105 kHz
							// 2: 52 kHz
							// 3: 26 kHz
							// 4: 13 kHz
	Out: Null
* Return Value: int
			// 0: Success
			// Non-0: Failure
* Comment:
* 编    号: No051
* 函 数 名: HBI_SetLPFLevel
* 功能描述: 设置低通滤波器档位参数
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int nLPFLevel - 档位
			[n]-Invalid
			[0]-210khz,默认210khz
			[1]-105khz
			[2]-52khz
			[3]-26khz
			[4]-13khz
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetLPFLevel(void *handle, int nLPFLevel);

/*********************************************************
* No: No052
* Function Name: HBI_GetLPFLevel
* Function Description: 	// Get pass filter
* Parameter Description:
	In:	void *handle 	// unsigned handle pointer
	Out: Null
* Return Value:
	int					// Low pass filter
							// 0: 210 kHz, default value is 210 kHz
							// 1: 105 kHz
							// 2: 52 kHz
							// 3: 26 kHz
							// 4: 13 kHz
* Comment:
* 编    号: No052
* 函 数 名: HBI_GetLPFLevel
* 功能描述: 获取低通滤波器档位参数
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		Out: 无
* 返 回 值：int
		[0]-210khz,默认210khz
		[1]-105khz
		[2]-52khz
		[3]-26khz
		[4]-13khz
		[n]-Invalid
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetLPFLevel(void *handle);

/*********************************************************
* No: No053
* Function Name: HBI_SetBinning
* Function Description: Binning mode setting
* Parameter Description:
	In:	void *handle 		// unsigned handle pointer
		unsigned int nbin		// Binning mode
								// 1: 1x1
								// 2: 2x2
								// 3: 3x3
								// 4: 4x4
	Out:	Null
* Return Value: int
			// 0: Success
			// Non-0: Failure
* Comment:
* 编    号: No053
* 函 数 名: HBI_SetBinning
* 功能描述: 设置图像binning方式
* 参数说明:
		In:
			void *handle - 句柄(无符号指针)
			unsigned int nbin - 1:1x1,2:2x2,3:3x3,4:4x4
		Out:无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetBinning(void *handle, unsigned int bin);

/*********************************************************
* No: No054
* Function Name: HBI_GetBinning
* Function Description: 		// Get binning mode setting value
* Parameter Description:
	In: 	void *handle 		// unsigned handle pointer
		unsigned int *nbin		// Binning mode
								// 1: 1x1
								// 2: 2x2
								// 3: 3x3
								// 4: 4x4
	Out: Null
* Return Value: int
			// 0: Success
			// Non-0: Failure
* Comment:
* 编    号: No054
* 函 数 名: HBI_GetBinning
* 功能描述: 获取图像binning方式
* 参数说明:
		In: void *handle - 句柄(无符号指针)
unsigned int *nbin - 1:1x1,2:2x2,3:3x3,4:4x4
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetBinning(void *handle, unsigned int *bin);

/*********************************************************
* No: No055
* Function Name: HBI_SetPreviewMode
* Function Description: Preview mode setting
* Parameter Description:
		In: void *handle - // unsigned handle pointer
			int inTriggMode
				1-Soft trigger
				3-High voltage trigger
				4-FreeAED
				5-Dynamic:Hvg Sync Mode
				6-Dynamic:Fpd Sync Mode
				7-Dynamic:Fpd Continue
				8-SAEC
				Other not supported。
			int inPreviewMode // Image acquire and display mode
							// 0: normal image,
							// 1: preview image
							// 2: preview + normal image
		Out:	Null
* Return Value: int
			// 0: Success
			// Non-0: Failure
* Comment:
* 编    号: No055
* 函 数 名: HBI_SetPreviewMode
* 功能描述: 设置Preview预览或取消预览
* 功能描述: 设置Preview预览或取消预览
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int inTriggMode
				1-软触发
				3-高压触发
				4-FreeAED
				5-Dynamic:Hvg Sync Mode
				6-Dynamic:Fpd Sync Mode
				7-Dynamic:Fpd Continue
				8-SAEC
				其他不支持。
			int inPreviewMode
				0-normal image
				1-preview image
		Out:无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetPreviewMode(void *handle, int inTriggMode, int inPreviewMode);

/*********************************************************
* No: No056
* Function Name: HBI_GetPreviewMode
* Function Description: 	// Get preview mode setting value
* Parameter Description:
	In: 	void *handle 	// unsigned handle pointer
	Out:	int *outmode 	// Image acquire and display mode  [0~2]
							// 0: normal image,
							// 1: preview image
							// 2: preview + normal image
* Return Value: int
		// 0: Success
		// Non-0: Failure
* Comment:
* 编    号: No056
* 函 数 名: HBI_GetPreviewMode
* 功能描述: 设置Preview
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		out:int *outMode - preview mode [0~2]
			0-normal image,1-preview image,2-preview image and normal image。
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetPreviewMode(void *handle, int *outMode);

/*********************************************************
* No: No057
* Function Name: HBI_SAecModeApply
* Function Description: 		// SAEC mode setting
* Parameter Description:
	In: 	void *handle  		// unsigned handle pointer
		int inPreviewMode	// 0: Normal image
							// 1: Preview image
		int inPerTime  		// FPD pre ready delay time
		int inPostTime 		// FPD post ready delay time
		Out: Null
* Return Value:
	int 					// 0: Success
							// Non-0: Failure
* Comment:
1. inPreviewMode=0(normal image)
2. inPreviewMode=1(preview image), user define inPerTime and inPostTime
* 编    号: No057
* 函 数 名: HBI_SAecModeApply
* 功能描述: 设置或取消SAEC模式
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int inPreviewMode
				0-normal image
				1-preview image
			int inPerTime  - fpd pre ready delay time
			int inPostTime - fpd post ready delay time
		Out:无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:分为两种情况：
			1》启用：inPreviewMode=1(preview image)，inPerTime和inPostTime(客户指定)
			2》取消：inPreviewMode=0(normal image)，inPerTime和inPostTime默认即可
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SAecModeApply(void *handle, int inPreviewMode, int inPerTime = 0, int inPostTime = 0);

/*********************************************************
* No: No058
* Function Name: HBI_SAecAcq
* Function Description: 	// SAEC image acquisition
* Parameter Description：
	In: 	void *handle   	// unsigned handle pointer
	Out: Null
* Return Value: int
			// 0: Success
			// Non-0: Failure
* Comment:
* 编    号: No058
* 函 数 名: HBI_SAecAcq
* 功能描述: SAEC采集上图指令
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SAecAcq(void *handle);

/*********************************************************
* No: No059
* Function Name: HBI_RegProgressCallBack
* Function Description: 	// Register callback function, feedback template or firmware downloading progress status
* Parameter Description：
	In: 	void *handle  	// unsigned handle pointer
		USER_CALLBACK_HANDLE_PROCESS handleStatusfun	// Register callback function
		void *pObject 	    // Object pointer,
	Out: Null
* Return Value: int
			// 0: Success
			// Non-0: Failure
* Comment:
* 编    号: No059
* 函 数 名: HBI_RegProgressCallBack
* 功能描述: 注册回调函数
* 参数说明: 处理固件升级反馈信息
		In: void *handle - 句柄(无符号指针)
			USER_CALLBACK_HANDLE_PROCESS handleStatusfun - 注册回调函数
			void* pObject - 对象指针，亦可为空
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_RegProgressCallBack(void *handle, USER_CALLBACK_HANDLE_PROCESS handleStatusfun, void* pObject);

/*********************************************************
* No: No060
* Function Name: HBI_GenerateTemplate
* Function Description:	// Quick generate offset, gain, defect template
* Parameter Description:
	In: 	void *handle		// unsigned handle pointer
		EnumIMAGE_ACQ_MODE _mode 	// Type of template
		OFFSET_TEMPLATE_MODE     	// Offset template
		GAIN_TEMPLATE_MODE        	// Gain template
		DEFECT_ACQ_GROUP1,        	// Defect template with group1 image
		DEFECT_ACQ_GROUP2,        	// Defect template with group2 image
		DEFECT_ACQ_AND_TEMPLATE	// Defect template with group3 image and generate template
int bprevew		// Whether to generate preview template
				// 1: yes
				// 0: no (default)
	Out: Null
* Return Value:
	int 				// 0: Success
						// Non-0: Failure
* Comment:
* 1、OFFSET_ TEMPLATE: Generate a dark field template, note that the ball tube is closed, and the firmware collects a set of dark field images and generates an Offset template
* 2、GAIN_ TEMPLATE: To generate a gain correction template, it is necessary to open the ball tube, adjust the dose switch, and start the plate to collect bright field images and generate a gain template
* 3、DEFECT_ TEMPLATE_ GROUP1: To generate a bad spot correction template, it is necessary to open the ball tube, adjust the dose, and start collecting the first group of bright field images on the flat panel
* 4、DEFECT_ TEMPLATE_ GROUP2: To generate a bad spot correction template, it is necessary to open the ball tube, adjust the dose, and start collecting the second group of bright field images on the flat panel
* 5、DEFECT_ TEMPLATE_ GROUP3: To generate a bad spot correction template, it is necessary to open the ball tube, adjust the dose, and start the plate to collect the third group of bright field images and generate a bad template
* 6、SOFTWARE_ OFFSET_ TEMPLATE: Generate a dark field template. Note that the ball tube is closed. The SDK collects a set of dark field images and generates an Offset template
* 编    号: No060
* 函 数 名: HBI_GenerateTemplate
* 功能描述: 快速生成模板
* 参数说明:
		In: void *handle - 句柄(无符号指针)
				EnumIMAGE_ACQ_CMD _mode - 生成模板类型
				OFFSET_TEMPLATE_TYPE      连续采集一组暗场图 - Firmware PreOffset Template
				GAIN_TEMPLATE_TYPE        连续采集一组亮场图 - gain Template
				DEFECT_TEMPLATE_GROUP1,   连续采集一组亮场图 - defect group1
				DEFECT_TEMPLATE_GROUP2,   连续采集一组亮场图 - defect group2
				DEFECT_ACQ_AND_TEMPLATE,  连续采集一组亮场图 - defect group3 and generate template
				SOFTWARE_OFFSET_TEMPLATE  连续采集一组暗场图 - Software PreOffset Template
			int bprevew - 是否生成preview模板，1-生成，0-不生成
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GenerateTemplate(void *handle, EnumIMAGE_ACQ_CMD _mode, int bprevew = 0);

/*********************************************************
* No: No061
* Function Name: HBI_DownloadTemplate
* Function Description: 	// Download calibration template
* Parameter Description:
	In: 	void *handle 	// unsigned handle pointer
		DOWNLOAD_FILE *downloadfile	// Template file object pointer
			  emUPLOAD_FILE_TYPE emfiletype-Template type(OFFSET_TMP-offset template.GAIN_TMP-gain template,DEFECT_TMP-defect template)
			  int nBinningtype-template binning type（1-binning1*1,2-binning2*2,3-binning3*3,4-binning4*4）
			  char filepath-Template absolute path
	Out: Null
* Return Value:
	int 			// 0: Success
					// Non-0: Failure
* Comment:
* 编    号: No061
* 函 数 名: HBI_DownloadTemplate
* 功能描述: 下载矫正模板
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			DOWNLOAD_FILE *downloadfile - 下载模板文件对象指针
			  int nModeID:Mode ID,when managing based on multiple Mode templates, it is neccessary to specify the Mode ID.
              emUPLOAD_FILE_TYPE emfiletype-模板类型(OFFSET_TMP-offset模板.GAIN_TMP-gain模板,DEFECT_TMP-defect模板)
	          int nBinningtype-模板binning类型（1-binning1*1,2-binning2*2,3-binning3*3,4-binning4*4）
	          char filepath-模板绝对路径
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:建议使用HBI_DownloadTemplateByType替换。
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_DownloadTemplate(void *handle, DOWNLOAD_FILE *downloadfile);

/*********************************************************
* No: No062
* Function Name: HBI_DownloadTemplateEx
* Function Description: 	// Download calibration template
* Parameter Description:
	In: 	void *handle 	// unsigned handle pointer
			DOWNLOAD_MODE *downloadmode - Download template file link list pointer
		  DOWNLOAD_FILE *downloadfile	- Template file object pointer
			emUPLOAD_FILE_TYPE emfiletype-Template type(OFFSET_TMP-offset template.GAIN_TMP-gain template,DEFECT_TMP-defect template)
			  int nBinningtype-template binning type（1-binning1*1,2-binning2*2,3-binning3*3,4-binning4*4）
			  char filepath-Template absolute path
	Out: Null
* Return Value:
	int 			// 0: Success
					// Non-0: Failure
* Comment:
* 编    号: No062
* 函 数 名: HBI_DownloadTemplateEx
* 功能描述: 下载矫正模板
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			DOWNLOAD_MODE *downloadmode - 下载模板文件链表指针
				DOWNLOAD_FILE *downloadfile - 下载模板文件对象指针
						emUPLOAD_FILE_TYPE emfiletype-模板类型(OFFSET_TMP-offset模板.GAIN_TMP-gain模板,DEFECT_TMP-defect模板)
						int nBinningtype-模板binning类型（1-binning1*1,2-binning2*2,3-binning3*3,4-binning4*4）
						char filepath-模板绝对路径
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_DownloadTemplateEx(void *handle, DOWNLOAD_MODE *downloadmode);

/*********************************************************
* No: No063
* Function Name: HBI_DownloadTemplateByType
* Function Description:     // Download calibration template by template type
* Parameter Description:
	In: 	void *handle 	// unsigned handle pointer
int infiletype //Download file types 0-gain template, 1-defect template, 2-offset template, others - not supported.
	Out: Null
* Return Value:
	int 				// 0: Success
						// Non-0: Failure
* Comment:
* 编    号: No063
* 函 数 名: HBI_DownloadTemplateByType
* 功能描述: 按照类型默认下载指定矫正模板文件
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int infiletype - 下载文件类型0-gain模板，1-defect模板，2-offset模板，其他-不支持
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_DownloadTemplateByType(void *handle, int infiletype);

/*********************************************************
* No: No064
* Function Name: HBI_StopDownloadTemplate
* Function Description: 	// Stop download calibration template
* Parameter Description:
	In:  void *handle 	    // unsigned handle pointer
	Out: Null
* Return Value: int
		 // 0: Success
		// Non-0: Failure
* Comment:
* 编    号: No064
* 函 数 名: HBI_StopDownloadTemplate
* 功能描述: 停止下载矫正模板
* 参数说明:
		In:  void *handle - 句柄(无符号指针)
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_StopDownloadTemplate(void *handle);

/*********************************************************
* No: No065
* Function Name: HBI_UpgradeFirmware
* Function Description: Update firmware information or wireless embedded software
* Parameter Description:
		In:  void *handle // unsigned handle pointer
			char *binfile - Bin file path
			int filetype - 0: FPGA software, 1: Embedded software
	Out: Null
* Return Value: int
		 // 0: Success
		 // Non-0: Failure
* Comment:
* 编    号: No065
* 函 数 名: HBI_UpgradeFirmware
* 功能描述: 升级固件信息或无线嵌入式软件
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			char *binfile - bin文件路径
			int filetype - 0:FPGA软件，1:Embedded软件，2-pcie软件
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_UpgradeFirmware(void *handle, char *binfile, int filetype=0);

/*********************************************************
* No: No066
* Function Name: HBI_StopUpdateFirmware
* Function Description: Stop updating firmware
* Parameter Description:
		In:  void *handle // unsigned handle pointer
	Out: Null
* Return Value: int
		 // 0: Success
		 // Non-0: Failure
* Comment:
* 编    号: No066
* 函 数 名: HBI_StopUpdateFirmware
* 功能描述: 停止更新固件
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		out:无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_StopUpdateFirmware(void *handle);

/*********************************************************
* No: No067
* Function Name: HBI_Dumping
* Function Name: Clear command
* Parameter Description:
		In:  void *handle // unsigned handle pointer
	Out: Null
* Return Value: int
		 // 0: Success
		 // Non-0: Failure
* Comment:
* 编    号: No067
* 函 数 名: HBI_Dumping
* 功能描述: 清空指令
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_Dumping(void *handle);

/*********************************************************
* No: No068
* Function Name: HBI_GetSysParamCfg
* Function Description: Read back the system RAM/ROM parameters, asynchronous events, and feed back the parameters in the callback function
* Parameter Description:
	In: void *handle - unsigned handle pointer
	int cmd  - rom or ram
	int type - user or admin
	Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No068
* 函 数 名: HBI_GetSysParamCfg
* 功能描述: 回读系统RAM/ROM参数,异步事件，在参数在回调函数中反馈
* 参数说明:
	In: void *handle - 句柄(无符号指针)
	int cmd  - rom or ram
	int type - user or admin
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetSysParamCfg(void *handle, int cmd, int type);

/*********************************************************
* No: No069
* Function Name: HBI_SetSysParamCfg
* Function Description: Set system RAM/ROM parameters
* Parameter Description:
	In: void *handle - unsigned handle pointer
	int cmd -
	int type -
	RegCfgInfo* pRegCfg -
	Out: Null
* Return Value：int
		   0: Success
		Non-0: Failure
* Comment:
* 编    号: No069
* 函 数 名: HBI_SetSysParamCfg
* 功能描述: 配置系统RAM/ROM参数
* 参数说明:
	In: void *handle - 句柄(无符号指针)
	int cmd -
	int type -
	RegCfgInfo* pRegCfg -
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetSysParamCfg(void *handle, int cmd, int type, RegCfgInfo* pRegCfg);

/*********************************************************
* No: No070
* Function Name: HBI_SetGigabitEther
* Function Description: Set network information parameters
* Parameter Description:
	In: void *handle - unsigned handle pointer
		EtherInfo* pEther,refer to <<HbiType.h>>。
	Out:None
* Return Value：int
		   0: Success
		Non-0: Failure
* Comment:
* 编    号: No070
* 函 数 名: HBI_SetGigabitEther
* 功能描述: 设置网络信息参数
* 参数说明:
	In: void *handle - 句柄(无符号指针)
		EtherInfo* pEther,见HbiType.h。
	Out:无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetGigabitEther(void *handle, EtherInfo* pEther);

/*********************************************************
* No: No071
* Function Name: HBI_RetransMissPacket
* Function Description: Single packet loss retransmission
* Parameter Description:
		In: void *handle - unsigned handle pointer
		Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No071
* 函 数 名: HBI_RetransMissPacket
* 功能描述: 单包丢包重传
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_RetransMissPacket(void *handle);

/*********************************************************
* No: No072
* Function Name: HBI_RetransMissFrame
* Function Description: Entire frame retransmission
* Parameter Description:
		In: void *handle - unsigned handle pointer
		Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No072
* 函 数 名: HBI_RetransMissFrame
* 功能描述: 整帧重传
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_RetransMissFrame(void *handle);

/*********************************************************
* No: No073
* Function Name: HBI_IsConnect
* Function Description: Determine whether the detector is connected
* Parameter Description:
		In: void *handle - unsigned handle pointer
* Return Value：int
		1 - connect
		0 - disconnect
* Comment:
* 编    号: No073
* 函 数 名: HBI_IsConnect
* 功能描述: 判断平板是为连接状态
* 参数说明:
		In: void *handle - 句柄(无符号指针)
* 返 回 值：int
		1 - 连接
		0 - 断开
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_IsConnect(void *handle);

/*********************************************************
* No: No074
* Function Name: HBI_GetCurFPDStatus
* Function Description: Get the current status of the detector
* Parameter Description:
		In: void *handle - unsigned handle pointer
		Out: Null
* Return Value：int
		0 - idle status，Idle state
		1 - busy status，Busy state，Reject the operation
* Comment:
* 编    号: No074
* 函 数 名: HBI_GetCurFPDStatus
* 功能描述: 获取平板当前状态
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		Out: 无
* 返 回 值：int
		0 - idle status，表示可以执行该操作
		1 - busy status，表示状态繁忙中，拒绝执行该操作
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetCurFPDStatus(void *handle);

/*********************************************************
* No: No075
* Function Name: HBI_SetCapacity
* Function Description: Set buffer capacity (size)
* Parameter Description:
		In: void *handle - unsigned handle pointer
			unsigned int nframesize - The number of frames that can be stored,
			The buffer size is equal to the frame size multiplied by the number of frames。
		Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure，Reference error code ‘HbiError.h'
* Comment:
* 编    号: No075
* 函 数 名: HBI_SetCapacity
* 功能描述: 设置缓冲区容量(大小)
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			unsigned int nframesize - 可存储的帧数,
			缓冲区大小等于帧大小乘以帧数。
		Out: 无
* 返 回 值：int
		0   成功
		非0 失败，见HbiError.h错误码
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetCapacity(void *handle, unsigned int nframesize);

/*********************************************************
* No: No076
* Function Name: HBI_SetRawStyle
* Function Description: Set whether to save the image and the image file format
* Parameter Description:
		In: void *handle - unsigned handle pointer
			bool bsave - Save or display,false:Display without saving，true:Save Don't Display
			bool bsingleraw - Save in a single file or multiple files, false: 1 frame data can be saved in one file, true: multiple frame data can be saved in one file
		Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No076
* 函 数 名: HBI_SetRawStyle
* 功能描述: 设置是否保存图像以及图像文件形式
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			bool bsave - 保存拟或显示,false:显示不保存，true:保存不显示
			bool bsingleraw - 保存在单个文件或多个文件,false:1帧数据保存1个文件，true:多帧数据可保存在一个文件中
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetRawStyle(void *handle, bool bsave, bool bsingleraw = false);

/*********************************************************
* No: No077
* Function Name: HBI_SetAqcProperty
* Function Description: Set collection properties
* Parameter Description:
		In: void *handle - unsigned handle pointer
			FPD_AQC_MODE _mode - Acquisition mode and parameters
		Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No077
* 函 数 名: HBI_SetAqcProperty
* 功能描述: 设置采集属性
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			FPD_AQC_MODE _mode - 采集模式以及参数
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetAqcProperty(void *handle, FPD_AQC_MODE _mode);

/*********************************************************
* No: No078
* Function Name: HBI_SetDebugMode
* Function Description: Switch debugging mode and production mode - used to debug PCIE interface
* Parameter Description:
	In: void *handle - unsigned handle pointer
		int mode - 0: production mode, 1-PCIE debugging mode
	Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:Debug PCIE interface
* 编    号: No078
* 函 数 名: HBI_SetDebugMode
* 功能描述: 切换调试模式和生产模式-用于调试PCIE接口
* 参数说明:
	In: void *handle - 句柄(无符号指针)
		int mode - 0:生产模式，1-PCIE调试模式
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetDebugMode(void *handle, int mode);

/*********************************************************
* No: No079
* Function Name: HBI_PcieSendBuff
* Function Description: Send data to PCIE address, valid only in PCIE debugging mode
* Parameter Description:
	In: void *handle - unsigned handle pointer
		ePCIE_WR_TYPE etype - 'Bar' address or 'ddr' address type
		long addr - 'Bar' address or 'ddr' address
		unsigned char *szSndBuff - Write Buffer
		int nlen - Buffer data length
	Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No079
* 函 数 名: HBI_PcieSendBuff
* 功能描述: 向PCIE地址发送数据，仅限于PCIE调试模式下有效
* 参数说明:
	In: void *handle - 句柄(无符号指针)
		ePCIE_WR_TYPE etype - bar地址或ddr地址类型
		long addr - ar地址或ddr地址
		unsigned char *szSndBuff - 写入缓冲区
		int nlen - 缓冲区数据长度
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_PcieSendBuff(void *handle, ePCIE_WR_TYPE etype, long addr, unsigned char *szSndBuff, int nlen);

/*********************************************************
* No: No080
* Function Name: HBI_PcieRecvBuff
* Function Description: Read PCIE address data, valid only in PCIE debug mode
* Parameter Description:
	In: void *handle - unsigned handle pointer
		ePCIE_WR_TYPE etype - 'Bar' address or 'ddr' address type
		long addr - 'Bar' address or 'ddr' address
		unsigned char *szRcvbuff - Receive Buffer
		int nlen - Buffer data length
	Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No080
* 函 数 名: HBI_PcieRecvBuff
* 功能描述: 读取PCIE地址数据，仅限于PCIE调试模式下有效
* 参数说明:
	In: void *handle - 句柄(无符号指针)
		ePCIE_WR_TYPE etype - bar地址或ddr地址类型
		long addr - ar地址或ddr地址
		unsigned char *szRcvbuff - 接收缓冲区
		int nlen - 缓冲区数据长度
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_PcieRecvBuff(void *handle, ePCIE_WR_TYPE _type, long addr, unsigned char *szRcvbuff, int nlen);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Distributed generation template is used for template testing. For integration, please refer to the interface of rapid generation template
// 分布生成模板，用于模板测试，集成请参考快速生成模板接口
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*********************************************************
* No: No081
* Function Name: HBI_InitOffsetMode
* Function Description: Initialize dark field template
* Parameter Description:
	In: void *handle - unsigned handle pointer
		CALIBRATE_INPUT_PARAM calibrate_param,refer to <<HbiType.h>>。
	Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No081
* 函 数 名: HBI_InitOffsetMode
* 功能描述: 初始化暗场模板
* 参数说明:
	In: void *handle - 句柄(无符号指针)
		CALIBRATE_INPUT_PARAM calibrate_param,见HbiType.h。
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_InitOffsetMode(void *handle, CALIBRATE_INPUT_PARAM calibrate_param);

/*********************************************************
* No: No082
* Function Name: HBI_InsertOffsetMode
* Function Description: Insert data into offset correction model
* Parameter Description:
	In: void *handle - unsigned handle pointer
		int group_id - group ID
		char *filepath - File path
	Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No082
* 函 数 名: HBI_InsertOffsetMode
* 功能描述: 向offset矫正模型中插入数据
* 参数说明:
	In: void *handle - 句柄(无符号指针)
		int group_id - 组ID
		char *filepath - 文件路径
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_InsertOffsetMode(void *handle, int group_id, char *filepath);

/*********************************************************
* No: No083
* Function Name: HBI_ClearOffsetMode
* Function Description: Clear offset correction model
* Parameter Description:
	In: void *handle - unsigned handle pointer
	Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No083
* 函 数 名: HBI_ClearOffsetMode
* 功能描述: 清空offset矫正模型
* 参数说明:
	In: void *handle - 句柄(无符号指针)
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_ClearOffsetMode(void *handle);

/*********************************************************
* No: No084
* Function Name: HBI_GenerateOffsetTemp
* Function Description: Generate offset template
* Parameter Description:
	In: void *handle - unsigned handle pointer
		int raw_num - Number of dark field pictures
		int bprevew - Whether to generate preview template, 1-generate, 0-do not generate
	Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No084
* 函 数 名: HBI_GenerateOffsetTemp
* 功能描述: 生成offset模板
* 参数说明:
	In: void *handle - 句柄(无符号指针)
		int raw_num - 暗场图个数
		int bprevew - 是否生成preview模板，1-生成，0-不生成
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GenerateOffsetTemp(void *handle, int raw_num, int bprevew = 0);

/*********************************************************
* No: No085
* Function Name: HBI_InitGainMode
* Function Description: Initialize gain correction model
* Parameter Description:
	In: void *handle - unsigned handle pointer
		CALIBRATE_INPUT_PARAM calibrate_param -Correction parameters
	Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No085
* 函 数 名: HBI_InitGainMode
* 功能描述: 初始化gain矫正模型
* 参数说明:
	In: void *handle - 句柄(无符号指针)
		CALIBRATE_INPUT_PARAM calibrate_param - 矫正参数
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_InitGainMode(void *handle, CALIBRATE_INPUT_PARAM calibrate_param);

/*********************************************************
* No: No086
* Function Name: HBI_InsertGainMode
* Function Description: Insert data into the gain correction model
* Parameter Description:
	In: void *handle - unsigned handle pointer
	int group_id - group ID
	char *filepath - File path
	Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No086
* 函 数 名: HBI_InsertGainMode
* 功能描述: 向gain矫正模型中插入数据
* 参数说明:
	In: void *handle - 句柄(无符号指针)
	int group_id - 组ID
	char *filepath - 文件路径
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_InsertGainMode(void *handle, int group_id, char *filepath);

/*********************************************************
* No: No087
* Function Name: HBI_ClearGainMode
* Function Description: Empty gain correction model
* Parameter Description:
	In: void *handle - unsigned handle pointer
	Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No087
* 函 数 名: HBI_ClearGainMode
* 功能描述: 清空gain矫正模型
* 参数说明:
	In: void *handle - 句柄(无符号指针)
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_ClearGainMode(void *handle);

/*********************************************************
* No: No088
* Function Name: HBI_GenerateGainTemp
* Function Description: Generate gain template
* Parameter Description:
	In: void *handle - unsigned handle pointer
		int group_sum - groups
		int per_group_num - Generate the number of pictures in each group of gain template
		int bprevew       - Whether to generate preview template, 1-generate, 0-do not generate
		Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No088
* 函 数 名: HBI_GenerateGainTemp
* 功能描述: 生成gain模板
* 参数说明:
	In: void *handle - 句柄(无符号指针)
		int group_sum - 组数
		int per_group_num - 每组包含图片个数
		int bprevew       - 是否生成preview模板，1-生成，0-不生成
		Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GenerateGainTemp(void *handle, int group_sum, int per_group_num, int bprevew = 0);

/*********************************************************
* No: No089
* Function Name: HBI_InitDefectMode
* Function Description: Initialize the defect correction model
* Parameter Description:
	In: void *handle - unsigned handle pointer
		CALIBRATE_INPUT_PARAM calibrate_param - Correction parameters
	Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No089
* 函 数 名: HBI_InitDefectMode
* 功能描述: 初始化defect矫正模型
* 参数说明:
	In: void *handle - 句柄(无符号指针)
		CALIBRATE_INPUT_PARAM calibrate_param - 矫正参数
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_InitDefectMode(void *handle, CALIBRATE_INPUT_PARAM calibrate_param);

/*********************************************************
* No: No090
* Function Name: HBI_InsertDefectMode
* Function Description: Insert data into the defect correction model
* Parameter Description:
	In: void *handle - unsigned handle pointer
		int group_id - group ID
		char *filepath - File path
	Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No090
* 函 数 名: HBI_InsertDefectMode
* 功能描述: 向defect矫正模型中插入数据
* 参数说明:
	In: void *handle - 句柄(无符号指针)
		int group_id - 组ID
		char *filepath - 文件路径
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_InsertDefectMode(void *handle, int group_id, char *filepath);

/*********************************************************
* No: No091
* Function Name: HBI_ClearDefectMode
* Function Description: Empty the defect correction model
* Parameter Description:
	In: void *handle - unsigned handle pointer
	Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No091
* 函 数 名: HBI_ClearDefectMode
* 功能描述: 清空defect矫正模型
* 参数说明:
	In: void *handle - 句柄(无符号指针)
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_ClearDefectMode(void *handle);

/*********************************************************
* No: No092
* Function Name: HBI_GenerateDefectTemp
* Function Description: Generate defect template
* Parameter Description:
	In: void *handle - unsigned handle pointer
	int group_sum - groups
	int per_group_num - Number of pictures in each group
	int bprevew       - Whether to generate preview template, 1-generate, 0-do not generate
	Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No092
* 函 数 名: HBI_GenerateDefectTemp
* 功能描述: 生成defect模板
* 参数说明:
	In: void *handle - 句柄(无符号指针)
	int group_sum - 组数
	int per_group_num - 每组包含图片个数
	int bprevew       - 是否生成preview模板，1-生成，0-不生成
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GenerateDefectTemp(void *handle, int group_sum, int per_group_num, int bprevew = 0);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 文件工作目录地址管理，暂未实现
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////*********************************************************
////* No: No093
////* Function Name: HBI_SetFileDirectory
////* Function Description: Set file storage path
////* Parameter Description:
////		In: void *handle - unsigned handle pointer
////			HBI_FILE_TYPE filetype - File type, enumeration class
////				0    IMAGE_FILE
////				1    TEMPLATE_FILE
////				char *dirpath - File path
////		Out: Null
////* Return Value：int
////     0: Success
////	   Non-0: Failure
////* Comment:
////* 编    号: No093
////* 函 数 名: HBI_SetFileDirectory
////* 功能描述: 设置文件存储路径
////* 参数说明:
////		In: void *handle - 句柄(无符号指针)
////			HBI_FILE_TYPE filetype - 文件类型，枚举类
////				0    IMAGE_FILE
////				1    TEMPLATE_FILE
////				char *dirpath - 文件路径
////		Out: 无
////* 返 回 值：int
////		0 - 成功
////		1 - 失败
////* 备    注:
////*********************************************************/
////HBI_FPD_DLL int __stdcall HBI_SetFileDirectory(void *handle, HBI_FILE_TYPE filetype, char *dirpath);

/*********************************************************
* No: No094
* Function Name: HBI_GetFileDirectory
* Function Description: Get the current file storage path
* Parameter Description:
		In: void *handle - unsigned handle pointer
			HBI_FILE_TYPE filetype - File type, enumeration class
		  0 IMAGE_ FILE - image file directory
		  1 TEMPLATE_ FILE - Calibration template file directory (refers to gain and defect template file path)
		  2 TEMPLATE_ IMG-calibration template middle image data file directory
		Out: Null
* Return Value：int
		NULL  - Failure
		char* - return path
* Comment:
* 编    号: No094
* 函 数 名: HBI_GetFileDirectory
* 功能描述: 获取当前文件存储路径
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			HBI_FILE_TYPE filetype - 文件类型，枚举类
				0    IMAGE_FILE-图像文件目录
				1    TEMPLATE_FILE-校准模板文件目录（指gain和defect模板文件路径）
	            2    TEMPLATE_IMG-校准模板中间图像数据文件目录
		Out: 无
* 返 回 值：int
		NULL  - 失败
		char* - 返回路径
* 备    注:
*********************************************************/
HBI_FPD_DLL char* __stdcall HBI_GetFileDirectory(void *handle, HBI_FILE_TYPE filetype);

/*********************************************************
* No: No095
* Function Name: HBI_GetCurTempFileFullDirectory
* Function Description: Get the full directory of the current template file according to the template type
* Parameter Description:
		In: void *handle  - unsigned handle pointer
		In: int tempType -Template type (default is 0)，0-offset template;1-gain template;2-defect template;
Out: Null
* Return Value：char*
		NULL - Failure
		char* - Return directory path
* Comment:
* 编    号: No095
* 函 数 名: HBI_GetCurTempFileFullDirectory
* 功能描述: 根据模板类型获取当前模板文件全目录
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		In: int tempType - 模板类型（默认为0），0-offset template;1-gain template;2-defect template;
		Out: 无
* 返 回 值：char*
		NULL  - 失败
		char* - 返回目录路径
* 备    注:
*********************************************************/
HBI_FPD_DLL char* __stdcall HBI_GetCurTempFileFullDirectory(void *handle, int temptype=0);

/*********************************************************
* No: No096
* Function Name: HBI_IsBusyNow
* Function Description: Judge whether the current SDK is being processed. The current operation is not allowed
* Parameter Description:
		In: void *handle - unsigned handle pointer
				eventtype - Generate template type
				1 Distribution generation template processing
				2. Rapid template generation is in progress
				3. During collection
				4. During parameter setting
		Out: Null
* Return Value：int
		0 - idle status，Indicates that the operation can be performed
		1 - busy status，Indicates that the operation is refused because the status is busy
* Comment:
* 编    号: No096
* 函 数 名: HBI_IsBusyNow
* 功能描述: 判断当前SDK是否正处理中，不允许当前操作
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			eventtype eventtype - 生成模板类型
				1    分布生成模板处理中
				2    快速生成模板处理中
				3,   采集过程中
				4,   设置参数过程中
		Out: 无
* 返 回 值：int
		0 - idle status，表示可以执行该操作
		1 - busy status，表示状态繁忙中，拒绝执行该操作
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_IsBusyNow(void *handle, int eventtype);

/*********************************************************
* No: No097
* Function Name: HBI_SetCuttingImageRect
* Function Description: set rect of cutting image
* Parameter Description:
		In: void *handle  - unsigned handle pointer
		In: CuttingImageRect *roi - The param of 'rect of cutting image'
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No097
* 函 数 名: HBI_SetCuttingImageRect
* 功能描述: set rect of cutting image
* 参数说明:
		In: void *handle  - 句柄(无符号指针)
		In: CuttingImageRect *roi - The param of 'rect of cutting image'
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetCuttingImageRect(void *handle, CuttingImageRect*roi);

/*********************************************************
* No: No098
* Function Name: HBI_GetCuttingImageRect
* Function Description: get rect of cutting image
* Parameter Description:
		In: void *handle - unsigned handle pointer
* Return Value：CuttingImageRect*
		NULL   - Failure
		Non-NULL - Success,The param of 'rect of cutting image'
* Comment:
* 编    号: No098
* 函 数 名: HBI_GetCuttingImageRect
* 功能描述: get rect of cutting image
* 参数说明:
		In: void *handle - 句柄(无符号指针)
* 返 回 值：CuttingImageRect*
		NULL   - 失败
		非NULL - 成功,The param of 'rect of cutting image'
* 备    注:
*********************************************************/
HBI_FPD_DLL CuttingImageRect* __stdcall HBI_GetCuttingImageRect(void *handle);

//////////////////////////////////////////////////////////////////////////
// User Mode Development
// Mode开发接口 add by mhyang 20230327
//////////////////////////////////////////////////////////////////////////
/*********************************************************
* No: No099
* Function Name: HBI_GetCurModeList
* Function Description: Get Mode List
* Parameter Description:
		In: void *handle - unsigned handle pointer
* Return Value：FPD_MODE_LIST*
		NULL   - Failure
		Non-NULL - Success,The param of User Acquisition Mode Parameter Configuration
* Comment:
* 编    号: No099
* 函 数 名: HBI_GetCurModeList
* 功能描述: 获取Mode List
* 参数说明:
		In: void *handle - 句柄(无符号指针)
* 返 回 值：FPD_MODE_LIST*
		NULL   - 失败
		Non-NULL - 成功,用户采集Mode列表信息
* 备    注:
*********************************************************/
HBI_FPD_DLL FPD_MODE_LIST* __stdcall HBI_GetCurModeList(void* handle);

/*********************************************************
* No: No100
* Function Name: HBI_UpdateModeList
* Function Description: Update Mode List
* Parameter Description:
		In: void *handle - unsigned handle pointer
			FPD_MODE_LIST* modelist -User Acquisition Mode Parameter Configuration
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No100
* 函 数 名: HBI_UpdateModeList
* 功能描述: 更新Mode List
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			FPD_MODE_LIST* modelist -用户采集Mode列表信息
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_UpdateModeList(void* handle, FPD_MODE_LIST* modelist);

/*********************************************************
* No: No101
* Function Name: HBI_GetCurModeData
* Function Description: Obtain the current Mode Data
* Parameter Description:
		In: void *handle - unsigned handle pointer
* Return Value：FPD_MODE_DATA*
		NULL   - Failure
		Non-NULL - Success,The param of fpd mode data information
* Comment:
* 编    号: No101
* 函 数 名: HBI_GetCurModeData
* 功能描述: 获取当前Mode Data
* 参数说明:
		In: void *handle - 句柄(无符号指针)
* 返 回 值：FPD_MODE_DATA*
		NULL   - 失败
		非-NULL - 成功,The param of fpd mode data information
* 备    注:
*********************************************************/
HBI_FPD_DLL FPD_MODE_DATA* __stdcall HBI_GetCurModeData(void* handle);

/*********************************************************
* No: No102
* Function Name: HBI_UpdateCurModeData
* Function Description: Update current Mode Data
* Parameter Description:
		In: void *handle - unsigned handle pointer
			FPD_MODE_DATA* mode -fpd mode data information
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No102
* 函 数 名: HBI_UpdateCurModeData
* 功能描述: 更新当前Mode Data
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			FPD_MODE_DATA* mode -fpd mode data information
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_UpdateCurModeData(void* handle, FPD_MODE_DATA* mode);

/*********************************************************
* No: No103
* Function Name: HBI_UpdateModeData
* Function Description: Update Mode Item Data through Mode ID
* Parameter Description:
		In: void *handle - unsigned handle pointer
			FPD_MODE_DATA* mode -fpd mode data information
			unsigned int id - mode id
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No103
* 函 数 名: HBI_UpdateModeData
* 功能描述: 更新Mode项Data通过mode ID
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			FPD_MODE_DATA* mode -fpd mode data information
			unsigned int id - mode id
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_UpdateModeData(void* handle, FPD_MODE_DATA* mode, unsigned int id = 0);

/*********************************************************
* No: No104
* Function Name: HBI_GetModeData
* Function Description: Obtain Mode Data through Mode ID
* Parameter Description:
		In: void *handle - unsigned handle pointer
			unsigned int id - mode id
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No104
* 函 数 名: HBI_GetModeData
* 功能描述: 获取Mode Data通过mode ID
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			unsigned int id - mode id
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL FPD_MODE_DATA* __stdcall HBI_GetModeData(void* handle, unsigned int id = 0);

/*********************************************************
* No: No105
* Function Name: HBI_GetModeListSize
* Function Description: Obtain the number of items for Mode
* Parameter Description:
		In: void *handle - unsigned handle pointer
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No105
* 函 数 名: HBI_GetModeListSize
* 功能描述: 获取Mode的item数目
* 参数说明:
		In: void *handle - 句柄(无符号指针)
* 返 回 值：int
		0   - 成功
		非0 - Mode列表中Mode项的个数
* 备    注:
*********************************************************/
HBI_FPD_DLL unsigned int __stdcall HBI_GetModeListSize(void* handle);

/*********************************************************
* No: No106
* Function Name: HBI_GetCurModeID
* Function Description: Obtain the current Mode ID
* Parameter Description:
		In: void *handle - unsigned handle pointer
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No106
* 函 数 名: HBI_GetCurModeID
* 功能描述: 获取当前Mode id
* 参数说明:
		In: void *handle - 句柄(无符号指针)
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetCurModeID(void* handle);

/*********************************************************
* No: No107
* Function Name: HBI_SwitchUserMode
* Function Description: Switch Mode
* Parameter Description:
		In: void *handle - unsigned handle pointer
		unsigned int modeid  - mode id
		FPD_MODE_DATA *PMODEDATA - fpd mode data information
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No107
* 函 数 名: HBI_SwitchUserMode
* 功能描述: 切换Mode项
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		unsigned int modeid  - mode id
		FPD_MODE_DATA *PMODEDATA - Mode项数据信息，可以为NULL
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SwitchUserMode(void* handle, unsigned int modeid = 0, FPD_MODE_DATA *PMODEDATA=NULL);

/*********************************************************
* No: No108
* Function Name: HBI_ModeExists
* Function Description: Determine whether the mode exists
* Parameter Description:
		In: void *handle - unsigned handle pointer
			FPD_MODE_DATA mode - fpd mode data information
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No108
* 函 数 名: HBI_ModeExists
* 功能描述: 判断mode的是否存在
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			FPD_MODE_DATA mode - Mode项数据信息
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_ModeExists(void* handle, FPD_MODE_DATA mode);

/*********************************************************
* No: No109
* Function Name: HBI_CheckCurtMode
* Function Description: Judge whether the mode is normal
* Parameter Description:
		In: void *handle - unsigned handle pointer
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No109
* 函 数 名: HBI_CheckCurtMode
* 功能描述: 判断mode是否正常
* 参数说明:
		In: void *handle - 句柄(无符号指针)
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL bool __stdcall HBI_CheckCurtMode(void* handle);

/*********************************************************
* No: No110
* Function Name: HBI_ModeCorrectTempalteEnable
* Function Description: The calibration template of Mode is effective or not effective
* Parameter Description:
		In: void *handle - unsigned handle pointer
			unsigned int id  - mode id
			bool bEnable  - 1 - vailid， 0-invaild
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No110
* 函 数 名: HBI_ModeCorrectTempalteEnable
* 功能描述: 当前Mode项的校准模板生效或不生效
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			unsigned int id  - mode id
			bool bEnable  - 1 - 有效， 0-无效
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_ModeCorrectTempalteEnable(void* handle, unsigned int id, bool bEnable = true);

/*********************************************************
* No: No111
* Function Name: HBI_AllModeCorrectTempalteEnable
* Function Description: All calibration templates for Mode items are valid or not valid
* Parameter Description:
		In: void *handle - unsigned handle pointer
			bool bEnable  - 1 - vailid， 0-invaild
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No111
* 函 数 名: HBI_AllModeCorrectTempalteEnable
* 功能描述: 所有的Mode项的校准模板生效或不生效
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			bool bEnable  - 1 - 有效， 0-无效
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_AllModeCorrectTempalteEnable(void* handle, bool bEnable = true);

/*********************************************************
* No: No112
* Function Name: HBI_SaveModeList2File
* Function Description: Save the current user mode list
* Parameter Description:
		In: void *handle - unsigned handle pointer
			char* szPath - file path
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No112
* 函 数 名: HBI_SaveModeList2File
* 功能描述: 保存当前user mode list
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			char* szPath - 文件路径
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SaveModeList2File(void* handle, char* szPath = NULL);

/*********************************************************
* No:No113
* Function name: HBI_DownloadModeTemplate (new)
* Function description: Download template based on Mode parameter
* Parameter Description:
	In: void * handle - Handle (unsigned pointer)
		Int intemptype - Download file types 0-gain template, 1-defect template, 2-offset template, others - Not supported
	Out: None
* Return value: int
	0- Success
	Non 0- Failed
* Note:
* 编    号:No113
* 函 数 名: HBI_DownloadModeTemplate (new)
* 功能描述: 基于Mode参数下载模板
* 参数说明:
	In: void * handle - 句柄(无符号指针)
		DOWNLOAD_MODE_FILE downloadfile - 下载模板信息
	Out: None
* 返 回 值: int
		0   - 成功
		非0 - 失败
* 备    注: Download template add by mhyang 20230606
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_DownloadModeTemplate(void* handle, DOWNLOAD_MODE_FILE downloadfile); 

/*********************************************************
* No:No114
* Function name: HBI_ DownloadModeTemplateByType (new)
* Function description: Download correction template files by template type (default to current Mode Id)
* Parameter Description:
	In: void * handle - Handle (unsigned pointer)
		Int intemptype - Download file types 0-gain template, 1-defect template, 2-offset template, others - Not supported
	Out: None
* Return value: int
		0- Success
		Non 0- Failed
*	Note:
* 编    号:No114
* 函 数 名: HBI_DownloadModeTemplateByType（新增）
* 功能描述: 按照模板类型下载矫正模板文件(默认为当前Mode Id)
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int intemptype - 下载文件类型0-gain模板，1-defect模板，2-offset模板，其他-不支持
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注: 下载模板 add by mhyang 20230606
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_DownloadModeTemplateByType(void* handle, int intemptype);

/*********************************************************
* No:No115
* Function name: HBI_DownloadModeTemplateByIdAndType (new)
* Function description: Download correction template files according to Mode Id and template type
* Parameter Description:
	In: void * handle - Handle (unsigned pointer)
		Unsigned int modeid - Mode Id [0,9]
		Int intemptype - Download file types 0-gain template, 1-defect template, 2-offset template, others - Not supported
	Out: None
*	Return value: int
	0- Success
	Non 0- Failed
* Note:
* 编    号:No115
* 函 数 名: HBI_DownloadModeTemplateByIdAndType（新增）
* 功能描述: 按照Mode Id和模板类型下载矫正模板文件
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			unsigned int modeid - Mode Id [0,9]
			int intemptype - 下载文件类型0-gain模板，1-defect模板，2-offset模板，其他-不支持
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注: 下载模板 add by mhyang 20230605
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_DownloadModeTemplateByIdAndType(void* handle, unsigned int modeid, int intemptype);

/*********************************************************
* No:No116
* Function Name: HBI_GetCurModeTemplateStatus（new）
* Function Description: 获取当前Mode项的固件校正模板状态
* Parameter Description:
		In: void *handle - unsigned handle pointer
			unsigned int modeid - Mode Id [0,9]
			int intemptype - Download file types 0-gain template, 1-defect template, 2-offset template, others - not supported
		Out: None
* Return Value：int
		0: Success
		Non-0: Failure
* Comment:
* 编    号:No116
* 函 数 名: HBI_GetCurModeTemplateStatus（新增）
* 功能描述: 获取当前Mode项的固件校正模板状态
* Parameter Description:
		In: void *handle - 句柄(无符号指针)
			int* nTempStatus - 模板状态,0-暗场亮场无效，1-暗场有效亮场无效，2-暗场无效亮场有效，3-暗场亮场有效, 其他-暗场亮场无效
		Out: None
* Return Value：int
		0: 成功
		Non-0: 失败
* Comment:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetCurModeTemplateStatus(void* handle, int* nTempStatus);
//////////////////////////////////////////////////////////////////////////
// User Mode Developmen
// Mode开发接口 add by mhyang 20230328
//////////////////////////////////////////////////////////////////////////

/*********************************************************
* No:No117
* Function Name: HBI_GetCurrentCorrectFilePath
* Function Description: Obtain the absolute path of the current template file according to the template type
* Parameter Description:
		In: void *handle  - unsigned handle pointer
		In: int tempType -Template type (default is 0)，0-offset template;1-gain template;2-defect template;3-defect map template;4-gainb template;
		Out: Null
* Return Value：char*
		NULL - Failure
		char* - Return absolute path
* Comment:
* 编    号:No117
* 函 数 名: HBI_GetCurrentCorrectFilePath
* 功能描述: 根据模板类型获取当前模板文件绝对路径
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		In: int tempType - 模板类型（默认为0），0-offset template;1-gain template;2-defect template;3-defect map template;4-gainb template;
		Out: 无
* 返 回 值：char*
		NULL  - 失败
		char* - 返回绝对路径
* 备    注:
*********************************************************/
HBI_FPD_DLL char* __stdcall HBI_GetCurrentCorrectFilePath(void *handle, int tempType=0);

/*********************************************************
* No:No118
* Function name: HBI_DetectorPrimaryAuthor
* Function description: 略
* Parameter Description:
	In: void * handle - Handle (unsigned pointer)
		Int pcode -
	Out: None
* Return value: int
	0- Success
	Non 0- Failed
* Note:
* 编    号:No118
* 函 数 名: HBI_DetectorPrimaryAuthor
* 功能描述: 略
* 参数说明:
	In: void * handle - 句柄(无符号指针)
		Int pcode - 
	Out: None
* Return value: int
	0- Success
	Non 0- Failed
* Note: add by mhyang 20211228
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_DetectorPrimaryAuthor(void *handle, char *pcode);

/*********************************************************
* No:No119
* Function name: HBI_DetectorSecondaryAuthor
* Function description: 略
* Parameter Description:
	In: void * handle - Handle (unsigned pointer)
		Int pcode -
	Out: None
* Return value: int
	0- Success
	Non 0- Failed
* Note:
* 编    号:No119
* 函 数 名: HBI_DetectorSecondaryAuthor
* 功能描述: 略
* 参数说明:
	In: void * handle - 句柄(无符号指针)
		Int pcode -
	Out: None
* Return value: int
	0- Success
	Non 0- Failed
* Note: add by mhyang 20220211
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_DetectorSecondaryAuthor(void *handle, char *pcode);

/*********************************************************
* No: No120
* Function Name: HBI_DownloadImage
* Function Description: Download image (wireless detector)
* Parameter Description:
		In: void *handle - unsigned handle pointer
			int beginnumber - Download start image sequence number
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No120
* 函 数 名: HBI_DownloadImage
* 功能描述: 下载图像(无线平板)
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int beginnumber - 下载开始图像序号
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_DownloadImage(void *handle, int beginnumber = 1);

/*********************************************************
* No: No121
* Function Name: HBI_SetSleepState
* Function Description: Set the detector to sleep (wireless detector)
* Parameter Description:
		In: void *handle - unsigned handle pointer
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No121
* 函 数 名: HBI_SetSleepState
* 功能描述: 设置平板为休眠状态(无线平板)
* 参数说明:
		In: void *handle - 句柄(无符号指针)
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetSleepState(void *handle);

/*********************************************************
* No: No122
* Function Name: HBI_WakeUpFpd
* Function Description: Wake up tablet from sleep (wireless detector)
* Parameter Description:
		In: void *handle - unsigned handle pointer
* Return Value：int
* 		   0: Success
*		   Non-0: Failure
* Comment:
* 编    号: No122
* 函 数 名: HBI_WakeUpFpd
* 功能描述: 从休眠状态唤醒平板(无线平板)
* 参数说明:
		In: void *handle - 句柄(无符号指针)
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_WakeUpFpd(void *handle);

/*********************************************************
* No: No123
* Function Name: HBI_EnableEmmcStorage
* Function Description: Emmc storage image enable (wireless detector)
* Parameter Description:
		In: void *handle - unsigned handle pointer
			int nenable - 1: Save, others: not save
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No123
* 函 数 名: HBI_EnableEmmcStorage
* 功能描述: Emmc存储图像使能(无线平板)
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int nenable - 1:存图,其他:不存图
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_EnableEmmcStorage(void *handle, int nenable = 0);

/*********************************************************
* No: No124
* Function Name: HBI_ReflashBatteryStatusInfo
* Function Description: Refresh battery information (wireless detector)
* Parameter Description:
	In: void *handle - unsigned handle pointer
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment: Asynchronous function, feedback through callback function
* 编    号: No124
* 函 数 名: HBI_ReflashBatteryStatusInfo
* 功能描述: 刷新电池信息(无线平板)
* 参数说明:
	In: void *handle - 句柄(无符号指针)
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注: 异步函数，通过回调函数反馈
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_ReflashBatteryStatusInfo(void *handle);

/*********************************************************
* No: No125
* Function Name: HBI_GetBatteryStatusInfo
* Function Description: Get battery information (wireless detector)
* Parameter Description:
	In: void *handle - unsigned handle pointer
		BATTERY_STATUS_INFO *pBatteryStatus - Battery status information, refer to <<HbiType.h>>

	Out:BATTERY_STATUS_INFO *pBatteryStatus
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:Synchronization function, parameters are synchronized and fed back to the upper computer through input parameters
* 编    号: No125
* 函 数 名: HBI_GetBatteryStatusInfo
* 功能描述: 获取电池信息(无线平板)
* 参数说明:
	In: void *handle - 句柄(无符号指针)
	    BATTERY_STATUS_INFO *pBatteryStatus - 电池状态信息 见<HbiType.h>

    Out:BATTERY_STATUS_INFO *pBatteryStatus
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:同步函数，参数同步后通过入参反馈给上位机
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetBatteryStatusInfo(void *handle, BATTERY_STATUS_INFO *pBatteryStatus);
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/*********************************************************
* No: No126
* Function Name: HBI_TriggerBinningAcqTime
* Function Description: 	// Work mode, binning, frame speed setting
* Parameter Description:
	In: 	void *handle 	// unsigned handle pointer
		int triggerMode	// Work mode
						// Static:
							// 01: Software trigger mode;
							// 02: Clear;
							// 03: High voltage generator(hvg) trigger mode;
							// 04: Auto exposure detection mode.
						// Dynamic:
							// 05: Hvg pulse sync in trigger mode;
							// 06: Fpd pulse sync out trigger Mode;
							// 07: Free run mode.
		unsigned char binning
							// 1: 1x1,
							// 2: 2x2,
							// 3: 3x3,
							// 4: 4x4，
		int time 			// Live acquisition span time, unit: ms
						// For example: 1000ms (1fps), 500ms (2fps), 250ms (4fps),125ms (8fps)
	Out: Null
* Return Value:
	int 			    // 0: Success
						// Non-0: Failure
* Comment:
* 编    号: No126
* 函 数 名: HBI_TriggerBinningAcqTime
* 功能描述: 设置触发模式、binning方式以及帧率（采集图像时间间隔）
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int triggerMode - 触发模式
				静态平板（每秒1帧）：1-软触发，2-Clear,3-高压触发，4-FreeAED
				动态平板（每秒2帧以上）：05-Dynamic:Hvg Sync Mode,06-Dynamic:Fpd Sync Mode,7-Continue Mode
			unsigned char binning - 1:1x1,2:2x2,3:3x3,4:4x4，其他不支持
			int time - 间隔时间,单位是毫秒ms,大于0
			int nfeedback - 1：设置成功后自动反馈固件参数，非1：反馈设置成功信号（见参数和信号见回调函数）
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_TriggerBinningAcqTime(void *handle, int triggerMode, unsigned char binning, int acqtime, int nfeedback = 0);

/*********************************************************
* No: No127
* Function Name: HBI_PgaBinningAcqTime
* Function Description: 	// PGA gain stage, binning, frame speed setting
* Parameter Description:
	In: 	void *handle 	// unsigned handle pointer
		int nPgaLevel	// PGA gain stage
									//[n]-Invalid,[1]-0.6pC,[2]-1.2pC,[3]-2.4pC,[4]-3.6pC,[5]-4.8pC,[6]-7.2pC,[7]-9.6pC
									//[8]-LFW (COMS),[9]-HFW (COMS),[10]-0.3pC,[11]-0.15pC,Default: 7.2 pC.
		unsigned char binning
							// 1: 1x1,
							// 2: 2x2,
							// 3: 3x3,
							// 4: 4x4，
		int acqtime 		// Live acquisition span time, unit: ms
		int nfeedback 	// 1: Automatic feedback config parameters if setting is successful
						// Non-1: Feedback setting success status
* Return Value:
	int 			// 0: Success
					// Non-0: Failure
* Comment:
* 编    号: No127
* 函 数 名: HBI_PgaBinningAcqTime
* 功能描述: 设置PGA档位、binning方式以及帧率（采集图像时间间隔）
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int nPgaLevel - PGA 档位，[n]-Invalid,[1]-0.6pC,[2]-1.2pC,[3]-2.4pC,[4]-3.6pC,[5]-4.8pC,[6]-7.2pC,默认7.2pC,[7]-9.6pC
												  [8]-LFW (COMS),[9]-HFW (COMS),[10]-0.3pC,[11]-0.15pC
			unsigned char binning - 1:1x1,2:2x2,3:3x3,4:4x4，其他不支持
			int acqtime - 间隔时间,单位是毫秒ms,大于0
			int nfeedback - 1：设置成功后自动反馈固件参数，非1：反馈设置成功信号（见参数和信号见回调函数）
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_PgaBinningAcqTime(void *handle, int nPgaLevel, unsigned char binning, int acqtime, int nfeedback = 0);

/*********************************************************
* No: No128
* Function Name: HBI_TriggerPgaBinningAcqTime
* Function Description:	// trigger mode, PGA gain stage, binning type, frame speed setting
* Parameter Description:
	In: 	void *handle 	// unsigned handle pointer
		int triggerMode 	// Work mode
						// Static:
							// 01: Software trigger mode;
							// 02: Clear;
							// 03: High voltage generator(hvg) trigger mode;
							// 04: Auto exposure detection mode.
						// Dynamic:
							// 05: Hvg pulse sync in trigger mode;
							// 06: Fpd pulse sync out trigger Mode;
							// 07: Free run mode.
		int nPgaLevel	// PGA gain stage
									//[n]-Invalid,[1]-0.6pC,[2]-1.2pC,[3]-2.4pC,[4]-3.6pC,[5]-4.8pC,[6]-7.2pC,[7]-9.6pC
									//[8]-LFW (COMS),[9]-HFW (COMS),[10]-0.3pC,[11]-0.15pC,Default: 7.2 pC.
		unsigned char binning
							// 1: 1x1,
							// 2: 2x2,
							// 3: 3x3,
							// 4: 4x4，
		int acqtime		// Live acquisition span time, unit: ms
		int nfeedback 	// 1: Automatic feedback config parameters if setting is successful
						// Non-1: Feedback setting success status
* Return Value:
	int 			    // 0: Success
						// Non-0: Failure
* Comment:
* 编    号: No128
* 函 数 名: HBI_TriggerPgaBinningAcqTime
* 功能描述: 设置触发模式、PGA档位、binning方式以及帧率（采集图像时间间隔）
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int triggerMode - 触发模式
				静态平板（每秒1帧）：1-软触发，2-Clear,3-高压触发，4-FreeAED
				动态平板（每秒2帧以上）：05-Dynamic:Hvg Sync Mode,06-Dynamic:Fpd Sync Mode,7-Continue Mode
			int nPgaLevel - PGA 档位，[n]-Invalid,[1]-0.6pC,[2]-1.2pC,[3]-2.4pC,[4]-3.6pC,[5]-4.8pC,[6]-7.2pC,默认7.2pC,[7]-9.6pC
									  [8]-LFW (COMS),[9]-HFW (COMS),[10]-0.3pC,[11]-0.15pC
			unsigned char binning - 1:1x1,2:2x2,3:3x3,4:4x4，其他不支持
			int acqtime - 间隔时间,单位是毫秒ms,大于0
			int nfeedback - 1：设置成功后自动反馈固件参数，非1：反馈设置成功信号（见参数和信号见回调函数）
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_TriggerPgaBinningAcqTime(void *handle, int triggerMode, int nPgaLevel, unsigned char binning, int acqtime, int nfeedback = 0);

/*********************************************************
* No: No129
* Function Name: HBI_PgaAcqTime
* Function Description: 	// PGA gain stage, frame speed setting
* Parameter Description:
	In:	void *handle 	// unsigned handle pointer
		int nPgaLevel	// PGA gain stage
									//[n]-Invalid,[1]-0.6pC,[2]-1.2pC,[3]-2.4pC,[4]-3.6pC,[5]-4.8pC,[6]-7.2pC,[7]-9.6pC
									//[8]-LFW (COMS),[9]-HFW (COMS),[10]-0.3pC,[11]-0.15pC,Default: 7.2 pC.
		int acqtime		// Live acquisition span time, unit: ms
		int nfeedback 	// 1: Automatic feedback config parameters if setting is successful
						// Non-1: Feedback setting success status
* Return Value: int
			// 0: Success
			// Non-0: Failure
* Comment:
* 编    号: No129
* 函 数 名: HBI_PgaAcqTime
* 功能描述: 设置PGA档位、及帧率（采集图像时间间隔）
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int nPgaLevel - PGA 档位，[n]-Invalid,[1]-0.6pC,[2]-1.2pC,[3]-2.4pC,[4]-3.6pC,[5]-4.8pC,[6]-7.2pC,默认7.2pC,[7]-9.6pC
			 		                  [8]-LFW (COMS),[9]-HFW (COMS),[10]-0.3pC,[11]-0.15pC
			int acqtime - 间隔时间,单位是毫秒ms,大于0
			int nfeedback - 1：设置成功后自动反馈固件参数，非1：反馈设置成功信号（见参数和信号见回调函数）
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_PgaAcqTime(void *handle, int nPgaLevel, int acqtime, int nfeedback = 0);

/*********************************************************
* No: No130
* Function Name: HBI_SwitchNetTypeWlanMode
* Function Description: Switch wireless 2.4G/5G network and WLAN AP/Client Mode (wireless flat panel detector)
* Parameter Description:
		In: void *handle - unsigned handle pointer
			int nNetType - Wireless network type，1-2.4G,2-5G,Other - invalid
			int nWlanMode- Wireless mode，1-AP Mode,2-Client Mode,3-Wired Mode,Other - invalid
* Return Value：int
	   // 0: Success
	   // Non-0: Failure
* Comment:
* 编    号: No130
* 函 数 名: HBI_SwitchNetTypeWlanMode
* 功能描述: 切换无线2.4G/5G网络和WLAN AP/Client Mode(无线平板)
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int nNetType - 无线网络类型，1-2.4G,2-5G,其他-无效
			int nWlanMode- 无线模式，1-AP Mode,2-Client Mode,3-Wired Mode,其他-无效
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SwitchNetTypeWlanMode(void *handle, int nNetType, int nWlanMode);

/*********************************************************
* No: No131
* Function Name: HBI_ReadPcieVersion
* Function Description: Get PCIe card version number
* Parameter Description:
		In: void *handle - unsigned handle pointer
		In/Out: char *szVersion,At least 16 bytes
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No131
* 函 数 名: HBI_ReadPcieVersion
* 功能描述: 获取PCIe卡版本号
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		In/Out: char *szVersion,至少为16个字节
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_ReadPcieVersion(void *handle, char *szVersion);

/*********************************************************
* No: No132
* Function Name: HBI_GeneratePreviewTemplate
* Function Description: Quickly generate Preview image template
* Parameter Description:
		In: void *handle - unsigned handle pointer
				EnumIMAGE_ACQ_CMD _mode - Template Type
				OFFSET_TEMPLATE_TYPE      preview-offset Template
				GAIN_TEMPLATE_TYPE        preview-gain Template
				DEFECT_TEMPLATE_GROUP1,   preview-defect Template
		Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No132
* 函 数 名: HBI_GeneratePreviewTemplate
* 功能描述: 快速生成Preview图像模板
* 参数说明:
		In: void *handle - 句柄(无符号指针)
				EnumIMAGE_ACQ_CMD _mode - 生成模板类型
				OFFSET_TEMPLATE_TYPE      生成preview-offset模板
				GAIN_TEMPLATE_TYPE        生成preview-gain模板
				DEFECT_TEMPLATE_GROUP1,   生成preview-defect模板
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GeneratePreviewTemplate(void *handle, EnumIMAGE_ACQ_CMD _mode);

/*********************************************************
* No: No133
* Function Name: HBI_SetImageRotateAndFlip
* Function Description: Rotate 90 ° or enable mirroring (SDK processing, applicable to low frame rate or small resolution tablets)
* Parameter Description:
		In: void *handle  - unsigned handle pointer
		In: int nRotate - 0-Do nothing;1-Left Rotate 90°;02-"Right Rotate 90°;
		In: int nFlip - 0-Do nothing;1-Horizontal mirror;2-Mirror vertically;
Out: Null
* Return Value：int
			   0: Success
			   Non-0: Failure
* Comment:image processing，By default, rotate before mirro
* 编    号: No133
* 函 数 名: HBI_SetImageRotateAndFlip（新增）
* 功能描述: 旋转90°或镜像使能，默认先旋转后镜像
* 参数说明:
		In: void *handle  - 句柄(无符号指针)
		In: int nRotate - 0-Do nothing;1-Right Rotate 90°;02-"Left Rotate 90°;
		In: int nFlip - 0-Do nothing;1-Vertically mirror;2-Horizontal Mirror vertically;
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:默认先旋转后镜像 ADD BY MHYANG 20220518
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetImageRotateAndFlip(void *handle, int nRotate, int nFlip);

/*********************************************************
* No: No134
* Function Name: HBI_Switch2Bit8Image
* Function Description: Switch or cancel Bit8 image, cancel automatic switch to Bit16 image (normal image)
* Parameter Description:
		In: void *handle - unsigned handle pointer
			int nbit8 - Bit8 Image switch control parameters, 0-Bit16 image, 1-Bit8 image, other - Bit16 image
			int nwindowwidth - window width，0~65535（Bit8 Image validation）
			int nwindowleveling - window level~65535（Bit8 Image validation）
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No134
* 函 数 名: HBI_Switch2Bit8Image
* 功能描述: 切换或取消Bit8图像，取消自动切换到Bit16图像（正常图像）
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int nbit8 - Bit8 图像开关控制参数，0-Bit16 图像,1-Bit8 图像,其他-Bit16 图像
			int nwindowwidth - 窗宽，0~65535（Bit8 图像生效）
			int nwindowleveling - 窗位，0~65535（Bit8 图像生效）
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_Switch2Bit8Image(void *handle, int nbit8, int nwindowwidth, int nwindowleveling);

/*********************************************************
* No: No135
* Function Name: HBI_OpenTemplateWizard
* Function Description: Open the correction template generation tool
* Parameter Description:
		In: void *handle - unsigned handle pointer
* Return Value：int
			   0: Success
			   Non-0: Failure
* Comment:
* 编    号: No135
* 函 数 名: HBI_OpenTemplateWizard
* 功能描述: 打开模板向导窗口(生成模板、设置主要参数)
* 参数说明:
		In: void *handle - 句柄(无符号指针)
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_OpenTemplateWizard(void *handle);

/*********************************************************
* No: No136
* Function Name: HBI_ReadFromUserDdr
* Function Description: Read user ddr information (customized)
* Parameter Description:
	In: void *handle - unsigned handle pointer
		unsigned char *szRcvbuff - Receive Buffer,size=1024
		int/out *nlen - Buffer data length pointer（<=1024），The initial value is szRcvbuff size
	Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No136
* 函 数 名: HBI_ReadFromUserDdr
* 功能描述: 读取用户ddr信息（用户定制）
* 参数说明:
	In: void *handle - 句柄(无符号指针)
		unsigned char *szRcvbuff - 接收缓冲区,size=1024
		int/out *nlen - 缓冲区数据长度指针（<=1024），初始值为szRcvbuff大小
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_ReadFromUserDdr(void* handle, unsigned char* szRcvBuff, int *nlen);

/*********************************************************
* No: No137
* Function Name: HBI_WriteToUserDdr
* Function Description: Write content to user ddr (user-defined)
* Parameter Description:
	In: void *handle - unsigned handle pointer
		unsigned char *szSndBuff - Write Buffer
		int nlen - Buffer data length（<=1024）
	Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No137
* 函 数 名: HBI_WriteToUserDdr
* 功能描述: 向用户ddr写入内容（用户定制）
* 参数说明:
	In: void *handle - 句柄(无符号指针)
		unsigned char *szSndBuff - 写入缓冲区
		int nlen - 缓冲区数据长度（<=1024）
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_WriteToUserDdr(void* handle, unsigned char* szSndBuff, int nlen);

/*********************************************************
* No: No138
* Function Name: HBI_SetRoiMode
* Function Description: -silicon ROI region setting function.
*       Currentlyonly 1613-125um、3030-140um、4343-100um、2530-100um are supported.
* Parameter Description:
	In: void *handle - unsigned handle pointer
		int nMode - ROI mode
		for example,1613-125um：0-Normal Image，w*h(1248*1024), Rect(0,0,1247,1023);
						 1-w*h(32*1024), Rect(608,0,639,1023);
						 2-w*h(1248*32), Rect(0,496,1247,527);
						 3-w*h(1248*64), Rect(0,480,1247,543);
						 4-w*h(1248*128),Rect(0,448,1247,575);
						 Others-Normal Image。
			 3030-140um：0-Normal Image，w*h(2048*2048), Rect(0,0,2047,2047);
						 1-w*h(1024*2048),Rect(512,0,1535,2047);
						 Others-Normal Image。
			 2530-100um：0-Normal Image，w*h(2432*3072), Rect(0,0,2431,3071);
						 1-w*h(1808*1800),Rect(316,636,2115,2435);
						 2-w*h(1312*1300),Rect(566,886,1865,2185);
						 Others-Normal Image。
		int nfeedback 	// 1: Automatic feedback config parameters if setting is successful
								// Non-1: Feedback setting success status
	Out: Null
* Return Value：int
			   0: Success
			   Non-0: Failure
* Comment:only 1613-125um、3030-140um、4343-100um、2530-100um are supported。
* 编    号: No138
* 函 数 名: HBI_SetRoiMode（新增）
* 功能描述: 非晶硅ROI区域设置函数
*           当前仅支持平板类型包括1613-125um、3030-140um、4343-100um、2530-100um。
* 参数说明:
	In: void *handle - 句柄(无符号指针)
		int nMode - Roi模式
		其中,1613-125um：0-Normal Image，w*h(1248*1024), Rect(0,0,1247,1023);
		                 1-w*h(32*1024), Rect(608,0,639,1023);
						 2-w*h(1248*32), Rect(0,496,1247,527);
						 3-w*h(1248*64), Rect(0,480,1247,543);
						 4-w*h(1248*128),Rect(0,448,1247,575);
						 其他-Normal Image。
		     3030-140um：0-Normal Image，w*h(2048*2048), Rect(0,0,2047,2047);
			             1-w*h(1024*2048),Rect(512,0,1535,2047);
						 其他-Normal Image。
			 2530-100um：0-Normal Image，w*h(2432*3072), Rect(0,0,2431,3071);
						 1-w*h(1808*1800),Rect(316,636,2115,2435);
						 2-w*h(1312*1300),Rect(566,886,1865,2185);
						 其他-Normal Image。
		int nfeedback - 1：设置成功后自动反馈固件参数，非1：反馈设置成功信号（见参数和信号见回调函数）,默认为1
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:当前仅支持平板类型包括1613-125um、3030-140um、2530-100um。
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetRoiMode(void* handle, int nMode, int nfeedback=1);

/*********************************************************
* No: No139
* Function Name: HBI_SetZoomModeRect
* Function Description: The ZOOM Mode region setting function for CMOS tablets
*           currently supports only CMOS - 1412-50um and CMOS - 0606-50um ,1917-120umtablet types.
* Parameter Description:
	In: void *handle - unsigned handle pointer
		CMOS_ZOOM_RECT stZoomRect - Zoom region
			The region is represented by left, top, right, and bottom.
			for example, when left, top, right and bottom are illegal values, the image will display the full image by default,
			1>When left=top=right=bottom=0, the whole picture is displayed;
			2>When eft<0 or left>width, the whole picture will be displayed, and others are the same.
		int nfeedback // 1: Automatic feedback config parameters if setting is successful
					// Non-1: Feedback setting success status
	Out: Null
* Return Value：int
			   0: Success
			   Non-0: Failure
* Comment: currently supports only CMOS - 1412-50um and CMOS - 0606-50um 1917-120um tablet types.
* 编    号: No139
* 函 数 名: HBI_SetZoomModeRect（新增）
* 功能描述: ZOOM Mode区域设置函数
*           当前仅支持平板类型包括CMOS-1412-50um、CMOS-0606-50um,1917-120um。
* 参数说明:
	In: void *handle - 句柄(无符号指针)
		CMOS_ZOOM_RECT stZoomRect - Zoom区域
			其中，区域用left,top,right,bottom来表示。
			例如：当left,top,right,bottom为非法数值时，图像默认显示全图，
			1>left=top=right=bottom=0时，显示全图；
			2>left<0或left>width时，显示全图，其他雷同。
		int nfeedback - 1：设置成功后自动反馈固件参数，非1：反馈设置成功信号（见参数和信号见回调函数）,默认为1
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:当前仅支持平板类型包括CMOS-1412-50um、CMOS-0606-50um，1917-120um,4343-100um。
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetZoomModeRect(void* handle, CMOS_ZOOM_RECT stZoomRect, int nfeedback = 1);

/*********************************************************
* No: No140
* Function Name: HBI_SetCorrectFileDirectory
* Function Description: User set correction file home directory
*           If this interface is not called, the calibration file defaults to the DLL directory
*           Setting failed. The default is the current path
* Parameter Description:
	In: void *handle - unsigned handle pointer
		char *pUserDir - User defined correction file directory，for example,
”E:\\***\\***\\
”	Out: Null
* Return Value：int
			   0: Success
			   Non-0: Failure
* Comment:
* 编    号: No140
* 函 数 名: HBI_SetCorrectFileDirectory
* 功能描述: 用户设置校正文件主目录
*           如果未调用该接口，校正文件默认为DLL目录
*           设置失败，默认为当前路径
* 参数说明:
	In: void *handle - 句柄(无符号指针)
		char *pUserDir - 用户自定义校正文件目录，例如:E:\\***\\***\\
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetCorrectFileDirectory(void* handle, char *pUserDir);

/*********************************************************
* No: No141
* Function Name: HBI_GetCorrectFileDirectory
* Function Description: Obtain the home directory of user correction files
* Parameter Description:
	In: void *handle - unsigned handle pointer
	Out: Null
* Return Value：int
			   0: Success
			   Non-0: Failure
* Comment:
* 编    号: No141
* 函 数 名: HBI_GetCorrectFileDirectory
* 功能描述: 获取用户校正文件主目录
* 参数说明:
	In: void *handle - 句柄(无符号指针)
	Out: 无
* 返 回 值：char *
	NULL   - 句柄异常或者不存在
	非NULL - 主目录
* 备    注:
*********************************************************/
HBI_FPD_DLL char* __stdcall HBI_GetCorrectFileDirectory(void* handle);

/*********************************************************
* No: No142
* Function Name: HBI_RestoreCorrectDefaultDirectory
* Function Description: Restore correction file default path
* Parameter Description:
	In: void *handle - unsigned handle pointer
	Out: Null
* Return Value：int
		   0: Success
			   Non-0: Failure
* Comment:
* 编    号: No142
* 函 数 名: HBI_RestoreCorrectDefaultDirectory
* 功能描述: 恢复校正文件默认路径
* 参数说明:
	In: void *handle - 句柄(无符号指针)
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_RestoreCorrectDefaultDirectory(void* handle);

/*********************************************************
* No: No143
* Function Name: HBI_SSGCTRegCallbackFunc（newly increased）
* Function Description: Register single-step template callback handler
* Parameter Description:
	In: void *handle - unsigned handle pointer
		SSGCT_CALLBACK_FUNC callbackfun - Registering Callbacks
		void *pContext- Object pointer. Generally, the upper computer software transfers the object pointer of a module to the SDK, and the callback function feeds back to the upper computer so that static or global functions can access other objects. It can also be null pointer.
	Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:Single Step Generate Template-SSGT
* 编    号: No143
* 函 数 名: HBI_SSGCTRegCallbackFunc（新增）
* 功能描述: 注册单步生成模板回调处理函数
* 参数说明:
	In: void *handle - 句柄(无符号指针)
		SSGCT_CALLBACK_FUNC callbackfun - 注册回调函数
		void *pContext- 对象指针，一般上位机软件将某个模块对象指针传入SDK，回调函数反馈给上位机，以便静态或全局函数访问其他对象，亦可为空指针。
	Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:Single Step Generate Template-SSGT
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SSGCTRegCallbackFunc(void* handle, SSGCT_CALLBACK_FUNC callbackfun, void* pObject);

/*********************************************************
* No: No144
* Function Name: HBI_SSCGTEnableMode（newly increased）
* Function Description: The single-frame acquisition preparation command activates the single-step image acquisition generation template mode or restores the normal image acquisition mode
* Parameter Description:
	In: void *handle - unsigned handle pointer
	   Int nMode - (1): generate offset template;
			(2) : Generate gain template;
			(3) : generate a defect template;
			(4) : generate gain and defect templates;
			(0 or other): Exit the single-step template generation mode and return to the normal drawing mode;
	   Int nGroups - Number of groups
	   Int nPreGroup - Number of images captured per group
	Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:Single Step Generate Correction Template-SSGT
* 编    号: No144
* 函 数 名: HBI_SSCGTEnableMode（新增）
* 功能描述: 激活单步采图生成模板模式或恢复正常采图模式
* 参数说明:
	In: void *handle - 句柄(无符号指针)
		int nMode - (1):生成offset模板;
					(2):生成gain模板;
					(3):生成defect模板;
					(4):生成gain和defect模板;
					(0或其他):退出单步生成模板模式,恢复到正常采图模式;
		int nGroups   - 组数
		int nPreGroup - 每组采集图像数目
	Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:Single Step Generate Correction Template-SSGT
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SSGCTEnableMode(void* handle, int nMode, int nGroups=3, int nPreGroup=3);

/*********************************************************
* No: No145
* Function Name: HBI_SSGCTPrepare（newly increased）
* Function Description:  Single frame acquisition preparation instruction
* Parameter Description:
	In : void* handle - unsigned handle pointer
	Out : None
* Return Value: int
		   0: Success
		   Non-0: Failure
* Comment: Single Step Generate Correction Template-SSGT
* 编    号: No145
* 函 数 名: HBI_SSGCTPrepare（新增）
* 功能描述:  单帧采集准备指令
* 参数说明: 
	In : void* handle - 句柄(无符号指针)
	Out : 无
* 返 回 值: int
	0 - 成功
	非0 - 失败
* 备    注: Single Step Generate Correction Template-SSGT
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SSGCTPrepare(void* handle);

/*********************************************************
* No: No146
* Function Name: HBI_SSGCTSingleShot（newly increased）
* Function Description: Single frame acquisition
* Parameter Description:
		In: void *handle - unsigned handle pointer
		Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment: Single Step Generate Correction Template-SSGT
* 编    号: No146
* 函 数 名: HBI_SSGCTSingleShot（新增）
* 功能描述: 单帧采集
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注: Single Step Generate Correction Template-SSGT
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SSGCTSingleShot(void* handle);

/*********************************************************
* No: No147
* Function Name: HBI_SSGCTGenerateTemplate（newly increased）
* Function Description: Generate Template
* Parameter Description:
		In: void *handle - unsigned handle pointer
		Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment: Single Step Generate Correction Template-SSGT
* 编    号: No147
* 函 数 名: HBI_SSGCTGenerateTemplate（新增）
* 功能描述: 生成模板
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注: Single Step Generate Correction Template-SSGT
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SSGCTGenerateTemplate(void* handle);

/*********************************************************
* No: No148
* Function Name: HBI_SSGCTDownloadTemplate（newly increased）
* Function Description: Download calibration template
* Parameter Description:
		In: void *handle - unsigned handle pointer
			int infiletype -Download file types: 1-offset template, 2-gain template, 3-defect template, others - not supported
		Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment: Single Step Generate Correction Template-SSGT
* 编    号: No148
* 函 数 名: HBI_SSGCTDownloadTemplate（新增）
* 功能描述: 下载校正模板
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		    int infiletype - 下载文件类型1-offset模板，2-gain模板，3-defect模板，其他-不支持
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注: Single Step Generate Correction Template-SSGT
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SSGCTDownloadTemplate(void* handle, int infiletype);

/*********************************************************
* No: No149
* Function Name: HBI_UpdateWirelessNetwork（newly increased）
* Function Description: Update wireless network configuration
* Parameter Description:
		In: void *handle    - unsigned handle pointer
			EtherInfo ether - Network configuration parameter structure, refer to <<HbiType.h>>
		Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment: wireless FPD network config
* 编    号: No149
* 函 数 名: HBI_UpdateWirelessNetwork（新增）
* 功能描述: 更新无线网络配置
* 参数说明:
		In: void *handle    - 句柄(无符号指针)
			EtherInfo ether - 网络配置参数结构体，见《HbiType.h》
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注: wireless FPD network config
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_UpdateWirelessNetwork(void* handle, EtherInfo ether);

/*********************************************************
* No: No150
* Function Name: HBI_GetWirelessNetWork（newly increased）
* Function Description: Get wireless network configuration (asynchronous message, callback function feedback)
* Parameter Description:
		In: void *handle - unsigned handle pointer
		Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment: wireless FPD network config
* 编    号: No150
* 函 数 名: HBI_GetWirelessNetWork（新增）
* 功能描述: 获取无线网络配置（异步消息，回调函数反馈）
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注: wireless FPD network config
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetWirelessNetwork(void* handle);

/*********************************************************
* No: No151
* Function Name: HBI_UpdateWirelessSecurityKey（newly increased）
* Function Description: Modify wireless network security key
* Parameter Description:
		In: void *handle - unsigned handle pointer
		int wirelessmode - Wireless network mode，1-AP mode，2-Client mode
		char *pkey       - Security key
		int nkeylen      - Security key length (10 significant characters)
		Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment: wireless FPD network config
* 编    号: No151
* 函 数 名: HBI_UpdateWirelessSecurityKey（新增）
* 功能描述: 修改无线网络安全密钥
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		int wirelessmode - 无线网络模式，1-AP模式，2-Client模式
		char *pkey       - 安全密钥
		int nkeylen      - 安全密钥长度（有效位10位字符）
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注: wireless FPD network config
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_UpdateWirelessSecurityKey(void* handle, int wirelessmode, char *pkey, int nkeylen);

/*********************************************************
* No: No152
* Function Name: HBI_UpdateSaveOption
* Function Description: Update Save Image Options
* Parameter Description:
		In: void *handle - unsigned handle pointer
		SAVE_IMAGE saveimge - Structure parameters
			unsigned char m_bySaveType;        // 0-show image only,1-save image only,2-show and save image;
			unsigned char m_bySaveFileType;     // 0-image,1-video;
			unsigned short m_bySaveNumber;     // Number of images included in the video
	   Out: Null
* Return Value：int
		0: Success
		Non-0: Failure
* Comment:
* 编    号: No152
* 函 数 名: HBI_UpdateSaveOption（新增）
* 功能描述: 更新保存图像选项
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		SAVE_IMAGE saveimge - 结构体参数
			unsigned char  m_bySaveType;        //0-show image only,1-save image only,2-show and save image;
			unsigned char  m_bySaveFileType;    //0-image,1-video;
			unsigned short m_bySaveNumber;      //Number of images included in the video,》0
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注: 
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_UpdateSaveOption(void* handle, SAVE_IMAGE saveimge);

/*********************************************************
* No: No153
* Function Name: HBI_SetPacketIntervalTime
* Function Description: Set Package Interval
* Parameter Description:
		In: void *handle - unsigned handle pointer
			int *in_itime - TIME，UNIT:um
Out: Null
* Return Value：int
			0: Success
			Non-0: Failure
* Comment:
* 编    号: No153
* 函 数 名: HBI_SetPacketIntervalTime（新增）
* 功能描述: 设置包间隔时间
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int *in_itime - 时间，单位:um
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetPacketIntervalTime(void* handle, int in_itime);

/*********************************************************
* No: No154
* Function Name: HBI_GetPacketIntervalTime（newly increased）
* Function Description: Get Package Interval
* Parameter Description:
		In: void *handle - unsigned handle pointer
		out:int *out_time - Time in microseconds
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No154
* 函 数 名: HBI_GetPacketIntervalTime（新增）
* 功能描述: 获取包间隔时间
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		out:int *out_time - 时间,单位是毫秒us
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetPacketIntervalTime(void* handle, int* out_time);

/*********************************************************
* No: No155
* Function Name: HBI_SetDummyParameter（newly increased）
* Function Description: Update Dummy parameters (parameters required for Dummy correction)
* Parameter Description:
		In: void *handle - unsigned handle pointer
			int nDummyStartCol - The region start column of Dummy
			int nDummyEndCol   - The Region End Column of Dummy
			int nDummyConstant - The constant of Dummy
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No155
* 函 数 名: HBI_SetDummyParameter（新增）
* 功能描述: 更新Dummy参数(Dummy校正所需参数)
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		    int nDummyStartCol - Dummy区域起始列
			int nDummyEndCol   - Dummy区域终止列
			int nDummyConstant - Dummy常量
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetDummyParameter(void* handle, int nDummyStartCol, int nDummyEndCol, int nDummyConstant);

/*********************************************************
* No: No156
* Function Name: HBI_AutoWindowWidthWindowLevel
* Function Description: The automatic window width and window level algorithm converts 16bits data into 8-bit unsigned data through window width and window level processing, and users can directly display 8-bit data.
* Parameter Description:
		In:     void *handle - unsigned handle pointer
		In/Out: unsigned char *imgbuff，Source data (16 bit unsigned small end data), destination data (8-bit unsigned data)
		In/Out: int *nbufflen, Buffer data length (halve the data size after conversion)
		In:     int nwidth   - Image data width
		In:     int nheight  - Image data high
Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No156
* 函 数 名: HBI_AutoWindowWidthWindowLevel（新增）
* 功能描述: 自动窗宽窗位算法将16位数据进行窗宽窗位处理转换为8位无符号数据，用户可以直接显示8位数据。
*           8位数据保存在imgbuff缓冲区内，数据长度减半保存在*nbufflen指针地址
* 参数说明:
		In:     void *handle - 句柄(无符号指针)
		In/Out: unsigned char *imgbuff，源数据（16位无符号小端数据），目的数据（8位无符号数据）
		In/Out: int *nbufflen，缓冲区数据长度（转换后数据大小减半）
		In:     int nwidth  - 图像数据宽
		In:     int nheight - 图像数据高
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_AutoWindowWidthWindowLevel(void* handle, unsigned char* imgbuff, int* nbufflen, int nwidth, int nheight);

/*********************************************************
* No: No157
* Function Name: HBI_UnRegProgressCallBack
* Function Description: Unregister progress registration callback function
* Parameter Description: Process firmware upgrade feedback
		In: void *handle - unsigned handle pointer
		Out: Null
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No157
* 函 数 名: HBI_UnRegProgressCallBack
* 功能描述: 注销进度注册回调函数
* 参数说明: 处理固件升级反馈信息
		In: void *handle - 句柄(无符号指针)
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_UnRegProgressCallBack(void* handle);

/*********************************************************
* No: No158
* Function Name: HBI_SetUdpMtuPacketType（newly increased）
* Function Description: Set the packet type of UDP maximum transmission unit
* Parameter Description:
		In: void *handle  - unsigned handle pointer
		In: int npacketType - 0-udp packet;1-udp packet jumbo;others-"udp packet°;
* Return Value：int
		   0: Success
		   Non-0: Failure
* Comment:
* 编    号: No158
* 函 数 名: HBI_SetUdpMtuPacketType（新增）
* 功能描述: 设置UDP最大传输单元的数据包类型
* 参数说明:
		In: void *handle  - 句柄(无符号指针)
		In: int npacketType - 0-udp packet;1-udp packet jumbo;others-"udp packet°;
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetUdpMtuPacketType(void* handle, int npacketType);

/*********************************************************
* No: No159
* Function Name: HBI_Rotate90AndFlip（newly increased）
* Function Description: Image rotation and image processing (only applicable to 16-bit unsigned image data, which is rotated first and then mirrored by default)
* Parameter Description:
		In:     void *handle - unsigned handle pointer
		In/Out: unsigned char *imgbuff - Pointer type input and output parameters, source data (16-bit unsigned small-end data), destination data (16-bit unsigned small-end data)
		In:     int nbufflen - Buffer data length
		In/Out: int *nwidth  - Pointer type input and output parameters, pointer address storage image data width
		In/Out: int *nheight - Pointer type input and output parameters, pointer address storage image data high
		In: int nClockwise - Rotate 90 °, 1 - right rotation, 2 - left rotation, other - no processing		In: int nVertical - Mirror processing, 1-vertical mirror, 2-horizontal mirror, other - no processing
* Return Value：int
		0: Success
		Non-0: Failure
* Comment:By default, rotate before mirror
* 编    号: No159
* 函 数 名: HBI_Rotate90AndFlip（新增）
* 功能描述: 图像旋转和镜像处理（仅适用于16位无符号类型图像数据，默认先旋转后镜像）     
* 参数说明:
		In:     void *handle - 句柄(无符号指针)
		In/Out: unsigned char *imgbuff - 指针类型入参和出参，源数据（16位无符号小端数据），目的数据（16位无符号小端数据）
		In:     int nbufflen - 缓冲区数据长度
		In/Out: int *nwidth  - 指针类型入参和出参，指针地址存放图像数据宽
		In/Out: int *nheight - 指针类型入参和出参，指针地址存放图像数据高
		In:     int nClockwise - 旋转90°，1-右旋转，2-左旋转，其他-不做处理
		In:     int nVertical - 镜像处理，1-垂直镜像，2-水平镜像，其他-不做处理
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:默认先旋转后镜像
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_Rotate90AndFlip(void *handle, unsigned char* imgbuff, int nbufflen, int* pnwidth, int* pnheight, int nClockwise, int nVertical);

/*********************************************************
* No: No160
* Function Name: HBI_ResetCorrectTemplate（newly increased）
* Function Description: Reset the calibration template file - rotate, mirror, reverse process and save, generally complete the process before downloading to the firmware
*          1、Find the generated calibration template file according to the current rotation and mirroring parameters (the template file has been rotated and mirrored);
*          2、Reverse process the calibration template file (mirror first and then reverse rotation)；
*       3、Automatically store the rotated calibration template file according to multi-template management rules；
* Parameter Description:
		In: void *handle  - unsigned handle pointer
		In: int nTemplateType - 0-offset template(16-bits);1-gain template(32-bits);2-defect template(16-bits);
* Return Value：int
		0: Success
		Non-0: Failure
* Comment:The purpose is to solve the problem that the firmware calibration calibration file is not synchronized. The firmware uses the generated calibration template file that has not been rotated and mirrored by the dynamic library
* 编    号: No160
* 函 数 名: HBI_ResetCorrectTemplate（新增）
* 功能描述: 重置校准模板文件-旋转、镜像逆向处理并保存，一般在下载到固件前完成处理
*          1、根据当前旋转和镜像参数查找已生成的校准模板文件（模板文件已被旋转和镜像处理）；
*          2、对该校准模板文件进行逆处理（先镜像后逆旋转）；
*          3、将旋转后的校准模板文件按照多模板管理规则自动存储；
* 参数说明:
		In: void *handle  - 句柄(无符号指针)
		In: int nTemplateType - 0-offset template(16-bits);1-gain template(32-bits);2-defect template(16-bits);
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:目的是为了解决固件校正校准文件不同步问题，固件使用的是未经过动态库旋转和镜像的生成的校准模板文件
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_ResetCorrectTemplate(void* handle, int nTemplateType);

/*********************************************************
* No: No161
* Function Name: HBI_IsExistsOfCurCorrectTemplate（newly increased）
* Function Description:Judge whether the current template file exists according to the template type
* Parameter Description:
		In: void *handle  - unsigned handle pointer
		In: int tempType -Template type (default is 0)，0-offset template;1-gain template;2-defect template;3-defect map template;4-gainb template;
* Return Value：int
		0: Success
		Non-0: Failure
* Comment:
* 编    号: No161
* 函 数 名: HBI_IsExistsOfCurCorrectTemplate（新增）
* 功能描述: 根据模板类型判断当前模板文件是否存在
* 参数说明:
		In: void *handle  - 句柄(无符号指针)
		In: int tempType - 模板类型（默认为0），0-offset template;1-gain template;2-defect template;3-defect map template;4-gainb template;
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_IsExistsOfCurCorrectTemplate(void* handle, int tempType=0);

/*********************************************************
* No: No162
* Function Name: HBI_GetCurZoomData（newly increased）
* Function Description: Get the current ZOOM area size data
* Parameter Description:
		In: void *handle  - unsigned handle pointer
		Out: unsigned int *pZoomLeft   - Zoom Rect Left
			 unsigned int *pZoomTop    - Zoom Rect Top
			 unsigned int *pZoomWidth  - Zoom Rect Width
			 unsigned int *pZoomHeight - Zoom Rect Height
* Return Value：int
			   0: Success
			   Non-0: Failure
* Comment:Rect data, where all are 0, indicating the full picture.
* 编    号: No162
* 函 数 名: HBI_GetCurZoomData（新增）
* 功能描述: 获取当前ZOOM区域大小数据
* 参数说明:
		In: void *handle  - 句柄(无符号指针)
		Out: unsigned int *pZoomLeft   - Zoom Rect Left
		     unsigned int *pZoomTop    - Zoom Rect Top
			 unsigned int *pZoomWidth  - Zoom Rect Width
			 unsigned int *pZoomHeight - Zoom Rect Height
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:Rect其中全部为0，表示全图。
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetCurZoomRect(void* handle, unsigned int *pZoomLeft, unsigned int *pZoomTop, unsigned int * pZoomWidth, unsigned int *pZoomHeight);

/*********************************************************
* No: No163
* Function Name: HBI_GetCurrentCorrectFileName（newly increased）
* Function Description: Get the current template file name
* Parameter Description:
		In: void *handle  - unsigned handle pointer
		In: int tempType -Template type (default is 0)，0-offset template;1-gain template;2-defect template;3-defect map template;4-gainb template;
* Return Value：char*
		NULL - Failure
		char* - Return file name
* Comment:
* 编    号: No163
* 函 数 名: HBI_GetCurrentCorrectFileName（新增）
* 功能描述: 获取当前模板文件名称
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		In: int tempType - 模板类型（默认为0），0-offset template;1-gain template;2-defect template;3-defect map template;4-gainb template;
* 返 回 值：char*
		NULL  - 失败
		char* - 返回文件名称
* 备    注:
*********************************************************/
HBI_FPD_DLL char* __stdcall HBI_GetCurrentCorrectFileName(void* handle, int tempType=0);

/*********************************************************
* No: No164
* Function Name: HBI_GetHbiUserCode
* Function Description: Get customer name information
* Parameter Description:
		In: void *handle  - unsigned handle pointer
		In/Out: char *szUserCode - A buff greater than 8 bytes is passed in, and the customer name information is backfilled after success
* Return Value：int
		0: Success
		Non-0: Failure
* Comment:
* 编    号: No164
* 函 数 名: HBI_GetHbiUserCode
* 功能描述: 获取客户名称信息
* 参数说明:
	In: void *handle - 句柄(无符号指针)
	In/Out: char *szUserCode - 传入>8个字节的buff，成功后回填客户名称信息
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetHbiUserCode(void* handle, char* szUserCode);

/*********************************************************
* No: No165
* Function Name: HBI_GetHbiProductCode
* Function Description: Get client product number
* Parameter Description:
		In: void *handle  - unsigned handle pointer
		In/Out: char *szProductCode -Get the customer name information, pass in a buff greater than 16 bytes, and backfill the client product number after success
* Return Value：int
		0: Success
		Non-0: Failure
* Comment:
* 编    号: No165
* 函 数 名: HBI_GetHbiProductCode
* 功能描述: 获取客户端产品编号
* 参数说明:
	In: void *handle - 句柄(无符号指针)
	In/Out: char *szProductCode -传入>16个字节的buff，成功后回填客户端产品编号
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetHbiProductCode(void* handle, char* szProductCode);

/*********************************************************
* No: No166
* Function name: HBI_ SetWorkModeOfComs (new)
* Function description: Set the working mode of COMS chip
* Parameter Description:
	In: void * handle - Handle (unsigned point)
		int nMode - The operating mode of the COMS chip
			[0] - Indicators DRS mode
			[1] - Indicators DDS mode
	Out: None
* Return value: int
		0- Success
		Non 0- Failed
* Remarks:
* 编    号: No166
* 函 数 名: HBI_SetWorkModeOfComs（新增）
* 功能描述: 设置COMS芯片的工作模式
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int nMode - COMS芯片的工作模式
			[0]-表示DRS模式
			[1]-表示DDS模式
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetWorkModeOfComs(void* handle, int nMode);

/*********************************************************
* No: No167
* Function name: HBI_GetWorkModeOfComs
* Function description: Obtain the working mode of COMS chip (newly added)
* Parameter Description:
	In: void * handle - Handle (unsigned pointer)
	Out: None
* Return value: int
		[0] - Indicates DRS mode
		[1] - Indicates DDS mode
*Remarks:
* 编    号: No167
* 函 数 名: HBI_GetWorkModeOfComs
* 功能描述: 获取COMS芯片的工作模式（新增）
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		Out: 无
* 返 回 值：int
			[0]-表示DRS模式
			[1]-表示DDS模式
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetWorkModeOfComs(void* handle);

/*********************************************************
* No: No168
* Function name: HBI_GetMtuAndReceiveBuffers
* Function description: Obtain network MTU size and receive buffer size (newly added)
* Parameter Description:
	In: void * handle - Handle (unsigned pointer)
	Out: None
* Return value: int
		0- Success
		Non 0- Failed
*Remarks:
* 编    号: No168
* 函 数 名: HBI_GetMtuAndReceiveBuffers
* 功能描述:获取网络MTU大小和接收缓冲区大小
* 参数说明:
	In: void * handle - 句柄(无符号指针)
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int _stdcall HBI_GetMtuAndReceiveBuffers(void* handle, unsigned long* mtusize, int* rcvsize);

// 连续采集重传使能 add by mhyang 20230605
/*********************************************************
* No: No169
* Function name: HBI_RetransmitPacketEnable
* Function description: Continuous acquisition retransmission enable (newly added)
* Parameter Description:
	In: void * handle - Handle (unsigned pointer)
		unsigned int nRetransmitted - data retransmission type,0:Re-Transmit Data,other:Prohibit data retransmission.
	Out: None
* Return value: int
		0- Success
		Non 0- Failed
* Remarks:
* No: No169
* 函 数 名: HBI_RetransmitPacketEnable
* 功能描述: 连续采集重传使能 add by mhyang 20230605
* 参数说明:
	In: void * handle - 句柄(无符号指针)
		unsigned int nRetransmitted - 数据重传开关,0:重传,其他:不发起重传
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_RetransmitPacketEnable(void* handle, unsigned int nRetransmitted);

/*********************************************************
* No: No170
* Function name: HBI_SetFpsOfTime（new）
* Function description: Set frame rate control time, support subtle
* Parameter Description:
		In: void *handle - Handle (unsigned pointer)
			unsigned int untimems - time interval,Unit is milliseconds，> 0ms
			unsigned short ustimeus - time interval,The unit is microseconds，>= 0ms.[0,999]
		Out: None
* Return value：int
		0   - Success
		Non 0 - Failed
* Remarks:eg 16 fps，ntimems=62ms， ntimeus=8um
* 编    号: No170
* 函 数 名: HBI_SetFpsOfTime（新增）
* 功能描述: 设置帧率控制时间
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			unsigned int untimems - 间隔时间,单位是毫秒ms，> 0ms
			unsigned short ustimeus - 间隔时间,单位是微秒us，>= 0ms.[0,999]
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:比如16帧，ntimems=62ms， ntimeus=8um
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetFpsOfTime(void* handle, unsigned int untimems, unsigned short ustimeus =0);

/*********************************************************
* No: No171
* Function name: HBI_SetFirmwareOvrelay（new）
* Function description: Set firmware overlay frame rate
* Parameter Description:
		In: void *handle - Handle (unsigned pointer)
			unsigned int noverlaynum -[0,255],0-Firmware not stacked，>0-Overlay frame rate
		Out: None
* Return value：int
		0   - Success
		Non 0 - Failed
* Remarks:
* 编    号: No171
* 函 数 名: HBI_SetFirmwareOverlay（新增）
* 功能描述: 设置固件叠加帧数
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			unsigned int noverlaynum -[0,255],其中0-固件不叠加，>0-叠加帧数
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetFirmwareOverlay(void* handle, unsigned int noverlaynum=0);

/*********************************************************
* No: No172
* Function name: HBI_RetransMissFrameEx（new）
* Function description: Full frame retransmission
* Parameter Description:
		In: void *handle - Handle (unsigned pointer)
			unsigned int frameid -Frame number to be retransmitted
		Out: None
* Return value：int
		0   - Success
		Non 0 - Failed
* Remarks:
* 编    号: No172
* 函 数 名: HBI_RetransMissFrameEx（新增）
* 功能描述: 整帧重传
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			unsigned int frameid -待重传帧号
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_RetransMissFrameEx(void* handle, unsigned int frameid);

/*********************************************************
* No: No173
* Function name: HBI_TriggerPgaBinningAcqTimeCorrect
* Function description: trigger mode, PGA gain stage, binning type, frame speed, correction enabling setting
* Parameter Description:
		In: void *handle - Handle (unsigned pointer)
		int triggerMode 	// Work mode
						// Static:
							// 01: Software trigger mode;
							// 02: Clear;
							// 03: High voltage generator(hvg) trigger mode;
							// 04: Auto exposure detection mode.
						// Dynamic:
							// 05: Hvg pulse sync in trigger mode;
							// 06: Fpd pulse sync out trigger Mode;
							// 07: Free run mode.
		int nPgaLevel	// PGA gain stage
									//[n]-Invalid,[1]-0.6pC,[2]-1.2pC,[3]-2.4pC,[4]-3.6pC,[5]-4.8pC,[6]-7.2pC,[7]-9.6pC
									//[8]-LFW (COMS),[9]-HFW (COMS),[10]-0.3pC,[11]-0.15pC,Default: 7.2 pC.
		unsigned char binning
							// 1: 1x1,
							// 2: 2x2,
							// 3: 3x3,
							// 4: 4x4，
		int acqtime		// Live acquisition span time, unit: ms
		IMAGE_CORRECT_ENABLE* pCorrect,refer to <<HbiType.h>>
		int nfeedback 	// 1: Automatic feedback config parameters if setting is successful
								// Non-1: Feedback setting success status
* Return value：int
		0   - Success
		Non 0 - Failed
* Remarks:
* 编    号: No173
* 函 数 名: HBI_TriggerPgaBinningAcqTimeCorrect
* 功能描述: 设置触发模式、PGA档位、binning方式以及帧率（采集图像时间间隔）、矫正使能
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int triggerMode - 触发模式
				静态平板（每秒1帧）：1-软触发，2-Clear,3-高压触发，4-FreeAED
				动态平板（每秒2帧以上）：05-Dynamic:Hvg Sync Mode,06-Dynamic:Fpd Sync Mode,7-Continue Mode
			int nPgaLevel - PGA 档位，[n]-Invalid,[1]-0.6pC,[2]-1.2pC,[3]-2.4pC,[4]-3.6pC,[5]-4.8pC,[6]-7.2pC,默认7.2pC,[7]-9.6pC
									  [8]-LFW (COMS),[9]-HFW (COMS),[10]-0.3pC,[11]-0.15pC
			unsigned char binning - 1:1x1,2:2x2,3:3x3,4:4x4，其他不支持
			int acqtime - 间隔时间,单位是毫秒ms,大于0
			IMAGE_CORRECT_ENABLE* pCorrect,见HbiType.h。
			int nfeedback - 1：设置成功后自动反馈固件参数，非1：反馈设置成功信号（见参数和信号见回调函数）
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_TriggerPgaBinningAcqTimeCorrect(void* handle, int triggerMode, int nPgaLevel, unsigned char binning, int acqtime, IMAGE_CORRECT_ENABLE* pCorrect);

/*********************************************************
* No: No174
* Function name: HBI_SetLogSwitch
* Function description: Log Switch
* Parameter Description:
		In: bool bwrite - true:write a log,false:do not write logs
* Return value：None
* Remarks:
* 编    号: No174
* 函 数 名: HBI_SetLogSwitch
* 功能描述: 日志开关
* 参数说明:
		In: bool bwrite - true:表示写日志,false:表示不写日志
* 返 回 值：无
* 备    注:调用默认或不调用该接口DLL都写日志，不写日志，设置成false即可。
*********************************************************/
HBI_FPD_DLL void __stdcall HBI_SetLogSwitch(bool bwrite = true);

/*********************************************************
* No: No175
* Function name: HBI_RetransMissPacketEx（new）
* Function description: Single/multiple packet retransmission
* Parameter Description:
		In: void *handle - Handle (unsigned pointer)
			unsigned int frameid -Frame number to be retransmitted
			unsigned int startpackagenum -Starting package number
			unsigned int endpackagenum -结End package number
		Out: None
* Return value：int
		0   - Success
		Non 0 - Failed
* Remarks:
* 编    号: No175
* 函 数 名: HBI_RetransMissPacketEx（新增）
* 功能描述: 单包/多包重传
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			unsigned int frameid -待重传帧号
			unsigned int startpackagenum -起始包号
			unsigned int endpackagenum -结束包号
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_RetransMissPacketEx(void* handle, unsigned int frameid, unsigned int startpackagenum, unsigned int endpackagenum);

/*********************************************************
* No: No176
* Function name: HBI_FirmwareReset（new）
* Function description: Firmware reset
* Parameter Description:
		In: void *handle - Handle (unsigned pointer)
		Out: None
* Return value：int
		0   - Success
		Non 0 - Failed
* Remarks:Phenomenon - Frame Number Zeroing
* 编    号: No176
* 函 数 名: HBI_FirmwareReset（新增）
* 功能描述: 固件重置
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:现象-帧号归零
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_FirmwareReset(void* handle);

/*********************************************************
* No: No177
* Function name: HBI_FirmwareManagementTempalteType（new）
* Function description: Firmware template management template method
* Parameter Description:
	In: void * handle - Handle (unsigned pointer)
		unsigned int uType - Firmware template management template method
			0-by binning type,
			1-by Muti-Mode type
			others-by binning type
		Out: None
* Return value：int
		0   - Success
		Non 0 - Failed
* Remarks:
* No: No177
* 函 数 名: HBI_FirmwareManagementTempalteType（新增）
* 功能描述: 固件模板管理模板方式
* 参数说明:
	In: void * handle - 句柄(无符号指针)
		unsigned int uType - 固件模板管理模板方式,0-by binning type,1-by Muti-Mode type,others-by binning type
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_FirmwareManagementTempalteType(void* handle, unsigned int uType);

/*********************************************************
* No: No178
* Function name: HBI_SetValidModeItemNumber（new）
* Function description: Set the number of valid Mode items
* Parameter Description:
	In: void * handle - Handle (unsigned pointer)
		unsigned int uNumber - Mode item number,[0,10]
		Out: None
* Return value：int
		0   - Success
		Non 0 - Failed
* Remarks:
* No: No178
* 函 数 名: HBI_SetValidModeItemNumber（新增）
* 功能描述: 设置有效Mode项数（Set the number of valid Mode items）
* 参数说明:
	In: void * handle - 句柄(无符号指针)
		unsigned int uNumber - Mode项数目,uNumber:[0,10]
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetValidModeItemNumber(void* handle, unsigned int uNumber);

////////////////////////////////////////////////////////////////////////
// The following HBI_ M* interface function not only updates the current firmware control parameters, but also synchronizes the parameters in the current Mode item
// 以下HBI_M***新增接口函数，除更新当前固件控制参数，另外同步当前Mode项中的参数
////////////////////////////////////////////////////////////////////////
/*********************************************************
* No: No179
* Function name: HBI_MUpdateTriggerMode（new）
* Function description: trigger mode setting
* Parameter Description:
		In: void *handle - Handle (unsigned pointer)
			int _triggerMode-	trigger mode
				  // 1: Software trigger mode
						// 3: High voltage generator trigger mode
						// 4: AED trigger mode
						// 5-Dynamic:Hvg Sync Mode
						// 6-Dynamic:Fpd Sync Mode
						// 7-Dynamic:Fpd Continue
						// 8-SAEC。
						// others-invaild
		Out:None
* Return value：int
		0   - Success
		Non 0 - Failed
* Remarks:
* 编    号: No179
* 函 数 名: HBI_MUpdateTriggerMode（新增）
* 功能描述: 设置触发模式
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int _triggerMode,1-软触发，3-高压触发，4-FreeAED,
				5-Dynamic:Hvg Sync Mode
				6-Dynamic:Fpd Sync Mode
				7-Dynamic:Fpd Continue
				8-SAEC。
		Out:无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_MUpdateTriggerMode(void* handle, int _triggerMode);

/*********************************************************
* No: No180
* Function name: HBI_MSetBinning（new）
* Function description: Binning mode setting
* Parameter Description:
		In:
			void *handle - Handle (unsigned pointer)
			unsigned int ubinning - 1:1x1,2:2x2,3:3x3,4:4x4
		Out:None
* Return value：int
		0   - Success
		Non 0 - Failed
* Remarks:
* 编    号: No180
* 函 数 名: HBI_MSetBinning（新增）
* 功能描述: 设置图像binning方式
* 参数说明:
		In:
			void *handle - 句柄(无符号指针)
			unsigned int ubinning - 1:1x1,2:2x2,3:3x3,4:4x4
		Out:无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_MSetBinning(void* handle, unsigned int ubinning);

/*********************************************************
* No: No181
* Function name: HBI_MSetPGALevel（new）
* Function description: PGA gain stage setting
* Parameter Description:
		In: void *handle - Handle (unsigned pointer)
			int mode    // PGA gain stage
									//[n]-Invalid,[1]-0.6pC,[2]-1.2pC,[3]-2.4pC,[4]-3.6pC,[5]-4.8pC,[6]-7.2pC,[7]-9.6pC
									//[8]-LFW (COMS),[9]-HFW (COMS),[10]-0.3pC,[11]-0.15pC,Default: 7.2 pC.
		Out: None
* Return value：int
		0   - Success
		Non 0 - Failed
* Remarks:
* 编    号: No181
* 函 数 名: HBI_MSetPGALevel（新增）
* 功能描述: 设置增益挡位,即设置可编程积分电容档位,提高灵敏度
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int mode - 模式
			[n]-Invalid
			[1]-0.6pC
			[2]-1.2pC
			[3]-2.4pC
			[4]-3.6pC
			[5]-4.8pC
			[6]-7.2pC,默认7.2pC
			[7]-9.6pC
			[8]-LFW (COMS)
			[9]-HFW (COMS)
			[10]-0.3pC
			[11]-0.15pC
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_MSetPGALevel(void* handle, int nGainLevel);

/*********************************************************
* No: No182
* Function name: HBI_MSetRoiMode（new）
* Function Description: -silicon ROI region setting function.
*       Currentlyonly 1613-125um、3030-140um、4343-100um、2530-100um are supported.
* Parameter Description:
	In: void *handle - unsigned handle pointer
		int nMode - ROI mode
		for example,1613-125um：0-Normal Image，w*h(1248*1024), Rect(0,0,1247,1023);
						 1-w*h(32*1024), Rect(608,0,639,1023);
						 2-w*h(1248*32), Rect(0,496,1247,527);
						 3-w*h(1248*64), Rect(0,480,1247,543);
						 4-w*h(1248*128),Rect(0,448,1247,575);
						 Others-Normal Image。
			 3030-140um：0-Normal Image，w*h(2048*2048), Rect(0,0,2047,2047);
						 1-w*h(1024*2048),Rect(512,0,1535,2047);
						 Others-Normal Image。
			 2530-100um：0-Normal Image，w*h(2432*3072), Rect(0,0,2431,3071);
						 1-w*h(1808*1800),Rect(316,636,2115,2435);
						 2-w*h(1312*1300),Rect(566,886,1865,2185);
						 Others-Normal Image。
		int nfeedback 	// 1: Automatic feedback config parameters if setting is successful
								// Non-1: Feedback setting success status
	Out: Null
* Return Value：int
			   0: Success
			   Non-0: Failure
* Comment:only 1613-125um、3030-140um、2530-100um are supported。
* 编    号: No182
* 函 数 名: HBI_MSetRoiMode（新增）
* 功能描述: 非晶硅ROI区域设置函数
*           当前仅支持平板类型包括1613-125um、3030-140um、4343-100um、2530-100um。
* 参数说明:
	In: void *handle - 句柄(无符号指针)
		int nMode - Roi模式
		其中,1613-125um：0-Normal Image，w*h(1248*1024), Rect(0,0,1247,1023);
						 1-w*h(32*1024), Rect(608,0,639,1023);
						 2-w*h(1248*32), Rect(0,496,1247,527);
						 3-w*h(1248*64), Rect(0,480,1247,543);
						 4-w*h(1248*128),Rect(0,448,1247,575);
						 其他-Normal Image。
			 3030-140um：0-Normal Image，w*h(2048*2048), Rect(0,0,2047,2047);
						 1-w*h(1024*2048),Rect(512,0,1535,2047);
						 其他-Normal Image。
			 2530-100um：0-Normal Image，w*h(2432*3072), Rect(0,0,2431,3071);
						 1-w*h(1808*1800),Rect(316,636,2115,2435);
						 2-w*h(1312*1300),Rect(566,886,1865,2185);
						 其他-Normal Image。
		int nfeedback - 1：设置成功后自动反馈固件参数，非1：反馈设置成功信号（见参数和信号见回调函数）,默认为1
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:当前仅支持平板类型包括1613-125um、3030-140um、4343-100um、2530-100um。
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_MSetRoiMode(void* handle, int nMode, int nfeedback = 1);

/*********************************************************
* No: No183
* Function name: HBI_MSetZoomModeRect（new）
* Function Description: The ZOOM Mode region setting function for CMOS tablets
*           currently supports only CMOS - 1412-50um and CMOS - 0606-50um ,1917-120umtablet types.
* Parameter Description:
	In: void *handle - unsigned handle pointer
		CMOS_ZOOM_RECT stZoomRect - Zoom region
			The region is represented by left, top, right, and bottom.
			for example, when left, top, right and bottom are illegal values, the image will display the full image by default,
			1>When left=top=right=bottom=0, the whole picture is displayed;
			2>When eft<0 or left>width, the whole picture will be displayed, and others are the same.
		int nfeedback // 1: Automatic feedback config parameters if setting is successful
					// Non-1: Feedback setting success status
	Out: Null
* Return Value：int
			   0: Success
			   Non-0: Failure
* Comment: 
* 编    号: No183
* 函 数 名: HBI_MSetZoomModeRect（新增）
* 功能描述: CMOS ZOOM Mode区域设置函数
*           当前仅支持平板类型包括CMOS-1412-50um、CMOS-0606-50um,1917-120um。
* 参数说明:
	In: void *handle - 句柄(无符号指针)
		CMOS_ZOOM_RECT stZoomRect - Zoom区域
			其中，区域用left,top,right,bottom来表示。
			例如：当left,top,right,bottom为非法数值时，图像默认显示全图，
			1>left=top=right=bottom=0时，显示全图；
			2>left<0或left>width时，显示全图，其他雷同。
		int nfeedback - 1：设置成功后自动反馈固件参数，非1：反馈设置成功信号（见参数和信号见回调函数）,默认为1
	Out: 无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:当前仅支持平板类型包括CMOS-1412-50um、CMOS-0606-50um。
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_MSetZoomModeRect(void* handle, CMOS_ZOOM_RECT stZoomRect, int nfeedback = 1);

/*********************************************************
* No: No184
* Function name: HBI_MSetSelfDumpingTime（new）
* Function Description: 	 // Self dumping span time setting
* Parameter Description:
	In: 		void *handle // unsigned handle pointer
			int time 		 // Self dumping span time, unit: ms
	Out: 	Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No184
* 函 数 名: HBI_MSetSelfDumpingTime（新增）
* 功能描述: 设置采集时间间隔(动态平板)
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int time - 间隔时间,单位是毫秒ms，> 0ms
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_MSetSelfDumpingTime(void* handle, int time);

/*********************************************************
* No: No185
* Function name: HBI_MSetLiveAcquisitionTime（new）
* Function Description: 	// Live acquisition span time setting (apply to static and wireless type FPD)
* Parameter Description:
	In: 	void *handle 	// unsigned handle pointer
		  int time			// Live acquisition span time, unit: ms
	Out: Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No185
* 函 数 名: HBI_MSetLiveAcquisitionTime(静态平板)（新增）
* 功能描述: 设置采集时间间隔(即可设置帧率)
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int time - 间隔时间,单位是毫秒ms，> 0ms
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_MSetLiveAcquisitionTime(void* handle, int time);

/*********************************************************
* No: No186
* Function name: HBI_MSetSinglePrepareTime（new）
* Function Description:     // Prepare delay time setting, which is also called exposure window
* Parameter Description:
	In: 	void *handle	// unsigned handle pointer
	Out:	int *out_itime 	// Time(0~10000), unit: ms
						// 0: “HBI_Prepare” and “HBI_SingleAcquisition” command are combined to realize single image acquisition
						// Non-0: FPD will delay “Prepare Delay Time” and automatically acquire one single image when the detector receives “HBI_Prepare” command.
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No186
* 函 数 名: HBI_MSetSinglePrepareTime（新增）
* 功能描述: 设置软触发单帧采集清空和采集之间的时间间隔
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int *in_itime - 时间 [0~10000] 单位:mm
				0-表示软触发单帧采集先HBI_SinglePrepare后HBI_SingleAcquisition完成单帧采集
				非0-表示软触发单帧采集，只需HBI_SinglePrepare即可按照延时时间完成单帧采集
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_MSetSinglePrepareTime(void* handle, int in_itime);

/*********************************************************
* No: No187
* Function name: HBI_MSetFpsOfTime（new）
* Function description: Set frame rate control time, support subtle
* Parameter Description:
		In: void *handle - Handle (unsigned pointer)
			unsigned int untimems - time interval,Unit is milliseconds，> 0ms
			unsigned short ustimeus - time interval,The unit is microseconds，>= 0ms.[0,999]
		Out: None
* Return value：int
		0   - Success
		Non 0 - Failed
* Remarks:eg 16fps，ntimems=62ms， ntimeus=8um
* 编    号: No187
* 函 数 名: HBI_MSetFpsOfTime（新增）
* 功能描述: 设置帧率控制时间
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			unsigned int untimems - 间隔时间,单位是毫秒ms，> 0ms
			unsigned short ustimeus - 间隔时间,单位是微秒us，>= 0ms.[0,999]
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:比如16帧，ntimems=62ms， ntimeus=8um
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_MSetFpsOfTime(void* handle, unsigned int untimems, unsigned short ustimeus = 0);

/*********************************************************
* No: No188
* Function name: HBI_MPgaAcqTime（new）
* Function Description: 	// PGA gain stage, frame speed setting
* Parameter Description:
	In:	void *handle 	// unsigned handle pointer
		int nPgaLevel	// PGA gain stage
									//[n]-Invalid,[1]-0.6pC,[2]-1.2pC,[3]-2.4pC,[4]-3.6pC,[5]-4.8pC,[6]-7.2pC,[7]-9.6pC
									//[8]-LFW (COMS),[9]-HFW (COMS),[10]-0.3pC,[11]-0.15pC,Default: 7.2 pC.
		int acqtime		// Live acquisition span time, unit: ms
		int nfeedback 	// 1: Automatic feedback config parameters if setting is successful
						// Non-1: Feedback setting success status
* Return Value: int
			// 0: Success
			// Non-0: Failure
* Comment:
* 编    号: No188
* 函 数 名: HBI_MPgaAcqTime（新增）
* 功能描述: 设置PGA档位、及帧率（采集图像时间间隔）
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int nPgaLevel - PGA 档位，[n]-Invalid,[1]-0.6pC,[2]-1.2pC,[3]-2.4pC,[4]-3.6pC,[5]-4.8pC,[6]-7.2pC,默认7.2pC,[7]-9.6pC
									  [8]-LFW (COMS),[9]-HFW (COMS),[10]-0.3pC,[11]-0.15pC
			int acqtime - 间隔时间,单位是毫秒ms,大于0
			int nfeedback - 1：设置成功后自动反馈固件参数，非1：反馈设置成功信号（见参数和信号见回调函数）
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_MPgaAcqTime(void* handle, int nPgaLevel, int acqtime, int nfeedback = 0);

/*********************************************************
* No: No189
* Function name: HBI_MPgaBinningAcqTime（new）
* Function Description: 	// PGA gain stage, binning, frame speed setting
* Parameter Description:
	In: 	void *handle 	// unsigned handle pointer
		int nPgaLevel	// PGA gain stage
									//[n]-Invalid,[1]-0.6pC,[2]-1.2pC,[3]-2.4pC,[4]-3.6pC,[5]-4.8pC,[6]-7.2pC,[7]-9.6pC
									//[8]-LFW (COMS),[9]-HFW (COMS),[10]-0.3pC,[11]-0.15pC,Default: 7.2 pC.
		unsigned char binning
							// 1: 1x1,
							// 2: 2x2,
							// 3: 3x3,
							// 4: 4x4，
		int acqtime 		// Live acquisition span time, unit: ms
		int nfeedback 	// 1: Automatic feedback config parameters if setting is successful
						// Non-1: Feedback setting success status
* Return Value:
	int 			// 0: Success
					// Non-0: Failure
* Comment:
* 编    号: No189
* 函 数 名: HBI_MPgaBinningAcqTime（新增）
* 功能描述: 设置PGA档位、binning方式以及帧率（采集图像时间间隔）
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int nPgaLevel - PGA 档位，[n]-Invalid,[1]-0.6pC,[2]-1.2pC,[3]-2.4pC,[4]-3.6pC,[5]-4.8pC,[6]-7.2pC,默认7.2pC,[7]-9.6pC
												  [8]-LFW (COMS),[9]-HFW (COMS),[10]-0.3pC,[11]-0.15pC
			unsigned char binning - 1:1x1,2:2x2,3:3x3,4:4x4，其他不支持
			int acqtime - 间隔时间,单位是毫秒ms,大于0
			int nfeedback - 1：设置成功后自动反馈固件参数，非1：反馈设置成功信号（见参数和信号见回调函数）
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_MPgaBinningAcqTime(void* handle, int nPgaLevel, unsigned char binning, int acqtime, int nfeedback = 0);

/*********************************************************
* No: No190
* Function name: HBI_MTriggerAndCorrectApplay（new）
* Function Description: 	// Work mode and image calibration setting
* Parameter Description:
	In: 	void *handle	// unsigned handle pointer
		int _triggerMode
						// 1: Software trigger mode
						// 3: High voltage generator trigger mode
						// 4: AED trigger mode
						// 5-Dynamic:Hvg Sync Mode
						// 6-Dynamic:Fpd Sync Mode
						// 7-Dynamic:Fpd Continue
						// 8-SAEC。
		IMAGE_CORRECT_ENABLE* pCorrect 	// refer to<< HbiType.h>>
	Out:	 Null
* Return Value: int
			// 0: Success
			// Non-0: Failure,refer to<<HbiError.h>>
* Comment:
* 编    号: No190
* 函 数 名: HBI_MTriggerAndCorrectApplay（新增）
* 功能描述: 设置触发模式和图像校正使能（工作站）新版本
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int _triggerMode,1-软触发，3-高压触发，4-FreeAED,
				5-Dynamic:Hvg Sync Mode
				6-Dynamic:Fpd Sync Mode
				7-Dynamic:Fpd Continue
				8-SAEC。
			IMAGE_CORRECT_ENABLE* pCorrect,见HbiType.h。
		Out:无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:替换HBI_WorkStationInitDllCfg和HBI_QuckInitDllCfg接口
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_MTriggerAndCorrectApplay(void* handle, int _triggerMode, IMAGE_CORRECT_ENABLE* pCorrect);

/*********************************************************
* No: No191
* Function name: HBI_MTriggerBinningAcqTime（new）
* Function Description: 	// Work mode, binning, frame speed setting
* Parameter Description:
	In: 	void *handle 	// unsigned handle pointer
		int triggerMode	// Work mode
						// Static:
							// 01: Software trigger mode;
							// 02: Clear;
							// 03: High voltage generator(hvg) trigger mode;
							// 04: Auto exposure detection mode.
						// Dynamic:
							// 05: Hvg pulse sync in trigger mode;
							// 06: Fpd pulse sync out trigger Mode;
							// 07: Free run mode.
		unsigned char binning
							// 1: 1x1,
							// 2: 2x2,
							// 3: 3x3,
							// 4: 4x4，
		int time 			// Live acquisition span time, unit: ms
						// For example: 1000ms (1fps), 500ms (2fps), 250ms (4fps),125ms (8fps)
	Out: Null
* Return Value:
	int 			    // 0: Success
						// Non-0: Failure
* Comment:
* 编    号: No191
* 函 数 名: HBI_MTriggerBinningAcqTime（新增）
* 功能描述: 设置触发模式、binning方式以及帧率（采集图像时间间隔）
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int triggerMode - 触发模式
				静态平板（每秒1帧）：1-软触发，2-Clear,3-高压触发，4-FreeAED
				动态平板（每秒2帧以上）：05-Dynamic:Hvg Sync Mode,06-Dynamic:Fpd Sync Mode,7-Continue Mode
			unsigned char binning - 1:1x1,2:2x2,3:3x3,4:4x4，其他不支持
			int time - 间隔时间,单位是毫秒ms,大于0
			int nfeedback - 1：设置成功后自动反馈固件参数，非1：反馈设置成功信号（见参数和信号见回调函数）
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_MTriggerBinningAcqTime(void* handle, int triggerMode, unsigned char binning, int acqtime, int nfeedback = 0);

/*********************************************************
* No: No192
* Function name: HBI_MTriggerPgaBinningAcqTime（new）
* Function description: trigger mode, PGA gain stage, binning type, frame speed setting
* Parameter Description:
		In: void *handle - Handle (unsigned pointer)
		int triggerMode 	// Work mode
						// Static:
							// 01: Software trigger mode;
							// 02: Clear;
							// 03: High voltage generator(hvg) trigger mode;
							// 04: Auto exposure detection mode.
						// Dynamic:
							// 05: Hvg pulse sync in trigger mode;
							// 06: Fpd pulse sync out trigger Mode;
							// 07: Free run mode.
		int nPgaLevel	// PGA gain stage
									//[n]-Invalid,[1]-0.6pC,[2]-1.2pC,[3]-2.4pC,[4]-3.6pC,[5]-4.8pC,[6]-7.2pC,[7]-9.6pC
									//[8]-LFW (COMS),[9]-HFW (COMS),[10]-0.3pC,[11]-0.15pC,Default: 7.2 pC.
		unsigned char binning
							// 1: 1x1,
							// 2: 2x2,
							// 3: 3x3,
							// 4: 4x4，
		int acqtime		// Live acquisition span time, unit: ms
		int nfeedback 	// 1: Automatic feedback config parameters if setting is successful
								// Non-1: Feedback setting success status
* Return value：int
		0   - Success
		Non 0 - Failed
* Remarks:
* 编    号: No192
* 函 数 名: HBI_MTriggerPgaBinningAcqTime（新增）
* 功能描述: 设置触发模式、PGA档位、binning方式以及帧率（采集图像时间间隔）
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int triggerMode - 触发模式
				静态平板（每秒1帧）：1-软触发，2-Clear,3-高压触发，4-FreeAED
				动态平板（每秒2帧以上）：05-Dynamic:Hvg Sync Mode,06-Dynamic:Fpd Sync Mode,7-Continue Mode
			int nPgaLevel - PGA 档位，[n]-Invalid,[1]-0.6pC,[2]-1.2pC,[3]-2.4pC,[4]-3.6pC,[5]-4.8pC,[6]-7.2pC,默认7.2pC,[7]-9.6pC
									  [8]-LFW (COMS),[9]-HFW (COMS),[10]-0.3pC,[11]-0.15pC
			unsigned char binning - 1:1x1,2:2x2,3:3x3,4:4x4，其他不支持
			int acqtime - 间隔时间,单位是毫秒ms,大于0
			int nfeedback - 1：设置成功后自动反馈固件参数，非1：反馈设置成功信号（见参数和信号见回调函数）
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_MTriggerPgaBinningAcqTime(void* handle, int triggerMode, int nPgaLevel, unsigned char binning, int acqtime, int nfeedback = 0);

/*********************************************************
* No: No193
* Function name: HBI_MTriggerPgaBinningAcqTimeCorrect（new）
* Function description: trigger mode, PGA gain stage, binning type, frame speed, correction enabling setting
* Parameter Description:
		In: void *handle - Handle (unsigned pointer)
		int triggerMode 	// Work mode
						// Static:
							// 01: Software trigger mode;
							// 02: Clear;
							// 03: High voltage generator(hvg) trigger mode;
							// 04: Auto exposure detection mode.
						// Dynamic:
							// 05: Hvg pulse sync in trigger mode;
							// 06: Fpd pulse sync out trigger Mode;
							// 07: Free run mode.
		int nPgaLevel	// PGA gain stage
									//[n]-Invalid,[1]-0.6pC,[2]-1.2pC,[3]-2.4pC,[4]-3.6pC,[5]-4.8pC,[6]-7.2pC,[7]-9.6pC
									//[8]-LFW (COMS),[9]-HFW (COMS),[10]-0.3pC,[11]-0.15pC,Default: 7.2 pC.
		unsigned char binning
							// 1: 1x1,
							// 2: 2x2,
							// 3: 3x3,
							// 4: 4x4，
		int acqtime		// Live acquisition span time, unit: ms
		IMAGE_CORRECT_ENABLE* pCorrect,refer to <<HbiType.h>>
		int nfeedback 	// 1: Automatic feedback config parameters if setting is successful
								// Non-1: Feedback setting success status
* Return value：int
		0   - Success
		Non 0 - Failed
* Remarks:
* 编    号: No193
* 函 数 名: HBI_MTriggerPgaBinningAcqTimeCorrect（新增）
* 功能描述: 设置触发模式、PGA档位、binning方式以及帧率（采集图像时间间隔）、矫正使能
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int triggerMode - 触发模式
				静态平板（每秒1帧）：1-软触发，2-Clear,3-高压触发，4-FreeAED
				动态平板（每秒2帧以上）：05-Dynamic:Hvg Sync Mode,06-Dynamic:Fpd Sync Mode,7-Continue Mode
			int nPgaLevel - PGA 档位，[n]-Invalid,[1]-0.6pC,[2]-1.2pC,[3]-2.4pC,[4]-3.6pC,[5]-4.8pC,[6]-7.2pC,默认7.2pC,[7]-9.6pC
									  [8]-LFW (COMS),[9]-HFW (COMS),[10]-0.3pC,[11]-0.15pC
			unsigned char binning - 1:1x1,2:2x2,3:3x3,4:4x4，其他不支持
			int acqtime - 间隔时间,单位是毫秒ms,大于0
			IMAGE_CORRECT_ENABLE* pCorrect,见HbiType.h。
			int nfeedback - 1：设置成功后自动反馈固件参数，非1：反馈设置成功信号（见参数和信号见回调函数）
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_MTriggerPgaBinningAcqTimeCorrect(void* handle, int triggerMode, int nPgaLevel, unsigned char binning, int acqtime, IMAGE_CORRECT_ENABLE* pCorrect);

/*********************************************************
* No: No194
* Function name: HBI_IsMutilMode（new）
* Function description: Check if the current firmware supports multiple modes
* Parameter Description:
		In: void *handle - Handle (unsigned pointer)
* Return value：bool
		true  - support
		false - Not Supported
* Remarks:The essence of Mode mode is that the firmware manages templates differently
* 编    号: No194
* 函 数 名: HBI_IsMutilMode（新增）
* 功能描述: 查询当前固件是否支持多Mode（多Mode方式本质是固件对模板的管理方式不同）
* 参数说明:
		In: void *handle - 句柄(无符号指针)
* 返 回 值：bool
		true  - 支持
		false - 不支持（默认支持binning版本）
* 备    注:
*********************************************************/
HBI_FPD_DLL bool __stdcall HBI_IsMutilMode(void* handle);

/*********************************************************
* No: No195
* Function name: HBI_SetTICOFImageType（new）
* Function description: Switching between normal and grayscale images
* Parameter Description:
		In: void *handle - Handle (unsigned pointer)
			unsigned int nImageType -Image type,2-gray image，1-normal image，others-normal image
		Out: None
* Return value：int
		0   - Success
		Non 0 - Failed
* Remarks:
* 编    号: No195
* 函 数 名: HBI_SetTICOFImageType（新增）
* 功能描述: 设置正常图像和灰度图切换
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			unsigned int nImageType -图像类型,其中2-灰度图，1-正常图像，others-正常图像
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetTICOFImageType(void* handle, unsigned int nImageType);

/*********************************************************
* No: No196
* Function name: HBI_ImportCfgFile2Detector（new）
* Function description: Importing configuration files into firmware
* Parameter Description:
		In: void *handle - Handle (unsigned pointer)
			int cmd -command code,
				EDL_COMMON_TYPE_SET_RAM_PARAM_CFG -Set RAM parameters
				EDL_COMMON_TYPE_SET_ROM_PARAM_CFG -Set ROM parameters
				EDL_COMMON_TYPE_SET_FACTORY_PARAM_CFG -Set FACTORY parameters
			char* cfgfile -firmware configuration file absolute path,file extension *.csv or *.txt
		Out: None
* Return value：int
		0   - Success
		Non 0 - Failed
* Remarks:configuration file format:Register_xxxx,Reserved,0x--
* 编    号: No196
* 函 数 名: HBI_ImportCfgFile2Detector（新增）
* 功能描述: 将配置文件导入固件
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		    int cmd -命令,
				EDL_COMMON_TYPE_SET_RAM_PARAM_CFG -获取RAM参数
				EDL_COMMON_TYPE_SET_ROM_PARAM_CFG -获取ROM参数
				EDL_COMMON_TYPE_SET_FACTORY_PARAM_CFG -获取FACTORY参数
			char* cfgfile -固件配置文件,后缀名*.csv或*.txt
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:配置文件内容必须符合格式:Register_xxxx,Reserved,0x--
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_ImportCfgFile2Detector(void* handle, int cmd, char* cfgfile);

/*********************************************************
* No: No197
* Function name: HBI_ExportDetectorCfg2File（new）
* Function description: Export firmware parameters to a file
* Parameter Description:
		In: void *handle - Handle (unsigned pointer)
			int cmd -command code,
				EDL_COMMON_TYPE_GET_RAM_PARAM_CFG -Get RAM parameters
				EDL_COMMON_TYPE_GET_ROM_PARAM_CFG -Get ROM parameters
				EDL_COMMON_TYPE_GET_FACTORY_PARAM_CFG -Get FACTORY parameters
			char* cfgfile -firmware configuration file absolute path,file extension *.csv or *.txt
		Out: None
* Return value：int
		0   - Success
		Non 0 - Failed
* Remarks:configuration file format:Register_xxxx,Reserved,0x--
* 编    号: No197
* 函 数 名: HBI_ExportDetectorCfg2File（新增）
* 功能描述: 将固件参数导出到文件
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			int cmd -命令,
				EDL_COMMON_TYPE_GET_RAM_PARAM_CFG -获取RAM参数
				EDL_COMMON_TYPE_GET_ROM_PARAM_CFG -获取ROM参数
				EDL_COMMON_TYPE_GET_FACTORY_PARAM_CFG -获取FACTORY参数
			char* cfgfile -固件配置文件,后缀名*.csv或*.txt
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:配置文件内容必须符合格式:Register_xxxx,Reserved,0x--
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_ExportDetectorCfg2File(void* handle, int cmd, char* cfgfile);

/*********************************************************
* No: No198
* Function name: HBI_InitDetectorParams（new）
* Function description: initialize the basic parameters of the flat panel detector
* Parameter Description:
		In: void *handle - Handle (unsigned pointer)
			DETECTOR_INIT_PARAMS param -refer to《HbiType.h》
				param._ucmodeid;          //mode id [0,9];
		  param._uctriggerType;     //1-soft trigger，3-hvg trigger,4-FreeAED,5-Dynamic:Hvg Sync Mode,6-Dynamic:Fpd Sync Mode,7-Dynamic:Fpd Continue,8-SAEC;
									//Only vaild in Binning mode,invaild in multi-mode mode,purpose compatible with diffrent version.
				param._ucOffsetCorrection;//0-No Offset Correction;1-Software PreOffset Correction;2-Firmware PostOffset Correction;3-Firmware PreOffset Correction;
				param._ucGainCorrection;  //0-No Gain Correction;1-Software Gain Correction;2-Firmware Gain Correction;
				param._ucDefectCorrection;//0-No Defect Correction;1-Software Defect Correction;2-Firmware Defect Correction;
				param._ucDummyCorrection; //0-No Dummy Correction;1-Software Dummy Correction;2-Firmware Dummy Correction;
		Out: None
* Return value：int
		0   - Success
		Non 0 - Failed
* Remarks:
* 编    号: No198
* 函 数 名: HBI_InitDetectorParams（新增）
* 功能描述: 初始化平板探测器基础参数
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			DETECTOR_INIT_PARAMS param -参数,详细参考《HbiType.h》
				param._ucmodeid;          //mode id [0,9];
	            param._uctriggerType;     //1-软触发，3-高压触发,4-FreeAED,5-Dynamic:Hvg Sync Mode,6-Dynamic:Fpd Sync Mode,7-Dynamic:Fpd Continue,8-SAEC;
										  //仅在Binning方式下有效,在多Mode方式下无效,因为Mode项中已包含触发模式参数，目的在于兼容不同版本。
				param._ucOffsetCorrection;//0-No Offset Correction;1-Software PreOffset Correction;2-Firmware PostOffset Correction;3-Firmware PreOffset Correction;
				param._ucGainCorrection;  //0-No Gain Correction;1-Software Gain Correction;2-Firmware Gain Correction;
				param._ucDefectCorrection;//0-No Defect Correction;1-Software Defect Correction;2-Firmware Defect Correction;
				param._ucDummyCorrection; //0-No Dummy Correction;1-Software Dummy Correction;2-Firmware Dummy Correction;
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_InitDetectorParams(void* handle, DETECTOR_INIT_PARAMS param);

/*********************************************************
* No: No199
* Function name: HBI_UpdateFPDSerialNumber
* Function Descriptio: Update product serial number
* Parameter Description:
		In:     void *handle - unsigned handle pointer
		In/Out: char *szSn,length=14
* Return value：int
		0   - Success
		Non 0 - Failed
* Remarks:
* 编    号: No199
* 函 数 名: HBI_UpdateFPDSerialNumber
* 功能描述: 更新产品序列号
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		In/Out: char *szSn,长度14位
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_UpdateFPDSerialNumber(void* handle, char* szSn);

/*********************************************************
* No: No200
* Function name: HBI_WindowWidthWindowLevel（new）
* Function Description: The automatic window width and window level algorithm converts 16bits data into 8-bit unsigned data through window width and window level processing, and users can directly display 8-bit data.
* Parameter Description:
		In:     void *handle - unsigned handle pointer
		In/Out: unsigned char *imgbuff，Source data (16 bit unsigned small end data), destination data (8-bit unsigned data)
		In/Out: int *nbufflen, Buffer data length (halve the data size after conversion)
		In:     int nwidth   - Image data width
		In:     int nheight  - Image data high
		In:     int nWindow   - window width
		In:     int nLeveling - window leveling
* Return value：int
		0   - Success
		Non 0 - Failed
* Remarks:
* 编    号: No200
* 函 数 名: HBI_WindowWidthWindowLevel（新增）
* 功能描述: 自动窗宽窗位算法将16位数据进行窗宽窗位处理转换为8位无符号数据，用户可以直接显示8位数据。
*           8位数据保存在imgbuff缓冲区内，数据长度减半保存在*nbufflen指针地址
* 参数说明:
		In:     void *handle - 句柄(无符号指针)
		In/Out: unsigned char *imgbuff，源数据（16位无符号小端数据），目的数据（8位无符号数据）
		In/Out: int *nbufflen，缓冲区数据长度（转换后数据大小减半）
		In:     int nwidth  - 图像数据宽
		In:     int nheight - 图像数据高
		In:     int nWindow   - 窗宽
		In:     int nLeveling - 窗位
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_WindowWidthWindowLevel(void* handle, unsigned char* imgbuff, int* nbufflen, int nwidth, int nheight, int nWindow, int nLeveling);

/*********************************************************
* No: No201
* Function name: HBI_WaitForReadyStatus（new）
* Function Description: Waiting for panel ready status
* Parameter Description:
		In: void *handle - unsigned handle pointer
		In: unsigned int tm - Waiting timeout，default 1200ms
* Return value：int
		0   - Success
		Non 0 - Failed
* Remarks:
* 编    号: No201
* 函 数 名: HBI_WaitForReadyStatus（新增）
* 功能描述: 等待平板ready状态
* 参数说明:
		In: void *handle - 句柄(无符号指针)
		In: unsigned int tm - 等待超时时间，默认1200ms
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_WaitForReadyStatus(void* handle, unsigned int tm = 1200);

/*********************************************************
* No: No202
* Function name: HBI_SetSelfDumpingTimeEx（new）
* Function description: Self dumping span time setting
* Parameter Description:
		In: void *handle - Handle (unsigned pointer)
			unsigned int untimems - time interval,Unit is milliseconds，> 0ms
			unsigned short ustimeus - time interval,The unit is microseconds，>= 0ms.[0,999]
		Out: None
* Return value：int
		0   - Success
		Non 0 - Failed
* Remarks:eg 16 fps，ntimems=62ms， ntimeus=8um
* 编    号: No202
* 函 数 名: HBI_SetSelfDumpingTimeEx（新增）
* 功能描述: 平板自清空时间设置（增加了微妙）
* 参数说明:
		In: void *handle - 句柄(无符号指针)
			unsigned int untimems - 间隔时间,单位是毫秒ms，> 0ms
			unsigned short ustimeus - 间隔时间,单位是微秒us，>= 0ms.[0,999]
		Out: 无
* 返 回 值：int
		0   - 成功
		非0 - 失败
* 备    注:比如16帧，ntimems=62ms， ntimeus=8um
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_SetSelfDumpingTimeEx(void* handle, unsigned int untimems, unsigned short ustimeus);

/*********************************************************
* No: No203
* Function Name: HBI_UpdatePcieDeviceId
* Function Description: Update PCIe device ID
* Parameter Description:
	In: void *handle - unsigned handle pointer
		unsigned short usdeviceid,refer to biType.h，pciedeviceidTable[PCIE_DEVID_MAX_NUMBER]。
	Out:None
* Return Value：int
		   0: Success
		Non-0: Failure
* Comment:
* 编    号: No203
* 函 数 名: HBI_UpdatePcieDeviceId
* 功能描述: 更新PCIe设备ID
* 参数说明:
	In: void *handle - 句柄(无符号指针)
		unsigned short usdeviceid,见HbiType.h中pciedeviceidTable[PCIE_DEVID_MAX_NUMBER]={7024, 7028, 7031, 7032, 7034, 7038}。
	Out:无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_UpdatePcieDeviceId(void* handle, unsigned short udeviceid);

/*********************************************************
* No: No204
* Function Name: HBI_GetCurFirmwareTemplate
* Function Description: Update PCIe device ID
* Parameter Description:
	In: void *handle - unsigned handle pointer
		unsigned short usdeviceid,refer to biType.h，pciedeviceidTable[PCIE_DEVID_MAX_NUMBER]。
	Out:None
* Return Value：int
		   0: Success
		Non-0: Failure
* Comment:
* 编    号: No204
* 函 数 名: HBI_GetCurFirmwareTemplate
* 功能描述: 获取当前固件模板，目前支持获取固件pre-offset、gain和defect模板
* 参数说明:
	In: void *handle - 句柄(无符号指针)
		unsigned char command, - 命令，见HbiType.h
			pre-offset模板：EDL_COMMON_FIRMWARE_OFFSET_TEMPLATE
			gain模板：EDL_COMMON_FIRMWARE_GAIN_TEMPLATE
			defect模板：EDL_COMMON_FIRMWARE_DEFECT_TEMPLATE
	Out:无
* 返 回 值：int
	0   - 成功
	非0 - 失败
* 备    注:
*********************************************************/
HBI_FPD_DLL int __stdcall HBI_GetCurFirmwareTemplate(void* handle, unsigned char command);

//
HBI_FPD_DLL int __stdcall HBI_RestartPcieDevice(void* handle);
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/***************************************************************************************
////////////////////////////////////////////////////////////////////////////////////////
// The export class is currently undeveloped and will not be provided
// 导出类 暂时未开发，不提供
////////////////////////////////////////////////////////////////////////////////////////
class HBI_FPD_DLL CHbiFpdDllClass
{
public:
	CHbiFpdDllClass();
	virtual ~CHbiFpdDllClass();

public:
	bool Init();
};
CHbiFpdDllClass *m_phbiimage = new CHbiFpdDllClass;
if (m_phbiimage != NULL)
{
	delete m_phbiimage;
	m_phbiimage = NULL;
}
if (m_phbiimage != NULL)
{
	m_phbiimage->Init();
}
***************************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // __HBI_FPD_H_
