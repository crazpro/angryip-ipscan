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

// ipscanDlg.cpp : implementation file
//

#include "globals.h"
#include "stdafx.h"
#include "ipscan.h"
#include "ipscanDlg.h"
#include "IPFeedDlgFactory.h"
#include "mmsystem.h"
#include "OptionsDlg.h"
#include "SearchDlg.h"
#include "InstallDlg.h"
#include "link.h"
#include "CommandLine.h"
#include "SaveToFile.h"
#include <winbase.h>
#include "MessageDlg.h"
#include "NetBIOSUtils.h"
#include "ScanUtilsInternal.h"
#include "Scanner.h"
#include "PortDlg.h"
#include "SelectColumnsDlg.h"
#include "QueryDlg.h"
#include "EditOpenersDlg.h"
#include "IPRangeDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

UINT g_nListOffset,g_nStatusHeight, g_nAdvancedOffset;
CIpscanDlg* d;
CWinApp *app;

unsigned long g_nStartItemIndex;
unsigned long g_nEndItemIndex;
unsigned long g_nCurrentItemIndex;

unsigned long g_nIPsScanned;

CAbstractIPFeed * g_pIPFeed;

BOOL g_bScanExistingItems = FALSE;

#define INDEX_CONTEXT_MENU	2
#define	INDEX_SHOW_MENU		4
#define	INDEX_OPEN_MENU		5

#define INDEX_FAVOURITES_MENU	3

class CAboutDlg : public CDialog
{
public:	
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CLink	m_linkForum;
	CLink	m_linkHomepage;
	CLink	m_linkEmail;
	CStatic	m_free;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	afx_msg void OnGoemail();
	afx_msg void OnGohttp();
	virtual BOOL OnInitDialog();
	afx_msg void OnAboutOK();		
	afx_msg void OnHttpForum();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, IDC_HTTP_FORUM, m_linkForum);
	DDX_Control(pDX, IDC_HTTP, m_linkHomepage);
	DDX_Control(pDX, IDC_EMAIL, m_linkEmail);
	DDX_Control(pDX, IDC_TXTFREE, m_free);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_BN_CLICKED(IDC_EMAIL, OnGoemail)
	ON_BN_CLICKED(IDC_HTTP, OnGohttp)
	ON_BN_CLICKED(IDOK, OnAboutOK)
	ON_BN_CLICKED(IDC_HTTP_FORUM, OnHttpForum)
	//}}AFX_MSG_MAP	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIpscanDlg dialog

CIpscanDlg::CIpscanDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIpscanDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIpscanDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32

	#ifdef DEBUG_MESSAGES
		AfxMessageBox("Dlg constructor()", 0, 0);
	#endif

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_szDefaultFileName = NULL;
	m_pToolTips = NULL;
}

void CIpscanDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIpscanDlg)
	DDX_Control(pDX, IDC_IPFEED, m_ctIPFeed);
	DDX_Control(pDX, IDC_BUTTON_TO_ADVANCED, m_btnAdvancedMode);
	DDX_Control(pDX, IDC_SCAN_PORTS, m_ctScanPorts);
	DDX_Control(pDX, IDC_WHATPORTS, m_ctWhatPorts);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Control(pDX, IDC_NUMTHREADS, m_numthreads);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Control(pDX, IDC_STATUS, m_statusctl);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CIpscanDlg, CDialog)
	//{{AFX_MSG_MAP(CIpscanDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_COMMAND(ID_IP_EXIT, OnIpExit)
	ON_BN_CLICKED(IDC_BUTTON1, OnButtonScan)
	ON_COMMAND(ID_HELP_ABOUT, OnHelpAbout)
	ON_COMMAND(ID_OPTIONS_OPTIONS, OnOptionsOptions)	
	ON_WM_TIMER()
	ON_COMMAND(ID_SCAN_SAVETOTXT, OnScanSavetotxt)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, OnRclickList)
	ON_COMMAND(ID_COMMANDS_IPCLIPBOARD, OnIPToClipboard)	
	ON_COMMAND(ID_SCAN_SAVESELECTION, OnScanSaveselection)
	ON_WM_SHOWWINDOW()
	ON_COMMAND(ID_OPTIONS_SAVEOPTIONS, OnOptionsSaveoptions)	
	ON_COMMAND(ID_SHOWNETBIOSINFO, OnShowNetBIOSInfo)
	ON_COMMAND(ID_HELP_ANGRYIPSCANNERWEBPAGE, OnHelpAngryipscannerwebpage)
	ON_COMMAND(ID_HELP_ANGRYZIBERSOFTWARE, OnHelpAngryzibersoftware)
	ON_COMMAND(ID_COMMANDS_RESCANIP, OnRescanIP)
	ON_COMMAND(ID_GOTO_NEXTALIVE, OnGotoNextalive)
	ON_COMMAND(ID_GOTO_NEXTDEAD, OnGotoNextdead)
	ON_COMMAND(ID_GOTO_NEXTOPENPORT, OnGotoNextopenport)
	ON_COMMAND(ID_GOTO_NEXTCLOSEDPORT, OnGotoNextclosedport)
	ON_COMMAND(ID_GOTO_HOSTNAME, OnGotoHostname)		
	ON_COMMAND(ID_HELP_COMMANDLINE, OnHelpCommandline)
	ON_COMMAND(ID_HELP_FORUM, OnHelpForum)
	ON_COMMAND(ID_OPTIONS_INSTALL_PROGRAM, OnOptionsInstallProgram)
	ON_WM_DESTROY()
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_BUTTON_TO_ADVANCED, OnButtonToAdvanced)
	ON_BN_CLICKED(IDC_SCAN_PORTS, OnScanPortsClicked)
	ON_BN_CLICKED(IDC_SELECT_PORTS, OnSelectPortsClicked)
	ON_COMMAND(ID_COMMANDS_SHOWDETAILS, OnCommandsShowIPdetails)
	ON_BN_CLICKED(IDC_SELECT_COLUMNS, OnSelectColumns)
	ON_COMMAND(ID_OPTIONS_SAVEDIMENSIONS, OnOptionsSavedimensions)
	ON_COMMAND(ID_UTILS_DELETEFROMLIST_DEADHOSTS, OnUtilsDeletefromlistDeadhosts)
	ON_COMMAND(ID_UTILS_DELETEFROMLIST_ALIVEHOSTS, OnUtilsDeletefromlistAlivehosts)
	ON_COMMAND(ID_UTILS_DELETEFROMLIST_CLOSEDPORTS, OnUtilsDeletefromlistClosedports)
	ON_COMMAND(ID_UTILS_DELETEFROMLIST_OPENPORTS, OnUtilsDeletefromlistOpenports)
	ON_COMMAND(ID_COMMANDS_DELETEIP, OnCommandsDeleteIP)
	ON_COMMAND(ID_OPTIONS_SHOWLASTSCANINFO, ShowCompleteInformation)
	ON_WM_CLOSE()
	ON_COMMAND(ID_FAVOURITES_ADDCURRENTRANGE, OnFavouritesAddcurrentrange)
	ON_COMMAND(ID_FAVOURITES_DELETEFAVOURITE, OnFavouritesDeleteFavourite)
	ON_COMMAND(ID_UTILS_WIPETRACESREMOVESETTINGSFROMREGISTRY, OnUtilsRemoveSettingsFromRegistry)	
	ON_COMMAND(ID_COMMANDS_OPENCOMPUTER_CONFIGURE, OnCommandsOpencomputerConfigure)
	ON_COMMAND(ID_HELP_DONATIONPAGE, OnHelpDonationPage)
	ON_COMMAND(ID_HELP_CHECKFORNEWERVERSION, OnHelpCheckForNewerVersion)
	ON_COMMAND(ID_HELP_DOWNLOADPLUGINS, OnHelpDownloadplugins)
	ON_COMMAND(ID_OPTIONS_SELECT_COLUMNS, OnSelectColumns)
	ON_COMMAND(ID_OPTIONS_SELECTPORTS, OnSelectPortsClicked)
	ON_CBN_SELCHANGE(IDC_IPFEED, UpdateCurrentIPFeedDialog)
	//}}AFX_MSG_MAP

	ON_COMMAND_RANGE(ID_MENU_SHOW_CMD_001, ID_MENU_SHOW_CMD_099, OnExecuteShowMenu)
	ON_COMMAND_RANGE(ID_MENU_OPEN_CMD_001, ID_MENU_OPEN_CMD_099, OnExecuteOpenMenu)
	ON_COMMAND_RANGE(ID_MENU_FAVOURITES_001, ID_MENU_FAVOURITES_249, OnExecuteFavouritesMenu)

