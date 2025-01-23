// CDlgBinningType.cpp: 实现文件
//

#include "stdafx.h"
#include "HB_SDK_DEMO2008.h"
#include "CDlgBinningType.h"
#include "afxdialogex.h"


// CDlgBinningType 对话框

IMPLEMENT_DYNAMIC(CDlgBinningType, CDialogEx)

CDlgBinningType::CDlgBinningType(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_BINNING_TYPE, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);//修改对话框的图标

	m_uBinningType = 1;
}

CDlgBinningType::~CDlgBinningType()
{
}

void CDlgBinningType::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgBinningType, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgBinningType::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgBinningType 消息处理程序


BOOL CDlgBinningType::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	// TODO:  在此添加额外的初始化
	((CComboBox *)GetDlgItem(IDC_COMBO_TEMPLATE_BINNING))->SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgBinningType::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	//
	int select = ((CComboBox *)GetDlgItem(IDC_COMBO_TEMPLATE_BINNING))->GetCurSel();
	if (select <= 0 || select > 3)
	{
		m_uBinningType = 1;
	}
	else
	{
		m_uBinningType = select + 1;
	}

	CDialogEx::OnOK();
}
