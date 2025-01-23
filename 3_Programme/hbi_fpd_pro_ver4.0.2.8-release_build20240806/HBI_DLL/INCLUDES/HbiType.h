/*---------------------------------------------------------------------------
* Copyright (c) 2019 DT Image Technology (Shanghai) Co., Ltd
* All rights reserved.
*
* File name: HbiType. h
* Document ID:
* Summary: flat panel detector data structure header file
* Current version: 3.0
* Author: mhyang
* Date: November 20, 2020
* * Copyright (c) 2019 地太科特影像科技有限公司
* All rights reserved.
*
* 文件名称: HbiType.h
* 文件标识:
* 摘    要: 平板探测器数据结构头文件
*
* 当前版本: 3.0
* 作    者: mhyang
* 完成日期: 2020/11/20
----------------------------------------------------------------------------*/
#ifndef __HBI_TYPE_H_
#define __HBI_TYPE_H_

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#ifndef NULL
#define NULL 0
#endif

// Maximun number of suporrortrd panel
// 平板可支持的最大个数是6
#define PCIE_DEVID_MAX_NUMBER 6
#define DETECTOR_MAX_NUMBER   6
#define QUEUE_MIN_SIZE        6     // Minimum buffer value
#define QUEUE_MAX_SIZE        12    // Maximum buffer value
#define MODE_LIST_TOTAL       10
#define MODE_ITEM_NUMBER      11
#define MODE_LIST_ITEM_NUMBER 111

// Flat panel detector's status
// 平板状态
#define FPD_DISCONN_STATUS             0   // fpd disconnect status
#define FPD_PREPARE_STATUS             1   // fpd prepare status                               ////#define FPD_STATUS_BUSY             0X01  // fpd busy status
#define FPD_READY_STATUS               2   // fpd ready status                                 ////#define FPD_STATUS_READY            0X02  // fpd ready status
#define FPD_DOOFFSET_TEMPLATE          3   // firmware do offset template                      ////#define FPD_STATUS_AED              0X03  // AED mode,avild image data
#define FPD_EXPOSE_STATUS              4   // fpd busy/expose status                           ////#define FPD_STATUS_PREPARE          0X04  // Software mode,prepare command feedback command
#define FPD_CONTINUE_READY             5   // continue ready
#define FPD_DWONLOAD_GAIN              6   // download gain template success ack respond       ////#define FPD_UPLOAD_GAIN_ANSWER      0X06  // Upload gain template answer  
#define FPD_DWONLOAD_DEFECT            7   // download defect template success ack respond     ////#define FPD_UPLOAD_DEFECT_ANSWER    0X07  // Upload defect template answer
#define FPD_DWONLOAD_OFFSET            8   // download offset template success ack respond
#define FPD_UPDATE_FIRMARE             9   // update firmware
#define FPD_RETRANS_MISS               10  // Packet loss message
#define FPD_STATUS_AED                 11  // AED mode,avild image data
#define FPD_STATUS_SLEEP               12  // wireless,sleep status
#define FPD_STATUS_WAKEUP              13  // wireless,wake up status
#define FPD_DOWNLOAD_NO_IMAGE          14  // wireless,download image,no image
#define FPD_DOWNLOAD_TAIL_IMAGE        15  // wireless,download image,last image
#define FPD_EMMC_MAX_NUMBER            16  // wireless,demmc save image:Maximum number of stored images
#define FPD_ENDTIME_WARNNING           17  // RTC,end time warnning
#define FPD_WLAN_BATTERY_STATUS        18  // wireless,Battery Status Message
#define FPD_SOCKET_RECONNECT_SUCCESS   19  // Socket reconnect successfully
#define FPD_FRAME_DATA_OK              20  // confirm completion og one frame of data. add by mhyang 20231108
#define FPD_ERROR_DETECTOR_TYPE        21  // The type of detector is abnormality. add by mhyang 20231214
#define FPD_CONN_SUCCESS               100 // fpd connect status

// data length
#define FPD_DATA_BITS                  16
#define PACKET_MAX_SIZE                1049
#define PCIE_PACKET_MAX_SIZE           1052
#define JUMBO_PACKET_MAX_SIZE          8218
#define IMG_PACKET_DATA_MAX_LEN        1024
#define JUMBO_IMG_PACKET_DATA_MAX_LEN  8192
#define DOUBLE_JUMBO_IMG_PACKET_DATA_MAX_LEN  16384  // 8192 * 2
#define CUSTOMEFFECTRECVDATASIZE       1034
#define LIVE_ACQ_MAX_FRAME_NUM         20
#define FLOAT_PACKET_DATA_MAX_LEN      2048
#define UNINT_PACKET_DATA_MAX_LEN      2048
#define USHORT_PACKET_DATA_MAX_LEN     4096
#define USHORT_PACKET_DATA_WIRELESS    512
#define FLOAT_PACKET_DATA_WIRELESS     256
#define PREVIEW_SPACE_NUMBER           4
#define MIN_VALUE                      1e-6
#define ISDOUBLE_ZERO(d) (std::abs(d) < MIN_VALUE)
#define DUMMY_IMG_MINWIDTH              512
#define DUMMY_MIN_START_COLUMN          4
#define DUMMY_MAX_START_COLUMN          32
#define DUMMY_MAX_END_COLUMN            512
#define DUMMY_RECT_MINWIDTH             32
#define DUMMY_CORRECT_CONSTANT          2000

// Custom wireless network card type
// 自定义无线网卡类型
#ifndef MIB_IF_TYPE_ETHERNET_WIRELESS
#define MIB_IF_TYPE_ETHERNET_WIRELESS 71
#endif

#ifndef MAX_IP_LEN
#define MAX_IP_LEN 16
#endif

// Notice:PCIe Register Address
// Base Addr
#define PCIE_BAR_BASE_ADDR       0x30000
// BAR Addr
#define BAR_FIRWARE_VERSION_ADDR 0x0
#define BAR_FIRWARE_YEAR_ADDR    0x1
#define BAR_FIRWARE_MONTH_ADDR   0x2
#define BAR_FIRWARE_DAY_ADDR     0x3
#define BAR_EVENT_ADDR           0x4
#define BAR_IMAGE_WIDTH_ADDR     0x5
#define BAR_IMAGE_HEIGHT_ADDR    0x6
#define BAR_PCIE_VERSION_ADDR    0x8    // PCIE Version Read Addr
#define BAR_PCIE_DEVICEID_ADDR   0x9    // PCIE device id write Addr
#define BAR_WRITE_ADDR           0x1000
#define BAR_READ_ADDR            0x2000
// DDR Addr
#define DDR_BASE_STEP            0x2000000
#define DDR_ROM0_ADDR            0x0
// DDR PAGE
#define DDR_ROM1_ADDR            0x4000000
#define DDR_ROM2_ADDR            0x8000000

// defectDefault threshold of bad points
// defect 坏点默认阈值
#define BADPOINT_MAX_THRESHOLD   10000

// dummy Default parameters
// dummy 默认参数
#define DUMMY_CORRECT_START_COL   10
#define DUMMY_CORRECT_END_COL     40
#define DUMMY_CORRECT_CONSTANT    2000

// 20220308
#define PREVIEW_IMAGE_CORRECTION 1

// 20220728
#define AUTO_CONNECT_DETECTOR_ENABLE 0
#define DETECTOR_TYPE_NUMBER 0x23

typedef enum
{
	PCIE_WR_BAR = 0,
	PCIE_WR_DDR
}ePCIE_WR_TYPE;

struct FpdPixelMatrixTable
{
	unsigned int fpd_num;
	unsigned int fpd_width;
	unsigned int fpd_height;
	unsigned int fpd_packet_size;
	unsigned int fpd_jumbo_size;
	unsigned int fpd_pitch;
};

// Notice:Fpd Pixel Matrix Table
// jumbo packet 8192kb,and normal 1024kb
static const FpdPixelMatrixTable HB_FPD_SIZE[] = {	
	{ 1, 3072, 3072, 18432, 2304, 140},/*,"4343-140um"*/             
	{ 2, 2560, 3072, 15360, 1920, 140},/*,"3543-140um"*/             
	{ 3, 1248, 1024, 2496,  312,  125},/*,"1613-125um"*/   
	{ 4, 2048, 2048, 8192,  1024, 140},/*,"3030-140um"*/             
	{ 5, 2816, 3584, 19712, 2464, 85 },/*,"2530-85um"*/              
	{ 6, 2048, 1792, 7168,  896 , 140},/*,"3025-140um"*/             
	{ 7, 4288, 4288, 35912, 4489, 100},/*,"4343-100um"*/             
	{ 8, 3072, 3840, 23040, 2880, 75 },/*,"2530-75um"*/
	{ 9, 1024, 1024, 2048,  256,  200},/*,"2121-200um"*/             
	{10, 2784, 2400, 13050, 1632, 50 },/*,"CMOS-1412-50um"*/         
	{11, 1056, 1200, 2475,  310,  50 },/*,"CMOS-0606-50um"*/  
	{12, 1536, 1536, 4608,  576,  148},/*,"IGZO-2323-148um"*/        
	{13, 2432, 3072, 14592, 1824, 100},/*,"2530-100um"*/             
	{14, 2048, 2048, 8192,  1024, 148},/*,"3030-148um"*/         
	{15, 3534, 4302, 29694, 3712, 100},/*,"3543-100um"*/
	{16, 1536, 1536, 4608,  576,  105},/*,"1616-105um"*/
	{17, 3072, 7680, 46080, 5760, 140},/*,"43100-140um"*/            
	{18, 1070, 1200, 2508,  314,  100},/*,"CMOS-1515-100um"*/        	      
	{19, 2063, 2049, 8257,  1033, 100},/*,"CMOS-2121-100um"*/   
	{20, 2331, 76,   347,   44,   100},/*,"CMOS-2301-100um"*/
	{21, 3095, 3073, 18577, 2323, 100},/*,"CMOS-3131-100um"*/
	{22, 1536, 1386, 4158,  520,  120},/*,"1917-120um"*/
	{23, 3072, 3072, 18432, 2304, 99 },/*,"3030-99um"*/
	{24, 2048, 2048, 8192,  1024, 75 },/*,"1515-75um"*/
	{25, 3488, 4356, 29676, 3710, 70 },/*,"2530-70um"*/
	{26, 3584, 3072, 21504, 2688, 85 },/*,"2530-85um-custom"*/
	{27, 3072, 1024, 6144,  768,  140},/*,"4315-140um"*/
	{28, 1536, 1536, 4608,  576,  85 },/*,"1313-85um"*/
	{29, 2048, 2048, 8192,  1024, 140},/*,"3030WOA-140um"*/
	{30, 3072, 3072, 18432, 2304, 140},/*,"4343WOA-140um"*/
	{31, 2048, 2048, 8192,  1024, 140},/*,"1818-140um"*/
	{32, 1536, 1536, 4608,  576,  111 },/*,"1717-111um"*/
	{33, 4302, 4302, 36147, 4519, 100},/*,"4343-100um"*/
	{34, 2400, 3032, 14213, 1771, 100},/*,"2530BOE-100um"*/
	{35, 2400, 3032, 14213, 1771, 100} /*,"2530CMO-100um"*/
};

