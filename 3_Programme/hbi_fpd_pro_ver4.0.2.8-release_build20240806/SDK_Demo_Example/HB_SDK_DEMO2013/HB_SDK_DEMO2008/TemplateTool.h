#pragma once
#define WM_DOWNLOAD_CB_MESSAGE (WM_USER + 104)  // ÏÂÔØdefect

typedef enum {
	FPD_OFFSET_TEMPLATE = 0X00,
	FPD_GAIN_TEMPLATE,
	FPD_DEFECT_ACQ_GROUP1,
	FPD_DEFECT_ACQ_GROUP2,
	FPD_DEFECT_ACQ_AND_TEMPLATE,
}EnumGENERATE_TEMPLATE;

// CTemplateTool dialog

class CTemplateTool : public CDialog
{
	DECLARE_DYNAMIC(CTemplateTool)

public:
	CTemplateTool(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTemplateTool();

// Dialog Data
	enum { IDD = IDD_QGENERATE_TEMPLATE_TOOL };
	HICON m_hIcon;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRadioOffset();
	afx_msg void OnBnClickedRadioGain();
	afx_msg void OnBnClickedRadioDefect();
	afx_msg void OnBnClickedBtnTemplateGenerate();
	//
	afx_msg LRESULT OnGenerateTemplateResult(WPARAM wparam, LPARAM lparam = 0);
	afx_msg LRESULT OnUpdateBtnTitle(WPARAM wParam, LPARAM lParam);
	static int DownloadTemplateCBFun(unsigned char command, int code, void *inContext);

public:
	int DoOffsetTemp();
	int DoGainTemp();
	int DoDefectTemp();
	int DoDefectGroup1();
	int DoDefectGroup2();
	int DoDefectGroup3();

public:
	/*
	* defect ÁÁ³¡Í¼×éºÅ
	* 0-first group acq
	* 1-second group acq
	* 2-third group acq and generate template
	*/
	EnumGENERATE_TEMPLATE enumTemplateType;
	unsigned int m_ungroupid;
};