END_MESSAGE_MAP()

int botot;

/////////////////////////////////////////////////////////////////////////////
// CIpscanDlg message handlers

BOOL CIpscanDlg::OnInitDialog()
{
	#ifdef DEBUG_MESSAGES
		AfxMessageBox("OnInitDialog() start", 0, 0);
	#endif

	CDialog::OnInitDialog();

	#ifdef DEBUG_MESSAGES
		AfxMessageBox("OnInitDialog(): custom init dialog", 0, 0);
	#endif
			
	// Internal initializations
	m_list.InitPostCreateStuff();	

	m_bSysCommand = FALSE;

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon	

	#ifdef DEBUG_MESSAGES
		AfxMessageBox("OnInitDialog() set icon ", 0, 0);
	#endif
	
	app = AfxGetApp();
	g_dlg = d = (CIpscanDlg*)app->m_pMainWnd;
	
	// Load default options
	g_options = new COptions();
	g_options->load();
	g_options->setWindowPos();	

	#ifdef DEBUG_MESSAGES
		AfxMessageBox("OnInitDialog(): COptions created ", 0, 0);
	#endif

	// Create the scanner object
	g_scanner = new CScanner();	

	#ifdef DEBUG_MESSAGES
		AfxMessageBox("OnInitDialog(): CScanner created ", 0, 0);
	#endif
	
	// Add columns to the list control
	g_scanner->initListColumns(&m_list);

	#ifdef DEBUG_MESSAGES
		AfxMessageBox("OnInitDialog(): list columns initialized ", 0, 0);
	#endif

	// Set button's bitmaps	
	m_bmpStart.LoadMappedBitmap(IDB_BMPSTART);
	m_bmpStop.LoadMappedBitmap(IDB_BMPSTOP);
	m_bmpKill.LoadMappedBitmap(IDB_BMPKILL);
	((CButton*)GetDlgItem(IDC_BUTTON1))->SetBitmap((HBITMAP)m_bmpStart.m_hObject);	
	// Load bitmaps for advanced mode button
	m_bmpHideAdvanced.LoadMappedBitmap(IDB_HIDE_ADVANCED);
	m_bmpShowAdvanced.LoadMappedBitmap(IDB_SHOW_ADVANCED);
	m_bmpSelectColumns.LoadMappedBitmap(IDB_SELECT_COLUMNS);
	((CButton*)GetDlgItem(IDC_SELECT_COLUMNS))->SetBitmap((HBITMAP)m_bmpSelectColumns.m_hObject);

	#ifdef DEBUG_MESSAGES
		AfxMessageBox("OnInitDialog(): bitmaps loaded ", 0, 0);
	#endif

	// Add Tooltips
	m_pToolTips = new CToolTipCtrl;
	m_pToolTips->Create(this);
	m_pToolTips->AddTool(GetDlgItem(IDC_BUTTON1), "Start/Stop Scanning");
	m_pToolTips->AddTool(GetDlgItem(IDC_BUTTON_TO_ADVANCED), "Show/Hide additional controls (incl. port selection)");
	m_pToolTips->AddTool(GetDlgItem(IDC_SELECT_COLUMNS), "Select columns to be scanned");	
	m_pToolTips->AddTool(GetDlgItem(IDC_SELECT_PORTS), "Select TCP ports to be scanned");	
	m_pToolTips->AddTool(GetDlgItem(IDC_NUMTHREADS), "Number of active scanning threads");	
	m_pToolTips->AddTool(GetDlgItem(IDC_IPFEED), "Change the way you want to choose scanned IP addresses");	
	m_pToolTips->Activate(TRUE);

	#ifdef DEBUG_MESSAGES
		AfxMessageBox("OnInitDialog(): Tooltips added ", 0, 0);
	#endif
	
	// Set window size
	RECT rc;
	m_btnAdvancedMode.GetWindowRect(&rc); g_nListOffset = rc.bottom;
	m_ctScanPorts.GetWindowRect(&rc); g_nAdvancedOffset = rc.bottom - g_nListOffset + 1;
	m_ctIPFeed.GetWindowRect(&rc); g_nListOffset -= (rc.top-5); 
	m_progress.GetWindowRect(&rc); g_nStatusHeight = rc.bottom-rc.top-2;		
	
	if (g_options->m_bScanPorts)
	{
		// Show advanced controls if port scanning is enabled
		m_bAdvancedMode = false;	// OnButtonToAdvanced() will change this to true

		((CButton*)GetDlgItem(IDC_BUTTON_TO_ADVANCED))->SetCheck(TRUE);
	}
	else
	{
		// Do not show advanced controls if port scanning is disabled
		m_bAdvancedMode = true;		// OnButtonToAdvanced() will change this to false
		
		g_nListOffset += g_nAdvancedOffset; // OnButtonToAdvanced() will subtract this
	}

	// Create IP feed factory
	m_pIPFeedFactory = new CIPFeedDlgFactory;

	// Take first IP feed as current
	m_dlgIPFeed = m_pIPFeedFactory->getIPFeed(0);	
	
	// Populate IP Feed listbox	
	for (int i = 0; i < m_pIPFeedFactory->getIPFeedCount(); i++)
	{		
		ASSERT(m_ctIPFeed.AddString(m_pIPFeedFactory->getIPFeedName(i)) >= 0);
	}

	// Update currently selected feeder
	m_ctIPFeed.SetCurSel(0);
	UpdateCurrentIPFeedDialog();


	OnButtonToAdvanced(); // Show/Hide advanced controls
	
	OnScanPortsClicked();	
	
	status(NULL);	// Ready

	#ifdef DEBUG_MESSAGES
		AfxMessageBox("OnInitDialog(): Window resized", 0, 0);
	#endif	

	m_nScanMode = SCAN_MODE_NOT_SCANNING;

	// Load menu			
	g_scanner->initMenuWithColumns(GetMenu()->GetSubMenu(INDEX_CONTEXT_MENU)->GetSubMenu(INDEX_SHOW_MENU));	// Show menu	
	m_menuContext = GetMenu()->GetSubMenu(INDEX_CONTEXT_MENU);	// TODO: Should not be stored!

	// Init favourites menu
	RefreshFavouritesMenu();

	// Init openers menu
	RefreshOpenersMenu();

	hAccel = LoadAccelerators(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_MENU1));


	#ifdef DEBUG_MESSAGES
		AfxMessageBox("OnInitDialog(): Menu loaded", 0, 0);
	#endif

	// Set title
	CString str;
	str.LoadString(IDS_VERSION);
	SetWindowText("Angry IP Scanner "+str);	

	m_dlgIPFeed->SetFocus();


	#ifdef DEBUG_MESSAGES
		AfxMessageBox("OnInitDialog(): Processing command-line", 0, 0);
	#endif

	// Process command-line
	CCommandLine cCmdLine;
	if (cCmdLine.process())
	{	
		AfxMessageBox("Note: command-line support is currently unstable", 0, 0);
		
		// Select appropriate IP Feed
		SelectIPFeedByType(cCmdLine.m_szIPFeedType);

		if (!m_dlgIPFeed->processCommandLine(cCmdLine.m_szIPFeedParams))
		{
			AfxMessageBox("Failed to interpret command-line parameters", 0, 0);

		}

		m_nCmdLineOptions = cCmdLine.m_nOptions;
		m_nCmdLineFileFormat = cCmdLine.m_nFileFormat;

		if (m_nCmdLineOptions & CMDO_SAVE_TO_FILE)
		{
			m_szDefaultFileName = new CString(cCmdLine.m_szFilename);
		}
		
		if (m_nCmdLineOptions & CMDO_START_SCAN)
		{
			CIpscanDlg::OnButtonScan();
		}		
	}
	else
	{
		m_nCmdLineOptions = 0;
	}

	#ifdef DEBUG_MESSAGES
		AfxMessageBox("OnInitDialog(): Finished", 0, 0);
	#endif		
	
	return FALSE; // return TRUE  unless you set the focus to a control
}

void CIpscanDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	nID &= 0xFFF0;

	if (nID == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else 
	{
		if (nID == SC_CLOSE)
		{
			m_bSysCommand = TRUE;
		}
		
		CDialog::OnSysCommand(nID, lParam);		
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CIpscanDlg::OnPaint() 
{
	if (m_nCmdLineOptions & CMDO_HIDE_WINDOW)
	{		
		ShowWindow(SW_HIDE);
	}

	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{	
		CDialog::OnPaint();	
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CIpscanDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CIpscanDlg::status(LPCSTR str) 
{
	if (str != NULL)
		SetDlgItemText(IDC_STATUS, str);
	else
		SetDlgItemText(IDC_STATUS, "Ready");
}

void CIpscanDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
		
	if (m_list.m_hWnd != NULL) 
	{
		HandleResizing(cx, cy);
	}	
}

void CIpscanDlg::OnIpExit() 
{	
	m_bSysCommand = TRUE;
	SendMessage(WM_CLOSE,0,0);	
}

void CIpscanDlg::UpdateCurrentIPFeedDialog()
{
	int nCurrentFeed = m_ctIPFeed.GetCurSel();
	
	// Hide current dlg
	m_dlgIPFeed->ShowWindow(SW_HIDE);	
	
	// Get and show new dlg
	m_dlgIPFeed = m_pIPFeedFactory->getIPFeed(nCurrentFeed);
	m_dlgIPFeed->ShowWindow(SW_SHOW);
	m_dlgIPFeed->SetFocus();
}

void CIpscanDlg::SelectIPFeedByType(const CString &szType)
{
	int nIndex = m_pIPFeedFactory->getIndexByType(szType);

	if (nIndex < 0)
		AfxMessageBox("Unknown IP Feed Type: " + szType);
	else
	{
		m_ctIPFeed.SetCurSel(nIndex);
		UpdateCurrentIPFeedDialog();
	}
}

void CIpscanDlg::RecreateIPFeed()
{
	// Destroy the previous IP feed object
	if (g_pIPFeed)
	{
		delete g_pIPFeed;
		g_pIPFeed = NULL;
	}

	// Create new IP feed object	
	g_pIPFeed = m_dlgIPFeed->createIPFeed();
}

void CIpscanDlg::OnButtonScan() 
{	
	if (m_nScanMode == SCAN_MODE_NOT_SCANNING) 
	{
		if ((!g_options->m_bScanPorts) && (g_options->m_neDisplayOptions == DISPLAY_OPEN))
		{
			MessageBox("Conflicting options: Display only open ports selected, but port scanning is not enabled!", NULL, MB_OK | MB_ICONHAND);
			return;
		}		

		if (!g_bScanExistingItems)
		{
			if (m_list.GetItemCount() > 0)
			{
				if (MessageBox("Are you sure you want to erase all previous scanning results?", NULL, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDNO)
				{
					// If user is unsure, then cancel scanning
					return;
				}
			}

			// Delete old results
			m_list.DeleteAllItems();
			
			// Rercreate IP Feed object
			RecreateIPFeed();

			if (!g_pIPFeed)
				return;	// An error occured - do nothing further

			// Start feeding of IPs
			g_pIPFeed->startFeeding();
			
		}
		else
		{
			// g_nStartItemIndex is set outside before calling this method
			g_nCurrentItemIndex = g_nStartItemIndex;
		}

		m_progress.SetRange(0,100);
		m_progress.SetPos(0);
		m_tickcount = GetTickCount()/1000;

		m_nScanMode = SCAN_MODE_SCANNING;
		m_list.SetHeaderClicksDisabled(TRUE);
		
		((CButton*)GetDlgItem(IDC_BUTTON1))->SetBitmap((HBITMAP)m_bmpStop.m_hObject); // stop scanning button		

		g_nThreadCount = 0;
		g_nIPsScanned = 0;
		m_bScanningAborted = FALSE;

		EnableMenuItems(FALSE);
		
		// Initialize scanning engine
		status("Initializing...");
		g_scanner->initScanning();

		SetTimer(1, g_options->m_nTimerDelay, NULL);

	} 
	else // m_nScanMode is SCAN_MODE_SCANNING or SCAN_MODE_FINISHING or SCAN_MODE_KILLING)
	{
		if (g_nThreadCount > 0) 
		{
			
			if (m_nScanMode == SCAN_MODE_FINISHING) 
			{
				if (MessageBox("Are you sure you want to interrupt scanning by killing all the threads?\nScanning results will be incomplete.",NULL,MB_YESNO | MB_ICONQUESTION) == IDNO) 
					return;
			
				// Kill threads
				status("Killing threads...");
				KillAllRunningThreads();	

				//Sleep(1000);	// sleep 1 second and give some threads a chance to exit
				//status(NULL);

				m_nScanMode = SCAN_MODE_KILLING;				
			}
			else
			if (m_nScanMode == SCAN_MODE_KILLING)
			{
				MessageBox("Please wait while killing active threads...", NULL, MB_OK | MB_ICONINFORMATION);
			}
			else // SCAN_MODE_SCANNING
			{
				// Stop scanning (but wait for existing threads)

				m_bScanningAborted = TRUE;
				
				m_progress.SetPos(100);				

				status("Wait for all threads to terminate...");
				((CButton*)GetDlgItem(IDC_BUTTON1))->SetBitmap((HBITMAP)m_bmpKill.m_hObject);
				m_nScanMode = SCAN_MODE_FINISHING;  // waiting can be interrupted
			}
			
		} 
		else // g_nThreadCount = 0
		{
			KillTimer(1);

			// Finalize IP feeder
			g_pIPFeed->finishFeeding();

			BOOL bShowScanInfo = (m_nScanMode != SCAN_MODE_KILLING) && !g_bScanExistingItems;

			m_nScanMode = SCAN_MODE_NOT_SCANNING;
			m_list.SetHeaderClicksDisabled(FALSE);

			status("Finalizing...");
			g_scanner->finalizeScanning();			
				
			((CButton*)GetDlgItem(IDC_BUTTON1))->SetBitmap((HBITMAP)m_bmpStart.m_hObject); // start scan bitmap			
				
			EnableMenuItems(TRUE);			

			m_progress.SetPos(0);

			status(NULL);	// Ready

			if (m_szDefaultFileName)
			{
				// Program was invoked via command-line, so save data to file & exit

				CSaveToFile tmp(d, FALSE, m_szDefaultFileName->GetBuffer(255), m_nCmdLineFileFormat, m_nCmdLineOptions & CMDO_APPEND_FILE);
					
				if (!(m_nCmdLineOptions & CMDO_NOT_EXIT))
					ExitProcess(0);
			}
			else
			{
				// Display final message box with statistics				

				int nTotalTime = GetTickCount() / 1000 - m_tickcount + 1;
				float fTimeForOneIP = (float) nTotalTime / g_nIPsScanned;

				CString szIPFeedInfo = g_pIPFeed->getScanSummary();

				m_szCompleteInformation.Format(
					"Scanning finished\r\n"
					"%u sec,  %.3f sec/host\r\n\r\n"
					"%s\r\n\r\n"						
					"IPs scanned:\t%u\r\n"
					"Alive hosts:\t%u\r\n",
					nTotalTime, fTimeForOneIP, szIPFeedInfo, g_nIPsScanned, g_scanner->m_nAliveHosts);

				if (g_options->m_bScanPorts)
				{						
					CString szPortInfo;
					szPortInfo.Format("With open ports:\t%u\r\n\r\n", g_scanner->m_nOpenPorts);
					m_szCompleteInformation += szPortInfo;

					int nTotalPortsScanned = g_options->m_bScanHostIfDead ? g_options->m_nPortCount * g_nIPsScanned : g_options->m_nPortCount * g_scanner->m_nAliveHosts;

					szPortInfo.Format("Ports scanned:\r\n%u / host,  %u total", g_options->m_nPortCount, nTotalPortsScanned);
					m_szCompleteInformation += szPortInfo;
				}

				if (bShowScanInfo)
				{
					// Get attention of the user					
					FlashWindow(TRUE);
					PlaySound("SystemExclamation", NULL, SND_ALIAS | SND_ASYNC);
					// Show info
					ShowCompleteInformation();					
				}
			}

			g_bScanExistingItems = FALSE;	// Reset this stuff
		}
	}
}

void CIpscanDlg::OnHelpAbout() 
{	
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}

void CIpscanDlg::OnOptionsOptions() 
{	
	COptionsDlg dlgOpt;
	dlgOpt.DoModal();	// It will get and put options using g_options

	// Update the main list
	m_list.SetScanPorts();
}

void CIpscanDlg::OnTimer(UINT nIDEvent) 
{	 	
	int nItemIndex = 0;

	BOOL bCurrentLessThanEnd;

	if (g_bScanExistingItems)	
		bCurrentLessThanEnd = g_nCurrentItemIndex < g_nEndItemIndex;
	else
		bCurrentLessThanEnd = g_pIPFeed->isNextIPAvailable();

	if (m_bScanningAborted)
		bCurrentLessThanEnd = FALSE;	// Stop unfinished scanning
	
	if (bCurrentLessThanEnd) 
	{
		if ((int) g_nThreadCount >= g_options->m_nMaxThreads - 1) 
			return;

		IPAddress nCurrentIP = g_pIPFeed->getNextIP();
		
		in_addr in;
		char *szIP;
		in.S_un.S_addr = htonl(nCurrentIP);
		szIP = inet_ntoa(in);
		status(szIP);		

		// This is decremented by each thread on exit
		g_nThreadCount++;	

		// Increase counter
		g_nIPsScanned++;

		if (g_bScanExistingItems)
		{
			AfxBeginThread(ThreadProcCallbackRescan, (LPVOID) g_nCurrentItemIndex);	
			g_nCurrentItemIndex++;

			if (g_nEndItemIndex != g_nStartItemIndex)	// To prevent division by 0 below
			{			
				m_progress.SetPos((g_nCurrentItemIndex - g_nStartItemIndex) * 100 / (g_nEndItemIndex - g_nStartItemIndex));
			}
		}
		else
		{
			AfxBeginThread(ThreadProcCallback, (LPVOID) nCurrentIP);	// Pass IP in Host byte order			
			
			m_progress.SetPos(g_pIPFeed->getPercentComplete());			
		}			
		
	} 
	else 
	{
		if (g_nThreadCount <= 0) 
		{
			if (g_bScanExistingItems)
				g_nEndItemIndex--;

			OnButtonScan();	// Change current state (scan mode)
		} 
		else 
		{
			if (m_nScanMode == SCAN_MODE_SCANNING)
			{
				OnButtonScan();	// Change current state (scan mode)				
			}
		}
	}
	
	CDialog::OnTimer(nIDEvent);
}


void CIpscanDlg::OnScanSavetotxt() 
{	
	CSaveToFile tmp(d);
}

void CIpscanDlg::OnScanSaveselection() 
{
	CSaveToFile tmp(d, TRUE);	
}

void CIpscanDlg::OnRclickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMLISTVIEW *lw = (NMLISTVIEW*)pNMHDR;
	POINT p;
	GetCursorPos(&p);
	m_menucuritem = lw->iItem;
	if (m_menucuritem != 0xFFFFFFFF) 
	{
		TrackPopupMenu(m_menuContext->m_hMenu,TPM_LEFTALIGN | TPM_RIGHTBUTTON,p.x,p.y,0,m_hWnd,NULL);
	}	
	*pResult = 0;
}

void CIpscanDlg::OnIPToClipboard() 
{
	m_list.CopyIPToClipboard();
}

void CAboutDlg::OnGoemail() 
{
	CLink::goToWriteMail();
}

void CAboutDlg::OnGohttp() 
{
	CLink::goToScannerHomepage();
}

void CAboutDlg::OnHttpForum() 
{
	CLink::goToHomepageForum();
}

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CString ver;
	ver.LoadString(IDS_VERSION);
	SetDlgItemText(IDC_VERSION,ver);	

	return TRUE;  
}

void CAboutDlg::OnAboutOK() 
{
	EndDialog(IDOK);
}

void CIpscanDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{	
	CDialog::OnShowWindow(bShow, nStatus);	
}

void CIpscanDlg::OnOptionsSaveoptions() 
{	
	g_options->save();
	
	MessageBox("Options and selected columns are successfully saved.", NULL, MB_OK | MB_ICONINFORMATION);
}

void CIpscanDlg::OnOptionsSavedimensions() 
{
	g_options->saveDimensions();

	MessageBox("Window size, position and widths of currently selected columns are successfully saved.", NULL, MB_OK | MB_ICONINFORMATION);
}

void CIpscanDlg::OnShowNetBIOSInfo() 
{
	status("Getting info...");
	m_list.ShowNetBIOSInfo();
	status(NULL);
}

void CIpscanDlg::OnHelpAngryipscannerwebpage() 
{	
	CLink::goToScannerHomepage();	
}

void CIpscanDlg::OnHelpAngryzibersoftware() 
{	
	CLink::goToHomepage();
}

void CIpscanDlg::OnHelpForum() 
{
	CLink::goToHomepageForum();	
}

void CIpscanDlg::OnHelpDonationPage() 
{
	CLink::goToDonationPage();	
}

void CIpscanDlg::OnHelpDownloadplugins() 
{
	CLink::goToPluginsPage();	
}

void CIpscanDlg::OnGotoNextalive() 
{
	m_list.GoToNextAliveIP();
}

void CIpscanDlg::OnGotoNextdead() 
{
	m_list.GoToNextDeadIP();
}

void CIpscanDlg::OnGotoNextopenport() 
{
	m_list.GoToNextOpenPortIP();
}

void CIpscanDlg::OnGotoNextclosedport() 
{
	m_list.GoToNextClosedPortIP();
}

void CIpscanDlg::OnGotoHostname() 
{
	m_list.GoToNextSearchIP();
}

BOOL CIpscanDlg::PreTranslateMessage(MSG* pMsg) 
{
	BOOL bDoAccelerator = TRUE;

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_DELETE)
	{
		if (pMsg->hwnd != m_list.m_hWnd)
			bDoAccelerator = FALSE;	// Do not let accelerator capture Del key from editboxes
	}

	if (bDoAccelerator && TranslateAccelerator(m_hWnd, hAccel, pMsg)) 
		return TRUE;

	if (m_pToolTips != NULL)
		m_pToolTips->RelayEvent(pMsg);

	// Check for Enter key presses
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		// If this is a list control
		if (pMsg->hwnd == m_list.m_hWnd)
		{
			if (m_list.GetCurrentSelectedItem(FALSE) >= 0)
			{
				OnCommandsShowIPdetails();
				return TRUE;
			}
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CIpscanDlg::OnHelpCommandline() 
{
	CCommandLine::displayHelp();
	
}

void CIpscanDlg::OnOptionsInstallProgram() 
{
	CInstallDlg dlgInst;
	dlgInst.DoModal();
}

void CIpscanDlg::OnDestroy() 
{
	CDialog::OnDestroy();	
	
	delete(g_scanner);
	delete(m_szDefaultFileName);
	delete(m_pToolTips);	
	delete(m_pIPFeedFactory);

	if (g_pIPFeed != NULL)
		delete(g_pIPFeed);

	// Delete the critical section object
	// removed because caused problems on Win9X: DeleteCriticalSection(&g_criticalSection);	

	// Kill all threads of this process	
	// This will terminate the process for sure after closing the main window		
	ExitProcess(0);
}

void CIpscanDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	
	CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CIpscanDlg::OnButtonToAdvanced() 
{
	if (m_bAdvancedMode)
	{
		// Hide advanced controls
		m_ctScanPorts.ShowWindow(SW_HIDE);
		m_ctWhatPorts.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_SELECT_PORTS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_SELECT_COLUMNS)->ShowWindow(SW_HIDE);
		
		g_nListOffset -= g_nAdvancedOffset;

		// Change button image		
		((CButton*)GetDlgItem(IDC_BUTTON_TO_ADVANCED))->SetBitmap((HBITMAP)m_bmpShowAdvanced.m_hObject);
	}
	else
	{
		// Show advanced controls
		m_ctScanPorts.ShowWindow(SW_SHOW);
		m_ctWhatPorts.ShowWindow(SW_SHOW);
		GetDlgItem(IDC_SELECT_PORTS)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_SELECT_COLUMNS)->ShowWindow(SW_SHOW);

		g_nListOffset += g_nAdvancedOffset;
		
		// Change button image		
		((CButton*)GetDlgItem(IDC_BUTTON_TO_ADVANCED))->SetBitmap((HBITMAP)m_bmpHideAdvanced.m_hObject);

		// Display port string
		if (g_options->m_aParsedPorts[0].nStartPort != 0)
		{
			m_ctWhatPorts.SetWindowText(g_options->m_szPorts);
		}
	}
	m_bAdvancedMode = !m_bAdvancedMode;
	RECT rc;
	GetClientRect(&rc);
	HandleResizing(rc.right-rc.left, rc.bottom-rc.top);
		
	m_dlgIPFeed->SetFocus();	
}

void CIpscanDlg::HandleResizing(int cx, int cy)
{
	// Resize window and reposition controls
	if (g_nStatusHeight > 0)
	{
		m_list.MoveWindow(0, g_nListOffset, cx, cy-g_nListOffset-(g_nStatusHeight+2), TRUE);
		m_statusctl.MoveWindow(0, cy-g_nStatusHeight/*18*/, cx/2, /*18*/g_nStatusHeight, TRUE);
		m_progress.MoveWindow(cx/2+1,cy-g_nStatusHeight,cx/2-1,g_nStatusHeight,TRUE);
	}
}

void CIpscanDlg::OnScanPortsClicked() 
{
	BOOL bChecked = m_ctScanPorts.GetCheck();

	if (m_nScanMode != SCAN_MODE_NOT_SCANNING)
	{
		m_ctScanPorts.SetCheck(g_options->m_bScanPorts);
		return;
	}

	if (bChecked && g_options->m_aParsedPorts[0].nStartPort == 0)
	{
		m_ctScanPorts.SetCheck(FALSE);
		
		if (AfxMessageBox("No ports are currently selected for scanning.\nDo you want to select them?", MB_ICONWARNING | MB_YESNO, 0) == IDYES)
		{
			OnSelectPortsClicked();
		}		

		return;
	}

	g_options->m_bScanPorts = bChecked;

	m_list.SetScanPorts();	
}

void CIpscanDlg::OnSelectPortsClicked() 
{
	CPortDlg dlg;
	if (dlg.DoModal() == IDOK)
	{
		LPCSTR szPortString = g_options->m_szPorts;

		if (szPortString[0] == 0)
		{
			m_ctWhatPorts.SetWindowText("N/A");
			m_ctScanPorts.SetCheck(FALSE);
		}
		else
		{
			m_ctWhatPorts.SetWindowText(g_options->m_szPorts);
			m_ctScanPorts.SetCheck(TRUE);
		}

		// To update properties
		OnScanPortsClicked();
	}
}

void CIpscanDlg::OnCommandsShowIPdetails() 
{	
	m_list.ShowIPDetails();	
}

void CIpscanDlg::OnExecuteShowMenu(UINT nID)
{
	if (m_nScanMode != SCAN_MODE_NOT_SCANNING)
		return;

	m_menucuritem = m_list.GetCurrentSelectedItem();
	
	if (m_menucuritem == -1)
		return;

	int nFunctionIndex = nID - ID_MENU_SHOW_CMD_001 + CL_STATIC_COUNT;

	char szBuffer[256];

	CString szIP = m_list.GetItemText(m_menucuritem, CL_IP);

	status(szIP);

	g_scanner->runInitFunction(nFunctionIndex, TRUE);
	g_scanner->runScanFunction(m_list.GetNumericIP(m_menucuritem), nFunctionIndex, (char*) &szBuffer, sizeof(szBuffer), TRUE);
	g_scanner->runFinalizeFunction(nFunctionIndex, TRUE);

	status(NULL);

	CString szResult;	
	m_menuContext->GetSubMenu(INDEX_SHOW_MENU)->GetMenuString(nFunctionIndex - CL_STATIC_COUNT, szResult, MF_BYPOSITION);
		
	
	szResult = "Result for a single value scan:\r\n\r\nIP: " + szIP + "\r\n\r\n" + szResult + ":\r\n";	
	szResult += szBuffer;

	CMessageDlg cMsgDlg;
	cMsgDlg.setMessageText(szResult);

	cMsgDlg.DoModal();

}

// Helper function to substitute column values 
void substituteColumnValues(CString &str)
{	
	//for (int i = 0; i < str.GetLength(); i++)
	//{
	//
	//}
	// TODO: write the real replacer, which works with all columns		
}

void CIpscanDlg::OnExecuteOpenMenu(UINT nID)
{		
	int nOpenerIndex = nID - ID_MENU_OPEN_CMD_001;
	
	if (g_options->m_aOpeners[nOpenerIndex].szName.GetLength() == 0)
		return;

	m_menucuritem = m_list.GetCurrentSelectedItem();
	
	if (m_menucuritem == -1)
		return;	
	
	CString szIP = m_list.GetItemText(m_menucuritem, CL_IP);

	status("Opening " + szIP + " (" + g_options->m_aOpeners[nOpenerIndex].szName + ")...");
	
	CString szExecute;
	CString szParameters;
	CString szWorkDir;
	CString szBatFilename;
	
	szExecute.Format(g_options->m_aOpeners[nOpenerIndex].szExecute, szIP);
	substituteColumnValues(szExecute);

	szWorkDir = g_options->m_aOpeners[nOpenerIndex].szWorkDir;

	// If this is a command-line program
	if (g_options->m_aOpeners[nOpenerIndex].bCommandLine)
	{
		// Get temp directory path
		char szTempPath[256];
		GetTempPath(sizeof(szTempPath), (char*) &szTempPath);

		// Generate a unique filename		
		szBatFilename.Format("%saips_temp_open_%u.bat", (char*) &szTempPath, (DWORD)GetTickCount());
		
		FILE *tmpf = fopen(szBatFilename, "wt");
		fprintf(tmpf, "@rem This is a temporary file generated by Angry IP Scanner\n\n"
					  "%s\n@echo .\n@echo Press any key to close window...\n@pause > nul\n"
					  "del %s\n", szExecute, szBatFilename);
		fclose(tmpf);

		szExecute = szBatFilename;
	}
	else
	{
		// Parse execution string in order to distinguish parameters
		
		int nQuotePos = szExecute.Find(TCHAR('"'));
		int nSpacePos;

		if (nQuotePos >= 0)
		{
			nQuotePos = szExecute.Find(TCHAR('"'), nQuotePos + 1);
			nSpacePos = szExecute.Find(TCHAR(' '), nQuotePos + 1);			
		}
		else
		{
			nSpacePos = szExecute.Find(TCHAR(' '), nQuotePos + 1);
		}

		// Extract parameters to a variable
		if (nSpacePos >= 0)
		{
			szParameters = szExecute.Mid(nSpacePos);
			szExecute.Delete(nSpacePos, szExecute.GetLength() - nSpacePos);
		}
	}

	if ((int)ShellExecute(0, NULL, szExecute, szParameters, (szWorkDir.GetLength() > 0) ? (LPCSTR) szWorkDir : NULL, SW_SHOWNORMAL) <= 32) 
	{
		MessageBox("This IP cannot be opened (ShellExecute \"" + szExecute + szParameters + "\" failed)", NULL, MB_OK | MB_ICONHAND);
	}	

	status(NULL);
}

BOOL CIpscanDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// If ESC key was pressed, then do not exit!
	if ((wParam == 2) && (!m_bSysCommand))
	{
		return FALSE;
	}
	else
	{
		m_bSysCommand = FALSE;
		return CDialog::OnCommand(wParam, lParam);
	}
}