// modified by mhyang 20231218
static const unsigned short pciedeviceidTable[PCIE_DEVID_MAX_NUMBER] = { 7024, 7028, 7031, 7032, 7034, 7038 };

// Notice:
typedef struct HBI_HANDLE_ST 
{
	HBI_HANDLE_ST()
	{
		_handle = NULL;
		_nfpdid = 0;
	}
	void *_handle;              // Device handle #设备对应句柄
	int  _nfpdid;               // Device ID     #设备ID
}HBI_HANDLE;

//Note:fpd communication Type
typedef enum
{
	UDP_COMM_TYPE = 0,
	UDP_JUMBO_COMM_TYPE,
	PCIE_COMM_TYPE,
	WALN_COMM_TYPE
}FPD_COMM_TYPE;

//Note:sofeware work mode invalid
typedef enum
{
	WORKMODE_PRODUCTION_MODE = 0x00, // Software production mode #软件生产模式
	WORKMODE_DEBUG_MODE = 0x01,      // Software debugging mode #软件调试模式
	WORKMODE_UNKNOWN
}eSOFTWARE_WORKMODE;

typedef enum
{
	IMAGE_FILE    = 0,
	TEMPLATE_FILE = 1,
	TEMPLATE_IMG  = 2
}HBI_FILE_TYPE;

typedef struct CommCfgSt
{
	CommCfgSt()
	{
		_type = FPD_COMM_TYPE::PCIE_COMM_TYPE;
		_remoteip[0] = '\0';
		_localip[0]  = '\0';
		_loacalPort  = 32986;
		_remotePort  = 32987;
		_pcieId      = 7024;
	}

	FPD_COMM_TYPE _type;
	// The network interface communication needs to be set, and PCIe needs to be set as long as the type
	// 网口通讯需要设置,PCIe只要设置类型即可
	char _remoteip[MAX_IP_LEN];
	char _localip[MAX_IP_LEN];
	unsigned short _loacalPort;
	unsigned short _remotePort;
	unsigned short _pcieId; // PCIe device ID #PCIe设备ID
}COMM_CFG;

//
typedef struct HbiDllInstanceSt
{
	HbiDllInstanceSt()
	{
		m_fpdId   = 0;
		m_fpdHand = NULL;
		m_commCfg._type        = FPD_COMM_TYPE::PCIE_COMM_TYPE;
		m_commCfg._remoteip[0] = '\0';
		m_commCfg._localip[0]  = '\0';
		m_commCfg._loacalPort  = 32986;
		m_commCfg._remotePort  = 32987;
		m_commCfg._pcieId      = 7024;
		m_bConnected = false;
	}
	//
	unsigned int m_fpdId;  // Flat panel detector's id                 # 平板id
	void* m_fpdHand;       // Flat panel detector's instance handle    # 平板实例句柄
	COMM_CFG m_commCfg;    // Flat panel detector's communication mode # 平板通讯方式
	bool m_bConnected;     // Flat panel detector's status             # 平板连接状态
}HBI_DLL_INSTANCE;

// Notice: Image Property
typedef struct ImageProperty
{
	ImageProperty()
	{
		nFpdNum        = 0;
		nwidth         = 0;
		nheight        = 0;
		datatype       = 0;
		ndatabit       = 0;
		nendian        = 0;
		packet_size    = 0;
		frame_size     = 0;
		tailPacketSize = 0;
		frame_number   = 2;
	}
	unsigned int nFpdNum;				//fpd Number
	unsigned int nwidth;			    //image width
	unsigned int nheight;			    //image height
	unsigned int datatype;			    //data type：0-unsigned char,1-char,2-unsigned short,3-float,4-double
	unsigned int ndatabit;			    //data bit:0-16bit,1-14bit,2-12bit,3-8bit
	unsigned int nendian;			    //endian:0-little endian,1-biger endian
	// 
	unsigned int packet_size;           //fpd send the number of packet per frame
	unsigned int frame_size;            //data size per frame
	unsigned int tailPacketSize;        //Tail packet size
	unsigned int frame_number;          //The number of buffer capacity,Integer,[2~32],buff_szie=frame_size * ncapacity
}IMAGE_PROPERTY;

// Notice:Fpd Trigger Work Mode
typedef enum
{
	INVALID_TRIGGER_MODE         = 0x00,// invalid trigger type                               #  无效模式
	STATIC_SOFTWARE_TRIGGER_MODE = 0x01,// Static: Software trigger mode,                     #  静态采集-软触发模式
	STATIC_PREP_TRIGGER_MODE     = 0x02,// Static: Reserved,                                  #  静态采集-软触发模式
	STATIC_HVG_TRIGGER_MODE      = 0x03,// Static: High voltage generator trigger mode,       #  静态采集-高压触发模式
	STATIC_AED_TRIGGER_MODE      = 0x04,// Static: AED trigger mode,                          #  静态采集-FreeAED触发模式
	DYNAMIC_HVG_TRIGGER_MODE     = 0x05,// Dynamic: High voltage generator pulse sync in mode.#  动态采集-高压同步模式
	DYNAMIC_FPD_TRIGGER_MODE     = 0x06,// Dynamic: Flat panel detector pulse sync out mode,  #  动态采集-FPD同步模式
	DYNAMIC_FPD_CONTINUE_MODE    = 0x07,// Dynamic: Free run mode (Software trigger mode),    #  动态采集-FPD Conitnue模式
	DYNAMIC_FPD_SAEC_MODE        = 0x08 // 08-Static:SAECMode;
}EnumTRIGGER_MODE;

// Notice:fpd software calibrate enable info.
typedef struct image_correct_enable_st
{
	image_correct_enable_st()
	{
		bFeedbackCfg = false;
		ucOffsetCorrection = 0;
		ucGainCorrection = 0;
		ucDefectCorrection = 0;
		ucDummyCorrection = 0;
	};
	bool bFeedbackCfg;                  //true-ECALLBACK_TYPE_ROM_UPLOAD Event,false-ECALLBACK_TYPE_SET_CFG_OK Event
	unsigned char ucOffsetCorrection;   //0-No Offset Correction;1-Software PreOffset Correction;2-Firmware PostOffset Correction;3-Firmware PreOffset Correction;
	unsigned char ucGainCorrection;	    //0-No Gain Correction;1-Software Gain Correction;2-Firmware Gain Correction;
	unsigned char ucDefectCorrection;   //0-No Defect Correction;1-Software Defect Correction;2-Firmware Defect Correction;
	unsigned char ucDummyCorrection;    //0-No Dummy Correction;1-Software Dummy Correction;2-Firmware Dummy Correction;
}IMAGE_CORRECT_ENABLE;

// Notice:fpd software calibrate enable info.
typedef struct software_calibrate_enable_st
{
	software_calibrate_enable_st()
	{
		enableOffsetCalib = false;
		enableGainCalib   = false;
		enableDefectCalib = false;
		enableDummyCalib  = false;
	};
	bool enableOffsetCalib;
	bool enableGainCalib;
	bool enableDefectCalib;
	bool enableDummyCalib;
}SOFTWARE_CALIBRATE_ENABLE;

// Notice:fpd software calibrate enable info.
typedef struct detector_init_param_st
{
	detector_init_param_st()
	{
		_ucmodeid           = 0;
		_uctriggerType      = 7;
		_ucOffsetCorrection = 3;
		_ucGainCorrection   = 2;
		_ucDefectCorrection = 2;
		_ucDummyCorrection  = 0;
	};
	unsigned char  _ucmodeid;            //mode id [0,9];
	unsigned char  _uctriggerType;       //1-soft trigger，3-hvg trigger,4-FreeAED,5-Dynamic:Hvg Sync Mode,6-Dynamic:Fpd Sync Mode,7-Dynamic:Fpd Continue,8-SAEC;
										 //Only vaild in Binning mode,invaild in multi-mode mode,purpose compatible with diffrent version.
	unsigned char _ucOffsetCorrection;   //0-No Offset Correction;1-Software PreOffset Correction;2-Firmware PostOffset Correction;3-Firmware PreOffset Correction;
	unsigned char _ucGainCorrection;	 //0-No Gain Correction;1-Software Gain Correction;2-Firmware Gain Correction;
	unsigned char _ucDefectCorrection;   //0-No Defect Correction;1-Software Defect Correction;2-Firmware Defect Correction;
	unsigned char _ucDummyCorrection;    //0-No Dummy Correction;1-Software Dummy Correction;2-Firmware Dummy Correction;
}DETECTOR_INIT_PARAMS;

// Notice: Live Acquisition'properties 
typedef enum
{
	PREOFFSET_IMAGE   = 0x01,       // preoffset template and image
	ONLY_IMAGE        = 0x02,       // only image
	ONLY_PREOFFSET    = 0x03,       // only preoffset template
	OVERLAY_16BIT_IMG = 0x04,       // overlap result is 16bit'image
	OVERLAY_32BIT_IMG = 0x05        // overlap result is 32bit'image
}EnumLIVE_ACQUISITION;

