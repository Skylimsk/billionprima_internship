// DetectorSettingDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "HB_SDK_DEMO2008.h"
#include "DetectorSettingDlg.h"
#include "afxdialogex.h"

#include "HB_SDK_DEMO2008Dlg.h"

extern CHB_SDK_DEMO2008Dlg* theDemo;
// CDetectorSettingDlg 对话框

IMPLEMENT_DYNAMIC(CDetectorSettingDlg, CDialogEx)

CDetectorSettingDlg::CDetectorSettingDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DETECTOR_SETTING, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);//修改对话框的图标
}

CDetectorSettingDlg::~CDetectorSettingDlg()
{
}

void CDetectorSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDetectorSettingDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_SAVE_SETTING, &CDetectorSettingDlg::OnBnClickedBtnSaveSetting)
END_MESSAGE_MAP()


// CDetectorSettingDlg 消息处理程序

BOOL CDetectorSettingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	// TODO:  在此添加额外的初始化
	InitCtrlData();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDetectorSettingDlg::InitCtrlData()
{
	if (theDemo != NULL)
	{
		SetDlgItemText(IDC_EDIT_DETECTOR_USER_NAME, theDemo->m_username);

		if (theDemo->m_bSupportDual) theDemo->m_bSupportDual = false;
		if (theDemo->m_uDefaultFpdid != 0) theDemo->m_uDefaultFpdid = 0;
		//
		CFPD_BASE_CFG *pCfg = NULL;
		DWORD dwAddress = 0;
		char buff[16] = { 0 };
		unsigned char *pIP = NULL;
		pCfg = theDemo->m_fpd_base;
		if (pCfg != NULL)
		{
			((CEdit *)GetDlgItem(IDC_EDIT_DETECTOR_A_ID))->SetWindowTextA("0");
			//
			if (0 <= pCfg->commType && pCfg->commType <= 3)
				((CComboBox *)GetDlgItem(IDC_COMBO_COMM_TYPE_A))->SetCurSel(pCfg->commType);
			else
				((CComboBox *)GetDlgItem(IDC_COMBO_COMM_TYPE_A))->SetCurSel(0);
			//
			dwAddress = inet_addr(pCfg->DstIP);
			pIP = (unsigned char *)&dwAddress;
			((CIPAddressCtrl*)GetDlgItem(IDC_IPADDRESS_DETECTOR_IP_A))->SetAddress(*pIP, *(pIP + 1), *(pIP + 2), *(pIP + 3));
			//
			memset(buff, 0x00, 16);
			sprintf(buff, "%u", pCfg->DstPort);
			SetDlgItemText(IDC_EDIT_DETECTOR_PORT_A, buff);
			//
			dwAddress = inet_addr(pCfg->SrcIP);
			pIP = (unsigned char *)&dwAddress;
			((CIPAddressCtrl*)GetDlgItem(IDC_IPADDRESS_LOCAL_IP_A))->SetAddress(*pIP, *(pIP + 1), *(pIP + 2), *(pIP + 3));
			//
			memset(buff, 0x00, 16);
			sprintf(buff, "%u", pCfg->SrcPort);
			SetDlgItemText(IDC_EDIT_LOCAL_PORT_A, buff);
		}
	}
}