void CIpscanDlg::EnableMenuItems(BOOL bEnable)
{
	UINT nEnable = bEnable ? MF_ENABLED : MF_GRAYED;

	CMenu *tmpMnu = GetMenu();

	tmpMnu->EnableMenuItem(ID_OPTIONS_OPTIONS, nEnable);
	tmpMnu->EnableMenuItem(ID_OPTIONS_SELECT_COLUMNS, nEnable);
	tmpMnu->EnableMenuItem(ID_OPTIONS_SELECTPORTS, nEnable);	

	tmpMnu->EnableMenuItem(ID_SCAN_SAVETOTXT, nEnable);
	tmpMnu->EnableMenuItem(ID_SCAN_SAVESELECTION, nEnable);

	tmpMnu->EnableMenuItem(ID_UTILS_DELETEFROMLIST_DEADHOSTS, nEnable);
	tmpMnu->EnableMenuItem(ID_UTILS_DELETEFROMLIST_ALIVEHOSTS, nEnable);
	tmpMnu->EnableMenuItem(ID_UTILS_DELETEFROMLIST_CLOSEDPORTS, nEnable);
	tmpMnu->EnableMenuItem(ID_UTILS_DELETEFROMLIST_OPENPORTS, nEnable);

	tmpMnu->EnableMenuItem(ID_OPTIONS_INSTALL_PROGRAM, nEnable);
	
	tmpMnu->EnableMenuItem(ID_COMMANDS_RESCANIP, nEnable);	
	tmpMnu->EnableMenuItem(ID_COMMANDS_DELETEIP, nEnable);	

	tmpMnu->EnableMenuItem(ID_SHOWNETBIOSINFO, nEnable);
	
	for (UINT i=0; i < tmpMnu->GetMenuItemCount(); i++)
	{
		tmpMnu->EnableMenuItem(ID_MENU_SHOW_CMD_001 + i, nEnable);
	}

	GetDlgItem(IDC_SELECT_COLUMNS)->EnableWindow(bEnable);
	GetDlgItem(IDC_SELECT_PORTS)->EnableWindow(bEnable);
}