// Notice: acq command:static and dynamic
typedef enum
{
	SINGLE_ACQ_DEFAULT_TYPE = 0x00, // Single acquisition #默认单帧采集
	LIVE_ACQ_DEFAULT_TYPE,          // Live acquisition   #默认连续采集
	// Multi step generation method #分布生成矫正模板，用于验证模板
	LIVE_ACQ_OFFSET_IMAGE,          // Generate offset template with dynamic live acquisition. #创建Offset模板-连续采集暗场图
	SINGLE_ACQ_GAIN_IMAGE,          // Generate gain template with static single acquisition.  #创建Gain模板-单帧采集亮场图	
	LIVE_ACQ_GAIN_IMAGE,            // Generate gain template with dynamic live acquisition.   #创建Gain模板-连续采集亮场图
	SINGLE_ACQ_DEFECT_IMAGE,        // Generate defect template with static single acquisition.#创建Defect模板-单帧采集亮场图
	LIVE_ACQ_DEFECT_IMAGE,          // Generate defect template with dynamic live acquisition. #创建Defect模板-连续采集亮场图
	// One-key quick generation mode #快速生成矫正模板(连续采集)，用于系统集成
	OFFSET_TEMPLATE_TYPE,           // Quick generation process of offset template-Firmware generation offset template #连续采集一组暗场图并生成offset模板，固件生成模板
	GAIN_TEMPLATE_TYPE,             // Quick generation process of gain template with 1 group image #连续采集一组亮场图并生成gain模板
	DEFECT_TEMPLATE_GROUP1,         // Quick generation process of defect template - group1. #连续采集一组亮场图 - defect group1
	DEFECT_TEMPLATE_GROUP2,         // Quick generation process of defect template - group2. #连续采集一组亮场图 - defect group2
	DEFECT_TEMPLATE_GROUP3,         // Quick generation process of defect template - group3. #连续采集一组亮场图 - defect group3 and generate template
	SOFTWARE_OFFSET_TEMPLATE,       // Quick generation process of offset template - SDK generates offset template #快速生成模板采集类型,连续采集一组暗场图 - SDK生成offset模板
	// Quickly generate correction template (single frame acquisition) #快速生成矫正模板(单帧采集)，用于系统集成，20220801
	SINGLE_GENERATE_OFFSET,         // collect a group of dark field images in a single frame - SDK generates offset template #单帧采集1组暗场图 - SDK生成offset模板
	SINGLE_GENERATE_GAIN,           // collect N groups of single frame (at least 3 groups by default) bright field images - SDK generates gain templates #单帧采集N组(默认至少3组)亮场图 - SDK生成gain模板
	SINGLE_GENERATE_DEFECT,         // collect N groups of single frame (at least 3 groups by default) bright field images - SDK generates defect templates #单帧采集N组(默认至少3组)亮场图 - SDK生成defect模板
	SINGLE_GENERATE_GAIN_DEFECT     // collect N groups of single frame (at least 3 groups by default) bright field images - SDK generates gain and defect templates #单帧采集N组(默认至少3组)亮场图 - SDK生成gain和defect模板
}EnumIMAGE_ACQ_CMD;

// Notice:fpd aqc mode
typedef struct fpd_aqc_mode_st
{
	fpd_aqc_mode_st()
	{
		eAqccmd    = SINGLE_ACQ_DEFAULT_TYPE;// default single shot type
		eLivetype  = ONLY_IMAGE;             // only image
		ngroupno   = 0;
		nAcqnumber = 0;
		ndiscard   = 0;
		nframeid   = 0;
	};
	EnumIMAGE_ACQ_CMD eAqccmd;      // Acquisition command #采集命令
	EnumLIVE_ACQUISITION eLivetype; // It is only limited to the detailed tasks of continuous acquisition (LIVE_ACQ_DEFAULT_TYPE). 1-The firmware makes the offset template and the figure above; 2-upper picture; 3-The firmware is only used as an offset template;								
	                                // 只限于连续采集(LIVE_ACQ_DEFAULT_TYPE)详细任务, 1-固件做offset模板并上图；2-只上图；3-固件做只做offset模板;
									// 4-Overlay: upload 16-bit average image; 5-Overlay: upload 32-bit accumulative sum image
									// 4-叠加：上传16位均图；5-叠加：上传32位均图；
	int  ngroupno;                  // group indication          #组号
	int  nAcqnumber;                // Actual acquisition frames #实际采集帧数
	int  ndiscard;                  // Number of frames ignored  #忽略的帧数
	int  nframeid;                  // counter                   #计数器
}FPD_AQC_MODE;

// Notice: upload template file
// template file type
typedef enum
{
	OFFSET_TMP = 0x00,
	GAIN_TMP   = 0x01,
	DEFECT_TMP = 0x02
}emUPLOAD_FILE_TYPE;

// template file attributes
typedef struct hbi_file_attr_st
{
	hbi_file_attr_st()
	{
		nModeID      = 0;
		emfiletype   = GAIN_TMP;
		nBinningtype = -1;
		filepath[0]  = '\0';
	};
	int nModeID;
	emUPLOAD_FILE_TYPE emfiletype;
	int nBinningtype;
	char filepath[MAX_PATH];
}DOWNLOAD_FILE;

// template file linked list
typedef struct download_mode_st
{
	download_mode_st()
	{
		pFileAttr = NULL;
		next = NULL;
	}
	DOWNLOAD_FILE *pFileAttr;
	struct download_mode_st* next;
}DOWNLOAD_MODE;


// Notice:template file type
typedef enum
{
	OFFSET_TEMPLATE = 0x00,
	GAIN_TEMPLATE   = 0x01,
	DEFECT_TEMPLATE = 0x02
}emTEMPLATE_TYPE;

// template file attributes
typedef struct download_mode_file_st
{
	download_mode_file_st()
	{
		m_byModeId        = 0;	
		m_byBinningType   = 1;
		m_byPgaLevel      = 6;
		m_unFpsTime       = 0;
		m_usKvValue       = 0;
		m_sZoomRectLeft   = 0;
		m_sZoomRectTop    = 0;
		m_sZoomRectWidth  = 0;
		m_sZoomRectHeight = 0;
		emtemplatetype    = GAIN_TEMPLATE;
	};
	unsigned char	m_byModeId;
	unsigned char	m_byBinningType;
	unsigned char 	m_byPgaLevel;
	unsigned int 	m_unFpsTime;
	unsigned short 	m_usKvValue;
	unsigned short	m_sZoomRectLeft;
	unsigned short	m_sZoomRectTop;
	unsigned short	m_sZoomRectWidth;
	unsigned short	m_sZoomRectHeight;
	emTEMPLATE_TYPE emtemplatetype;
}DOWNLOAD_MODE_FILE;

// upload process status
typedef enum {
	UPLOAD_FILE_START = 0x00,
	UPLOAD_FILE_DURING = 0x01,
	UPLOAD_FILE_STOP
}emUPLOAD_FILE_STATUS;

// Notice: After Each Member Variables, show Variables enum , 
// before '-' is variables' value, after '-' is the meaning of the value;
// each value departed by ';' symbol
enum eRequestCommType
{
	EDL_COMMON_TYPE_INVALVE               = 0X00,
	EDL_COMMON_TYPE_GLOBAL_RESET          = 0X01,
	EDL_COMMON_TYPE_PREPARE               = 0X02,
	EDL_COMMON_TYPE_SINGLE_SHORT          = 0X03,
	EDL_COMMON_TYPE_LIVE_ACQ              = 0X04,
	EDL_COMMON_TYPE_STOP_ACQ              = 0X05,
	EDL_COMMON_TYPE_PACKET_MISS           = 0X06,
	EDL_COMMON_TYPE_FRAME_MISS            = 0X07,
	EDL_COMMON_TYPE_DUMMPING              = 0X08,
	EDL_COMMON_TYPE_FPD_STATUS            = 0X09,
	EDL_COMMON_TYPE_END_RESPONSE          = 0X0A, // End response packet
	EDL_COMMON_TYPE_CONNECT_FPD           = 0X0B, // connect fpd
	EDL_COMMON_TYPE_DOWNLOAD_IMAGE        = 0X0C, // wireless download image
	EDL_COMMON_TYPE_SLEEP_STATE           = 0X0D, // wireless set sleep state
	EDL_COMMON_TYPE_WAKE_UP               = 0X0E, // wireless wake up
	EDL_COMMON_TYPE_DISCONNECT_FPD        = 0X0F, // disconnect fpd
	EDL_COMMON_TYPE_SET_RAM_PARAM_CFG     = 0X10,
	EDL_COMMON_TYPE_GET_RAM_PARAM_CFG     = 0X11,
	EDL_COMMON_TYPE_SET_ROM_PARAM_CFG     = 0X12,
	EDL_COMMON_TYPE_GET_ROM_PARAM_CFG     = 0X13,
	EDL_COMMON_TYPE_SET_FACTORY_PARAM_CFG = 0X14,
	EDL_COMMON_TYPE_GET_FACTORY_PARAM_CFG = 0X15,
	EDL_COMMON_TYPE_RESET_FIRM_PARAM_CFG  = 0X16,
	EDL_COMMON_TYPE_WRITE_USER_DDR        = 0X17,// add by MH.YANG 11/7/2022
	EDL_COMMON_TYPE_READ_USER_DDR         = 0X18,// add by MH.YANG
	EDL_COMMON_TYPE_FIRMWARE_RESET        = 0X1A,// add by MH.YANG 20231115
	EDL_COMMON_FIRMWARE_OFFSET_TEMPLATE   = 0X1B,// add by MH.YANG 20240726
	EDL_COMMON_FIRMWARE_GAIN_TEMPLATE     = 0X1C,
	EDL_COMMON_FIRMWARE_DEFECT_TEMPLATE   = 0X1D,
	EDL_COMMON_UPLOAD_OFFSET_TEMPLATE     = 0X2E,// Upload offset templateadd by MH.YANG 30/12
	EDL_COMMON_UPLOAD_GAIN_TEMPLATE       = 0X2F,// Upload gain template
	EDL_COMMON_UPLOAD_DEFECT_TEMPLATE     = 0X30,// Upload defect template
	EDL_COMMON_PRIMARY_AUTHOR             = 0X31,//
	EDL_COMMON_SECONDARY_AUTHOR           = 0X32,//
	EDL_COMMON_WLAN_UPDATE_NETWORK        = 0X33,// add by MH.YANG 22/8/2022
	EDL_COMMON_WLAN_UPDATE_SECURITY_KEY   = 0X34,// add by MH.YANG 22/8/2022
	EDL_COMMON_WLAN_GET_NETWORK           = 0X35,// add by MH.YANG 22/8/2025
	EDL_COMMON_UPDATE_PCIE_INIT           = 0X4A,// Update PCIe software Init
	EDL_COMMON_UPDATE_PCIE_VERSION        = 0X4B,// Update PCIe software File
	EDL_COMMON_RESTART_PCIE_DEVICE        = 0X4C,// Restart PCIe device
	EDL_COMMON_ERASE_FIRMWARE             = 0X4F,// Erase firmware package request
	EDL_COMMON_UPDATE_FIRMWARE            = 0X50,// Update firmware package request
	EDL_COMMON_SSGT_ENABLE_MODE           = 0XFA,// SSGT update Mode 20220811
	EDL_COMMON_SSGT_DISABLE_MODE          = 0XFB,// SSGT update Mode 20220811
	EDL_COMMON_UPDATE_EMBEDDED_INIT       = 0XFC,// Update embedded software Init
	EDL_COMMON_UPDATE_EMBEDDED_SOFTWARE   = 0XFD,// Update embedded software File
	EDL_COMMON_TYPE_SWITCH_WALN_MODE      = 0XFE,// Switch wlan ap/client Mode and 2.4G/5G net Type
	EDL_COMMON_TYPE_SET_AQC_MODE          = 0XFF
};

