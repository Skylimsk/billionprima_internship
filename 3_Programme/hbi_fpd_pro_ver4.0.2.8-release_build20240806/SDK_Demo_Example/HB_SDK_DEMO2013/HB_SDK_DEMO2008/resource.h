//{{NO_DEPENDENCIES}}
// Microsoft Visual C++ generated include file.
// Used by HB_SDK_DEMO2008.rc

// Prevents multiple inclusions
#ifndef RESOURCE_H
#define RESOURCE_H

// About Box Identifiers
#define IDM_ABOUTBOX                    0x0010
#define IDD_ABOUTBOX                    100
#define IDS_ABOUTBOX                    101

// Main Dialog Identifier
#define IDD_HB_SDK_DEMO2008_DIALOG      102
#define IDP_SOCKETS_INIT_FAILED         103
#define IDR_MAINFRAME                   128

// Template Generation Tool Dialog
#define IDD_QGENERATE_TEMPLATE_TOOL     129

// Main Menu Resource
#define IDR_MENU_MAIN                   130

// Icons for Various Statuses
#define IDI_ICON_BUSY                   131
#define IDI_ICON_CONNECT                132
#define IDI_ICON_PREPARE                134
#define IDI_ICON_READY                  135
#define IDI_ICON_TEMPLATE               136
#define IDI_ICON_DISCONNECT             1010
#define IDI_ICON_CONTINUE_READY         142
#define IDI_ICON_EXPOSE                 143
#define IDI_ICON_DEFECT_ACK             144
#define IDI_ICON_GAIN_ACK               145
#define IDI_ICON_OFFSET_ACK             146
#define IDI_ICON_RETRANS_MISS           147
#define IDI_ICON_UPDATE_FIRMWARE        148
#define IDI_ICON_CONTINUE_OFFSET        149
#define IDI_ICON_DISCONNECT_B           1046

// Detector Setting Dialog
#define IDD_DETECTOR_SETTING            138
#define IDD_DIALOG_BINNING_TYPE         150
#define IDD_UPDATE_FIRMWARE_TOOL        153

// Button Controls
#define IDC_BTN_CONN                    1000   // Connect Button
#define IDC_BTN_DISCONN                 1001   // Disconnect Button
#define IDC_BTN_LIVE_ACQ                1002   // Start Live Acquisition
#define IDC_BTN_STOP_LIVE_ACQ           1003   // Stop Live Acquisition
#define IDC_BTN_TEMPLATE_GENERATE       1009   // Generate Template Button
#define IDC_BTN_DOWNLOAD_TEMPLATE       1043   // Download Template Button
#define IDC_BTN_SAVE_SETTING            1060   // Save Settings Button
#define IDC_BUTTON_SET_GAIN_MODE        1029   // Set Gain Mode Button
#define IDC_BUTTON_SET_PGA_LEVEL        1031   // Set PGA Level Button
#define IDC_BUTTON_SINGLE_SHOT          1032   // Single Shot Button
#define IDC_BTN_SET_TRIGGER_MODE        1033   // Set Trigger Mode Button
#define IDC_BUTTON_SET_LIVE_ACQUISITION_TM 1034 // Set Live Acquisition Time Button
#define IDC_BUTTON_GET_CONFIG           1036   // Get Configuration Button
#define IDC_BTN_SET_TRIGGER_CORRECTION  1037   // Set Trigger Correction Button
#define IDC_BTN_FIRMWARE_CORRECT_ENABLE 1038   // Enable Firmware Correction Button
#define IDC_BUTTON_GET_GAIN_MODE        1039   // Get Gain Mode Button
#define IDC_BUTTON_SET_BINNING          1040   // Set Binning Button
#define IDC_BUTTON_GET_SERIAL_NUMBER    1041   // Get Serial Number Button
#define IDC_BUTTON_GET_BINNING          1049   // Get Binning Button
#define IDC_BUTTON_SET_PREPARE_TM       1050   // Set Prepare Time Button
#define IDC_BUTTON_GET_PREPARE_TM       1051   // Get Prepare Time Button
#define IDC_BUTTON_OPEN_CONFIG          1059   // Open Configuration Button
#define IDC_BUTTON_GET_FIRMWARE_VER     1073   // Get Firmware Version Button
#define IDC_BUTTON_GET_SDK_VER          1074   // Get SDK Version Button
#define IDC_BUTTON_GET_FPD_SN           1075   // Get FPD Serial Number Button
#define IDC_BUTTON_UPDATE_PGA_BINNING_FPS 1083 // Update PGA Binning FPS Button
#define IDC_BUTTON_OPEN_PATH            1091   // Open File Path Button
#define IDC_BTN_GET_IMAGE_PROPERTY      1030   // Get Image Property Button
#define IDC_BUTTON_SINGLE_PREPARE       1047   // Single Prepare Button
#define IDC_BTN_UPDATE_TRIGGER_BINNING_FPS 1082 // Update Trigger Binning FPS Button
#define IDC_BTN_UPDATE_TRIGGER_PGA_BINNING_FPS 1084 // Update Trigger PGA Binning FPS Button
#define IDC_BTN_UPDATE_PACKET_INTERVAL_TIME 1094 // Update Packet Interval Time Button
#define IDC_BTN_GET_PACKET_INTERVAL_TIME 1095 // Get Packet Interval Time Button