void CIpscanDlg::OnSelectColumns() 
{
	CSelectColumnsDlg cDlg;
	cDlg.DoModal();
}


void CIpscanDlg::OnUtilsDeletefromlistDeadhosts() 
{
	status("Deleting...");	
	CString szMessage;
	szMessage.Format("%d items were deleted.", m_list.DeleteAllDeadHosts());
	MessageBox(szMessage, NULL, MB_OK | MB_ICONINFORMATION);
	status(NULL);	// Ready
}

void CIpscanDlg::OnUtilsDeletefromlistAlivehosts() 
{	
	status("Deleting...");
	CString szMessage;
	szMessage.Format("%d items were deleted.", m_list.DeleteAllAliveHosts());
	MessageBox(szMessage, NULL, MB_OK | MB_ICONINFORMATION);	
	status(NULL);	// Ready
}

void CIpscanDlg::OnUtilsDeletefromlistClosedports() 
{
	if (!g_options->m_bScanPorts)
	{
		MessageBox("Port scanning is not enabled.", NULL, MB_OK | MB_ICONHAND);
		return;
	}

	status("Deleting...");
	CString szMessage;
	szMessage.Format("%d items were deleted.", m_list.DeleteAllClosedPortsHosts());
	MessageBox(szMessage, NULL, MB_OK | MB_ICONINFORMATION);		
	status(NULL);	// Ready
}