// Notice: After Each Member Variables, show Variables enum , 
// before '-' is variables' value, after '-' is the meaning of the value;
// each value departed by ';' symbol
enum eCallbackEventCommType
{
	ECALLBACK_TYPE_INVALVE             = 0X00, // invalid 
	ECALLBACK_TYPE_COMM_RIGHT          = 0X01, // not enabled 
	ECALLBACK_TYPE_COMM_WRONG          = 0X02, // not enabled
	ECALLBACK_TYPE_DUMMPLING           = 0X03, // not enabled
	ECALLBACK_TYPE_ACQ_END             = 0X04, // not enabled
	ECALLBACK_TYPE_UPDATE_FIRMWARE     = 0X06, // Update old firmware package answer 
	ECALLBACK_TYPE_ERASE_FIRMWARE      = 0X07, // Update new firmware package answer
	ECALLBACK_TYPE_FPD_STATUS          = 0X09, // Status package
	ECALLBACK_TYPE_ROM_UPLOAD          = 0X10, // Flat panel detector's firmware parameter #平板固件参数
	ECALLBACK_TYPE_RAM_UPLOAD          = 0X11, // Flat panel detector's firmware parameters 
	ECALLBACK_TYPE_FACTORY_UPLOAD      = 0X12, // Flat panel detector's firmware parameters
	ECALLBACK_TYPE_WLAN_BATTERY        = 0X13, // Wireless battery message
	ECALLBACK_TYPE_BUFFER_WARNING      = 0X14, // buffer wornning
	ECALLBACK_TYPE_ILLEGAL_PACKAGE_NUM = 0X15, // illegal package number
	ECALLBACK_TYPE_USER_DDR_UPLOAD     = 0X17, // User DDR update message #用户ddr反馈 
	ECALLBACK_TYPE_WIRELESS_NETWORK    = 0X18, // wireless network configuration BY MHYANG 20220825
	ECALLBACK_TYPE_WIRELESS_SETKEYOK   = 0X19, // wireless network configuration BY MHYANG 20220827
	ECALLBACK_FIRMWARE_TEMPLATE_STATUS = 0X20, // Firmware template status BY MHYANG 20230627
	ECALLBACK_TYPE_SINGLE_IMAGE        = 0X51, // Feedback single frame image
	ECALLBACK_TYPE_MULTIPLE_IMAGE      = 0X52, // Feedback continuous image
	ECALLBACK_TYPE_PREVIEW_IMAGE       = 0X53, // Feedback preview image
	ECALLBACK_TYPE_LIVE_IMAGE          = 0X54, // new,panel 1917 20231031
	ECALLBACK_TYPE_PACKET_MISS         = 0X5B, // Packet loss message
	ECALLBACK_TYPE_OFFSET_TMP          = 0X5C, // Feedback offset template
	ECALLBACK_OVERLAY_NUMBER           = 0X5D, // Overlay: counter
	ECALLBACK_OVERLAY_16BIT_IMAGE      = 0X5E, // Overlay: upload 16-bit average image; 
	ECALLBACK_OVERLAY_32BIT_IMAGE      = 0X5F, // Overlay: upload 32-bit accumulative sum image
	ECALLBACK_TYPE_8BIT_IMAGE          = 0X60, // Convert 16-bit original image to 8-bit bitmap #将16位原图转8位图
	ECALLBACK_TYPE_SENDTO_WIZARD       = 0XA0, // Message fed back to the template wizard
	ECALLBACK_TYPE_PACKET_MISS_MSG     = 0XA4, // Packet loss information
	ECALLBACK_TYPE_THREAD_EVENT        = 0XA5, // Processing thread messages
	ECALLBACK_TYPE_ACQ_DISCARDDED      = 0XA6, // Feedback frame abandonment information
	ECALLBACK_TYPE_OFFSET_ERR_MSG      = 0XA7, // Generate offset template message
	ECALLBACK_TYPE_GAIN_ERR_MSG        = 0XA8, // Generate gain template message
	ECALLBACK_TYPE_DEFECT_ERR_MSG      = 0XA9, // Generate defect template message
	ECALLBACK_TYPE_NET_ERR_MSG         = 0XAA, // Network exception message
	ECALLBACK_TYPE_SET_CFG_OK          = 0XAB, // Set parameters successfully  #设置参数成功
	ECALLBACK_TYPE_SAVE_SUCCESS        = 0XAC, // Image saved successfully #保存图像成功
	ECALLBACK_TYPE_GENERATE_TEMPLATE   = 0XAD, // Generate Template #生成模板
	ECALLBACK_TYPE_FILE_NOTEXIST       = 0XAE, // file does not exist #文件不存在
	ECALLBACK_TYPE_WORK_STATUS         = 0XAF  // Working status (production mode or test mode) #工作状态(生产模式或测试模式)
};

enum eCallbackUpdateFirmwareStatus
{
	ECALLBACK_UPDATE_STATUS_START     = 0XB0, // Start status or Erase Firmaware
	ECALLBACK_UPDATE_STATUS_PROGRESS  = 0XB1, // progress
	ECALLBACK_UPDATE_STATUS_RESULT    = 0XB2  // the Processing Result
};

enum eCallbackTemplateStatus
{
	ECALLBACK_TEMPLATE_BEGIN          = 0X00,
	ECALLBACK_TEMPLATE_INVALVE_PARAM  = 0X01,
	ECALLBACK_TEMPLATE_MALLOC_FAILED  = 0X02,
	ECALLBACK_TEMPLATE_SEND_FAILED    = 0X03,
	ECALLBACK_TEMPLATE_STATUS_ABORMAL = 0X04,
	ECALLBACK_TEMPLATE_FRAME_NUM      = 0X05,
	ECALLBACK_TEMPLATE_TIMEOUT        = 0X06,
	ECALLBACK_TEMPLATE_MEAN           = 0X07,
	ECALLBACK_TEMPLATE_GENERATE       = 0X08,
	ECALLBACK_TEMPLATE_RESULT         = 0X09
};

// Notice:SSGCT callbacke event status
// single step generate correction template （SSGCT）
typedef enum
{
	// SSGT-Mode Begin
	SSGCT_WORKING_MODE_BEGIN           = 0X00, // Enter the SSGT mode and initialize the Mode data model #进入SSGT模式，初始化Mode数据模型
	// Image acquisition process	   
	SSGCT_FEEDBACK_GRAY_MEAN           = 0X01, // Feedback the grayscale value of the current image, and save it after the user confirms that the grayscale value is appropriate, otherwise it will be ignored # 反馈当前图像的灰度值，用户确认灰度值合适后保存，否则忽略掉
	SSGCT_ACQUISITION_NEXT_IMAGE       = 0X02, // After the image is saved and inserted into the Mode data model, the next frame of image can be collected #图像完成保存并插入到Mode数据模型中，可以采集下一帧图像
	SSGCT_ACQUISITION_NEXT_GROUP       = 0X03, // Start collecting the next frame of image #开始采集下一帧图像
	SSGCT_ACQUISITION_COMPLETE         = 0X04, // After image acquisition and mode data model creation, template can be generated #采集图像完成，Mode数据模型创建完成，可以生成模板
	// generate template process	   
	SSGCT_GENERATE_TEMPLATE_START      = 0X05, // Template generation start
	SSGCT_GENERATE_TEMPLATE_ERROR      = 0X06, // Exception in build template
	SSGCT_GENERATE_TEMPLATE_CALCULAING = 0X07, // Template generation calculation in progress
	SSGCT_GENERATE_TEMPLATE_RESULT     = 0X08, // Template generation result (1-OFFSET,2-GAIN,3-DEFECT,4-GAIN AND DEFECT)
	// download template process	   
	SSGCT_DOWNLOAD_TEMPLATE_START      = 0X09, // download start
	SSGCT_DOWNLOAD_TEMPLATE_PROGRESS   = 0X0A, // progress
	SSGCT_DOWNLOAD_TEMPLATE_RESULT     = 0X0B, // the Processing Result
	// SSGT-Mode End				   
	SSGCT_WORKING_MODE_END             = 0X0C  // Exit the SSGT mode and destroy the Mode data model #退出SSGT模式，销毁Mode数据模型
}eSSGCTCallbackEvent;

// Notice:Image data type
typedef enum eHbiDataType
{
	EHBI_8UC1  = 0X01, // 1 - 8bits
	EHBI_16UC1 = 0X02, // 2 - 16bits
	EHBI_32FC1 = 0X04  // 4 - 32bits
}emHBI_DATA_TYPE;

// Notice:Image data object
// image buff size(8bits)  = uwidth * uheight;
// image buff size(16bits) = uwidth * uheight * 2;
// image buff size(32bits) = uwidth * uheight * 4;
typedef struct IMAGE_DATA_ST
{
	IMAGE_DATA_ST()
	{
		uwidth    = 3072;
		uheight   = 3072;
		ndatabits = emHBI_DATA_TYPE::EHBI_16UC1;
		uframeid  = 0;
		databuff  = NULL;
		datalen   = 0;
	}
	unsigned int uwidth;	   // image width
	unsigned int uheight;	   // image height
	emHBI_DATA_TYPE ndatabits; // data bit:1-8bit,2-16bit,4-32bit,default-16bit
	unsigned int uframeid;     // frame id
	void * databuff;           // buffer address
	unsigned int datalen;      // buffer size
}ImageData;

