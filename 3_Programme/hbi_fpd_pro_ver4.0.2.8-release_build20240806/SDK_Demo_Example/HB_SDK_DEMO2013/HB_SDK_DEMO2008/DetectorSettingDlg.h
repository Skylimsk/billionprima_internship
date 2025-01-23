#pragma once


// CDetectorSettingDlg 对话框

class CDetectorSettingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDetectorSettingDlg)

public:
	CDetectorSettingDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CDetectorSettingDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DETECTOR_SETTING };
#endif
	HICON m_hIcon;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnSaveSetting();

public:
	void InitCtrlData();
	void SaveCfg2IniFile();
};