void CDetectorSettingDlg::OnBnClickedBtnSaveSetting()
{
	// TODO: 在此添加控件通知处理程序代码
	this->UpdateData(TRUE);

	if (theDemo == NULL)
	{
		::AfxMessageBox("err:theDemo is NULL!");
		return;
	}
	//
	if (theDemo->m_IsOpen)
	{
		::AfxMessageBox("warnning:disconnect fpd!");
		return;
	}
	//
	unsigned int usize = theDemo->m_fpdbasecfg.size();
	if (usize == 0)
	{
		::AfxMessageBox("err:m_fpdbasecfg is error!");
		return;
	}
	// 更新变量
	CFPD_BASE_CFG *pCfg = NULL;
	DWORD dwAddress = 0;
	char buff[16] = { 0 };
	unsigned char *pIP = NULL;
	int j = 0;

	// add by mhyang 20220905
	memset(theDemo->m_username, 0x00, 64);
	((CEdit*)GetDlgItem(IDC_EDIT_DETECTOR_USER_NAME))->GetWindowTextA(theDemo->m_username, 63);

	//
	if (theDemo->m_bSupportDual) theDemo->m_bSupportDual = false;
	if (theDemo->m_uDefaultFpdid != 0) theDemo->m_uDefaultFpdid = 0;
	//
	pCfg = theDemo->m_fpd_base;
	if (pCfg != NULL)
	{
		pCfg->detectorId = 0;
		pCfg->commType = ((CComboBox *)GetDlgItem(IDC_COMBO_COMM_TYPE_A))->GetCurSel();
		((CIPAddressCtrl *)GetDlgItem(IDC_IPADDRESS_DETECTOR_IP_A))->GetAddress(dwAddress);
		pIP = (unsigned char *)&dwAddress;
		j = sprintf(pCfg->DstIP, "%u.%u.%u.%u", *(pIP + 3), *(pIP + 2), *(pIP + 1), *pIP);
		pCfg->DstIP[j] = '\0';
		memset(buff, 0x00, 16);
		((CEdit *)GetDlgItem(IDC_EDIT_DETECTOR_PORT_A))->GetWindowTextA(buff, 8);
		pCfg->DstPort = atoi(buff);
		//
		((CIPAddressCtrl *)GetDlgItem(IDC_IPADDRESS_LOCAL_IP_A))->GetAddress(dwAddress);
		pIP = (unsigned char *)&dwAddress;
		j = sprintf(pCfg->SrcIP, "%u.%u.%u.%u", *(pIP + 3), *(pIP + 2), *(pIP + 1), *pIP);
		pCfg->SrcIP[j] = '\0';
		memset(buff, 0x00, 16);
		((CEdit *)GetDlgItem(IDC_EDIT_LOCAL_PORT_A))->GetWindowTextA(buff, 8);
		pCfg->SrcPort = atoi(buff);
	}

	//
	size_t len = strlen(theDemo->m_username);
	if (len > 0)
	{
		theDemo->SetWindowTextA(theDemo->m_username);
	}
	else
	{
		theDemo->SetWindowTextA(_T("shanghaihaobo"));
	}

	// 写入到文件
	SaveCfg2IniFile();
}


void CDetectorSettingDlg::SaveCfg2IniFile()
{
	if (theDemo == NULL)
	{
		::AfxMessageBox("err:theDemo is NULL!");
		return;
	}
	//
	if (theDemo->m_IsOpen)
	{
		::AfxMessageBox("warnning:disconnect fpd!");
		return;
	}
	//
	unsigned int usize = theDemo->m_fpdbasecfg.size();
	if (usize == 0)
	{
		::AfxMessageBox("err:m_fpdbasecfg is error!");
		return;
	}
	// 更新变量
	char szPath[MAX_PATH] = { 0 };
	memset(szPath, 0x00, MAX_PATH);
	sprintf(szPath, "%s\\DETECTOR_CFG.INI", theDemo->m_path);

	// add by mhyang 20220905
	WritePrivateProfileString("FPD_CFG", "USER_NAME", theDemo->m_username, szPath);

	//
	char tmp[16] = { 0 };
	memset(tmp, 0x00, 16);
	int j = 0;
	//
	CFPD_BASE_CFG *pCfg = theDemo->m_fpd_base;
	if (pCfg != NULL)
	{
		pCfg->detectorId = 0;
		WritePrivateProfileString("DETECTOR_A", "DETECTOR_ID", "0", szPath);
		j = sprintf(tmp, "%u", pCfg->commType);
		tmp[j] = '\0';
		WritePrivateProfileString("DETECTOR_A", "COMM_TYPE", tmp, szPath);
		//
		WritePrivateProfileString("DETECTOR_A", "DETECTOR_IP", pCfg->DstIP, szPath);
		j = sprintf(tmp, "%u", pCfg->DstPort);
		tmp[j] = '\0';
		WritePrivateProfileString("DETECTOR_A", "DETECTOR_PORT", tmp, szPath);
		//
		WritePrivateProfileString("DETECTOR_A", "LOCAL_IP", pCfg->SrcIP, szPath);
		j = sprintf(tmp, "%u", pCfg->SrcPort);
		tmp[j] = '\0';
		WritePrivateProfileString("DETECTOR_A", "LOCAL_PORT", tmp, szPath);
	}
	::AfxMessageBox("Save success!");
}