// Notice: call back function
// @USER_CALLBACK_HANDLE_ENVENT
// @pContext:position machine object point
// @ufpdId:detector id
// @byteEventid:enum eEventCallbackCommType
// @PVEventParam1:fpd config or image data buffer addr
// @nEventParam2:param2，eg,image data or detector status
// @nEventParam3:param3，eg,frame rate or status
// @nEventParam4:param4，eg,pcie event or Reserved or extended
typedef int(*USER_CALLBACK_HANDLE_ENVENT)(void *pContext, int ufpdId, unsigned char byteEventId, void *PVEventParam1, int nEventParam2, int nEventParam3, int nEventParam4);

// Notice: call back function
// @USER_CALLBACK_HANDLE_PROCESS
// @cmd:enum eAnswerCallbackCommType
// @retcode: return code
// @inContext: position machine object point
typedef int(*USER_CALLBACK_HANDLE_PROCESS)(unsigned char cmd, int retcode, void *inContext);

// Notice: call back function
// @SSGCT_CALLBACK_FUNC
// @inContext: position machine object point
// @cmd: enum eAnswerCallbackCommType
// @pvParam1:imgage gray value,image name
// @nParam2: group number
// @nParam3: Image serial number
typedef int(*SSGCT_CALLBACK_FUNC)(void* inContext, unsigned char cmd, void* pvParam1, int nParam2, int nParam3);

// Notice: calibrate template raw file call back info
typedef struct callback_raw_info_st
{
	callback_raw_info_st()
	{
		szRawName[0] = '\0';
		dMean = 0.0;
	};
	char   szRawName[MAX_PATH];	// Feedback data file name #返回数据文件名称
	double dMean;               // Gray mean               #灰度均值
}ECALLBACK_RAW_INFO;

// Notice:generate calibrate template input param
typedef struct calibrate_input_param_st {
	calibrate_input_param_st()
	{
		image_w       = 0;
		image_h       = 0;
		roi_x         = 0;
		roi_y         = 0;
		roi_w         = 0;
		roi_h         = 0;
		group_sum     = 0;
		per_group_num = 0;
	}
	int image_w;       // image width
	int image_h;       // image height
	int roi_x;         // ROI left
	int roi_y;         // ROI top
	int roi_w;         // ROI width
	int roi_h;         // ROI height
	int group_sum;     // group sum
	int per_group_num; // num per group
}CALIBRATE_INPUT_PARAM;

// Notice:PPD zoom param 
typedef struct fpd_zoom_rect_st
{
	unsigned int left; // zoom begin 
	unsigned int top;
	unsigned int width;
	unsigned int height;
}FPD_ZOOM_RECT;

// Notice:CMOS PPD Zoom rect param 
typedef struct coms_zoom_rect_st
{
	unsigned int uleft;    // left  #左
	unsigned int utop;     // top   #顶
	unsigned int uright;   // right #右 
	unsigned int ubottom;  // down  #下 
}CMOS_ZOOM_RECT;

// Notice: After Each Member Variables, show Variables enum , 
// before '-' is variables' value, after '-' is the meaning of the value;
// each value departed by ';' symbol
#pragma pack(1)

// Notice:wireless battery info
typedef struct battery_status_info_st
{
	battery_status_info_st()
	{
		usTemperature = 0;
		usHumidity = 0;
		uzStatusCmd = 0x12;
		uzPercentage = 0;
		uzBatteryStatus = 0;
		usRemainingTime = 0;
		uzIsWlan24G5G = 0;
		uzIsApClientMode = 0;
	}
	unsigned short usTemperature;       // Detector Temperature
	unsigned short usHumidity;          // Detector Humidity
	unsigned char uzStatusCmd;          // Battery Status command,default:0x12
	unsigned char uzPercentage;         // Battery Percentage,0~100
	unsigned char uzBatteryStatus;      // Battery Status,01-Battery charging,02-Battery powered,03-Power supply,04-Charging complete.
	unsigned short usRemainingTime;     // Remaining Charging Time,unit:minute
	unsigned char uzIsWlan24G5G;        // WLAN:1-2.4G,2-5G,others-Invalid
	unsigned char uzIsApClientMode;     // WLAN:AP Mode or Client Mode,1-AP Mode,2-Client Mode,others-Invalid
}BATTERY_STATUS_INFO;

// Notice:Rect of cutting images：9
typedef struct CuttingImageRect_st
{
	CuttingImageRect_st()
	{
		m_byCuttingEnable = 0;
		m_sCutRectLeft    = 0;
		m_sCutRectTop     = 0;
		m_sCutRectWidth   = 0;
		m_sCutRectHeight  = 0;
	}
	unsigned char  m_byCuttingEnable;	   //01 enbale or disable Cutting Image,1-enbale,非1-disable
	unsigned short m_sCutRectLeft;         //02 ROI rect left
	unsigned short m_sCutRectTop;          //02 ROI rect top
	unsigned short m_sCutRectWidth;        //02 ROI rect Width
	unsigned short m_sCutRectHeight;       //02 ROI rect Height
}CuttingImageRect;

// Notice:save image：4
typedef struct save_image_st
{
	save_image_st()
	{
		m_bySaveType     = 0;
		m_bySaveFileType = 1;
		m_bySaveNumber   = 1;
	}
	unsigned char  m_bySaveType;        //0-show image only,1-save image only,2-show and save image;
	unsigned char  m_bySaveFileType;    //0-image,1-video;
	unsigned short m_bySaveNumber;      //Number of images included in the video,》0
}SAVE_IMAGE;

// System Manufacture Information：100, Register_1~Register_100
typedef struct SysBaseInfo_st
{
	// Register_1 // 
	unsigned char   	m_byProductType;      				//01	//01-Flat Panel Detector;  02-Linear Array Detector
	// Register_2
	unsigned char    	m_byXRaySensorType;   				//01	//01-Amorphous Silicon; 02-CMOS; 03-IGZO; 04-LTPS; 05-Amorphous Selenium
	// Register_3
	unsigned char    	m_byPanelSize;        				//01	//inch Panel Size,01-1717;02-1417;03-1414;04-1212;05-1012;06-0912;07-0910;08-0909;09-0506;10-0505;11-0503
	// Register_4
	unsigned char   	m_byPixelPitch;						//01	//0-Invalid;1-27um;2-50um;3-75um;4-85um;5-100um;6-125um;7-140um;8-148um;9-150um;10-180um;11-200um;12-205um;13-400um;14-1000um;15-70um;16-84um;17-99um;18-105um;19-111um;20-120um;
	// Register_5
	unsigned char       m_byPixelMatrix;     				//01	//01-"3072 x 3072"; 02-"2560 x 3072"; 03-"1248 x 1024"; 04-"2048 x 2048"; 05-"2816 x 3584"; 06-"2048 x 1792"; 
																	//07-"4288 x 4288"; 08-"3072 x 3840"; 09-"1024 x 1024"; 10-"2784 x 2400"; 11-"1056 x 1200"; 12-"1536 x 1536"; 
																	//13-"2432 x 3072"; 14-"2048 x 2048"; 15-"3534 x 4302"; 16-"1536 x 1536"; 17-"3072 x 7680"; 18-"1070 x 1200";
																	//19-"2063 x 2049"; 20-"2331 x 76"; 21-"3095 x 3073"; 
	// Register_6
	unsigned char       m_byScintillatorType;				//01	//01-DRZ Standard; 02-DRZ High; 03-DRZ Plus; 04-PI-200; 05-Structured GOS; 06-CSI Evaporation; 07-CSI Attach;
	// Register_7
	unsigned char       m_byScanLineFanoutMode;				//01	//01-Single Side Single Gate; 02-Dual Side Single Gate; 03-Single Side Dual Gate; 04-Dual Side Dual Gate;
	// Register_8
	unsigned char       m_byReadoutLineFanoutMode;  		//01	//01-Single Side Single Read; 02-Dual Side Single Read; 03-Single Side Dual Read; 04-Dual Side Dual Read;
	// Register_9
	unsigned char       m_byFPDMode;						//01	//01-Static; 02-Dynamic;
	// Register_10
	unsigned char       m_byFPDStyle;						//01	//01-Fixed; 02-Wireless; 03-Portable;
	// Register_11
	unsigned char       m_byApplicationMode;        		//01	//01-Medical; 02-Industry;
	// Register_12
	unsigned char       m_byGateChannel;					//01	//01-"600 Channel"; 02-"512 Channel"; 03-"384 Channel"; 04-"256 Channel"; 05-"128 Channel"; 06-"064 Channel"
	// Register_13
	unsigned char		m_byGateType;						//01	//01-"HX8677"; 02-"HX8698D"; 03-"NT39565D"
	// Register_14
	unsigned char		m_byAFEChannel;						//01	//01-"64 Channel"; 02-"128 Channel"; 03-"256 Channel"; 04-"512 Channel";
	// Register_15
	unsigned char		m_byAFEType;						//01	//01-"AFE0064"; 02-"TI COF 2256"; 03-"AD8488"; 04-"ADI COF 2256";
	// Register_16~Register_17
	unsigned short		m_sGateNumber;						//02	//Gate Number
	// Register_18~Register_19
	unsigned short		m_sAFENumber;						//02	//AFE Number	
	// Register_20~Register_21
	unsigned short		m_sImageWidth;						//02	//Image Width // modified by mhyang 20191220
	// Register_22~Register_23
	unsigned short		m_sImageHeight;						//02	//Image Height
	// Register_24~Register_37
	unsigned char		m_cSnNumber[14];				    //14	//sn number   // modified by mhyang 20200401
	// Register_38~Register_46.
	CuttingImageRect    m_stCuttingRect;                    //09    //Rect Of Cutting Image;
	// Register_47~Register_53
	unsigned char       m_byAuthorizationEndTime1;          //01	//Authorization End Time1 // modified by mhyang 20211118
	unsigned char       m_byAuthorizationEndTime2;          //01	//Authorization End Time2
	unsigned char       m_byAuthorizationEndTime3;          //01	//Authorization End Time3
	unsigned char       m_byAuthorizationEndTime4;          //01	//Authorization End Time4
	unsigned char       m_byAuthorizationEndTime5;          //01	//Authorization End Time5
	unsigned char       m_byAuthorizationEndTime6;          //01	//Authorization End Time6
	unsigned char       m_byAuthorizationStatus;            //01	//Authorization Status
	// Register_54~Register_59
	unsigned char       m_byRtcTime1;						//01	//RTC Time1 // modified by mhyang 20211119
	unsigned char       m_byRtcTime2;						//01	//RTC Time2
	unsigned char       m_byRtcTime3;						//01	//RTC Time3
	unsigned char       m_byRtcTime4;						//01	//RTC Time4
	unsigned char       m_byRtcTime5;						//01	//RTC Time5
	unsigned char       m_byRtcTime6;						//01	//RTC Time6
	// Register_60~Register_66
	unsigned char       m_bySecondaryAuthorEndTime1;        //01	//Secondary Authorization End Time1 // modified by mhyang 20220209
	unsigned char       m_bySecondaryAuthorEndTime2;        //01	//Secondary Authorization End Time2
	unsigned char       m_bySecondaryAuthorEndTime3;        //01	//Secondary Authorization End Time3
	unsigned char       m_bySecondaryAuthorEndTime4;        //01	//Secondary Authorization End Time4
	unsigned char       m_bySecondaryAuthorEndTime5;        //01	//Secondary Authorization End Time5
	unsigned char       m_bySecondaryAuthorEndTime6;        //01	//Secondary Authorization End Time6
	unsigned char       m_bySecondaryAuthorStatus;          //01	//Secondary Authorization Status
	// Register_67~Register_72
	unsigned char       m_bySecondaryRtcTime1;				//01	//Secondary RTC Time1 // modified by mhyang 20220209
	unsigned char       m_bySecondaryRtcTime2;				//01	//Secondary RTC Time2
	unsigned char       m_bySecondaryRtcTime3;				//01	//Secondary RTC Time3
	unsigned char       m_bySecondaryRtcTime4;				//01	//Secondary RTC Time4
	unsigned char       m_bySecondaryRtcTime5;				//01	//Secondary RTC Time5
	unsigned char       m_bySecondaryRtcTime6;				//01	//Secondary RTC Time6	
	// Register_73
	unsigned char		m_cMaxFps;				            //01	//max fps   // modified by mhyang 20220308
	// Register_74~75
	unsigned char		m_byRotate90;						//01	//00-"Do nothing";01-"Right Rotate 90°Enable";02-"Left Rotate 90°Enable";
	unsigned char		m_byToFlip;							//01	//00-"Do nothing";01-"Vertically mirror Enable";02-"Horizontal mirror Enable",03-firmware Vertically mirror,04-firmware Horizontal mirror;05-firmware Vertically mirror And Horizontal mirror;
	// Register76
	unsigned char		m_byBit8Image;						//01	//00-"bit16 image";01-"bit8 image"
	// Register77~Register80
	unsigned int 		m_unWindowWidth;					//04	//window width
	// Register81~Register84
	unsigned int 		m_unWindowLeveling;					//04	//window leveling
	// Register85~Register88
	SAVE_IMAGE          m_stSaveOption;                     //04    //save image; // add by mhyang 20220830
	// Register89
	unsigned char		m_byMtuUdpPacketType;				//01	//00-"udp";01-"udp jumbo";others-"udp"
	// Register_90~Register_97.
	unsigned char		m_szUserCode[8];                    //08	// user code information
	// Register_98
	unsigned char		m_byDataNoReTransmit;               //01	// data retransmission type,0:Re-Transmit Data,other:Prohibit data retransmission.
	// Register_99~Register_100.
	unsigned char		m_abyFirmManageTemplateType;	    //01    //0-binning type,1-Mode type,Firmware Management Template method
	// Register_100.
	unsigned char		m_abyModeItemNumber;				//01    //Number of valid Mode items，Supports up to 10 groups
}SysBaseInfo;