void CIpscanDlg::OnUtilsDeletefromlistOpenports() 
{
	if (!g_options->m_bScanPorts)
	{
		MessageBox("Port scanning is not enabled.", NULL, MB_OK | MB_ICONHAND);
		return;
	}

	status("Deleting...");
	CString szMessage;
	szMessage.Format("%d items were deleted.", m_list.DeleteAllOpenPortsHosts());
	MessageBox(szMessage, NULL, MB_OK | MB_ICONINFORMATION);		
	status(NULL);	// Ready	
}

void CIpscanDlg::OnRescanIP() 
{
	if (m_nScanMode != SCAN_MODE_NOT_SCANNING)
	{
		return;
	}

	if (m_list.GetCurrentSelectedItem(FALSE) == -1)
	{
		MessageBox("No items selected", NULL, MB_OK | MB_ICONHAND);
		return;
	}

	POSITION pos = m_list.GetFirstSelectedItemPosition();
	int nItemIndex;	

	g_bScanExistingItems = TRUE;
	g_nStartItemIndex = 0;
	g_nEndItemIndex = 1;	// We will process items manually
	
	OnButtonScan();

	g_nEndItemIndex = g_nStartItemIndex;	// Timer will wait for completion only	

	while ((nItemIndex = m_list.GetNextSelectedItem(pos)) >= 0)
	{				
		CString szIP = m_list.GetItemText(nItemIndex, CL_IP);
		status(szIP);

		m_list.ZeroResultsForItem(nItemIndex);
						
		g_nThreadCount++;	// This is decremented by threads on exit
		AfxBeginThread(ThreadProcCallbackRescan, (LPVOID) nItemIndex);

		Sleep(g_options->m_nTimerDelay);
	}		
	
}


