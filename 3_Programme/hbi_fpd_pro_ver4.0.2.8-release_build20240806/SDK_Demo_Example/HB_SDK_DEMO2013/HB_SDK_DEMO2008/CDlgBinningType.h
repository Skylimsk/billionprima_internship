#pragma once


// CDlgBinningType 对话框

class CDlgBinningType : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgBinningType)

public:
	CDlgBinningType(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CDlgBinningType();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_BINNING_TYPE };
#endif
	HICON m_hIcon;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();

public:
	unsigned int m_uBinningType;
};
