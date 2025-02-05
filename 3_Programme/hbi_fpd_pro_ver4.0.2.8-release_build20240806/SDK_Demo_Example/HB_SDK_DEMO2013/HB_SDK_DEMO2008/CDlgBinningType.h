#pragma once


// CDlgBinningType Dialog Box

class CDlgBinningType : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgBinningType)

public:
	CDlgBinningType(CWnd* pParent = nullptr);   // Standard Constructor
	virtual ~CDlgBinningType();

// Dialog Box Data 
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_BINNING_TYPE };
#endif
	HICON m_hIcon;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV Support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();

public:
	unsigned int m_uBinningType;
};
