/*********************************************************************
 * This is a part of Angry IP Scanner source code                    *
 * http://www.angryziber.com/ipscan/                                 *
 *                                                                   *
 * Written by Angryziber                                             *
 *                                                                   *
 * You may distribute this code as long as this message is not       *
 * removed and it is clear who has written it.                       *
 * You may not rename the program and distribute it.                 *
 *********************************************************************/

// DetailsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ipscan.h"
#include "DetailsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDetailsDlg dialog


CDetailsDlg::CDetailsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDetailsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDetailsDlg)
	m_szScannedInfo = _T("");
	m_szPortList = _T("");
	//}}AFX_DATA_INIT
}


void CDetailsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDetailsDlg)
	DDX_Text(pDX, IDC_SCANNED_INFO, m_szScannedInfo);
	DDX_Text(pDX, IDC_PORT_LIST, m_szPortList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDetailsDlg, CDialog)
	//{{AFX_MSG_MAP(CDetailsDlg)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDetailsDlg message handlers

void CDetailsDlg::addScannedInfo(LPCSTR szInfo)
{
	m_szScannedInfo += szInfo;
	m_szScannedInfo += "\r\n����������������������������������������\r\n";
}

void CDetailsDlg::setPorts(LPCSTR szPorts)
{
	m_szPortList = "";

	for (int i = 0; szPorts[i] != 0; i++)
	{
		if (szPorts[i] == ',')
			m_szPortList += "\r\n";
		else
			m_szPortList += szPorts[i];
	}
}

BOOL CDetailsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	return TRUE;  
}

void CDetailsDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// Remove annoing selection from the dialog box
	CEdit *pEdit = (CEdit*) GetDlgItem(IDC_SCANNED_INFO);
	pEdit->SetSel(0, 0);
	
}

BOOL CDetailsDlg::PreTranslateMessage(MSG* pMsg) 
{
	// Check for Enter key presses
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		EndDialog(IDOK);	// Close window on ENTER
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}