// System Manufacture Information：50, Register_151~Register_178
typedef struct SysManuInfo_st 
{
	// Register_1~Register_4                                //04 
	unsigned char       m_byHead[4];
	// Register_5~Register_20
	unsigned char       m_szProductCode[16];   			    //16	//Product code information
	// Register_21~Register_22
	unsigned char       m_byEmbeddedSoftwareVersion1;		//01	//Embedded Software Version
	unsigned char       m_byEmbeddedSoftwareVersion2;		//01	//Embedded Software Version
	// Register_23~Register_26
	unsigned char       m_byEmbeddedSoftwareBuildingTime1;	//01	//Embedded Software Building Time
	unsigned char       m_byEmbeddedSoftwareBuildingTime2;	//01	//Embedded Software Building Time
	unsigned char       m_byEmbeddedSoftwareBuildingTime3;	//01	//Embedded Software Building Time
	unsigned char       m_byEmbeddedSoftwareStatus;		    //01	//Embedded Software Status 0-release,1-rc,2-beta
	// Register_27~Register_28
	unsigned short      m_sMasterFPGAFirmwareVersion;		//02	//Master FPGA Firmware Version
	// Register_29~Register_31
	unsigned char       m_byMasterFPGAFirmwareBuildingTime1;//01	//Master FPGA Firmware Building Time1
	unsigned char       m_byMasterFPGAFirmwareBuildingTime2;//01	//Master FPGA Firmware Building Time2
	unsigned char       m_byMasterFPGAFirmwareBuildingTime3;//01	//Master FPGA Firmware Building Time3
	// Register_32~Register_34
	unsigned char       m_byMasterFPGAFirmwareStatus1;		//01	//Master FPGA Firmware Status1
	unsigned char       m_byMasterFPGAFirmwareStatus2;		//01	//Master FPGA Firmware Status2
	unsigned char       m_byMasterFPGAFirmwareStatus3;      //01	//Master FPGA Firmware Status3
	// Register_35~Register_36
	unsigned char		m_byMCUFirmwareVersion1;			//01	//MCU Firmware Version1
	unsigned char		m_byMCUFirmwareVersion2;			//01	//MCU Firmware Version2
	// Register_37~Register_40
	unsigned char		m_byMCUFirmwareBuildingTime1;		//01	//MCU Firmware Building Time1
	unsigned char		m_byMCUFirmwareBuildingTime2;		//01	//MCU Firmware Building Time2
	unsigned char       m_byMCUFirmwareBuildingTime3;		//01	//MCU Firmware Building Time3
	unsigned char       m_byMCUFirmwareStatus;				//01	//MCU Firmware Status
	// Register_41~Register_44
	unsigned char		m_szBatterySN[4];		            //04	//Battery Serial number
	// Register_45~Register_50
	unsigned char		m_abySysManuInfoReserved[6];        //6     //Reserved Size
}SysManuInfo;

// System Status Information：28, Register_151~Register_178
typedef struct SysStatusInfo_st 
{
	// Register_1~Register_4 // 
	unsigned char		m_byHead[4];
	// Register_5~Register_8
	unsigned int		m_unTemperature;					//04	//Temperature
	// Register_9~Register_12
	unsigned int 		m_unHumidity;						//04	//Humidity
	// Register_13~Register_16
	unsigned int 		m_unDose;							//04	//Dose
	// Register_17~Register_28
	unsigned char		m_abySysStatusInfoReserved[12];     //12    //Reserved Size
}SysStatusInfo;

// Gigabit Ethernet Information：40, Register_179~Register_218
typedef struct EtherInfo_st 
{
	// Register_1~Register_4 // 
	unsigned char       m_byHead[4];
	// Register_5~Register_10
	unsigned char       m_bySourceMAC[6];                   //06	//Source MAC
	// Register_11~Register_14
	unsigned char       m_bySourceIP[4];                    //04	//Source IP
	// Register_15~Register_16
	unsigned short		m_sSourceUDPPort;					//02	//Source UDP Port
	// Register_17~Register_22
	unsigned char       m_byDestMAC[6];                     //06	//Dest MAC
	// Register_23~Register_26
	unsigned char       m_byDestIP[4];                      //04	//Dest IP
	// Register_27~Register_28
	unsigned short		m_sDestUDPPort;						//02	//Dest UDP Port	
	// Register_29
	unsigned char       m_byWlanGHz;                        //01	//1-2.4GHz,2-5GHz
	// Register_30
	unsigned char       m_byWlanMode;                       //01	//1-AP Mode,2-Client Mode,3-Wired Mode;
	// Register_31~Register_40
	unsigned char		m_szSecurityKey[10];                //10	//Network Security Key ADD BY MHYANG 20220822
}EtherInfo;

// High Voltage Generator Interface Trigger Mode Information：21, Register_219~Register_239
typedef union signal_value
{
	struct
	{
		unsigned int  b0 : 1;
		unsigned int  b1 : 1;
		unsigned int  b2 : 1;
		unsigned int  b3 : 1;
		unsigned int  b4 : 1;
		unsigned int  b5 : 3;
	}BITE_VALUE;
	unsigned char unValue;
}UNION_SIGNAL_VALUE;
// 
typedef struct HiVolTriggerModeInfo_st
{
	// Register_1~Register_4  
	unsigned char		m_byHead[4];                //04	
	// Register_5 HVG_Trig Mode(03)			        
	unsigned char		m_byPolarityParam;          //01	bit4:NON,bit3:i_HVG_Request,bit2:i_HVG_Sync_In,bit1:o_FPD_Ready,bit0:o_FPD_Sync_Out
	unsigned char		m_byHVGRequestParam03;      //01	bit4:NON,bit3:i_HVG_Request,bit2:i_HVG_Sync_In,bit1:0,bit0:0
	unsigned char		m_byFPDReadyParam03;        //01	bit4:NON,bit3:0,bit2:0,bit1:o_FPD_Ready,bit0:o_FPD_Sync_Out
	// Register_8 HVG_SYNC Mode(05)			        
	unsigned char		m_byHVGRequestParam05;      //01	bit4:NON,bit3:i_HVG_Request,bit2:i_HVG_Sync_In,bit1:0,bit0:0
	unsigned char		m_byFPDReadyParam05;        //01	bit4:NON,bit3:0,bit2:0,bit1:o_FPD_Ready,bit0:o_FPD_Sync_Out
	// Register_10 FPD_SYNC Mode(06)		        
	unsigned char		m_byHVGRequestParam06;      //01	bit4:NON,bit3:i_HVG_Request,bit2:i_HVG_Sync_In,bit1:0,bit0:0
	unsigned char		m_byFPDReadyParam06;        //01	bit4:NON,bit3:0,bit2:0,bit1:o_FPD_Ready,bit0:o_FPD_Sync_Out
	// Register_12 FPD_SYNC Mode(08)		        
	unsigned char		m_byHVGRequestParam08;      //01	bit4:NON,bit3:i_HVG_Request,bit2:i_HVG_Sync_In,bit1:0,bit0:0
	unsigned char		m_byFPDReadyParam08;        //01	bit4:NON,bit3:0,bit2:0,bit1:o_FPD_Ready,bit0:o_FPD_Sync_Out
	unsigned char		m_byHVGExplore08;           //01	bit4:NON,bit3:i_HVG_Request,bit2:i_HVG_Sync_In,bit1:0,bit0:0
	// Register_15~Register_21
	unsigned char		m_byHiVolTriggerReserved[7];//07	
}HiVolTriggerModeInfo;

