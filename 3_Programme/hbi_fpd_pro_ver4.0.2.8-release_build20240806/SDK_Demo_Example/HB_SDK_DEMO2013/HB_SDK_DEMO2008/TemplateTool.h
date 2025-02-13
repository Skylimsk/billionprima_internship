#pragma once
#define WM_DOWNLOAD_CB_MESSAGE (WM_USER + 104)  // Download defect template message

// Enumeration for different template generation types
typedef enum {
	FPD_OFFSET_TEMPLATE = 0X00,  // Offset correction template
	FPD_GAIN_TEMPLATE,          // Gain correction template
	FPD_DEFECT_ACQ_GROUP1,      // Defect acquisition - Group 1
	FPD_DEFECT_ACQ_GROUP2,      // Defect acquisition - Group 2
	FPD_DEFECT_ACQ_AND_TEMPLATE // Defect acquisition and template generation
} EnumGENERATE_TEMPLATE;

// CTemplateTool dialog class
class CTemplateTool : public CDialog
{
	DECLARE_DYNAMIC(CTemplateTool)

public:
	CTemplateTool(CWnd* pParent = NULL);   // Standard constructor
	virtual ~CTemplateTool();

	// Dialog Data
	enum { IDD = IDD_QGENERATE_TEMPLATE_TOOL };
	HICON m_hIcon;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support for data binding
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog(); // Initialize dialog
	afx_msg void OnBnClickedRadioOffset();  // Handler for offset radio button
	afx_msg void OnBnClickedRadioGain();    // Handler for gain radio button
	afx_msg void OnBnClickedRadioDefect();  // Handler for defect radio button
	afx_msg void OnBnClickedBtnTemplateGenerate(); // Handler for generate template button

	// Callback functions for template generation result and button title update
	afx_msg LRESULT OnGenerateTemplateResult(WPARAM wparam, LPARAM lparam = 0);
	afx_msg LRESULT OnUpdateBtnTitle(WPARAM wParam, LPARAM lParam);
	static int DownloadTemplateCBFun(unsigned char command, int code, void* inContext);

public:
	// Functions for generating different types of templates
	int DoOffsetTemp();     // Generate offset template
	int DoGainTemp();       // Generate gain template
	int DoDefectTemp();     // Generate defect template
	int DoDefectGroup1();   // Perform defect acquisition for Group 1
	int DoDefectGroup2();   // Perform defect acquisition for Group 2
	int DoDefectGroup3();   // Perform defect acquisition and template generation for Group 3

public:
	/*
	 * Defect acquisition group ID:
	 * 0 - First group acquisition
	 * 1 - Second group acquisition
	 * 2 - Third group acquisition and template generation
	 */
	EnumGENERATE_TEMPLATE enumTemplateType; // Stores selected template type
	unsigned int m_ungroupid;               // Stores selected defect group ID
};