void CIpscanDlg::OnCommandsDeleteIP() 
{
	if (m_nScanMode != SCAN_MODE_NOT_SCANNING)
	{
		MessageBox("Cannot delete while scanning", NULL, MB_OK | MB_ICONHAND);
		return;
	}

	if (m_list.GetCurrentSelectedItem(FALSE) == -1)
	{
		MessageBox("No items selected", NULL, MB_OK | MB_ICONHAND);
		return;
	}	
	

	m_list.DeleteSelectedItems();
}

void CIpscanDlg::ShowCompleteInformation()
{
	// Show "Scanning complete" information box
	if (m_szCompleteInformation.GetLength() == 0)
		m_szCompleteInformation = "No last scan information yet.";

	CMessageDlg cMsgDlg;
	cMsgDlg.setMessageText(m_szCompleteInformation);
	cMsgDlg.DoModal();
}


void CIpscanDlg::KillAllRunningThreads()
{	
	// Synchronization has been removed from here to prevent the risk of
	// freezing due to killing a thread before leaving the critical section	
	
	//EnterCriticalSection(&g_criticalSection);				

	for (UINT i=0; i < sizeof(g_threads)/sizeof(g_threads[0]); i++) 
	{
		if (g_threads[i] != THREAD_DEAD) 
		{			
			// Old stuff used here:
			//TerminateThread(g_hThreads[i], 0);
			//CloseHandle(g_hThreads[i]);

			// Tell the thread that it must commit a suicide
			g_threads[i] = THREAD_MUST_DIE;
		}
	}

	//LeaveCriticalSection(&g_criticalSection);
}