// Radio Buttons
#define IDC_RADIO_SHOW_PIC              1004   // Show Picture Option
#define IDC_RADIO_SAVE_PIC              1005   // Save Picture Option
#define IDC_RADIO_GAIN                  1005   // Gain Option
#define IDC_RADIO_OFFSET                1007   // Offset Option
#define IDC_RADIO_DEFECT                1008   // Defect Option

// IP and Port Fields
#define IDC_IPADDRESS_REMOTE            1011   // Remote IP Address Input
#define IDC_EDIT_REMOTE_PORT            1012   // Remote Port Input
#define IDC_IPADDRESS_LOCAL             1013   // Local IP Address Input
#define IDC_EDIT_LOCAL_PORT             1014   // Local Port Input

// Combo Boxes
#define IDC_COMBO_TRIGGER_MODE          1015   // Trigger Mode Selection
#define IDC_COMBO_ENABLE_OFFSET         1016   // Enable Offset Option
#define IDC_COMBO_ENABLE_GAIN           1017   // Enable Gain Option
#define IDC_COMBO_ENABLE_DEFECT         1018   // Enable Defect Option
#define IDC_COMBO_PGA_LEVEL             1026   // PGA Level Selection
#define IDC_COMBO_BINNING               1027   // Binning Selection
#define IDC_COMBO_CONN_OFFSETTEMP       1042   // Connection Offset Temperature Selection
#define IDC_COMBO_LIVE_ACQ_MODE         1044   // Live Acquisition Mode Selection
#define IDC_COMBO_COMM_TYPE_A           1044   // Communication Type A Selection
#define IDC_COMBO_COMM_TYPE_B           1045   // Communication Type B Selection
#define IDC_COMBO_CUR_DETECTOR          1072   // Current Detector Selection
#define IDC_COMBO_TEMPLATE_BINNING      1086   // Template Binning Selection

// Edit Controls (Text Inputs)
#define IDC_EDIT_MAX_FRAME_SUM          1019   // Max Frame Sum Input
#define IDC_EDIT_DISCARD_FRAME_NUM      1021   // Discard Frame Number Input
#define IDC_EDIT_MAX_FPS                1023   // Max FPS Input
#define IDC_EDIT_FIRMWARE_FILE_PATH     1090   // Firmware File Path Input
#define IDC_EDIT_PACKET_INTERVAL_TIME   1093   // Packet Interval Time Input
#define IDC_EDIT_LIVE_ACQ_TIME          1079   // Live Acquisition Time Input
#define IDC_EDIT_DETECTOR_USER_NAME     1096   // Detector User Name Input
#define IDC_EDIT_PREPARE_TIME           1048   // Prepare Time Input
#define IDC_EDIT_DETECTOR_A_ID          1061   // Detector A ID Input
#define IDC_EDIT_DETECTOR_PORT_A        1063   // Detector A Port Input
#define IDC_EDIT_LOCAL_PORT_A           1065   // Local Port A Input
#define IDC_EDIT_DETECTOR_B_ID          1066   // Detector B ID Input
#define IDC_EDIT_DETECTOR_PORT_B        1068   // Detector B Port Input
#define IDC_EDIT_LOCAL_PORT_B           1070   // Local Port B Input
#define IDC_EDIT_DETECTOR_USER_NAME     1096   // Detector User Name Input