// System Configuration Information：128, Register_240~Register_367	
typedef struct SysCfgInfo_st 
{
	// Register_1~Register_4 
	unsigned char       m_byHead[4];
	// Register_5
	unsigned char       m_byTriggerMode;					//01	// 01-"Static: Software Mode" 02-"Static: Prep Mode"; 03-"Static Hvg Trigger Mode"; 
														    		// 04-Static AED Trigger Mode, 05-Dynamic Hvg In Mode,06-Dynamic Fpd Out Mode.
	// Register_6										    
	unsigned char 	    m_byPreviewModeTransmission;		//01	//00-"Just transmmit full resolution image matrix"; 01-"Just transmmit the preview image"; 
	                                                                //02-"Transmmit the preview image and full resolution image"
	// Register_7										    
	unsigned char 	    m_byPositiveVsNegativeIntegrate;	//01	//01-"Electron"; 02-"Hole" //04-"Static: Inner Trigger Mode"; 05-"Dynamic: Sync In Mode"; 06-"Dynamic: Sync Out Mode"
														    
	// Register_8										    
	unsigned char 	    m_byBinning;						//01	//01-"1x1"; 02-"2x2 Binning"; 03-"3x3 Binning"; 04-"4x4 Binning";
														    
	// Register_9										    
	unsigned char       m_byIntegratorCapacitorSelection;	//01	//01-"Integrator Capacitor PGA0"; 02-"Integrator Capacitor PGA1"; 03-"PGA2";......;08-"Integrator Capacitor PGA7"

	// Register_10
	unsigned char       m_byAmplifierGainSelection;			//01	//01-"Amplifier Gain AMP0"; 02-"Amplifier Gain AMP1"; 03-"Amplifier Gain AMP2"; 04-"Amplifier Gain AMP3";
	                                                                //05-"Amplifier Gain AMP4"; 06-"Amplifier Gain AMP5"; 07-"Amplifier Gain AMP6"; 08-"Amplifier Gain AMP7";
	// Register_11
	unsigned char		m_bySelfDumpingEnable;				//01	//01-"Enable Dumping Process"; 02-"Disable Dumping Process";
	// Register12
	unsigned char		m_bySelfDumpingProcessing;			//01	//01-"Clear Process for Dumping"; 02-"Acquisition Process for Dumping";

	// add 20190705
	unsigned char       m_byClearFrameNumber;               //01	//01-"Clear Frame"; 02-"Clear Frame";03-"Clear Frame";04-"Clear Frame";

	// Register_13
	unsigned char		m_byPDZ;							//01	//01-"Disable PDZ"; 02-"Enable PDZ"
	// Register_14
	unsigned char		m_byNAPZ;							//01	//01-"Disable NAPZ"; 02-"Enable NAPZ"
	// Register15
	unsigned char		m_byZoomMode;	                    //01	//00-"Normal"; 01-"mode1";02-"mode2";03-"mode3";04-"mode4";05-"mode5";06-"mode6";07-"mode7";08-"mode8";09-"mode9";   
	// Register_16~Register_17
	unsigned short		m_sZoomRectTop;				        //02	//Zoom Begin Row Number
	// Register_18~Register_19
	unsigned short		m_sZoomRectBottom;				    //02	//Zoom End Row Number
	// Register_20~Register_21
	unsigned short		m_sZoomRectLeft;			        //02	//Zoom Begin Column Number
	// Register_22~Register_23
	unsigned short		m_sZoomRectRight;				    //02	//Zoom End Column Number
	// Register24~Register27
	unsigned int 		m_unSelfDumpingSpanTime;			//04	//unit: ms
	// Register28~Register31
	unsigned int 		m_unContinuousAcquisitionSpanTime;	//04	//unit: ms
	// Register32~Register35
	unsigned int 		m_unPreDumpingDelayTime;			//04	//unit: ms
	// Register36~Register39
	unsigned int 		m_unPreAcquisitionDelayTime;		//04	//unit: ms
	// Register40~Register43
	unsigned int 		m_unPostDumpingDelayTime;			//04	//unit: ms
	// Register44~Register47
	unsigned int 		m_unPostAcquisitionDelayTime;		//04	//unit: ms
	// Register48~Register51
	unsigned int 		m_unSyncInExposureWindowTime;		//04	//unit: ms
	// Register52~Register55
	unsigned int 		m_unSyncOutExposureWindowTime;		//04	//unit: ms
	// Register56~Register59
	unsigned int 		m_unTFTOffToIntegrateOffTimeSpan;	//04	//unit: ms
	// Register_60~Register_63
	unsigned int 		m_unIntegrateTime;					//04	//unit: ms
	// Register_64~Register_67
	unsigned int 		m_unPreFrameDelay;					//04	//unit: ms
	// Register_68~Register_71
	unsigned int 		m_unPostFrameDelay;					//04	//unit: ms
	// Register_72~Register_75
	unsigned int 		m_unSaturation;					    //04	//unit: ms
	// Register_76~Register_79
	unsigned int 		m_unClipping;					    //04	//unit: ms
	// Register_80~Register_81
	unsigned short		m_sHvgRdyMode;				        //02	//0-"Edge Trigger"; !0-"[1~255]:Config Delay,unit:n*128 ms"
	// Register_82~Register_83
	unsigned short		m_sSaecPreRdyTm;				    //02	//SAEC pre ready time
	// Register_84~Register_85
	unsigned short		m_sSaecPostRdyTm;				    //02	//SAEC post ready time
	// Register_86
	unsigned char       m_byDefectThreshold;                //01	//Defect correction:Calculating the threshold of bad points
	// Register_87~Register_90
	unsigned int 		m_unSleepTime;                      //04	//energy conservation settings: 睡眠（Sleep,unit:minute）
	// Register_91~Register_94
	unsigned int 		m_unMaxBadPointtNumber;             //04	//defect template:Support the maximum threshold of bad points
	// Register_95~Register_96
	unsigned short		m_sPacketIntervalTm;				//02	//UDP packet Interval time,unit,um(微秒)
	// Register_97
	unsigned char		m_abyDTDIEnable;					//01	//DTDI：1-enable，others-disable
	// Register_98  
	unsigned char		m_abyDTDIIntegralSeries;            //01	//DTDI：Integral Series
	// Register_99
	unsigned char		m_abyDTDIdataType;                  //01	//DTDI：overlay data type，0-16bit Cumulative sum，1-32bit Cumulative sum
	// Register_100~Register_101
	unsigned short		m_sLiveAcquisitionMicrosecond;      //02	//m_sFpsMicrosecond microsecond,1ms = 1000us [0, 999]
	// Register_102
	unsigned char		m_abyFmOverlayNumber;               //01	// Number of firmware overlays [0,255]
	// Register_103
	unsigned char		m_abyPulseWidth;                    //01	// Pulse width required parameter
	// Register_104~Register_105
	unsigned short		m_sSelfDumpingMicrosecond;          //02	// microsecond,1ms = 1000us [0, 999]
	// Register_106~Register_120
	unsigned char		m_abySysCfgInfoReserved[15];        //15	//Reserved Data
	// Register_121~Register_122
	unsigned short		m_sGainb;					        //02	//offset
	// Register_123~Register_126
	unsigned short 		m_usVoltage;					    //02	//unit: kv
	unsigned short 		m_unMsOfDos;					    //02	//unit: ms
	// Register_127
	unsigned char       m_byExTriggerAcqType;               //01    //Reference EnumIMAGE_ACQ_CMD type
}SysCfgInfo;

// Image Calibration Configuration：20, Register_368~Register_387
typedef struct ImgCaliCfg_st 
{
	// Register_1~Register_4 
	unsigned char		m_byHead[4];                        //04    //head 
	// Register_5
	unsigned char		m_byOffsetCorrection;				//01	//00-"Do nothing";01-"Software Offset Correction"; 02-"Hardware Offset Correction";
	// Register_6
	unsigned char		m_byGainCorrection;					//01	//00-"Do nothing";01-"Software Gain Correction";   02-"Hardware Gain Correction"
	// Register_7
	unsigned char		m_byDefectCorrection;				//01	//00-"Do nothing";01-"Software Defect Correction"; 02-"Software Defect Correction"
	// Register_8
	unsigned char		m_byDummyCorrection;				//01	//00-"Do nothing";01-"Software Dummy Correction";  02-"Software Dummy Correction"
	// Register_9~Register_10
	unsigned short		m_sDummyStartCol;		            //02     //Dummy Area Start Column
	// Register_11~Register_12
	unsigned short		m_sDummyEndCol;		                //02     //Dummy Area End Column
	// Register_13~Register_14
	unsigned short		m_sDummyConstant;		            //02     //Dummy Constant
	//// Register_15~Register_20
	unsigned char		m_abyImgCaliCfgReserved[6];		    //06//////Registers 114(include) Are Reserved;
}ImgCaliCfg;

// Voltage Adjust Configuration：48, Register_388~Register_435
typedef struct VolAdjustCfg_st 
{
	// Register_1~Register_4
	unsigned char		m_byHead[4];
	// Register_5~Register_8
	unsigned int 		m_unVCOM;							//04	//VCOM
	// Register_9~Register_12
	unsigned int 		m_unVGG;							//04	//VGG
	// Register_13~Register_16
	unsigned int 		m_unVEE;							//04	//VEE
	// Register_17~Register_20
	unsigned int 		m_unVbias;							//04	//Vbias	
	// Register_21~Register_36
	unsigned int 		m_unComp[4];						//04	//Comp1	// Register114
	// Register_37~Register_48
	unsigned char		m_abyVolAdjustCfgReserved[12];		//12//////Registers 114(include) Are Reserved;
}VolAdjustCfg;