void CIpscanDlg::OnClose() 
{
	// Terminate all threads
	if (m_nScanMode == SCAN_MODE_SCANNING)
		KillTimer(1);	// For safety	

	KillAllRunningThreads();

	// Perform auto-saving
	if (g_options->m_bAutoSave)
	{
		g_options->save();
		g_options->saveDimensions();
	}

	CDialog::OnClose();	
}

void CIpscanDlg::OnFavouritesAddcurrentrange() 
{
	// Rercreate IP Feed object (in case it is old or doesn't exist)
	RecreateIPFeed();

	// Add only if it was created without errors
	if (g_pIPFeed)
	{
		// Pass serialized IP Feed to the adder routine
		g_options->addFavourite(m_dlgIPFeed->serialize());
	
		RefreshFavouritesMenu();
	}
}

void CIpscanDlg::RefreshFavouritesMenu()
{
	g_options->initFavouritesMenu(GetMenu()->GetSubMenu(INDEX_FAVOURITES_MENU));
}

void CIpscanDlg::RefreshOpenersMenu()
{
	g_options->initOpenersMenu(GetMenu()->GetSubMenu(INDEX_CONTEXT_MENU)->GetSubMenu(INDEX_OPEN_MENU));
}

void CIpscanDlg::OnExecuteFavouritesMenu(UINT nID)
{	
	if (m_nScanMode != SCAN_MODE_NOT_SCANNING)
		return;

	int nFavouriteIndex = nID - ID_MENU_FAVOURITES_001;
	
	// Feed the serialized content to the feeder dialog
	m_dlgIPFeed->unserialize(g_options->m_aFavourites[nFavouriteIndex].szContent);	

	//
	// TODO: add support for different feeders
	//
}

void CIpscanDlg::OnFavouritesDeleteFavourite() 
{
	g_options->deleteFavourite();	

	RefreshFavouritesMenu();
}

void CIpscanDlg::OnUtilsRemoveSettingsFromRegistry() 
{
	g_options->removeSettingsFromRegistry();	
}

void CIpscanDlg::OnCommandsOpencomputerConfigure() 
{
	CEditOpenersDlg dlg;
	dlg.DoModal();
}

void CIpscanDlg::OnHelpCheckForNewerVersion() 
{
	CIpscanApp::CheckForNewerVersion();
}