// Check Boxes
#define IDC_CHECK_SUPPORT_DUAL          1071   // Support Dual Detector Checkbox

// Spin Controls (Number Adjusters)
#define IDC_SPIN_MAX_FRAME_SUM          1020   // Max Frame Sum Adjuster
#define IDC_SPIN_DISCARD_FRAME_NUM      1022   // Discard Frame Number Adjuster
#define IDC_SPIN_MAX_FPS                1024   // Max FPS Adjuster

// Static Text Controls (Labels)
#define IDC_STATIC_PICTURE_ZONE         1006   // Picture Zone Label
#define IDC_STATIC_DETECTOR_A           1052   // Detector A Label
#define IDC_STATIC_PICTURE_ZONE_B       1045   // Picture Zone B Label
#define IDC_STATIC_FIRMWARE_VERSION     1076   // Firmware Version Label
#define IDC_STATIC_SDK_VERSION          1077   // SDK Version Label
#define IDC_STATIC_SERIAL_NUMBER        1078   // Serial Number Label
#define IDC_STATIC_UPDATE_MSG           1092   // Update Message Label

// Firmware Update Controls
#define IDC_BUTTON_FIRMWARE_VER         1028   // Get Firmware Version Button
#define IDC_BUTTON_SOFTWARE_VER         1035   // Get Software Version Button
#define IDC_BUTTON_UPDATE_START         1088   // Start Update Button
#define IDC_BUTTON_UPDATE_STOP          1089   // Stop Update Button
#define IDC_PROGRESS_UPGRADE_FIRMWARE   1087   // Firmware Update Progress Bar

// Group Boxes (UI Sections)
#define IDC_GROUP_BOX_CONN              1054   // Connection Group
#define IDC_GROUP_BOX_CORRECTION        1055   // Correction Group
#define IDC_GROUP_BOX_BINNING           1056   // Binning Group
#define IDC_GROUP_BOX_LIVE_ACQ          1057   // Live Acquisition Group
#define IDC_GROUP_BOX_SIGLE_ACQ         1058   // Single Acquisition Group
#define IDC_GROUP_BOX_CUSTOM            1085   // Custom Settings Group

// Menu Commands
#define ID_FILE_GENERATETEMPLATE        32771  // Generate Template Menu Item
#define ID_HELP_ABOUTDEMO               32772  // About Demo Menu Item
#define ID_FILE_SETTING                 32773  // Settings Menu Item
#define ID_FILE_OPEN_TEMPLATE_WIZARD    32775  // Open Template Wizard Menu Item
#define ID_FIRMWARE_UPDATE_TOOL         32776  // Firmware Update Tool Menu Item

// IP Address Controls
#define IDC_IPADDRESS_DETECTOR_IP_A     1062   // Detector A IP Address Input
#define IDC_IPADDRESS_LOCAL_IP_A        1064   // Local IP for Detector A
#define IDC_IPADDRESS_DETECTOR_IP_B     1067   // Detector B IP Address Input
#define IDC_IPADDRESS_LOCAL_IP_B        1069   // Local IP for Detector B


// Next default values for new objects
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        155
#define _APS_NEXT_COMMAND_VALUE         32777
#define _APS_NEXT_CONTROL_VALUE         1097
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif

#endif // RESOURCE_H