// TI COF Parameter Configuration：84, Register_436~Register_519
typedef struct TICOFCfg_st 
{
	// Register_1~Register_4  
	unsigned char		m_byHead[4];
	// Register_5~Register_66
	unsigned short		m_sTICOFRegister[31]; // 0.3pc and 0.15pc add by mhyang 20231013
	// Register_67
	unsigned char		m_abyTFTonTime;       //01	// TFT open time, [0, 255]
	// Register_68
	unsigned char		m_abyTFToffTime;      //01	// TFT close time, [0, 255]
	// Register_69
	unsigned char		m_abyMCLKFrequency;   //01	// MCLK clock frequency, [10, 20]								   											  
	// Register_70~Register_84
	unsigned char		m_abyTICOFCfgReserved[15];
}TICOFCfg;

//CMOS Parameter Configuration：116, Register_520~Register_635
typedef struct CMOSCfg_st 
{
	// Register_1~Register_4
	unsigned char		m_byHead[4];
	// Register_5~Register_68
	unsigned short		m_sCMOSRegister[32];
	// Register_69~Register_116
	unsigned char		m_abyCMOSCfgReserved[48];
}CMOSCfg;

// ADI COF Parameter Configuration：34, Register_636~Register_668
typedef struct ADICOFCfg_st 
{
	// Register_1~Register_4
	unsigned char		m_byHead[4];
	// Register_5~Register_34
	unsigned short		m_sADICOFRegister[15];  //30
}ADICOFCfg;

// Notice:fpd mode data information：22
typedef struct fpd_mode_data_st
{
	// Register_1							  					  
	unsigned char		m_byBinningType;	  //01	//Binning Type,1-binning1x1,2-binning2x2,3-binning3x3,binning4-4x4,
	// Register_2~5							  
	unsigned int 		m_unFpsTime;	      //04	//Fps Time,unit: ms->us
	// Register_6							  
	unsigned char 		m_byPgaLevel;	      //01	//Pga Level
	// Register_7~Register_8				  
	unsigned short 		m_usKvValue;	      //02	//Dose Kv Value
	// Register_9~Register_10				  
	unsigned short		m_sZoomRectLeft;	  //02	//Zoom Rect Left
	// Register_11~Register_12				  
	unsigned short		m_sZoomRectTop;		  //02	//Zoom Rect Top
	// Register_13~Register_14				  
	unsigned short		m_sZoomRectWidth;	  //02	//Zoom Rect Width
	// Register_15~Register_16				  
	unsigned short		m_sZoomRectHeight;    //02	//Zoom Rect Height
	// Register_17							  
	unsigned char		m_byFpdTrigger;       //01	//Fpd Trigger Mode
	// Register_18~21							  
	unsigned int 		m_unPrepareTime;	  //04	//Prepare Time,unit: ms
	// Register_22							  
	unsigned char 		m_byTempStatus;	      //01	//0-Both dark and bright field templates are invalid，1-Only dark field templates are valid
	// 2-Only the bright field template is valid，3-Both dark and bright field templates are valid，others-Both dark and bright field templates are invalid
}FPD_MODE_DATA;

// User Acquisition Mode Parameter Configuration：221
typedef struct fpd_mode_list_st
{
	// Register_1
	unsigned char		m_byCurModeId;                 //01	//Current Mode Id,[0~10]
	// Register_2~Register_221
	FPD_MODE_DATA       m_stModeArray[MODE_LIST_TOTAL];//220 //Current Mode DATA Array
}FPD_MODE_LIST;

// User Acquisition Mode Parameter Configuration：225, Register_670~Register_894
typedef struct UserAcqModeCfg_st
{
	// Register_1~Register_4 //
	unsigned char		m_byHead[4];          //04  //head
	// Register_5
	unsigned char		m_byCurModeId;        //01	//Current Mode Id,[0~10]
	// Register_6~Register_225
	FPD_MODE_DATA       m_stModeList[MODE_LIST_TOTAL];//220 //Current Mode List Array
}UserModeCfg;

// 1024 byte regedit
typedef struct RegCfgInfo_s
{
	// System base Information：100,Register_1~Register_100
	SysBaseInfo m_SysBaseInfo;
	// System Manufacture Information：50,Register_101~Register_150
	SysManuInfo m_SysManuInfo;
	// System Status Information：28,Register_151~Register_178
	SysStatusInfo m_SysStatusInfo;
	// Gigabit Ethernet Information：40,Register_179~Register_218
	EtherInfo m_EtherInfo;
	// High Voltage Generator Interface Trigger Mode Information：21,Register_219~Register_239
	HiVolTriggerModeInfo m_HiVolTriggerModeInfo;
	// System Configuration Information：128,Register_240~Register_367
	SysCfgInfo m_SysCfgInfo;
	// Image Calibration Configuration：20,Register_368~Register_387
	ImgCaliCfg m_ImgCaliCfg;
	// Voltage Adjust Configuration：48,Register_388~Register_435
	VolAdjustCfg m_VolAdjustCfg;
	// TI COF Parameter Configuration：84,Register_436~Register_519
	TICOFCfg m_TICOFCfg;
	// CMOS Parameter Configuration：116,Register_520~Register_635
	CMOSCfg m_CMOSCfg;
	// ADI COF Parameter Configuration：34,Register_636~Register_669
	ADICOFCfg m_ADICOFCfg;
	// User Mode Parameter Configuration：225,Register_670~Register_894
	UserModeCfg m_UserModeCfg;
	///////////////////////
	// Reserved Fields:130,,Register_895~Register_1024
	unsigned char m_byReservedFields[130]; 
}RegCfgInfo;
#pragma pack()
/////////////////////////////////////////////////////////////////////////////
extern char g_szDllPath1[MAX_PATH];                         // global variable-Current dynamic library directory #全局变量-当前动态库所在目录
extern HBI_DLL_INSTANCE g_hbiInstance[DETECTOR_MAX_NUMBER]; // global variable-dynamic library instance          #全局变量-动态库实例
////////////////////////////////////////////////////////////////////////////
////#if 0
////Notice:The flat panel detector's information table.
////struct HbiFpdInfoTable
////{
////	unsigned int fpd_num;       // Detector model
////	unsigned int fpd_width;     // Detector width
////	unsigned int fpd_height;    // Detector height
////	unsigned int fpd_pitch;     // Pixel size
////	unsigned int fpd_commtype;  // Communication mode:0-UDP_COMM_TYPE,1-UDP_JUMBO_COMM_TYPE,2-PCIE_COMM_TYPE,3-WALN_COMM_TYPE
////	unsigned int fpd_fps;       // Maximum frame rate (binning 1x1)
////	char fpd_name[32];          // Detector name£¬The current definition supports up to 32 valid characters
////};
//////
////static const HbiFpdInfoTable HB_FPD_TABLE[] = {
////	{ 1, 3072, 3072, 140, 0, 1,  "4343-140um"      },
////	{ 2, 2560, 3072, 140, 0, 1,  "3543-140um"      },
////	{ 3, 1248, 1024, 125, 1, 20, "1613-125um"      },
////	{ 4, 2048, 2048, 140, 0, 1,  "3030-140um"      },
////	{ 5, 2816, 3584, 85,  0, 1   "2530-85um"       },
////	{ 6, 2048, 1792, 140, 0, 1,  "3025-140um"      },
////	{ 7, 4288, 4288, 100, 0, 1,  "4343-100um"      },
////	{ 8, 3072, 3840, 75,  0, 1   "2530-75um"       },
////	{ 9, 1024, 1024, 200, 0, 1,  "2121-200um"      },
////	{10, 2784, 2400, 50,  0, 1   "CMOS-1412-50um"  },
////	{11, 1056, 1200, 50,  0, 1   "CMOS-0606-50um"  },
////	{12, 1536, 1536, 148, 0, 1,  "IGZO-2323-148um" },
////	{13, 2432, 3072, 100, 0, 1,  "2530-100um"      },
////	{14, 2048, 2048, 148, 0, 1,  "3030-148um"      },
////	{15, 3534, 4302, 100, 0, 1,  "3543-100um"      },
////	{16, 1536, 1536, 105, 0, 1,  "1616-105um"      },
////	{17, 3072, 7680, 140, 0, 1,  "43100-140um"     },
////	{18, 1070, 1200, 100, 0, 1,  "CMOS-1515-100um" },
////	{19, 2063, 2049, 100, 0, 1,  "CMOS-2121-100um" },
////	{20, 2331, 76,   100, 0, 1,  "CMOS-2301-100um" },
////	{21, 3095, 3073, 100, 0, 1,  "CMOS-3131-100um" },
////	{22, 1536, 1386, 120, 0, 1,  "1917-120um"      },
////	{23, 3072, 3072, 99,  0, 1,  "3030-99um"       },
////	{24, 2048, 2048, 75,  0, 1,  "1515-75um"       },
////	{25, 3488, 4356, 70,  0, 1,  "2530-70um"       },
////	{26, 3584, 3072, 85,  0, 1,  "2530-85um-custom"}
////};
////int main()
////{
////	int nFpdNum = 1; // panel number #平板序号 m_byPanelSize》=1
////	int j = nFpdNum - 1;
////	int fpd_num       = HB_FPD_INFO[j].fpd_num;
////	int fpd_width     = HB_FPD_INFO[j].fpd_width;
////	int fpd_height    = HB_FPD_INFO[j].fpd_height;
////	int fpd_pitch     = HB_FPD_INFO[j].fpd_pitch;
////	int fpd_commtype  = HB_FPD_INFO[j].fpd_commtype;
////	int fpd_fps       = HB_FPD_INFO[j].fpd_fps;
////    printf("fpd_name=%s"\n", fpd_name)
////	printf("fpd_num=%d,fpd_pitch=%d\n", fpd_num, fpd_pitch);
////	printf("fpd_width=%d,fpd_height=%d\n", fpd_width, fpd_height);
////	printf("fpd_fps=%d,fpd_commtype=%d\n", fpd_fps,fpd_commtype);
////	return 0;
////}
////#endif
////////////////////////////////////////////////////////////////////////////
#endif // __HBI_TYPE_H_
