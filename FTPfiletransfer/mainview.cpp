//***********************************************************
//* TJFTP - A Win 95/NT FTP client							*
//* Written February, 1997 in Visual C++ 5.0				*
//*	Recompiled March, 2005 in Visual Studio .NET			*
//*															*
//* Copyright © Jim Dunne 1997 - 2005.  All Rights Reserved	*
//*															*
//* Written by Jim Dunne									*
//* PO Box 33632											*
//* Dayton, OH 45433										*
//*															*
//* topjimmy@topjimmy.net									*
//* www.topjimmy.net/tjs									*
//***********************************************************
// mainview.cpp : implementation of the CMainView class
//

#include "stdafx.h"
#include "TJFTP.h"
#include "mainfrm.h"
#include "TJFTPDoc.h"
#include "mainview.h"
#include "csock.h"
#include "ftpconnect.h"
#include <stdio.h>
#include <direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainView

IMPLEMENT_DYNCREATE(CMainView, CFormView)

BEGIN_MESSAGE_MAP(CMainView, CFormView)
	//{{AFX_MSG_MAP(CMainView)
	ON_MESSAGE(WM_USER_ASYNC_HOST_LOOKUP, OnAsyncHost)
	ON_BN_CLICKED(IDC_BUTTON_MAIN_CONNECT, OnClickedButtonMainConnect)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_LOCAL, OnGetDispInfo)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_REMOTE, OnGetDispInfoRem)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_LOCAL, OnColumnClick)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_LOCAL, OnDblclkListLocal)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_REMOTE, OnColumnclickListRemote)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_REMOTE, OnDblclkListRemote)
	ON_CBN_SELCHANGE(IDC_COMBO_LOCDIR, OnSelchangeComboLocdir)
	ON_BN_CLICKED(IDC_BUTTON_MAIN_LOCREFRESH, OnClickedButtonMainLocrefresh)
	ON_CBN_SELCHANGE(IDC_COMBO_REMDIR, OnSelchangeComboRemdir)
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_LIST_LOCAL, OnBeginlabeleditListLocal)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST_LOCAL, OnEndlabeleditListLocal)
	ON_BN_CLICKED(IDC_BUTTON_MAIN_REMREFRESH, OnClickedButtonMainRemrefresh)
	ON_BN_CLICKED(IDC_BUTTON_MAIN_LOCMKDIR, OnClickedButtonMainLocmkdir)
	ON_BN_CLICKED(IDC_BUTTON_MAIN_LOCDEL, OnClickedButtonMainLocdel)
	ON_BN_CLICKED(IDC_BUTTON_MAIN_REMMKDIR, OnClickedButtonMainRemmkdir)
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_LIST_REMOTE, OnBeginlabeleditListRemote)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST_REMOTE, OnEndlabeleditListRemote)
	ON_BN_CLICKED(IDC_BUTTON_MAIN_REMDEL, OnClickedButtonMainRemdel)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_MAIN_LOCSEND, OnClickedButtonMainLocsend)
	ON_BN_CLICKED(IDC_BUTTON_MAIN_REMRET, OnClickedButtonMainRemret)
	ON_BN_CLICKED(IDC_RADIO_MAIN_BINARY, OnClickedRadioMainBinary)
	ON_BN_CLICKED(IDC_RADIO_MAIN_Ascii, OnClickedRadioMainAscii)
	ON_BN_CLICKED(IDC_RADIO_MAIN_AUTO, OnClickedRadioMainAuto)
	ON_BN_CLICKED(IDC_BUTTON_MAIN_EXIT, OnButtonMainExit)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_USER_STREAM, OnStream)
	ON_MESSAGE(WM_USER_LISTEN, OnListenSocket)
	ON_MESSAGE(WM_USER_ACCEPT, OnAccept)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_STATUS, OnUpdateStatus)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_UTILITY, OnUpdateUtility)

	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CFormView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainView::CMainView() : CFormView(CMainView::IDD)
//
// Sample Call: Not called by the user.
//
// Description: Called when the CMainView Class is created.  Initializes   
// class member variables.
//
// Input/Output data elements:
//	- Input: Argument CMainView::IDD.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
CMainView::CMainView()
	: CFormView(CMainView::IDD)
{
	m_bWinSockOK = FALSE;
	m_nPlaceOnList = 0;
	m_pStream = NULL;

	//{{AFX_DATA_INIT(CMainView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// TODO: add construction code here

}

/////////////////////////////////////////////////////////////////////////////
// CMainView::~CMainView()
//
// Sample Call: Not called by the user.
//
// Description: The destructor for CMainView.
//	- Destroys and deletes any listening, data, or stream sockets.
//	- Shuts down Winsock.
//	- Writes flag values to the Registry.
//
// Input/Output data elements: None.
//
// Error handling:
//	- If Winsock ShutDown() fails, an error message box is displayed.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
CMainView::~CMainView()
{
	// Free the stream and WinSock objects
	if (m_pListenSocket)
	{	//If a listening socket is open, shut it down
		m_pListenSocket->DestroySocket();
		delete m_pListenSocket;
		m_pListenSocket = NULL;
	}
	if (m_pDataSocket)
	{	//If a data socket is open, shut it down
		m_pDataSocket->DestroySocket();
		delete m_pDataSocket;
		m_pDataSocket = NULL;
	}

	if (m_pStream)
	{	//If a stream socket exists, shut it down
		m_pStream->DestroySocket();
		delete m_pStream;
		m_pStream = NULL;
	}
	if(m_pWinsock)
	{	//If m_pWinsock Cwinsock object exists, shut it down
		m_pWinsock->Shutdown();
		if(m_pWinsock->m_nStatus == CWINSOCK_WINSOCK_ERROR)
		{
			wsprintf(m_lpszMessage, "WSACleanup Failure: ( %s)",
				m_pWinsock->ErrorBox(WSAGetLastError(), ""));
			MessageBox(m_lpszMessage, "TJLookUp", 
				MB_OK | MB_ICONEXCLAMATION);
		}						
		delete m_pWinsock;
		m_pWinsock = NULL;
	}

	//Write flag settings to the Registry 
	AfxGetApp()->WriteProfileInt("Options","Binary", m_bBinary); 
	AfxGetApp()->WriteProfileInt("Options","Auto", m_bAuto);
}

void CMainView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMainView)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BOOL CMainView::PreCreateWindow(CREATESTRUCT& cs)
{

	return CFormView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainView diagnostics

#ifdef _DEBUG
void CMainView::AssertValid() const
{
	CFormView::AssertValid();
}

void CMainView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CTJFTPDoc* CMainView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTJFTPDoc)));
	return (CTJFTPDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainView message handlers

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnInitialUpdate() 
//
// Sample Call: Not called by the user.
//
// Description: Handles all the initialization when the main dialog box 
// is first started.
//	- Initializes variables.
//	- Disables buttons.
//	- Get a pointer to the main frame window.
//	- Creates the CProgressCtrl control.
//	- Gets the current year in FILETIME format.
//	- Changes the main window title.
//	- Get the settings for the "Binary" and "Auto" buttons, and sets 
//		their states.
//	- Initializes the local list path and the local and remote 
//		list controls.
//	- Starts the Winsock services.
//	- Displays the "Connect" dialog box.
//
// Input/Output data elements: None.
//
// Error handling:
//	- If the local host lookup fails, an error message is printed 
//		to the screen.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();

	//Initialize variables
	m_nBytesReceived = 0;
	m_bLocMkDir = FALSE;
	m_bRemMkDir = FALSE;
	m_bSendFile = FALSE;
	m_bReadReady = FALSE;
	m_bConClose = FALSE;
	m_csBytes = "";
	m_bConnect = FALSE; 

	//Disable non-applicable buttons
	DisableFunctions();

	//Get a pointer to the main frame window
	CMainFrame *pMainFrame = (CMainFrame *)AfxGetApp()->m_pMainWnd;

	RECT rc;	//Declare a RECT object

	//Get the rectangle for the progress indicator on the status bar
	pMainFrame->m_wndStatusBar.GetItemRect(1, &rc);
	//Create the CProgressCtrl control
    VERIFY (m_wndProgress.Create(WS_CHILD | WS_VISIBLE, rc,
		&pMainFrame->m_wndStatusBar, 1));
    m_wndProgress.SetRange(0, 100);
    m_wndProgress.SetStep(2);

	//Get the current year in FILETIME format
	SYSTEMTIME stTemp;
	::GetLocalTime(&stTemp);
	//Set the time to beginning of this year 
	stTemp.wMilliseconds = 0;
	stTemp.wSecond = 0;
	stTemp.wDay = 1;
	stTemp.wMonth = 1;
	stTemp.wHour = 0;
	stTemp.wMinute = 0;

	if(!::SystemTimeToFileTime(&stTemp, &m_ftYear))
	{
		AfxMessageBox("Couldn't get FILETIME time");
	}

	//Change the main window title
	AfxGetMainWnd()->SetWindowText(TJFTP);

	//Get the settings for the "Binary" and "Auto" buttons
	m_bBinary = AfxGetApp()->GetProfileInt("Options","Binary", 1); 
	m_bAuto = AfxGetApp()->GetProfileInt("Options","Auto", 1);

	if(m_bBinary)
	{
		//Press the "Binary" radio button
		CButton *pRadBin = (CButton *)GetDlgItem(IDC_RADIO_MAIN_BINARY);
		pRadBin->SetCheck(TRUE);
	}
	else
	{
		//Press the "Ascii" radio button
		CButton *pRadAsc = (CButton *)GetDlgItem(IDC_RADIO_MAIN_Ascii);
		pRadAsc->SetCheck(TRUE);
	}

	CButton *pRadAuto = (CButton *)GetDlgItem(IDC_RADIO_MAIN_AUTO);
	if(m_bAuto)
		//Press the "Auto" radio button
		pRadAuto->SetCheck(TRUE);

	//Initialize the local list path and list control
	m_csLocPath = "C:\\";
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_LOCAL);
	m_imglDrives.Create(IDR_DRIVEIMAGES, 16, 1, RGB(255, 0, 255));

	pList->SetImageList(&m_imglDrives, LVSIL_SMALL);
	pList->InsertColumn (0, "File Name", LVCFMT_LEFT, 115);
	pList->InsertColumn (1, "Size", LVCFMT_RIGHT, 60);
	pList->InsertColumn (2, "Last Modified", LVCFMT_CENTER, 93);

	InitLocList(m_csLocPath);
	pList->SortItems(CompareFunc, 0);

	//Initialize the remote list control
	CListCtrl* pRemList = (CListCtrl*)GetDlgItem(IDC_LIST_REMOTE);
	pRemList->SetImageList(&m_imglDrives, LVSIL_SMALL);
	pRemList->InsertColumn (0, "File Name", LVCFMT_LEFT, 115);
	pRemList->InsertColumn (1, "Size", LVCFMT_RIGHT, 60);
	pRemList->InsertColumn (2, "Last Modified", LVCFMT_CENTER, 93);

	//Start the Winsock services
	m_nWinSockError = 0;
	m_pWinsock = new CWinsock;
	if (m_pWinsock->Startup() == CWINSOCK_NOERROR && 
		(m_bWinSockOK == FALSE))
	{
		m_bWinSockOK = TRUE;

		//Change the "Status" pane
		m_csStatus = "Winsock started OK";
		PrintToStatus("%s    Started OK", 
			m_pWinsock->m_wsaData.szDescription);
		if((gethostname((LPSTR)szString, MAXHOSTNAMELEN))
			==SOCKET_ERROR)
		{
			m_csStatus = "Local Host lookup failed";
			PrintToStatus("Local Host lookup failed ( %s)",
				m_pWinsock->ErrorBox(WSAGetLastError(), ""));
		}
		else
  			lstrcpy(m_lpszHost, szString);

		m_bLocalFlag = 1;
		m_hAsyncHost = WSAAsyncGetHostByName(m_hWnd, WM_USER_ASYNC_HOST_LOOKUP,
			m_lpszHost, m_achHostEnt, MAXGETHOSTSTRUCT);
	}
		else
		m_nWinSockError = WSAGetLastError();
	
	//Display the "Connect" dialog
	SetTimer(1,0, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnClickedButtonMainConnect() 
//
// Sample Call: OnClickedButtonMainConnect();
//
// Description: Called when the "Connect" or "Close" button is pressed.
//	- If button in "Close" mode, begins closing actions.
//	- Sets button text to "Connect", disables "Cancel" button.
//	- Gets pointers to the remote list control and combobox.
//	- Clears the remote list control and combobox.
//	- Disables operations buttons.
//	- Cleans up any open file or sockets, and returns.
//	- If button in "Connect" mode, gets pointers to the local list 
//		control and combobox.
//	- Opens the "Connect" dialog box.
//	- If the user presses the "Cancel" button, returns.
//	- If the user presses the "OK" button, changes button text to "Close".
//	- Gets the current host settings, including the local path.
//	- Draws the local directory in the local list control.
//	- Sets the main window caption to reflect the current host name.
//	- If no m_pStream object exists,  creates new CStreamSocket object.
//	- Creates a new stream socket.
//	- Attempts to connect to the server.
//
// Input/Output data elements: None.
//
// Error handling:
//	- If the stream socket creation or connection attempt fail, an error 
//		message is printed to the screen, and the 
//		CStreamSocket object is destroyed.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnClickedButtonMainConnect() 
{
	if(m_bConClose)
	{	//If the button is active (showing "Close")
		m_bConClose = FALSE;
		CButton *pbCon = (CButton *)GetDlgItem(IDC_BUTTON_MAIN_CONNECT);
		pbCon->SetWindowText("Connect");

		//Get pointers to the remote list control and combobox
		CListCtrl* pRemList = (CListCtrl*)GetDlgItem(IDC_LIST_REMOTE);
		CComboBox *pcbRem = (CComboBox *)GetDlgItem(IDC_COMBO_REMDIR);

		//Clear the remote list control and combobox
		pRemList->DeleteAllItems();
		pcbRem->ResetContent();

		m_nBytesReceived = 0;
		m_nTotalSent = 0;
		DisableFunctions();

		if(m_cfUserFile)
		{	//If a file is open, close it
			m_cfUserFile->Close();
			delete m_cfUserFile;
			m_cfUserFile = NULL;
		}
		if (m_pStream)
		{	//If a stream socket is open
			AfxGetMainWnd()->SetWindowText(TJFTP);
			if(m_bConnect)
			{
				SendCommand("QUIT", "");
				ReadResponse();
			}
			//Destroy the socket
			m_pStream->DestroySocket();
			delete m_pStream;
			m_pStream = NULL;
			m_csStatus = "Disconnected";
		}
		if (m_pListenSocket)
		{	//If the listening socket is open
			m_pListenSocket->DestroyDataSocket();
			delete m_pListenSocket;
			m_pListenSocket = NULL;
		}
		if (m_pDataSocket)
		{	//If a data socket is open
			m_pDataSocket->DestroySocket();
			delete m_pDataSocket;
			m_pDataSocket = NULL;
		}
		 return;
	}

	//Get pointers to local list control and combobox
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_LOCAL);
	CComboBox *pcbLoc = (CComboBox *)GetDlgItem(IDC_COMBO_LOCDIR);
	
	m_csStatus = "Starting new session...";
	//Letting the idle processing mechanism update the status bar
	UpdateControls();

	//Open the "Connect" dialog box
	CFtpConnect dlg;
	m_nConDialStatus = dlg.DoModal();
	
	if(m_nConDialStatus == IDCANCEL)
	{	//If the user pressed the "Cancel" button
		m_csStatus = "Idle";
		return ;
	}

	if(m_nConDialStatus == IDOK)
	{	//If the user presed the "OK" button
		//Set the first button's text to "Close"
		CButton *pbCon = (CButton *)GetDlgItem(IDC_BUTTON_MAIN_CONNECT);
		pbCon->SetWindowText("Close");

		m_csStatus = "Attempting to connect...";

		//Set the flag in case the user presses the "Close" button
		m_bConClose = TRUE;
		//Get the Current host settings 
		csHost = AfxGetApp()->GetProfileString("Hosts\\Current",
			"Host Address"); 
		csUser = AfxGetApp()->GetProfileString("Hosts\\Current",
			"User"); 
		csPass = AfxGetApp()->GetProfileString("Hosts\\Current",
			"Pass"); 
		m_csRemDir = AfxGetApp()->GetProfileString("Hosts\\Current",
			"Remote Directory"); 
		m_csLocDir = AfxGetApp()->GetProfileString("Hosts\\Current",
			"Local Directory"); 
		m_csLocPath = m_csLocDir;

		//Draw the local directory in the list control
		DrawLocList();
		//Perform idle processing
		UpdateControls();

		//Set the main window caption to reflect the current host
		wsprintf(m_lpszMessage,TJFTP);
		strcat(m_lpszMessage, " - "); 
		strcat(m_lpszMessage, AfxGetApp()->GetProfileString("Hosts\\Current", "Profile")); 
		AfxGetMainWnd()->SetWindowText(m_lpszMessage);

		if(m_pStream == NULL)
		{	//If no m_pStream object exists,
			// initialize the stream socket object
			m_pStream = new CStreamSocket(this, WM_USER_STREAM);
			//Create the stream socket
			if (m_pStream->CreateSocket() != CWINSOCK_NOERROR)
			{	//If socket creation fails...
				m_csStatus = "Couldn't create the stream socket";
				PrintToStatus("Couldn't create the stream socket", NULL);
				CButton *pbCon = (CButton *)GetDlgItem(IDC_BUTTON_MAIN_CONNECT);
				pbCon->SetWindowText("Connect");
				m_bConClose = FALSE;
				delete m_pStream;
				m_pStream = NULL;
				return;
			}
		}
		//Copy the Current host address to a string pointer
		wsprintf(m_pszServer,csHost);
		
		//Connect the client to the server
		if (m_pStream->Connect(m_pszServer, 21) != CWINSOCK_NOERROR)
		{	//If the connection failed
			m_csStatus = "Couldn't connect to the server";
			PrintToStatus("Couldn't connect to the server", NULL);
			CButton *pbCon = (CButton *)GetDlgItem(IDC_BUTTON_MAIN_CONNECT);
			pbCon->SetWindowText("Connect");
	   		m_bConClose = FALSE;
			m_pStream->DestroySocket();
			delete m_pStream;
			m_pStream = NULL;
			return;
		}
	
	}
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::SendCommand(CString csString1, CString csString2)
//
// Sample Call: SendCommand("USER ", "Jim");
//
// Description: Sets the "wait" cursor and sends the command out 
// on the stream socket.
//	- Sets the "wait" cursor.
//	- Combines csString1 and csString2 and adds a CR/LF to the end.
//	- Sends the string out on the stream socket using the "Write" command.
//
// Input/Output data elements:
//	- Input: CString arguments csString1 and csString2.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::SendCommand(CString csString1, CString csString2)
{
	CWaitCursor cWait;	//Set the "wait" cursor
	//Combine csString1 and csString2 and add a carriage return to the end
	wsprintf(m_pszBuf,"%s%s\r\n", csString1, csString2);
	//Send the string out on the stream socket
	m_pStream->Write(strlen(m_pszBuf)/*+ 1*/, m_pszBuf);
	return;
}

/////////////////////////////////////////////////////////////////////////////
// LONG CMainView::OnAsyncHost(WPARAM wParam, LPARAM lParam)
//
// Sample Call: WSAAsyncGetHostByName(m_hWnd, WM_USER_ASYNC_HOST_LOOKUP,
//			m_stringHostName, m_achHostEnt, MAXGETHOSTSTRUCT);
//
// Description: Called when the asynchronous lookup is done.  The message 
// and the function are linked by the ON_MESSAGE(WM_USER_ASYNC_HOST_LOOKUP, 
// OnAsyncHost) entry in the message maps. It contains either the requested 
// lookup information, or an error message if the lookup failed.
//	- Typecasts lParam to WSAGETASYNCERROR to get any error messages.
//	- Assigns a HOSTENT host entry pointer to the buffer m_achHostEnt.
//	- Copies the four-byte IP address into an Internet address structure.
//	- Formats the results, converting the IP address into a string 
//		and an unsigned long variable.
//	- Cleans up and returns.
//
// Input/Output data elements:
//	- Input: Arguments WPARAM wParam and LPARAM lParam.
//	- Output: Returns zero.
//
// Error handling:
//	-If the host lookup fails, prints an error message to the screen.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
LONG CMainView::OnAsyncHost(WPARAM wParam, LPARAM lParam)
{	//If there is an error message
	if (WSAGETASYNCERROR(lParam) != 0)
	{	//If this is a local lookup 
		if (m_bLocalFlag)
		{
			m_csStatus = "Local Host lookup failed";
			PrintToStatus("Local Host lookup failed ( %s)",
				m_pWinsock->ErrorBox(WSAGETASYNCERROR(lParam), ""));
		}
		else
		{	//This is a normal lookup
			m_csStatus = "Host lookup failed";
			PrintToStatus("Host lookup failed ( %s)",
				m_pWinsock->ErrorBox(WSAGETASYNCERROR(lParam), ""));
		}
	}
	else
	{	
		//Assign a hostent host entry pointer to the buffer
		m_pHostEnt = (PHOSTENT)m_achHostEnt;

		//Copy the four byte IP address into
		//an Internet address structure
		memcpy(&m_in, m_pHostEnt->h_addr, 4);

		//Format the results, converting the 
		//IP address into a string
		if (m_bLocalFlag)
		{	//If this is for the local lookup
			csMain.LoadString(IDS_STRING61205);
			wsprintf(m_lpszMessage, csMain,
				m_pHostEnt->h_name, inet_ntoa(m_in));
			m_csStatus.Format("  %s   %s",
				m_pHostEnt->h_name, inet_ntoa(m_in));
			PrintToStatus(m_lpszMessage, NULL);
			//This is the local IP address
			m_ulIPLocal = inet_addr(inet_ntoa(m_in));
		}
		else
		{
			//Put the resolved IP address into m_ulIPAddress 
			m_ulIPAddress = inet_addr(inet_ntoa(m_in));
			wsprintf(m_lpszMessage, "Host %s has IP address %s",
					m_pHostEnt->h_name, inet_ntoa(m_in));
			PrintToStatus(m_lpszMessage, NULL);
		}
	}
//Cleanup
m_hAsyncHost = 0;
m_bLocalFlag = 0;
return 0L;
}

/////////////////////////////////////////////////////////////////////////////
// LONG CMainView::OnStream(WPARAM wParam, LPARAM lParam)
//
// Sample Call: WSAAsyncSelect(m_s, m_hWnd, 
//	CWINSOCK_EVENT_NOTIFICATION, lEvent) where
//			lEvent = FD_READ | FD_WRITE | FD_ACCEPT | FD_CLOSE | FD_CONNECT;
//
// Description: Receives messages from the stream socket.
//	- Switch statement handles cases based on value of wParam.
//	- If the socket is done writing:
//		- Gets the written data.
//		- Strips the CR/LF from the end, and prints the data to the screen, 
//		hiding passwords, if a PASS command message was sent.
//	- If there was an error writing, shows an error message to the screen 
//		and status bar with the data that was meant to be sent.
//	- If the socket is done reading, sets Timer 2 if m_bReadReady is TRUE.
//		- If m_bReadReady is FALSE, sets it TRUE.
//	- If there was an error reading, shows an error message box.
//	- If a successful connection was made:
//		- Reads the connection request response.
//		- Starts the login sequence.
//		- Shows the login directory in the remote list if the 
//			login was successful.
//	- If the connection was lost, sets m_bConnect to False .
//
// Input/Output data elements:
//	- Input: Arguments WPARAM wParam and LPARAM lParam.
//	- Output: Returns zero.
//
// Error handling:
//	- If any of the ReadResponse() calls fails, the stream socket is killed.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
LONG CMainView::OnStream(WPARAM wParam, LPARAM lParam)
{
	CWaitCursor cWait;		//Set the wait cursor
	LPVOID pDataWritten;	//Pointer to data that is completely written
	CString csTemp,
			csDoneWrite;

	switch (wParam)
	{
	case CWINSOCK_READY_TO_ACCEPT_CONNECTION:
	break;	//Don't use this here

    case CWINSOCK_DONE_WRITING:
		//The socket tells us that it has written out all data
		//lParam = pointer to data that was sent
		//Typecast lParam as LPVOID to get at the data
		pDataWritten = (LPVOID)lParam;	//Put data into LPVOID object
		csDoneWrite.Format("%s",pDataWritten);	//Put data into a string
		//Parse individual lines and show them
		//Strip the CR/LF from the line
		csTemp = csDoneWrite.Left(csDoneWrite.Find("\r\n"));
		//If the password was sent, don't show it to the screen 
		if(csTemp.Left(4) == "PASS")
			PrintToStatus("PASS *****", NULL);	//Show asterisks instead
		else	//otherwise, print the line to the status window
			PrintToStatus(csTemp, NULL);
		free(pDataWritten);	//Free the memory for the pDataWritten
		pDataWritten = NULL;
		break;

    case CWINSOCK_ERROR_WRITING:
		//If there was an error writing the data to the socket
		//lParam = pointer to data that generated error sending
		pDataWritten = (LPVOID)lParam;
		csTemp.Format("%s",pDataWritten);
		csTemp = csTemp.Left(csTemp.Find("\r\n"));
		//Show which data didn't get sent
		csDoneWrite.Format("Error sending data (%s)", csTemp);
		PrintToStatus(csDoneWrite, NULL);
		m_csStatus = csDoneWrite;	//Show error on status bar
		UpdateControls();
		free(pDataWritten);
		pDataWritten = NULL;
		break;

	case CWINSOCK_DONE_READING:
		//The socket is finished reading data
		if(m_bReadReady)	//If m_bReadReady = TRUE
			SetTimer(2,100, NULL);	//Set timer 2 to 100 ms
		else	//If m_bReadReady = FALSE
			m_bReadReady = TRUE;
		break;

	case CWINSOCK_ERROR_READING:
		AfxMessageBox("Error Reading");
		break;

	case CWINSOCK_YOU_ARE_CONNECTED:
		//Successful connection - show message on status bar
		m_bConnect = TRUE; 
		m_csStatus = "Connected to server";
		UpdateControls();
		//Read the connection request response
		if(!ReadResponse())
		{
			//If an error, kill the stream object
			KillStream();
		}
		else
		{	//Send login commands and read the responses
			//Clean-up if errors are received
			SendCommand("USER ", csUser);
			if(!ReadResponse())
				KillStream();
			else
			{
				SendCommand("PASS ", csPass);
				if(!ReadResponse())
					KillStream();
				else
				{
					SendCommand("CWD ", m_csRemDir);
					if(!ReadResponse())
						KillStream();
					else
					{	//If successful login, show the login directory
						//in the Remote list control
						if(PostDirectory())
							EnableFunctions();
					}
				}
			}
		}
		break;

	case CWINSOCK_LOST_CONNECTION:
		// server closed the connection
		m_bConnect = FALSE; 
		break;

    default:
      break;
  }
  return 0L;
}

/////////////////////////////////////////////////////////////////////////////
// LONG CMainView::OnListenSocket(WPARAM wParam, LPARAM lParam)
//
// Sample Call: WSAAsyncSelect(m_s, m_hWnd, 
//	CWINSOCK_EVENT_NOTIFICATION, lEvent) where
//			lEvent = FD_READ | FD_WRITE | FD_ACCEPT | FD_CLOSE;
//
// Description: Receives messages from the listening socket.
//	- Handles the CWINSOCK_READY_TO_ACCEPT_CONNECTION message, indicating 
//		a connection request from the FTP server.
//	- Checks to make sure a data socket is not already open.
//	- Accepts the connection request by creating a new CStreamSocket object, 
//		and then calling the Winsock function "accept()" with the object.
//	- If the connection is accepted, destroys the listening socket.
//
// Input/Output data elements:
//	- Input: Arguments WPARAM wParam and LPARAM lParam. 
//	- Output: Returns zero.
//
// Error handling:
//	- If the Accept() command fails, deletes the new CStreamSocket object.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
LONG CMainView::OnListenSocket(WPARAM wParam, LPARAM lParam)
{	
	CWaitCursor cWait;

	switch (wParam)
	{
    case CWINSOCK_READY_TO_ACCEPT_CONNECTION:
		//The FTP server wants to connect to us
		//Make sure the server is not already servicing a client
		if (m_pDataSocket != NULL)	
			{	//If a data socket already exists
				PrintToStatus("Already servicing a client", NULL);
				m_csStatus = "Already servicing a client";
				UpdateControls();
				break;
			}

		int nStatus;
		//Accept the client connection by creating a new socket...
		m_pDataSocket = new CStreamSocket(this, WM_USER_ACCEPT);
		//And creating a data socket from the new socket  
		nStatus = m_pListenSocket->Accept(m_pDataSocket);

		if (nStatus != CWINSOCK_NOERROR)
		{	//If the socket creation failed
			delete m_pDataSocket;
			m_pDataSocket = NULL;
			PrintToStatus("Error accepting connection from server", NULL);
			m_csStatus = "Error accepting connection from server";
			UpdateControls();
			break;
		}
		else
		{	//If the Accept socket creation was successful
			m_csStatus = "Accepted server connection";
			UpdateControls();
			//Don't need the listening socket anymore
			m_pListenSocket->DestroyDataSocket();
			delete m_pListenSocket;
			m_pListenSocket = NULL;
		}
		break;

	default:
		break;
	}
	return (0L);
}

/////////////////////////////////////////////////////////////////////////////
// LONG CMainView::OnAccept(WPARAM wParam, LPARAM lParam)
//
// Sample Call: WSAAsyncSelect(m_s, m_hWnd, 
//	CWINSOCK_EVENT_NOTIFICATION, lEvent) where
//			lEvent = FD_READ | FD_WRITE | FD_ACCEPT | FD_CLOSE;
//
// Description: Receives messages from the data socket.
//	- Initializes variables.
//	- Switch statement handles cases based on value of wParam.
//	- If the socket has finished writing:
//		- Updates the progress control and statistics on the status bar
//		- If the complete file has been sent:
//			- Sets the progress control to 0.
//			- Destroys and deletes the data socket.
//			- Closes and deletes the file handle.
//	- If there is an error writing data - prints the data that could not 
//		be written to the screen.
//	- If the socket is done reading:
//		- Reads the data from the socket and puts into a character string.
//		- If a file is being received:
//			- Updates the progress control and status bar satistics.
//			- If receiving in ASCII mode, finds every line of data and 
//				writes to the user file.
//			- If receiving in binary, writes data to the user file.
//		- If a file listing is being received:
//			- Copies the data to a string.
//			- Updates the "Bytes Received:" stat on the status bar.
//			- Finds each line of text and adds to m_csRemFileList.
//	- If the server connected to the client successfully, gets the server's 
//		name and the port it connected on and shows this information on 
//		the status bar.
//	- If the connection is lost:
//		- If sending or receiving a file, set the progress control to 0, 
//			close and delete the user file handle, and refresh the local list.
//		- If receiving a file list:
//			- Deletes all items from the remote list.
//			- Parses each line of m_csRemFileList for file/directory information.
//			- Adds items to the remote list, sorts them, and redraws the list.
//		- Cleans up the data and listening sockets.
//
// Input/Output data elements:
//	- Input: Arguments WPARAM wParam and LPARAM lParam.
//	- Output: Returns zero.
//
// Error handling:
//	- If there is an error getting the connecting server's information, 
//		prints an error message to the screen and updates the status bar.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
LONG CMainView::OnAccept(WPARAM wParam, LPARAM lParam)
{	
	CWaitCursor cWait;
	int nStrCount = 0,		//Position in string
		nLen = 0,			//Length
		nStatus = 0,		//Error status
		nCrLfPos = 0,		//CR/LF position		
		ncsLineLen = 0;		//Line length
	char pszData[READ_BUF_LEN + 1];		//Informational message
	char pszMessage[READ_BUF_LEN + 1];	//Informational message
	CString csLine,
			csTemp,
			csDoneWrite;
	LPVOID pDataWritten;  //Pointer to data that is completely written
	LPVOID pDataRead;     //Pointer to data just read
	SOCKADDR_IN sin;      //Internet address of client
	IN_ADDR in;           //IP address of client

  switch (wParam)
  {
    case CWINSOCK_DONE_WRITING:
		//The data socket has finished writing
		//lParam = pointer to data that was sent
		//Update the progress indicator on the status bar
		m_wndProgress.SetPos((m_nTotalSent * 100)/m_nFileSize);
		//And the status message on the status bar
		m_csStatus.Format("%u of %u bytes sent, %u%% done",
			m_nTotalSent, m_nFileSize, (m_nTotalSent * 100)
			/m_nFileSize);

		m_bPieceDone = TRUE;	//This piece of data is done
		if(m_bWriteDone)
		{	//If the whole file has been sent
			m_csStatus = "Transfer complete.";
			m_wndProgress.SetPos(0);
			UpdateControls();	//Make sure status bar gets updated
			m_bWriteDone = FALSE;
			//Clean up the socket and file handle
			m_pDataSocket->DestroySocket();
			delete m_pDataSocket;
			m_pDataSocket = NULL;

			m_cfUserFile->Close();
			delete m_cfUserFile;
			m_cfUserFile = NULL;
			//Indicate that there is no file transfer going on
			m_csListOrFile == "";
		}
		break;

	case CWINSOCK_ERROR_WRITING:
		//There was an error writing the data
		//lParam = pointer to data that generated error sending
		pDataWritten = (LPVOID)lParam;
		wsprintf(pszMessage, "Error sending data (%s)", pDataWritten);
		PrintToStatus(pszMessage, NULL);	//Show the data 
		free(pDataWritten);
		pDataWritten = NULL;
		break;

	case CWINSOCK_DONE_READING:
		//Socket is done reading
		//lParam = # data chunks in queue
		//Clear string buffers
		memset(pszData, 0, READ_BUF_LEN + 1);
		*pszData = '\0';	//Clear pszData
		//Read socket data and copy to buffer
		pDataRead = m_pDataSocket->Read(&nLen);
		wsprintf(pszData,"%s",pDataRead);
		//Append terminator to string
		pszData[nLen] = '\0';
		//For the progress control
		m_nBytesReceived += nLen;
		//If we are receiving a file
		if(m_csListOrFile == "Receiving File")
		{	//Update the progress indicator and status bar message
			m_wndProgress.SetPos((m_nBytesReceived * 100)/m_nFileSize);
			m_csStatus.Format("%u of %u bytes received, %u%% done",
				m_nBytesReceived, m_nFileSize, (m_nBytesReceived * 100)
				/m_nFileSize);
			UpdateControls();	//Make sure status bar gets updated

			//If this is an ASCII transfer  
			if(!m_bBinary ||(m_bAuto && 
				m_csLocalFile.Right(3)== "txt"))
			{	//Copy the data to cur
				char* cur = (char*)pszData;
				char* cr = strchr(cur, '\r'); //Location of the CR
				while (cr)	//While there is another CR
				{	//Terminate the string
					*cr++ = 0;
					//Will go from cur to the null termination we just put in
					m_cfUserFile->Write((char*) cur,strlen(cur));
					cur = cr;	//Skip over the null
					cr = strchr(cur, '\r');	//Search again
				}
				//Write out any leftover amount
				m_cfUserFile->Write((char*) cur,strlen(cur));
			}
			else	//This is binary data, just write it 
				m_cfUserFile->Write((char*)pDataRead, nLen);
		}
		//If we are receiving a file list
		if(m_csListOrFile == "Receiving List")
		{	//Copy the data to pszData
			wsprintf(pszData,"%s",pDataRead);
			//Show bytes recieved in third status pane
			m_csBytes.Format("Bytes received: %u", m_nBytesReceived);
			UpdateControls();	//Make sure status bar gets updated
			//Append terminator to string
			pszData[nLen] = '\0';
			csLine.Format(pszData);	//Convert data to a CString
			//Add the new data to any leftover data from before
			csLine = m_csLeftovers + csLine;
			//While a CR/LF exists
			while((nCrLfPos = csLine.Find("\r\n")) != -1)
			{	//Get the line length
				ncsLineLen = csLine.GetLength();
				//csTemp = everything before the first CR/LF
				csTemp = csLine.Left(nCrLfPos);
				//Add the whole line, including CR/LF, to m_csRemFileList
				m_csRemFileList += csLine.Left(nCrLfPos + 2);
				//csLine = the data minus the current line (including
				//the CR/LF)
				csLine = csLine.Right(ncsLineLen - nCrLfPos - 2);
			}
			//What's left are leftovers with no CR/LF
			m_csLeftovers = csLine; 
		}
		free(pDataRead);
		pDataRead = NULL;
		break;

	case CWINSOCK_ERROR_READING:
		break;

	case CWINSOCK_YOU_ARE_CONNECTED:
		//We've connected to the server
		//Print out the client information
		nStatus = m_pStream->GetPeerName(&sin);
		if (nStatus == CWINSOCK_NOERROR)
		{	//If no errors, show connection info
			memcpy(&in, &sin.sin_addr.s_addr, 4);
			//Show which port the server connected to
			m_csStatus.Format("Server %s connected to data port %d",
				inet_ntoa(in), ntohs(m_sinaddrDataPort.sin_port));
			UpdateControls();	//Make sure status bar gets updated

		}
		else
		{	//Couldn't get server name
			PrintToStatus("Error getting server name", NULL);
			m_csStatus = "Error getting server name";
		}
		break;

    case CWINSOCK_LOST_CONNECTION:
		//Client closed the connection
		if((m_csListOrFile == "Receiving File") ||
			(m_csListOrFile == "Sending File"))
		{	//Set the progress indicator to 0 and clean up
			m_wndProgress.SetPos(0);				
			m_cfUserFile->Close();
			delete m_cfUserFile;
			m_cfUserFile = NULL;
			m_csListOrFile == "";
			OnClickedButtonMainLocrefresh();
		}

		if(m_csListOrFile == "Receiving List")
		{	//Ready to draw the remote list - get a pointer to the control 
			CListCtrl* pRemList = (CListCtrl*)GetDlgItem(IDC_LIST_REMOTE);
			CWaitCursor cWait;
			//Free all item memory and clear the list
			FreeItemMemoryRem();
			pRemList->DeleteAllItems();
			//Don't redraw it till we're done
			SetRedraw(FALSE);
			//Add the ".." item to the list
			AddItemRemList(0, "..", "0", m_ftYear, 0);
			//Parse the list for individual lines
			while((nCrLfPos = m_csRemFileList.Find("\r\n")) != -1)
			{	//Find data up to the first CR/LF
				ncsLineLen = m_csRemFileList.GetLength();
				//csTemp = left side of data before CR/LF 
				csTemp = m_csRemFileList.Left(nCrLfPos);
				ParseInfo(csTemp);	//Parse lines and display
				//Strip off the left side of the data
				m_csRemFileList = m_csRemFileList.Right(ncsLineLen
					- nCrLfPos - 2);
			}
			//Sort the items in the list
			pRemList->SortItems(CompareFuncRem, 0);
			//Redraw the list once all items are there 
			SetRedraw(TRUE);
			InvalidateRect(NULL,FALSE);
			m_csListOrFile == "";
		}
		//Clean up sockets
		if(m_pDataSocket)
		{
			m_pDataSocket->DestroySocket();
			delete m_pDataSocket;
			m_pDataSocket = NULL;
			m_csStatus = "Data connection closed";
			UpdateControls();	//Make sure status bar gets updated
		}
		if(m_pListenSocket)
		{
			m_pListenSocket->DestroySocket();
			delete m_pListenSocket;
			m_pListenSocket = NULL;
		}
		break;

    default:
      break;
  }
 	return (0L);
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::ParseInfo(CString csLine)
//
// Sample Call: ParseInfo(csLine);
//
// Description: Parses the file list and adds the item to the 
// remote list control.
//	- Initializes variables.
//	- Finds the beginning of the file name.
//	- Determines if item is a file, directory, or link, and assigns the 
//	appropriate value to Dirlist.
//	- Parses the file name, date and time, and the file size from the string.
//	- Calls AddItemRemList to add the item to the list.
//
// Input/Output data elements:
//	- Input: Argument CString csLine.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::ParseInfo(CString csLine)
{
	FILETIME FileTime;	//Create a FILETIME object
	int DirList = 0,
		position = 0,
		nLineLen = csLine.GetLength(),
		nSkips = 0;
	CString tmp,
			csFileName,
			csRemFileSize;

	//Find the beginning of the filename - looks for a space in the line
	for(int i=1; i<nLineLen; i++)
	{	//If a space is found that is not preceded by a space
		if((csLine[i] == ' ') && (csLine[i -1] != ' '))
			nSkips++;
		//If we are at the 9th character
		if(nSkips == 8)
		{
			position = i;
			break;
		}
	}
	//If the first character is a ...
	if (csLine[0] == 'd')
		DirList = 0;	//It is a directory
	else 
		if(csLine[0] == 'l')
			DirList = 1;	//It is a link
	else
		if((csLine[0] == '-') || (csLine[0] == 's'))
			DirList = 2;	//It is a file or a system file
	else return;
	//The filename is at the right end of the string
	csFileName = csLine.Right(nLineLen - position - 1);
	//Get the left side of the string, minus the filename
	tmp = csLine.Left(position);
	//The file's date and time is the last 12 chars of the new string  
	FileTime = TimeConversion(tmp.Right(12));	//Convert it
	//Get the left side of the string, minus the date/time
	tmp = tmp.Left(tmp.GetLength() - 13);
	//Find the first space, beginning at the right side of the string
	position = tmp.ReverseFind(' ');
	//The filesize is now at the end of the string
	csRemFileSize = tmp.Right(tmp.GetLength() - position - 1);
	//Add the entry to the remote list control, if it is not "." or ".."
	if((csFileName != ".") && (csFileName != ".."))
		AddItemRemList(0, csFileName, csRemFileSize,
			FileTime, DirList);	
}

/////////////////////////////////////////////////////////////////////
// CMainView Remote list control functions

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnDblclkListRemote(NMHDR* pNMHDR, LRESULT* pResult) 
//
// Sample Call: OnDblclkListRemote(pNMHDR, pResult);
//
// Description: Called when the user double-clicks the mouse on the 
// remote list  control.
//	- Initializes variables.
//	- Gets pointers to the remote list control and combobox.
//	- Determines if the mouse pointer was over a list item when clicked. 
//	- Get the text of the item's label.
//	- Sets the attributes for the LV_ITEM object.
//	- Gets the item info for the selected item.
//	- If selection is ".." (up arrow icon), sends a "CDUP" command.
//	- If selection is a directory or link:
//		- If a link, parses the directory name from it.
//		- Gets the directory path from the combobox.
//		- Adds the item's text to the end of the path.
//		- Sends the CWD command with the new path.
//		- Reads response and updates the remote list control
//	- If the selection is a file, calls OnClickedButtonMainRemret().
//
// Input/Output data elements:
//	- Input: Arguments NMHDR* pNMHDR and LRESULT* pResult.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnDblclkListRemote(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CString csText,
			csRemCBTxt;
	int nIndex = 0,
		nLen = 0,
		nDir = 0,
		nPosition=0;
	//Get pointers to the remote list control and combobox
	CListCtrl* pRemList = (CListCtrl*)GetDlgItem(IDC_LIST_REMOTE);
	CComboBox* pcbRem = (CComboBox*)GetDlgItem(IDC_COMBO_REMDIR);
	LV_ITEM lvRem;

	m_bReadReady = FALSE;	//Set the read flag
	m_nBytesReceived = 0;	//Reset bytes counter 

	if((nIndex = pRemList->GetNextItem(-1, LVNI_SELECTED)) != -1)
	{	//If the mouse pointer was over a list item
		CWaitCursor cWait;
		//Get the text of the item's label
		csText = pRemList->GetItemText(nIndex, 0);
		//Set attributes for the LV_ITEM object
		lvRem.mask = LVIF_IMAGE;	//Image bit is valid
		lvRem.iItem = nIndex;	//Get info for the clicked item
		lvRem.iSubItem = 0;	//Don't want info on subitems 
		pRemList->GetItem(&lvRem);	//Get the info

		//If selection is ".." (up arrow)
		if(lvRem.iImage == 6)
		{	//Change directory up
			SendCommand("CDUP", "");
			if(ReadResponse())
				PostDirectory();
		}
		//If selection is a directory or link
		if((lvRem.iImage == 3) || (lvRem.iImage == 4))
		{	//If selection is a link
			if(lvRem.iImage == 4)
			{	//Get the directory path from the link path
				csText = csText.Right(csText.GetLength() - 
					csText.Find(" ->") - 4);
				if(csText.Left(1) == "/")
					csText = csText.Right(csText.GetLength() - 1);
			}		
			
			//Get the directory path from the combobox
			pcbRem->GetLBText(pcbRem->GetCurSel(), csRemCBTxt);
			//If the end of the path is not a "/"
			if(csRemCBTxt.Right(1) != '/')
				//Add a "/", and then the new directory
				csRemCBTxt += "/" + csText;
			else	//"/" is already there, just add new directory
				csRemCBTxt += csText + "/";
			//Change the remote host to the new directory
			SendCommand("CWD ", csRemCBTxt);
			if(ReadResponse())
				PostDirectory();
			else if(lvRem.iImage == 4)
				OnClickedButtonMainRemret();
		}
		//If selection is a file
		if(lvRem.iImage == 5)
			OnClickedButtonMainRemret();
	}
	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnColumnclickListRemote(NMHDR* pNMHDR, LRESULT* pResult) 
//
// Sample Call: OnColumnclickListRemote(pNMHDR, pResult);
//
// Description: Called when the user left-clicks the mouse on a column 
// header  on the remote list control.
//	- Creates an NM_LISTVIEW* pointer by typecasting pNMHDR.
//	- Gets a pointer to the remote list control.
//	- Sorts the list based on which column header was clicked. 
//
// Input/Output data elements:
//	- Input: Arguments NMHDR* pNMHDR and LRESULT* pResult.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnColumnclickListRemote(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	//Get a pointer to the remote list control
	CListCtrl* pRemList = (CListCtrl*)GetDlgItem(IDC_LIST_REMOTE);
	//Sort the list based on which column header was clicked (iSubItem)
    (pRemList->SortItems(CompareFuncRem, pNMListView->iSubItem));
}

/////////////////////////////////////////////////////////////////////////////
// BOOL CMainView::AddItemRemList (int nIndex, LPCTSTR lpszRemFile,
//		CString csFileSize, FILETIME FileTime, int DirFile)
//
// Sample Call: AddItemRemList(0, csFileName, csRemFileSize,
//			FileTime, DirList);
//
// Description: Adds an item to the remote list control.
//	- Gets a pointer to the control.
//	- Initializes a new ITEMINFO object, pItem, and sets its member's variables.
//	- Initializes a new LV_ITEM object, lvi, and sets its member's variables.
//	- Sets the icon next to the item based on what the item's DirFile 
//		value is.  DirFile = the bitmap's 0-based index in the bitmap image list.
//	- Inserts the item with InsertItem(). 
//
// Input/Output data elements:
//	- Input: Arguments int nIndex, LPCTSTR lpszRemFile, CString csFileSize, 
//		FILETIME FileTime, int DirFile.
//	- Output:  Boolean return value.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
BOOL CMainView::AddItemRemList (int nIndex, LPCTSTR lpszRemFile,
		CString csFileSize, FILETIME FileTime, int DirFile)
{	//Get a pointer to the control
	CListCtrl* pRemList = (CListCtrl*)GetDlgItem(IDC_LIST_REMOTE);
    ITEMINFO* pItem;
	pItem = new ITEMINFO;  //Initialize a new ITEMINFO object
	//Set the pItem variables
    pItem->dwFileAttributes = DirFile;
	pItem->strFileName = lpszRemFile;
	pItem->nFileSizeLow =atoi(csFileSize);
	pItem->ftLastWriteTime = FileTime;

    LV_ITEM lvi;
	//Set the masks for the LV_ITEM lvi
    lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
	lvi.state = 0;
    lvi.iItem = nIndex; 
    lvi.iSubItem = 0;

	//Set the icon next to the item based on what the item is
	//If it's a directory
	if (DirFile == 0)
		lvi.iImage = 3;		//Use the third image in the bitmap 
	else	//If it's a link
		if (DirFile == 1)
			lvi.iImage = 4;
	else	//If it's a file
		if(DirFile == 2)
			lvi.iImage = 5;
	//If it's the ".." directory, use the "UP" arrow graphic
	if (!(lstrcmp(lpszRemFile, "..")))
		lvi.iImage = 6;
	//The text is CALLBACK, i.e., the application handles it, not us
	lvi.pszText = LPSTR_TEXTCALLBACK;
	//Set the lvi.lParam by typecasting pItem to LPARAM
    lvi.lParam = (LPARAM) pItem;
	//Insert the item, now that all parameters are specified
    if (pRemList->InsertItem (&lvi) == -1)
        return FALSE;

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnGetDispInfoRem (NMHDR* pnmh, LRESULT* pResult)
//
// Sample Call: Not called by the user.
//
// Description: Assigns values to the remote list control's items and subitems.
//	- Places info into LV_DISPINFO* structure plvdi by typecasting 
//		pnmh as LV_DISPINFO*.
//	- Creates ITEMINFO* structure pItem and puts the info into pItem by 
//		typecasting plvdi as ITEMINFO*.
//	- Switch statement uses plvdi->item.iSubItem for cases:
//		- case 0: SubItem 0 is the file name - copies the file name 
//			to the item structure.
//		- case 1: File size - formats the file size into a string and copies 
//			to plvdi->item.pszText.
//		- case 2: Date and time:
//			- Creates a CTime object from the file's FILETIME descriptor.
//			- Formats a string to show the date and time.
//			- Copies the date/time info to the list control.
//
// Input/Output data elements:
//	- Input: Arguments NMHDR* pnmh, LRESULT* pResult.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnGetDispInfoRem (NMHDR* pnmh, LRESULT* pResult)
{	
    CString string;
	//Place info into plvdi by typecasting pnmh as LV_DISPINFO*
    LV_DISPINFO* plvdi = (LV_DISPINFO*) pnmh;
	//If the mask is set to text
    if (plvdi->item.mask & LVIF_TEXT)
	{
 		//Put the info into pItem by typecasting
       ITEMINFO* pItem = (ITEMINFO*) plvdi->item.lParam;

	   //Which item are we dealing with?
		switch (plvdi->item.iSubItem)
		{

        case 0: //SubItem 0 is the file name
			//Copy the file name to the item structure
			::lstrcpy (plvdi->item.pszText, (LPCTSTR) pItem->strFileName);
      	      break;

        case 1: // File size
			//Don't show file size for ".."
			if(pItem->strFileName == "..")
				::lstrcpy (plvdi->item.pszText, "");
			else
			{	//Format and show the filesize
				string.Format ("%u", pItem->nFileSizeLow);
				::lstrcpy (plvdi->item.pszText, (LPCTSTR) string);
			}
			break;

		case 2: // Date and time
			//Don't show for ".." directory
			if(pItem->strFileName == "..")
				::lstrcpy (plvdi->item.pszText, "");
			else
			{	//Create a CTime object from the file's FILETIME
				// descriptor
				CTime time (pItem->ftLastWriteTime);
				int nHour = time.GetHour();
				//Format to show the date and time
				string.Format ("%0.2d/%0.2d/%0.2d  %0.2d:%0.2d",
				time.GetMonth(), time.GetDay(), time.GetYear() % 100,
					nHour, time.GetMinute());
				//Copy the date/time info to the list control
				::lstrcpy (plvdi->item.pszText, (LPCTSTR)string);
			}
			break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// int CALLBACK CMainView::CompareFuncRem (LPARAM lParam1, LPARAM lParam2,
//	LPARAM lParamSort)
//
// Sample Call: SortItems(CompareFuncRem, 0);
//
// Description: A callback function - sorts the remote list control.
//	- Fills in ITEMINFO* structures by typecasting lParams to ITEMINFO*.
//	- Determines return value nResult based on whether items are files 
//		or directories.  nResult tells function which item precedes which. 
//	- Switch statement uses lParamSort for cases - lParamSort is 
//		the subitem to sort on:
//		- case 0: SubItem 0 is the file name - sorts by file name.
//		- case 1: SubItem 1 is the file size - sorts by file size.
//		- case 2: SubItem 2 is the file date and time - sorts by 
//			date and time:
//
// Input/Output data elements:
//	- Input: Arguments LPARAM lParam1, LPARAM lParam2, lPARAM lParamSort. 
//	- Output: Returns integer value nResult.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
int CALLBACK CMainView::CompareFuncRem (LPARAM lParam1, LPARAM lParam2,
	LPARAM lParamSort)
{	//Fill in ITEMINFO structures by typecasting lParams to ITEMINFO*
    ITEMINFO* pItem1 = (ITEMINFO*) lParam1;
    ITEMINFO* pItem2 = (ITEMINFO*) lParam2;
    int nResult;

	//If item 1 is the hidden directory "..", return -1, means that 
	//item 1 precedes item 2
	if(pItem1->strFileName == "..") 
	{
	   nResult = -1;
	   return nResult;
	}
	//If item 2 is the hidden directory "..", return 1, means that 
	//item 2 precedes item 1
	if(pItem2->strFileName == "..") 
	{
	   nResult = 1;
	   return nResult;
	}
	//If comparing drive letters, item 1 always preceds item 2
	if((pItem2->strFileName.GetAt(1) == ':') && 
		!(pItem1->strFileName.GetAt(1) == ':'))
	{
	   nResult = -1;
	   return nResult;
	}
	//If item 1 is a directory and item 2 is not, item 1 precedes
	if ((pItem1->dwFileAttributes == 0) &&
	   !(pItem2->dwFileAttributes == 0))
	{
	   nResult = -1;
	   return nResult;
	}
	//If item 2 is a directory and item 1 is not, item 2 precedes
	if ((pItem2->dwFileAttributes == 0) &&
	   !(pItem1->dwFileAttributes == 0))
	{
	   nResult = 1;
	   return nResult;
	}

    switch (lParamSort)
	{	//lParamSort is the subitem to sort on 
		case 0: //Sorting by file name
			nResult = pItem1->strFileName.CompareNoCase
				(pItem2->strFileName);
			break;

		case 1: //Sorting by file size
			nResult = pItem1->nFileSizeLow - pItem2->nFileSizeLow;
			break;

		case 2: //Sort by date and time
			nResult = ::CompareFileTime (&pItem1->ftLastWriteTime,
            &pItem2->ftLastWriteTime);
			break;
    }
    return nResult;
}

/////////////////////////////////////////////////////////////////////
// CMainView local list control functions
//

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnDblclkListLocal(NMHDR* pNMHDR, LRESULT* pResult) 
//
// Sample Call: OnDblclkListLocal(pNMHDR, pResult);
//
// Description: Called when the user double-clicks on the local list.
//	- Gets a pointer to the local list control.
//	- Initializes variables.
//	- Determines if the mouse pointer was over a list item when clicked. 
//	- Sets the attributes for the LV_ITEM object.
//	- Gets the item's information using GetItem().
//	- If the item being double-clicked is a file, sends the file 
//		using OnClickedButtonMainLocsend() and returns.
//	- Gets the text of the item's label.
//	- If the item is a drive, sets the drive as the local path.
//	- If the item's text is "..", truncates the local path list 
//		to the parent directory.
//	- If the item is a normal directory, modifies the local path string.
//	- Gets the directory's attributes using FindFirstFile() .
//	- Adds the new list and redraws it using DrawLocList().
//
// Input/Output data elements:
//	- Input arguments NMHDR* pNMHDR and LRESULT* pResult.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnDblclkListLocal(NMHDR* pNMHDR, LRESULT* pResult) 
{	//Get a pointer to the list control
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_LOCAL);

	CString csTemp;
	int nIndex = 0;

    HANDLE hFind;
    WIN32_FIND_DATA fddc;

	LV_ITEM lvLoc;

	if((nIndex = pList->GetNextItem(-1, LVNI_SELECTED)) != -1)
	{	//If a list item was under the mouse pointer
		CWaitCursor cWait;

		//Set attributes for the LV_ITEM object
		lvLoc.mask = LVIF_IMAGE;	//Image bit is valid
		lvLoc.iItem = nIndex;	//Get info for the clicked item
		lvLoc.iSubItem = 0;	//Don't want info on subitems 
		pList->GetItem(&lvLoc);	//Get the info
		
		//If selection is a file
		if(lvLoc.iImage == 5)
		{
			//Send the file
			OnClickedButtonMainLocsend();
			return;
		}

		//Get that item's text
		csTemp = pList->GetItemText(nIndex, 0);
		//If the item is a drive, list it
		if(csTemp[1] == ':')	
			m_csLocPath = csTemp;
		//If "..", truncate path list to parent directory
		else if(csTemp == "..")	
		{	
			m_csLocPath = m_csLocPath.Left(m_csLocPath.ReverseFind('\\'));
			if(m_csLocPath.GetLength() < 3)
				m_csLocPath += '\\';
		}
		else	//Should be a normal directory 
		{	//Check if the string ends in"\"
			if(m_csLocPath.Right(1) == '\\')
				//If so, just add the name to the end
				csTemp = m_csLocPath + csTemp;
			else
				//If not, add "\" and then the name
				csTemp = m_csLocPath + "\\" + csTemp;
			//If the item is a directory, list it
			if ((hFind = ::FindFirstFile((LPCTSTR)csTemp,
				&fddc)) != INVALID_HANDLE_VALUE)
			{
				if (fddc.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					m_csLocPath = csTemp;
				CloseHandle (hFind);
			}
		}
	//Add the new list and redraw it
	DrawLocList();
	}
	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnGetDispInfo (NMHDR* pnmh, LRESULT* pResult)
//
// Sample Call: Not called by the user.
//
// Description: Assigns values to the local list control's items and subitems.
//	- Places info into LV_DISPINFO* structure plvdi by 
//		typecasting pnmh as LV_DISPINFO*.
//	- Creates ITEMINFO* structure pItem and puts the info into pItem by 
//		typecasting plvdi as ITEMINFO*.
//	- Switch statement uses plvdi->item.iSubItem for cases:
//		- case 0: SubItem 0 is the file name - copies the file name 
//			to the item structure.
//		- case 1: File size - formats the file size into a string and copies 
//			to plvdi->item.pszText.
//		- case 2: Date and time:
//			- Creates a CTime object from the file's FILETIME descriptor.
//			- Formats a string to show the date and time.
//			- Copies the date/time info to the list control.
//
// Input/Output data elements:
//	- Input: Arguments NMHDR* pnmh, LRESULT* pResult.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnGetDispInfo (NMHDR* pnmh, LRESULT* pResult)
{	
	CString string;
	//Place info into plvdi by typecasting pnmh as LV_DISPINFO*
	LV_DISPINFO* plvdi = (LV_DISPINFO*) pnmh;
	//If the mask is set to text
    if (plvdi->item.mask & LVIF_TEXT)
	{
 		//Put the info into pItem by typecasting
		ITEMINFO* pItem = (ITEMINFO*) plvdi->item.lParam;

	   //Which item are we dealing with?
        switch (plvdi->item.iSubItem)
		{

        case 0: //File name
			::lstrcpy (plvdi->item.pszText, (LPCTSTR) pItem->strFileName);
      	      break;

        case 1: //File size
			//Don't show file size for drives and directories
			if((pItem->strFileName.GetAt(1)== ':') || 
				(pItem->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			::lstrcpy (plvdi->item.pszText, "");
			else
			{
			string.Format ("%u", pItem->nFileSizeLow);
			::lstrcpy (plvdi->item.pszText, (LPCTSTR) string);
			}
			break;

        case 2: //Date and time
			//Don't show for drives or ".." directory
			if((pItem->strFileName.GetAt(1) == ':')
				|| (pItem->strFileName == ".."))
            ::lstrcpy (plvdi->item.pszText, "");
			else
			{
				CTime time(pItem->ftLastWriteTime);
				int nHour = time.GetHour();
				string.Format ("%0.2d/%0.2d/%0.2d  %0.2d:%0.2d",
				time.GetMonth(), time.GetDay(), time.GetYear() % 100,
					nHour, time.GetMinute());
				::lstrcpy (plvdi->item.pszText, (LPCTSTR)string);
			}
			break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnColumnClick (NMHDR* pnmh, LRESULT* pResult)
//
// Sample Call: OnColumnClick (pnmh, pResult);
//
// Description: Called when the user left-clicks the mouse on a column 
// header  on the local list control.
//	- Creates an NM_LISTVIEW* pointer by typecasting pNMHDR.
//	- Gets a pointer to the local list control.
//	- Sorts the list based on which column header was clicked. 
//
// Input/Output data elements:
//	- Input: Arguments NMHDR* pNMHDR and LRESULT* pResult.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
void CMainView::OnColumnClick (NMHDR* pnmh, LRESULT* pResult)
{
	NM_LISTVIEW* pnmlv = (NM_LISTVIEW*) pnmh;
	//Get a pointer to the local list control
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_LOCAL);
	//Sort the list based on which column header was clicked
	(pList->SortItems(CompareFunc, pnmlv->iSubItem));
}


/////////////////////////////////////////////////////////////////////////////
// int CALLBACK CMainView::CompareFunc (LPARAM lParam1, LPARAM lParam2,
//	LPARAM lParamSort)
//
// Sample Call: SortItems(CompareFunc, 0);
//
// Description: A callback function - sorts the local list control.
//	- Fills in ITEMINFO* structures by typecasting lParams to ITEMINFO*.
//	- Determines return value nResult based on whether items are files, 
//		directories, or drives.  nResult tells function which item 
//		precedes which. 
//	- Switch statement uses lParamSort for cases - lParamSort is the 
//		subitem to sort on:
//		- case 0: SubItem 0 is the file name - sorts by file name.
//		- case 1: SubItem 1 is the file size - sorts by file size.
//		- case 2: SubItem 2 is the file date and time - sorts by date and time:
//
// Input/Output data elements:
//	- Input: Arguments LPARAM lParam1, LPARAM lParam2, lPARAM lParamSort.
//	- Output: Returns integer value nResult.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
int CALLBACK CMainView::CompareFunc (LPARAM lParam1, LPARAM lParam2,
	LPARAM lParamSort)
{
    ITEMINFO* pItem1 = (ITEMINFO*) lParam1;
    ITEMINFO* pItem2 = (ITEMINFO*) lParam2;
    int nResult;

	//Check to see if new folder is being added, put below ".."	
	if((pItem1->strFileName == "") && 
		(pItem2->strFileName == ".."))
	{
	   nResult = 1;
	   return nResult;
	}
	//Check to see if new folder is being added, put below ".."	
	if((pItem1->strFileName == "..") && 
		(pItem2->strFileName == ""))
	{
	   nResult = -1;
	   return nResult;
	}
	//If item 1 is a drive letter and item 2 is not, item 2 precedes
	if((pItem1->strFileName.GetAt(1) == ':') && 
		!(pItem2->strFileName.GetAt(1) == ':'))
	{
	   nResult = 1;
	   return nResult;
	}
	//If item 2 is a drive letter and item 1 is not, item 2 precedes
	if((pItem2->strFileName.GetAt(1) == ':') && 
		!(pItem1->strFileName.GetAt(1) == ':'))
	{
	   nResult = -1;
	   return nResult;
	}
	//If item 1 is a directory and item 2 is not, item 1 precedes
	if ((pItem1->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
	   !(pItem2->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
	   nResult = -1;
	   return nResult;
	}
	//If item 2 is a directory and item 1 is not, item 2 precedes
	if ((pItem2->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
	   !(pItem1->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
	   nResult = 1;
	   return nResult;
	}

    switch (lParamSort)
	{	//lParamSort is the subitem to sort on 
    case 0: //File name
        nResult = pItem1->strFileName.CompareNoCase (pItem2->strFileName);
        break;

    case 1: //File size
        nResult = pItem1->nFileSizeLow - pItem2->nFileSizeLow;
        break;

    case 2: //Date and time
        nResult = ::CompareFileTime (&pItem1->ftLastWriteTime,
            &pItem2->ftLastWriteTime);
        break;
    }
    return nResult;
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::InitLocList(LPCTSTR pszPath)
//
// Sample Call: InitLocList("C:\");
//
// Description: Initializes the local list control - gets the list's 
// items and draws list.
//	- Get a pointer to the local list control.
//	- Checks to see if the directory specified by pszPath is valid. 
//	- If the directory path already exists in the local combo box, 
//		select it; else, add it to the combobox and select it.
//	- Begins adding files and subdirectories to the list control using 
//		FindFirstFile(), FindNextFile(), and AddItem)().
//	- Adds all local drives to the list using GetLogicalDrive().
//
// Input/Output data elements:
//	- Input: Arguments LPCTSTR pszPath.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
void CMainView::InitLocList(LPCTSTR pszPath)
{	//pszPath is the directory to be shown
    CString strPath = pszPath;
	//Get a pointer to the local list control
	CComboBox *pcbLoc = (CComboBox *)GetDlgItem(IDC_COMBO_LOCDIR);
	//Check to see if the directory is valid
	if(_chdir(strPath))
	{	//If not, show an error box and default to the root
		AfxMessageBox("Couldn't change directory - defaulting to C:\\");
		strPath = "C:\\";
		m_csLocPath = strPath;
	}

	//If the selection exists in combo box, select it; else, add
	if(pcbLoc->FindStringExact(-1, m_csLocPath) == CB_ERR)
	{				
		pcbLoc->AddString(m_csLocPath);	//Add the string
		//Show it in the edit box portion of the combobox
		pcbLoc->SetCurSel(pcbLoc->FindStringExact(-1, m_csLocPath));
	}
	else	//Already exists, so just select and show it
		pcbLoc->SetCurSel(pcbLoc->FindStringExact(-1, m_csLocPath));

	//If the last character of the path is not "\", add it 
    if (strPath.Right(1) != "\\")
        strPath += "\\";
    strPath += "*.*";	//Select all items in the directory

    HANDLE hFind;
    WIN32_FIND_DATA fd;
	//Begin adding files to the list - file info is put into fd
    if ((hFind = ::FindFirstFile ((LPCTSTR) strPath, &fd)) !=
        INVALID_HANDLE_VALUE)
	{
		//Don't show the "." directory
		if(lstrcmp(".", fd.cFileName))
            AddItem (&fd);
		//Find and add the rest of the files and directories
       while (::FindNextFile (hFind, &fd))
		{			
              if (!AddItem (&fd))
                  break;
		}
        CloseHandle (hFind);
    }

    int nPos = 0;
    CString strDrive = "?:\\";
	//Put the drives into an integer - drive A: is bit 0, B is 1, etc
    DWORD dwDriveList = ::GetLogicalDrives ();

    while (dwDriveList)	//While drives remain to be listed
	{
        if (dwDriveList & 1)
		{	//Set the drive letter ? in strDrive
            strDrive.SetAt(0, 0x41 + nPos);
			//Add the drive to the list
			AddDriveNode (strDrive);
        }
		//Shift the bits one bit right in dwDriveList
        dwDriveList >>= 1;
        nPos++;
    }
}

/////////////////////////////////////////////////////////////////////////////
// BOOL CMainView::AddItem(WIN32_FIND_DATA* pfd)
//
// Sample Call: AddItem(&pfd);
//
// Description: Adds an item to the local list control.
//	- Get a pointer to the local list control.
//	- Initializes ITEMINFO* structure pItem and fills in member variables 
//		from from pfd's member variables. 
//	- Creates LV_ITEM structure lvi and sets its the mask flags.
//	- Specifies the bitmap image for the item depending on what kind 
//		of item it is.
//	- Inserts the item into the list.
//
// Input/Output data elements:
//	- Input: Argument WIN32_FIND_DATA* pfd.
//	- Output: Boolean return value.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
BOOL CMainView::AddItem(WIN32_FIND_DATA* pfd)
{	//Get a pointer to the control
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_LOCAL);
    ITEMINFO* pItem;
	pItem = new ITEMINFO;
	//Initialize pItem from pfd
    pItem->dwFileAttributes = pfd->dwFileAttributes;
	pItem->strFileName = pfd->cFileName;
	pItem->nFileSizeLow = pfd->nFileSizeLow;
	pItem->ftLastWriteTime = pfd->ftLastWriteTime;

    LV_ITEM lvi;
	//Set the mask flags for lvi
    lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
	lvi.state = 0;
    lvi.iItem = 0; 
    lvi.iSubItem = 0;
	//If item is not a directory
	if (!(pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		//Specify the file bitmap
		lvi.iImage = 5;
	}
		else	//Directory bitmap
		lvi.iImage = 3;
	//Specify the "UP" arrow bitmap for ".."	
	if (!(lstrcmp(pfd->cFileName, "..")))
		lvi.iImage = 6;

	lvi.pszText = LPSTR_TEXTCALLBACK; 
    lvi.lParam = (LPARAM) pItem;
	//Insert the item into the list
    if (pList->InsertItem (&lvi) == -1)
        return FALSE;

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// BOOL CMainView::AddDriveNode (CString& strDrive)
//
// Sample Call: AddDriveNode(strDrive);
//
// Description: Adds a drive item to the local list control.
//	- Get a pointer to the local list control.
//	- Initializes ITEMINFO* structure pItem and fills in the drive name 
//		and time from the local drive time. 
//	- Creates LV_ITEM structure lvi and sets its mask flags and other
//		 member variables.
//	- Finds out what kind of drive it is, and assigns a bitmap image 
//		depending on the drive type.
//	- Inserts the item into the list.
//
// Input/Output data elements:
//	- Input: Argument CString& strDrive.
//	- Output: Boolean return value.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
BOOL CMainView::AddDriveNode (CString& strDrive)
{
	//Get a pointer to the control
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_LOCAL);
    CString string;

	FILETIME dwLocDriveTime; 
    ITEMINFO* pItem;

	pItem = new ITEMINFO;

    pItem->dwFileAttributes = 0;
	pItem->strFileName = strDrive;
	pItem->nFileSizeLow = 0;
	pItem->ftLastWriteTime = dwLocDriveTime;

    LV_ITEM lvi;
    lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
	lvi.state = 0;
    lvi.iItem = 0; 
    lvi.iSubItem = 0;
	lvi.pszText = LPSTR_TEXTCALLBACK; 
    lvi.lParam = (LPARAM) pItem;

	//Find out what kid of drive it is
    UINT nType = ::GetDriveType ((LPCTSTR) strDrive);

    switch (nType)
	{
	//Specify bitmap image based on drive type
    case DRIVE_REMOVABLE:
		lvi.iImage = 1;
        pList->InsertItem (&lvi);
        break;

    case DRIVE_FIXED:
		lvi.iImage = 0;
        pList->InsertItem(&lvi);
        break;

    case DRIVE_REMOTE:
		lvi.iImage = 0;
        pList->InsertItem(&lvi);
        break;

    case DRIVE_CDROM:
		lvi.iImage = 2;
        pList->InsertItem(&lvi);
        break;

    case DRIVE_RAMDISK:
		lvi.iImage = 0;
        pList->InsertItem(&lvi);
        break;

    default:
        return FALSE;
    }
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::FreeItemMemory ()
//
// Sample Call: FreeItemMemory ();
//
// Description: Frees memory used by pItems in the local list.
//	- Gets a pointer to the local list control.
//	- Finds out how many items are in the list, and deletes 
//		the item data for each. 
//
// Input/Output data elements: None.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
void CMainView::FreeItemMemory ()
{	//Get a pointer to the control
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_LOCAL);
	//How many items are there?
    int nCount = pList->GetItemCount();
    if(nCount)
	{	//Delete the item data
        for(int i=0; i<nCount; i++)
            delete (ITEMINFO*) pList->GetItemData(i);
    }
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::FreeItemMemoryRem()
//
// Sample Call: FreeItemMemoryRem();
//
// Description: Frees memory used by pItems in the remote list.
//	- Gets a pointer to the remote list control.
//	- Finds out how many items are in the list, and deletes 
//		the item data for each.
//
// Input/Output data elements: None.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
void CMainView::FreeItemMemoryRem()
{
	CListCtrl* pRemList = (CListCtrl*)GetDlgItem(IDC_LIST_REMOTE);

    int nCount = pRemList->GetItemCount();
    if(nCount)
	{
        for(int i=0; i<nCount; i++)
            delete (ITEMINFO*) pRemList->GetItemData(i);
    }
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::PrintToStatus(LPCSTR lpStr, LPCSTR lpStr1)
//
// Sample Call: PrintToStatus("Hi There", NULL);
//
// Description: Prints a line to the list box and scrolls, if necessary.
//	- If lpStr1 is not NULL, adds lpStr1 to the end of lpStr.
//	- Gets a pointer to the list box control.
//	- Adds the string to the list box, and scrolls to 
//		the bottom, if necessary. 
//
// Input/Output data elements:
//- Input: Arguments LPCSTR lpStr, LPCSTR lpStr1.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
void CMainView::PrintToStatus(LPCSTR lpStr, LPCSTR lpStr1)
{	//Write the string to the status box
	if(lpStr1 == NULL)
		wsprintf(m_lpszMessage, lpStr);
	else
		wsprintf(m_lpszMessage, lpStr, lpStr1);
	//Insert the text into the status box
	CListBox *plb = (CListBox *)GetDlgItem(IDC_LIST_STATUS);
	plb->SetTopIndex(plb->AddString(m_lpszMessage));
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::PrintToStatusSpace()
//
// Sample Call: PrintToStatusSpace();
//
// Description: Prints a blank line to the list box.
//	- Gets a pointer to the list box control.
//	- Adds the string to the list box, and scrolls to 
//		the bottom, if necessary. 
//
// Input/Output data elements: None.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::PrintToStatusSpace()
{
	//Write an empty new line to the status box
	CListBox *plb = (CListBox *)GetDlgItem(IDC_LIST_STATUS);
	plb->SetTopIndex(plb->AddString(m_lpszMessage));
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnSelchangeComboLocdir() 
//
// Sample Call: Not called by the user.
//
// Description: Called when the user changes the local combobox selection.
//	- Gets pointers to the local list control and combobox.
//	- Gets index of new combobox selection and it's text.
//	- Resets the official local path based on the selection's text.
//	- Adds the new list to the local list control and redraws it. 
//
// Input/Output data elements: None.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnSelchangeComboLocdir() 
{	
	CWaitCursor cWait;
	char SelTemp[1000];
	//Get pointers to the local list and combobox
	CComboBox *pcbLoc = (CComboBox *)GetDlgItem(IDC_COMBO_LOCDIR);
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_LOCAL);
	// Get index of new combobox selection and then it's text
    int index = pcbLoc->SendMessage(CB_GETCURSEL, (WORD)0, 0L);
    pcbLoc->SendMessage(CB_GETLBTEXT, (WORD)index, (LONG)SelTemp);
	//Reset the official local path
	m_csLocPath.Format(SelTemp);
	//Add the new list and redraw it
	DrawLocList();
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnSelchangeComboRemdir() 
//
// Sample Call: Not called by the user.
//
// Description: Called when the user changes the remote combobox selection.
//	- Gets pointers to the remote list control and combobox.
//	- Gets index of new combobox selection and it's text.
//	- Changes the remote directory listing by sending a CWD command.
//
// Input/Output data elements: None.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnSelchangeComboRemdir() 
{
	CWaitCursor cWait;
	char SelTempRem[1000];

	//Get pointers to the remote list and combobox
	CComboBox *pcbRem = (CComboBox *)GetDlgItem(IDC_COMBO_REMDIR);
	CListCtrl* pRemList = (CListCtrl*)GetDlgItem(IDC_LIST_REMOTE);
	m_nBytesReceived = 0;

	//Get index of new combobox selection and then it's text
    int index = pcbRem->SendMessage(CB_GETCURSEL, (WORD)0, 0L);
    pcbRem->SendMessage(CB_GETLBTEXT, (WORD)index, (LONG)SelTempRem);
	//Change the remote directory listing
	SendCommand("CWD ", SelTempRem);
	if(ReadResponse())
		PostDirectory();
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnClickedButtonMainLocrefresh() 
//
// Sample Call: OnClickedButtonMainLocrefresh();
//
// Description: Called when the user pushes the local "Refresh" button.
//	- Redraws the local list, showing the m_csLocPath *.* directory.
//
// Input/Output data elements: None.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnClickedButtonMainLocrefresh() 
{	//Redraw the local list, showing the m_csLocPath *.* directory
	DrawLocList();
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnClickedButtonMainRemrefresh() 
//
// Sample Call: OnClickedButtonMainRemrefresh();
//
// Description: : Called when the user pushes the remote "Refresh" button.
//	- Reposts the current directory to the remote list by 
//		calling PostDirectory().
//
// Input/Output data elements: None.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnClickedButtonMainRemrefresh() 
{	//Re post the current directory to the remote list
	PostDirectory();
	return;
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::DrawLocList() 
//
// Sample Call: DrawLocList();
//
// Description: Redraws the local list control.
//	- Gets a pointer to the local list control.
//	- Sets the redraw option to FALSE to keep the list from showing 
//		all the deletion and addition of items.
//	- Deletes all items in the list.
//	- Calls InitLocList() to add all items from the m_csLocPath path.
//	- Sorts the items by calling CompareFunc.
//	- Sets the redraw option to TRUE.
//	- Calls InvalidateRect() to force the list to be redrawn with 
//		the new items.
//
// Input/Output data elements: None.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::DrawLocList() 
{	//Get a pointer to the list
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_LOCAL);
	//Don't redraw the list yet
	SetRedraw(FALSE);
	pList->DeleteAllItems();	//Delete all list items
	InitLocList(m_csLocPath);	//Add items from m_csLocpath
	//Sort items on directory/file names (subitem 0)
	pList->SortItems(CompareFunc, 0);
	SetRedraw(TRUE);	//Now allow redraws
	InvalidateRect(NULL,FALSE);	//Redraw the list
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnBeginlabeleditListLocal(NMHDR* pNMHDR, LRESULT* pResult) 
//
// Sample Call: Not called by the user.
//
// Description: Called when the user initiates a label edit 
// on the local list.
//	- Gets a pointer to the local list control.
//	- Gets the label text of the selected item.
//	- If the item is a drive, disallows label editing by setting *pResult to 1.
//
// Input/Output data elements:
//	- Input: Arguments NMHDR* pNMHDR, LRESULT* pResult.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnBeginlabeleditListLocal(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int nBLE,
		nLen = 0;
	//Get a pointer to the list
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_LOCAL);

	//Get the label text of the selected item
	if((nBLE = pList->GetNextItem(-1, LVNI_SELECTED)) != -1)
		pList->GetItemText(nBLE, 0, m_pszLocLE, nLen);

	//If the item is a drive, disallow label editing
	if(m_pszLocLE[1] == ':')
		*pResult = 1;
	else
		*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnBeginlabeleditListRemote(NMHDR* pNMHDR, LRESULT* pResult) 
//
// Sample Call: Not called by the user.
//
// Description: Called when the user initiates a label edit on the remote list.
//	- Creates an LV_DISPINFO* pDispInfo structure and sets it equal to 
//		pNMHDR, which is typecast to LV_DISPINFO*.
//	- Gets a pointer to the remote list control.
//	- Gets the label text of the selected item.
//
// Input/Output data elements:
//	- Input arguments NMHDR* pNMHDR, LRESULT* pResult.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnBeginlabeleditListRemote(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	int nBLER,
		nLen = 0;
	//Get a pointer to the list
	CListCtrl* pRemList = (CListCtrl*)GetDlgItem(IDC_LIST_REMOTE);

	//Get the current label text of the selected item
	if((nBLER = pRemList->GetNextItem(-1, LVNI_SELECTED)) != -1)
		pRemList->GetItemText(nBLER, 0, m_pszRemLE, nLen);
	
	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnEndlabeleditListLocal(NMHDR* pNMHDR, LRESULT* pResult) 
//
// Sample Call: Not called by the user.
//
// Description: Called when the user finishes a label edit on the local list.
//	- Gets pointers to the local list and combobox.
//	- Creates an LV_ITEM lvEditLabel structure.
//	- Creates an LV_DISPINFO* plvdi structure and sets it equal to 
//		pNMHDR, which is typecast to LV_DISPINFO*.
//	- Determines if a change was made to the label - if not, refreshes 
//		the local list to remove the new folder, and returns.
//	- Else, gets the label text of the new item.
//	- Gets the index and text of the current combobox selection.
//	- Adds the new name to the end of the current path.
//	- Attempts to add or rename the directory.
//	- Sets the new item label text.
//	- Refreshes the local list whether or not the label change was valid.
//
// Input/Output data elements:
//	- Input arguments NMHDR* pNMHDR, LRESULT* pResult.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnEndlabeleditListLocal(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int nELE;
	CString csOldTxt,
			csNewTxt,
			csPath;
	//Get pointers to the local list and combobox
	CComboBox* pcbLoc = (CComboBox*)GetDlgItem(IDC_COMBO_LOCDIR);
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_LOCAL);
	LV_ITEM lvEditLabel;
	//Get plvdi info by typecasting pNMHDR
	LV_DISPINFO* plvdi = (LV_DISPINFO*) pNMHDR;
	
	//Was a change made to the label at all?
	if(!plvdi->item.pszText)
	{	//No change
		wsprintf(m_pszLocLE,"");
		m_bLocMkDir = FALSE;
		//Refresh the list
		OnClickedButtonMainLocrefresh();
		return;
	}
	else	//Get the new label
		csNewTxt.Format(plvdi->item.pszText);
	
	//If new label is not NULL and isn't the same as the old label
	if((csNewTxt != "") && (csOldTxt != csNewTxt))
	{	//get the index of the current combobox selection
		nELE = pcbLoc->GetCurSel();
		//Get the text of the current combobox selection
		pcbLoc->GetLBText(nELE ,csPath);
		//If the text isn't NULL
		if( csPath != "")
		{	//Add the name to the end of the path
			if(csPath.Right(1) != '\\')	//If path doesn't end in "\"
			{	//Add "\" and the name to the end
				csOldTxt = csPath + "\\" + m_pszLocLE;
				csNewTxt = csPath + "\\" + csNewTxt;
			}
			else
			{	//Add the name to the end
				csOldTxt = csPath + m_pszLocLE;
				csNewTxt = csPath + csNewTxt;
			}
			//Renaming or making a directory?
			if(m_bLocMkDir)	//Making a directory
			{	//If directory is created successfully
				if(!(::_mkdir(csNewTxt)))
					//Change the label text to the new name 
					lvEditLabel.pszText = plvdi->item.pszText;
				else	//Label text is NULL
					lvEditLabel.pszText = m_pszLocLE;
			}
			else
			{
				//Change the label onscreen
				if(!(::rename(csOldTxt, csNewTxt)))
					//If successful, change the label text
					lvEditLabel.pszText = plvdi->item.pszText;
				else	//label text doesn't change
					lvEditLabel.pszText = m_pszLocLE;
			}
		}
		//Set the mask to Text
		lvEditLabel.mask = LVIF_TEXT;
		//Set the same item
		lvEditLabel.iItem = plvdi->item.iItem;
		//Change the label text (subitem 0)
		lvEditLabel.iSubItem = 0;
		//Set the item text
		pList->SetItem(&lvEditLabel);
	}
	else
	{	//The label change wasn't valid - refresh the list
		m_bLocMkDir = FALSE;
		wsprintf(m_pszLocLE, "");
		OnClickedButtonMainLocrefresh();
		return;
	}
	m_bLocMkDir = FALSE;
	wsprintf(m_pszLocLE,"");
	OnClickedButtonMainLocrefresh();
	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnEndlabeleditListRemote(NMHDR* pNMHDR, LRESULT* pResult) 
//
// Sample Call: Not called by the user.
//
// Description: Called when the user finishes a label edit on the remote list.
//	- Creates an LV_DISPINFO* pDispInfo structure and sets it equal to 
//		pNMHDR, which is typecast to LV_DISPINFO*.
//	- Determines if a change was made to the label - if not, returns.
//	- Else, adds directory with MKD command, or renames file 
//		with RNFR and RNTO commands.
//	- Ends mouse capture to release mouse pointer from the screen.
//
// Input/Output data elements:
//	- Input arguments NMHDR* pNMHDR, LRESULT* pResult.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnEndlabeleditListRemote(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	CString csNewTxt;

	//Was a change made to the label at all?
	if(!pDispInfo->item.pszText)	//No
	{
		wsprintf(m_pszRemLE,"");
		m_bRemMkDir = FALSE;
		return;
	}
	else
		csNewTxt.Format(pDispInfo->item.pszText);	//Get the new label

	//If new label is not NULL and isn't the same as the old label
	if((csNewTxt != "") && (csNewTxt != m_pszRemLE))
	{
		//Renaming or making a directory?
		if(m_bRemMkDir)
		{	//Making directory
			SendCommand("MKD ", csNewTxt);
			if(ReadResponse())	//If successful, PWD
				PostDirectory();
		}
		else
		{	//Renaming a file
			SendCommand("RNFR ", m_pszRemLE);
			if(ReadResponse())	//If RNFR successful
			{	//Rename
				SendCommand("RNTO ", csNewTxt);
				if(ReadResponse())	//If successful, PWD
					PostDirectory();
			}
		}
	}
	//End mouse capture - WM_LBUTTONUP message has not been received
	ReleaseCapture();
	m_bRemMkDir = FALSE;
	wsprintf(m_pszRemLE,"");
	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnClickedButtonMainLocmkdir() 
//
// Sample Call: OnClickedButtonMainLocmkdir();
//
// Description: Called when the user clicks the local "Make Directory" button.
//	- Gets a pointer to the local list.
//	- Puts the local time into a SYSTEMTIME object.
//	- Changes the system time to file time.
//	- Sets directory attributes in the WIN32_FIND_DATA structure.
//	- Adds the new item to the list by calling AddItem().
//	- Sorts the list.
//	- Finds the item in the list with FindItem().
//	- Sets the list's focus to that item.
//	- Automatically begins the label edit by calling EditLabel().
//
// Input/Output data elements: None.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnClickedButtonMainLocmkdir() 
{	//Initialize and declare variables
	WIN32_FIND_DATA fdMkDir;
	FILETIME ftMkDir;
	SYSTEMTIME stLocTime;
	LV_FINDINFO fiMkDir;

	//Get a pointer to the local list
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_LOCAL);
	int nMDIndex;

	m_bLocMkDir = TRUE;	//Falg indicates that we are making a directory
	//Put the local time into a SYSTEMTIME object
	::GetLocalTime(&stLocTime);
	//Change sytem time to file time
	::SystemTimeToFileTime(&stLocTime, &ftMkDir);

	//This is a directory
    fdMkDir.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	wsprintf(fdMkDir.cFileName,"");	//The initial directory name is NULL 
	fdMkDir.nFileSizeLow = 0;
	fdMkDir.ftLastWriteTime = ftMkDir;
	fiMkDir.flags = LVFI_STRING;
	fiMkDir.psz = "";

	//Add the new item to the list
	AddItem(&fdMkDir);
	pList->SortItems(CompareFunc, 0);	//Sort the list
	//Find the item in the list
	nMDIndex = pList->FindItem(&fiMkDir, -1);
	//Set the list's focus to that item
	pList->SetFocus();
	//Begin the label edit automatically
	pList->EditLabel(nMDIndex);
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnClickedButtonMainRemmkdir() 
//
// Sample Call: OnClickedButtonMainRemmkdir();
//
// Description: Called when the user clicks the remote "Make Directory" button.
//	- Gets a pointer to the remote list.
//	- Puts the local time into a SYSTEMTIME object.
//	- Changes the sytem time to file time.
//	- Adds the new item to the list by calling AddItemRemList().
//	- Sorts the list.
//	- Finds the item in the list with FindItem().
//	- Sets the list's focus to that item.
//	- Automatically begins the label edit by calling EditLabel().
//
// Input/Output data elements: None.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnClickedButtonMainRemmkdir() 
{
	FILETIME ftMkDir;
	SYSTEMTIME stLocTime;
	LV_FINDINFO fiMkDir;
	//Get a pointer to the remote list
	CListCtrl* pRemList = (CListCtrl*)GetDlgItem(IDC_LIST_REMOTE);
	int nMDIndex;

	m_bRemMkDir = TRUE;

	::GetLocalTime(&stLocTime);
	::SystemTimeToFileTime(&stLocTime, &ftMkDir);

	fiMkDir.flags = LVFI_STRING;
	fiMkDir.psz = "";

	AddItemRemList (0, "", "0", ftMkDir, 0);
	pRemList->SortItems(CompareFuncRem, 0);
	//Find the item in the list
	nMDIndex = pRemList->FindItem(&fiMkDir, -1);
	//Set the list's focus to that item
	pRemList->SetFocus();
	//Begin the label edit automatically
	pRemList->EditLabel(nMDIndex);
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnClickedButtonMainLocdel() 
//
// Sample Call: OnClickedButtonMainLocdel();
//
// Description: Called when the user clicks the local "Delete" button.
//	- Gets pointers to the local list control and combobox.
//	- Creates an LV_ITEM lvLocDel structure and sets the image mask.
//	- Detects if any items are selected - if, so, loops while 
//		detecting all selected items.
//	- Gets the item's text from the list control.
//	- Gets the item's index number and text from the combobox.
//	- Adds the item's name to the end of the path list .
//	- Gets the item's bitmap image info to determine what type of 
//		item the selection is.
//	- Deletes the file or directory.
//	- Refreshes the local list to show the updated listing.
//
// Input/Output data elements: None.
//
// Error handling:
//	- If the directory or file cannot be removed, shows an error dialog box.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnClickedButtonMainLocdel() 
{	//Get pointers to the list and combobox controls
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_LOCAL);
	CComboBox* pcbLoc = (CComboBox*)GetDlgItem(IDC_COMBO_LOCDIR);
	CString csLDDPath,
			csLDDTemp;
	int nLDDIndex = 0,
		nLDD = 0;
	LV_ITEM lvLocDel;
	lvLocDel.mask = LVIF_IMAGE;	//Set the mask to image
	lvLocDel.iSubItem = 0;
	//Any items selected?
	if((nLDDIndex = pList->GetNextItem(-1, LVNI_SELECTED)) != -1)
	{	//If yes, loop 'til no more items are selected
		while((nLDDIndex = pList->GetNextItem((nLDDIndex - 1), LVNI_SELECTED)) != -1)
		{	
			lvLocDel.iItem = nLDDIndex;	//Set the item's index in the LV_ITEM structure
			//Get the item's text
			csLDDTemp = pList->GetItemText(nLDDIndex, 0);
			//Get the current selection's list number
			nLDD = pcbLoc->GetCurSel();
			//Get the text for that selection - it's the current path
			pcbLoc->GetLBText(nLDD ,csLDDPath);
			//If the path isn't NULL
			if( csLDDPath != "")
			{	//If the path doesn't end in "\", add it and the name to the end
				if(csLDDPath.Right(1) != '\\')
					csLDDPath = csLDDPath + "\\" + csLDDTemp;
			else	//Add just the name to the end
				csLDDPath = csLDDPath + csLDDTemp;
			//Get icon info to determine what type selection is
			pList->GetItem(&lvLocDel);
			//If selection is a directory
			if((lvLocDel.iImage == 3) /*|| (lvRem.iImage == 4)*/)	
			{	//Directory deletion successful?
				if(::_rmdir(csLDDPath))
				{	//If not, show error box
					csLDDTemp.Format("Couldn't delete directory %s! " \
						"Directory must be empty before deleting.",
						csLDDPath);
					AfxMessageBox(csLDDTemp);
				}
			}
			else	//If selection is a file
				if(lvLocDel.iImage == 5)
				{//File deleted?
					if(!DeleteFile(csLDDPath))
					{	//If not, show error box
						csLDDTemp.Format("Couldn't delete file %s!",
							csLDDPath);
						AfxMessageBox(csLDDTemp);
					}
				}
			}	
			nLDDIndex++;
		}
		OnClickedButtonMainLocrefresh();	//Refresh the list
	}
	return;
}	

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnClickedButtonMainRemdel() 
//
// Sample Call: OnClickedButtonMainRemdel();
//
// Description: Called when the user clicks the remote "Delete" button.
//	- Gets pointers to the remote list control.
//	- Creates an LV_ITEM lvRemDel structure and sets the image mask.
//	- Detects if any items are selected - if, so, loops while 
//		detecting all selected items.
//	- Gets the item's text from the list control.
//	- Gets the item's image information with GetItem().
//	- If the item is a directory, sends the RMD command.
//	- If the item is a file, sends the DELE command.
//	- Refreshes the list by calling PostDirectory().
//
// Input/Output data elements: None.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnClickedButtonMainRemdel() 
{	//Get pointers to the list and comboboxes
	CListCtrl* pRemList = (CListCtrl*)GetDlgItem(IDC_LIST_REMOTE);
	CString csRDDPath,
			csRDDTemp;
	int nRDDIndex = 0,
		nRDD = 0;
	LV_ITEM lvRemDel;
	lvRemDel.mask = LVIF_IMAGE;
	lvRemDel.iSubItem = 0;
	//Any items selected?
	if((nRDDIndex = pRemList->GetNextItem(-1, LVNI_SELECTED)) != -1)
	{//If yes, loop 'til no more items are selected
		while((nRDDIndex = pRemList->GetNextItem((nRDDIndex - 1), LVNI_SELECTED)) != -1)
		{
			lvRemDel.iItem = nRDDIndex;
			csRDDTemp = pRemList->GetItemText(nRDDIndex, 0);
			//Get icon info to determine what type selection is
			pRemList->GetItem(&lvRemDel);
			//If selection is a directory
			if((lvRemDel.iImage == 3) /*|| (lvRem.iImage == 4)*/)
			{	//Directory deleted?
				SendCommand("RMD ", csRDDTemp);	//Delete the remote directory
				if(!ReadResponse())	//No, break
					break;
			}
			else
				if(lvRemDel.iImage == 5)//If selection is a file
				{	//File deleted?
					//Delete the remote file
					SendCommand("DELE ", csRDDTemp);
					if(!ReadResponse())	//If failed, break
						break;
				}
			nRDDIndex++;
		}
		PostDirectory();	//PWD
	}
	return;
}	

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnClickedButtonMainRemret() 
//
// Sample Call: OnClickedButtonMainRemret();
//
// Description: Called when the user clicks the Remote "Retrieve" button.
//	- Gets a pointer to the remote list control.
//	- Detects if any items are selected - if, so, loops while 
//		detecting all selected items and putting their names into 
//		a string list.
//	- If an item is selected:
//		- Starts a message pump to pump messages while waiting for a 
//			previous retrieval to complete.
//		- Parses the string for the next file name.
//		- Gets a new list pointer, as the previous one is now no good.
//		- Finds the item in the list, based on its label text.
//		- Gets the item's info with GetItem();
//		- If the item is not a directory, continues, else, prints 
//			an error message to the screen and returns.
//		- Opens either a binary or ASCII  user file, based on the type 
//			specified, and sends the TYPE command.
//		- If successful, creates a local port and sends the RETR command.
//
// Input/Output data elements: None.
//
// Error handling:
//	- If a directory has been selected, prints an error message to the screen.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnClickedButtonMainRemret() 
{	//Get the list pointer
	CListCtrl* pRemList = (CListCtrl*)GetDlgItem(IDC_LIST_REMOTE);
	CString csRRList,
		csTemp;
	int nRRIndex = 0,
		nRR = 0;
	MSG msg;
	//Specify that we're going to recieve a file
	m_csListOrFile = "Receiving File";
	LV_ITEM lvRet;
	LV_FINDINFO fiRet;
	lvRet.iSubItem = 0;
	lvRet.mask = LVIF_IMAGE | LVIF_PARAM;
	m_nTotalSent = 0;

	//Is an item selected?
	if((nRRIndex = pRemList->GetNextItem(-1, LVNI_SELECTED)) != -1)
	{	//Yes, find all selected items 
		while((nRRIndex = pRemList->GetNextItem((nRRIndex - 1),
			LVNI_SELECTED)) != -1)
		{	//Get the selected items and put them into a string
			csRRList += pRemList->GetItemText(nRRIndex, 0) + "\r\n";
			nRRIndex++;
		}
		//If a file is selected, retrieve it
		while((csRRList != "\r\n") && 
			(nRR = csRRList.Find("\r\n")) != -1)
		{	//Sit and spin while previous file is retrieved
			while(m_cfUserFile)
			{	//Pump messages while waiting
				while(::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) 
				{ 
					if(!AfxGetApp()->PumpMessage()) 
					{	//If user wants to Quit, break 
						::PostQuitMessage(0); 
						 break; 
					} 
				} 
			}
			//Parse the string for the next file name
			m_csLocalFile = csRRList.Left(nRR);
			csRRList = csRRList.Right(csRRList.GetLength() - nRR - 2);
			//Get a new list pointer - previous one is no good
			CListCtrl* pRemList = (CListCtrl*)GetDlgItem(IDC_LIST_REMOTE);
			fiRet.flags = LVFI_STRING;
			fiRet.psz = m_csLocalFile;
			lvRet.iItem = pRemList->FindItem(&fiRet, -1);
			//Get the selected item's information
			pRemList->GetItem(&lvRet);
			//Put the info into an ITEMINFO structure
			ITEMINFO* pItem = (ITEMINFO*) lvRet.lParam;
			//Get the file size
			m_nFileSize = pItem->nFileSizeLow;

			//It is a link
			if(lvRet.iImage == 4)
			{
				m_csLocalFile = m_csLocalFile.Right(
					m_csLocalFile.GetLength() - 
					m_csLocalFile.ReverseFind('/') - 1);
			}


			//Don't retrieve directories
			if((lvRet.iImage != 3) /*|| (lvRem.iImage == 4)*/)	
			{	
				m_nBytesReceived = 0;	//Counter for the progress control
				if(!m_bBinary || (m_bAuto && m_csLocalFile.Right(3)== "txt"))
				{	//If "Binary" is not specified, and this is a text file,
					//open a text file
					m_cfUserFile = new CStdioFile(m_csLocalFile,
					  CFile::modeWrite | CFile::typeText | CFile::modeCreate);
					//Tell server it's an ASCII transfer
					SendCommand("TYPE A","");
				}
				else	//It's a binery transfer
				{	//Create a binary file
					m_cfUserFile = new CFile(m_csLocalFile, 
						CFile::modeWrite | CFile::typeBinary | CFile::modeCreate);
					//Tell server it's a binary transfer
					SendCommand("TYPE I","");
				}
				//If TYPE command is successful
				if(ReadResponse())
				{	//If "PORT" is successful
					if(Port())
					{	//Retrieve the file
						SendCommand("RETR ", m_csLocalFile);
						if(ReadResponse())	//Read the responses
							ReadResponse();
					}
				}
			}
			else	//We can't retrieve directories
				PrintToStatus("Can't retrieve directory \"%s\""\
				"- Directory transfers not allowed",
					m_csLocalFile);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnClickedButtonMainLocsend() 
//
// Sample Call: OnClickedButtonMainLocsend();
//
// Description: Called when the user clicks the local "Send" button.
//	- Gets a pointer to the local list control.
//	- Detects if any items are selected - if, so, loops while detecting 
//		all selected items and putting their names into a string list.
//	- If there are any files left to send:
//		- Parses the string for the next file name.
//		- Gets a new list pointer, as the previous one is now no good.
//		- Finds the item in the list, based on its label text.
//		- Gets the item's info with GetItem();
//		- Starts a message pump to pump messages while waiting for a previous 
//			send to complete, if any.
//		- If the item is not a directory, continues, else, prints an error 
//			message to the screen and returns.
//		- Opens either a binary or ASCII  user file, based on the type 
//			specified, and sends the TYPE command.
//		- If successful, sends the STOR command.
//		- If successful:
//			- Gets the size of the file to be sent.
//			- Pumps messages if other data is currently being sent.
//			- Pumps messages while waiting for a data socket to be created.
//			- Reads a WRITE_BUF_LEN amount of data from the file.
//			- Writes the data with the data socket Write() command.
//			- Updates the progress control with the total amount sent.
//			- When all data is sent, reads the response from the server.
//		- If a directory was selected, print an error message to the screen.
//		- Refresh the remote list with OnClickedButtonMainRemrefresh(). 
//
// Input/Output data elements: None.
//
// Error handling:
//	- If a directory has been selected, prints an error message to the screen.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnClickedButtonMainLocsend() 
{
	CWaitCursor cWait;
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_LOCAL);
	char pData[WRITE_BUF_LEN];	//Write buffer binary
	memset(pData, 0, WRITE_BUF_LEN + 1);	//Clear the buffer
	*pData = '\0';
	MSG msg;
	CString csLSList;
	UINT amount = 0;
	int nLSIndex = 0,
		nLS = 0,
		nLength = 0;
	LV_FINDINFO fiSend;
	LV_ITEM lvSend;
	lvSend.mask = LVIF_IMAGE | LVIF_PARAM;
	lvSend.iSubItem = 0;
	m_csListOrFile = "Sending File";	//Specify sending a file
	m_bWriteDone = FALSE;	//Flag to tell us when the write's done
	m_bPieceDone = TRUE;	//Flag to tell when this data has been written

	//Any items selected?
	if((nLSIndex = pList->GetNextItem(-1, LVNI_SELECTED)) != -1)
	{	//If yes, find all selected items
		while((nLSIndex = pList->GetNextItem((nLSIndex - 1),
			LVNI_SELECTED)) != -1)
		{	//Get the selected items and put them into a string
			csLSList += pList->GetItemText(nLSIndex, 0) + "\r\n";
			nLSIndex++;
		}
		//While there is data left
		while((csLSList != "\r\n") && 
			(nLS = csLSList.Find("\r\n")) != -1)
		{	//Get the next file name
			m_csLocalFile = csLSList.Left(nLS);
			//Strip the curent name from the list
			csLSList = csLSList.Right(csLSList.GetLength() - nLS - 2);
			//Get a pointer to the list
			CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_LOCAL);
			fiSend.flags = LVFI_STRING;
			fiSend.psz = m_csLocalFile;
			//Find this item in the list
			lvSend.iItem = pList->FindItem(&fiSend, -1);
			pList->GetItem(&lvSend);	//Get the item's info
			ITEMINFO* pItem = (ITEMINFO*) lvSend.lParam;
			//Get the item's file size
			m_nFileSize = pItem->nFileSizeLow;
			m_nBytesReceived = 0;
			m_nTotalSent = 0;

			//Sit and spin while previous file is retrieved
			while(m_cfUserFile)	//While a file handle is open
			{	//Pump messages while waiting
				while(::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) 
				{ 
					if(!AfxGetApp()->PumpMessage()) 
					{	//If user wants to exit, break 
						::PostQuitMessage(0); 
						 break; 
					} 
				} 
			}

			//Don't send directories
			if(lvSend.iImage != 3)
			{	//Open the specified file
				m_cfUserFile = new CFile(m_csLocalFile, CFile::modeRead | 
					CFile::shareDenyWrite | CFile::typeBinary);
				if(m_bBinary)
				{	//If a binary transfer
					if(Port())
						SendCommand("TYPE I","");
				}
				else
				{	//An ASCII transfer
					if(Port())
						SendCommand("TYPE A","");
				}
				//If TYPE command successful
				if(ReadResponse())
				{	//Send the STOR command
					SendCommand("STOR ", m_csLocalFile);
					if(ReadResponse())
					{	//If STOR is successful,
						//Set the amount sent to 0
						m_nTotalSent = 0;
						//nLength initialized as the file size
						nLength = m_cfUserFile->GetLength();
						//While there is still datat to send
						while(nLength > 0)
						{	//If other data is being sent
							while(!m_bPieceDone)
							{	//Pump messages while waiting
								while(::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) 
								{ 
									if(!AfxGetApp()->PumpMessage()) 
									{ 
										::PostQuitMessage(0); 
										 break; 
									} 
								} 
							}
							//While no data socket is available
							while(!m_pDataSocket)
							{	//Pump messages while waiting
								while(::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) 
								{ 
									if(!AfxGetApp()->PumpMessage()) 
									{ 
										::PostQuitMessage(0); 
										 break; 
									} 
								} 
							}
							//Flag says we are sending data
							m_bPieceDone = FALSE;
							//Read WRITE_BUF_LEN amount of data from the file
							amount = m_cfUserFile->Read(pData, WRITE_BUF_LEN);
							//Terminate the data buffer with NULL
							pData[amount] = '\0';
							//Write the data with the data socket
							m_pDataSocket->Write(amount, pData);
							//Delete the amount sent from the file size
							nLength -= amount;
							//For the progress control
							m_nTotalSent += amount;
							//Update frame items by idle time processing
							UpdateControls();
						}
						//We are done with the write
						m_bWriteDone = TRUE;
					}
					ReadResponse();
				}
			}
			else
			{//Tried to send a directory
				PrintToStatus("Can't send directory \"%s\""\
				"- Directory transfers not allowed", 
					m_csLocalFile);
				return;
			}
		}
		//Refresh the Remote list
		OnClickedButtonMainRemrefresh();
	}
}

/////////////////////////////////////////////////////////////////////////////
// FILETIME CMainView::TimeConversion(CString csString)
//
// Sample Call: TimeConversion(csString);
//
// Description: Converts the time given by the server for remote files to 
// a time  useable by the program.
//	- Converts text month designations to numbers (i.e. "Jan" = 1).
//	- Sets seconds and milliseconds to 0 (not specified by server).
//	- If hours and minutes are separated by ":", get days, hours, and minutes:
//	- Convert system time to file time.
//	- Gets hours, minutes, and days from the string.
//	- Gets year from local time, since year not specified by the server.
//	- Checks the year against the local year to make sure the year is correct.
//	- If hours and minutes were not given, set to 0:
//	- Gets year and day from the string.
//	- Converts the system time to a file time.
//	- Converts the local file time to file time based on the Coordinated 
//		Universal Time (UTC).
//
// Input/Output data elements:
//	- Input: Argument CString csString.
//	- Output: Returns FILETIME structure.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
FILETIME CMainView::TimeConversion(CString csString)
{
	CString Temp;
	int nComp = 0;
	SYSTEMTIME SysTime, LocalTime;
	FILETIME  FileTime, ftLocalTime, ftUTC;

	//Convert months to numbers
	if(csString.Left(3) == "Jan")
		SysTime.wMonth = 1; else
	if(csString.Left(3) == "Feb")
		SysTime.wMonth = 2;else
	if(csString.Left(3) == "Mar")
		SysTime.wMonth = 3;else
	if(csString.Left(3) == "Apr")
		SysTime.wMonth = 4;else
	if(csString.Left(3) == "May")
		SysTime.wMonth = 5;else
	if(csString.Left(3) == "Jun")
		SysTime.wMonth = 6;else
	if(csString.Left(3) == "Jul")
		SysTime.wMonth = 7;else
	if(csString.Left(3) == "Aug")
		SysTime.wMonth = 8;else
	if(csString.Left(3) == "Sep")
		SysTime.wMonth = 9;else
	if(csString.Left(3) == "Oct")
		SysTime.wMonth = 10;else
	if(csString.Left(3) == "Nov")
		SysTime.wMonth = 11;else
	if(csString.Left(3) == "Dec")
		SysTime.wMonth = 12;
	
	//Seconds and milliseconds are 0
	SysTime.wSecond = 0;
	SysTime.wMilliseconds = 0;
	//If hours and minutes are separated by ":" 
	if(csString[9] == ':')
	{
		::GetLocalTime(&LocalTime);
		::SystemTimeToFileTime(&LocalTime, &ftLocalTime);
		//Convert system time to file time 
		Temp = csString.Left(9);
		//Get hours - last 2 digits of Temp
		SysTime.wHour = atoi(Temp.Right(2));
		//Get minutes - last 2 digits of csString
		SysTime.wMinute = atoi(csString.Right(2));
		Temp = csString.Left(6);
		//Get day - last 2 digits of Temp
		SysTime.wDay = atoi(Temp.Right(2));
		//Get year from local time, since year not specified
		//by the server
		SysTime.wYear = LocalTime.wYear;
		//Check to make sure year number is correct
		if(::SystemTimeToFileTime(&SysTime, &FileTime))
		{	//Compare the server's time to our time
			nComp = ::CompareFileTime (&FileTime,&ftLocalTime);
			//Their time is less than ours
			if(nComp == -1)
				//Make their year our year
				SysTime.wYear = LocalTime.wYear;
			//Their time is equal to ours
			if(nComp == 0)
				//Make their year our year
				SysTime.wYear = LocalTime.wYear;
			//Their time is greater than ours
			if(nComp == 1)
				//Their year is ours - 1
				SysTime.wYear = LocalTime.wYear -1;
		}
	}
	else
	{	//Hours and minutes were not given, set to 0
		SysTime.wHour = 0;
		SysTime.wMinute = 0;
		//Year is specified, so get it
		SysTime.wYear = atoi(csString.Right(4));
		//Find the first space from the right end
        nComp = csString.ReverseFind(' ');
		Temp = csString.Left(nComp - 1);
		//Get the day
		SysTime.wDay =atoi(Temp.Right(2));
	}
	//Convert the system time to a filetime
	::SystemTimeToFileTime(&SysTime, &FileTime);
	//Convert the local file time to file time based on the 
	//Coordinated Universal Time (UTC). 
	::LocalFileTimeToFileTime(&FileTime, &ftUTC);
	return(ftUTC);
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnTimer(UINT nIDEvent) 
//
// Sample Call: Not called directly by user.  Called upon timeout of a Set
// Timer() call.
//
// Description: Called when a timer goes off.
//	- If nIDEvent = 1, kills Timer 1 and calls OnClickedButtonMainConnect().
//		This is the connect dialog that is displayed when the 
//		appllication starts.
//	- If nIDEvent = 2, if m_bReadReady is FALSE, kills Timer 2 and sets 
//		m_bReadReady to TRUE.
//
// Input/Output data elements:
//	- Input argument UINT nIDEvent.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnTimer(UINT nIDEvent) 
{	//nIDEvent is the timer number specified by SetTimer()
	//Timer 1 is set by CMainView::OnInitialUpdate(), opens
	//the "Connect" screen when the application starts
	if(nIDEvent == 1)
	{	//Kill Timer 1 and call the function
		KillTimer(1);
		OnClickedButtonMainConnect();
	}
	//Timer 2 is set by CMainView::OnStream()when the socket is
	//done reading - keeps checking until m_breadReady is 
	//false, then sets it true
	if(nIDEvent == 2)
	{
		if(!m_bReadReady)
		{	//If m_bReadReady is FALSE, kill Timer 2 and set
			KillTimer(2);
			m_bReadReady = TRUE;
		}
	}
		CFormView::OnTimer(nIDEvent);
}

/////////////////////////////////////////////////////////////////////////////
// BOOL CMainView::ReadResponse()
//
// Sample Call: ReadResponse();
//
// Description: Reads command responses coming in on the stream 
// (control) socket.
//	- Initializes variables, clears the string buffer.
//	- If the socket is not ready for a read, loops, waiting for 
//		m_bReadReady to be set to TRUE and pumping messages.
//	- Copies data into buffer from stream socket.
//	- Copies buffer to a CString.
//	- Parses individual lines and prints them to the screen.
//	- Clears the buffer and frees its memory.
//	- If the message is not an error message, return TRUE, else return FALSE.
//
// Input/Output data elements:
//	- Output: Returns a Boolean variable.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
BOOL CMainView::ReadResponse()
{
	CWaitCursor cWait;
	LPVOID pDataRead;		// pointer to data just read
	int nCrLfPos,
		ncsLineLen,
		nStrCount = 0,
		nLen = 0;
	char pszMessage[READ_BUF_LEN + 1];	// Buffer to receive data
	CString csLine,
			csTemp,
			csDoneWrite;
	MSG msg;
	//Clear string buffer
	memset(pszMessage, 0, READ_BUF_LEN + 1);
	//Pump messages while waiting
loop:	while(!m_bReadReady)
	{
	    while (::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ) 
		{ 
			if ( !AfxGetApp()->PumpMessage( ) ) 
			{ 
				::PostQuitMessage(0); 
				 break; 
			} 
		} 
	}

	m_bReadReady = FALSE;
	//Copy data into buffer from stream socket
	pDataRead = m_pStream->Read(&nLen);
	//Copy data into char string
	wsprintf(pszMessage,"%s",pDataRead);
	//Append terminator to string
	pszMessage[nLen] = '\0';
	//Make message a CString
	csLine.Format(pszMessage);
	//Parse individual lines and show them
	while((nCrLfPos = csLine.Find("\r\n")) != -1)
	{	//While there is a CR/LF remaining
		ncsLineLen = csLine.GetLength();
		//Strip the CR/LF and put into csTemp
		csTemp = csLine.Left(nCrLfPos);
		//Take the current string out of csLine
		csLine = csLine.Right(ncsLineLen - nCrLfPos - 2);
		//Print the data to the string
		PrintToStatus(csTemp, NULL);
	}
	//Free memory and clear buffer
	*(char *)pDataRead = '\0';
	free(pDataRead);
	memset(pszMessage, 0, READ_BUF_LEN + 1);
	*pszMessage = '\0';

	if(csTemp[3] == '-')	//Multiline response
		goto loop;			//Read next line

	m_csResponse = csTemp;
	//If these are not error messages
	if((csTemp[0] == '1') || (csTemp[0] == '2') || 
		(csTemp.Left(3) == "331") || (csTemp.Left(3) == "350"))
		return TRUE;
	else
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// BOOL CMainView::PostDirectory()
//
// Sample Call: PostDirectory();
//
// Description: Sends a PWD command and handles the response.
//	- Gets a pointer to the remote combobox.
//	- Sends the PWD command.
//	- Reads the response.
//	- Gets the directory name from the response.
//	- If the directory doesn't exist in the combobox, add it to the box 
//		and select it.  If the directory already exists, selects it.
//	- Calls Port().  If the call is successful:
//		- Receiving a list.  If ASCII is specified, send the TYPE A command.
//		- Send the LIST command, and read both responses.
//		- If both responses are read OK, returns TRUE, else returns FALSE.
//
// Input/Output data elements:
//	- Output: Returns a Boolean variable.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
BOOL CMainView::PostDirectory()
{	//Gat e pointer to the remote combobox
	CComboBox* pcbRem = (CComboBox*)GetDlgItem(IDC_COMBO_REMDIR);
	CString csTemp;

	SendCommand("PWD", "" );
	if(ReadResponse())	//If command was successful
	{
		//Copy the reply to csTemp
		csTemp = m_csResponse;
		//csTemp = everything to the left of the last " character
		csTemp = csTemp.Left(csTemp.ReverseFind('"'));
		//csTemp = everything to the right of the first " - 
		//we now have the directory name
		csTemp = csTemp.Right(csTemp.GetLength() - 
			csTemp.ReverseFind('"') - 1);

		//If directory exists in combo box, use it; else, add and select
		if(pcbRem->FindStringExact(-1, csTemp) == CB_ERR)
		{	//String doesn't exist - add it			
			pcbRem->AddString(csTemp);
			//And select it in the combobox
			pcbRem->SetCurSel(pcbRem->FindStringExact(-1, csTemp));
		}
		else
			pcbRem->SetCurSel(pcbRem->FindStringExact(-1, csTemp));
		//Not sending a file
		m_bSendFile = FALSE;
		//Clear the buffer
		memset(m_pszBuf, 0, 100);
		*m_pszBuf = '\0';
		//If PORT command was successful
		if(Port())
		{	//Receiving a list
			m_csListOrFile = "Receiving List";
			if(m_bBinary)
			{	//If ASCII is specified
				SendCommand("TYPE A", "");
				ReadResponse();
			}
			//Send LIST command
			SendCommand("LIST", "");
			if(ReadResponse())	//If no error responses
			{
				if(ReadResponse())
					return TRUE;
				else
					return FALSE;
			}
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::EnableFunctions()
//
// Sample Call: EnableFunctions();
//
// Description: Enables various buttons.
//
// Input/Output data elements: None.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::EnableFunctions()
{	//Enable buttons
	GetDlgItem(IDC_BUTTON_MAIN_REMRET)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_MAIN_REMREFRESH)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_MAIN_REMMKDIR)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_MAIN_REMDEL)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_MAIN_LOCSEND)->EnableWindow(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::DisableFunctions()
//
// Sample Call: DisableFunctions();
//
// Description: Disables various buttons.
//
// Input/Output data elements: None.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::DisableFunctions()
{	//Disable buttons
	GetDlgItem(IDC_BUTTON_MAIN_REMRET)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_MAIN_REMREFRESH)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_MAIN_REMMKDIR)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_MAIN_REMDEL)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_MAIN_LOCSEND)->EnableWindow(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// BOOL CMainView::Port()
//
// Sample Call: Port();
//
// Description: Creates a data socket and sends the PORT command.
//	- Creates a new CStreamSocket object.
//	- Creates a data socket with the object.
//	- Creates a SOCKADDR_IN object and initializes it.
//	- Gets the stream socket name.
//	- Gets the listening socket's port information.
//	- Gets the current local IP address.
//	- Formats a string for the PORT command with the IP address 
//		and port information.
//	- Writes the PORT command out on the control socket.
//	- Reads the server's response - if not an error message, returns TRUE, 
//		else, returns FALSE.
//
// Input/Output data elements:
//	- Output: Returns a Boolean variable.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
BOOL CMainView::Port()
{
	//Create a listening socket
	if(!m_pListenSocket)
	{ // initialize the stream socket object
		m_pListenSocket = new CStreamSocket(this, 
			WM_USER_LISTEN);
		//Create a data socket
		if (m_pListenSocket->CreateDataSocket() != 
			CWINSOCK_NOERROR)
		{	//If socket creation fails
			PrintToStatus("Listen socket creation failed", NULL);
			m_csStatus = "Listen socket creation failed";
			delete m_pListenSocket;
			m_pListenSocket = NULL;
			return FALSE;
		}
	}
	
		//Set the PORT arguments
		SOCKADDR_IN cont_address;	//Create SOCKADDR_IN object
		cont_address.sin_addr  = m_in;
		int ncont_addressLen = sizeof(cont_address);
		//Get the socket name and put in cont_address
		getsockname(m_pStream->m_s, (LPSOCKADDR)&cont_address,
			&ncont_addressLen);
		//port is the listening sockets port
		unsigned char *port = (unsigned char *)
			&(m_pListenSocket->m_sinaddrAssigned.sin_port);
		//Copy the info for status bar message
		memcpy(&m_sinaddrDataPort, 
			&m_pListenSocket->m_sinaddrAssigned, sizeof(SOCKADDR_IN));
		//host is the current IP address
		unsigned char *host = (unsigned char *)
			&(cont_address.sin_addr);
		//Format the port command with host and port info
	    wsprintf(m_pszBuf, "PORT %i,%i,%i,%i,%i,%i\r\n",
			host[0], host[1], host[2], host[3], port[0], port[1]);
		//Write the PORT command out on the stream socket
		m_pStream->Write(strlen(m_pszBuf)/*+ 1*/, m_pszBuf);
		//If PORT command successful
		if(ReadResponse())
			return TRUE;
		else	//else
			return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnClickedRadioMainBinary() 
//
// Sample Call: OnClickedRadioMainBinary();
//
// Description: Called when the "Binary" button is clicked.
//	- Gets pointers to the "Binary" and "Ascii" buttons.
//	- Sets the "Binary" button.  Unsets the "Ascii" button.
//	- Sets the binary flag.
//
// Input/Output data elements: None.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnClickedRadioMainBinary() 
{
	//Get pointers to teh "Binary" and "Ascii" buttons
	CButton *pRadBin = (CButton *)GetDlgItem(IDC_RADIO_MAIN_BINARY);
	CButton *pRadAsc = (CButton *)GetDlgItem(IDC_RADIO_MAIN_Ascii);
	//Show "Binary" button as checked, "Ascii" button as unchecked
	pRadBin->SetCheck(TRUE);
	pRadAsc->SetCheck(FALSE);
	m_bBinary = TRUE;	//Set the flag
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnClickedRadioMainAscii() 
//
// Sample Call: OnClickedRadioMainAscii();
//
// Description: Called when the "Ascii" button is clicked.
//	- Gets pointers to the "Binary" and "Ascii" buttons.
//	- Sets the "Ascii" button.  Unsets the "Binary" button.
//	- Sets the binary flag.
//
// Input/Output data elements: None.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnClickedRadioMainAscii() 
{
	//Get pointers to teh "Binary" and "Ascii" buttons
	CButton *pRadBin = (CButton *)GetDlgItem(IDC_RADIO_MAIN_BINARY);
	CButton *pRadAsc = (CButton *)GetDlgItem(IDC_RADIO_MAIN_Ascii);
	//Show "Ascii" button as checked, "Binary" button as unchecked
	pRadBin->SetCheck(FALSE);
	pRadAsc->SetCheck(TRUE);
	m_bBinary = FALSE;	//Set flag
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnClickedRadioMainAuto() 
//
// Sample Call: OnClickedRadioMainAuto();
//
// Description: Called when the "Auto" button is clicked.
//	- If m_bAuto is TRUE:
//		- Sets m_bAuto to FALSE.
//		- Gets a pointer to the button.
//		- Unchecks the button.
//	- If m_bAuto is FALSE, sets it to TRUE.
//
// Input/Output data elements: None.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnClickedRadioMainAuto() 
{
	if(m_bAuto)
	{	//If m_bAuto is TRUE
		m_bAuto = FALSE;
		//Get a pointer to the button
		CButton *pRadAuto = (CButton *)GetDlgItem(IDC_RADIO_MAIN_AUTO);
		pRadAuto->SetCheck(FALSE);	//Uncheck the button
	}
	else
		m_bAuto = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::KillStream()
//
// Sample Call: KillStream();
//
// Description: Destroys the stream socket and object.
//	- If m_pStream is positive, destroys the socket, and deletes and 
//		sets the object to NULL.
//
// Input/Output data elements: None.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::KillStream()
{
	if (m_pStream)
	{
		m_pStream->DestroySocket();
		delete m_pStream;
		m_pStream = NULL;
	}

}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnButtonMainExit() 
//
// Sample Call: OnButtonMainExit();
//
// Description: Called when the "Exit" button is pressed - sends a 
// WM_CLOSE message to the main window.
//
// Input/Output data elements: None.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnButtonMainExit() 
{
	AfxGetMainWnd()->SendMessage(WM_CLOSE);	
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnUpdateStatus(CCmdUI *pCmdUI) 
//
// Sample Call: Not called by the user.
//
// Description: Updates the status text in the first status bar pane.
//	- Enables the pane.
//	- Formats the status message string.
//	- Sets the text string in the status pane.
//
// Input/Output data elements:
//	- Input: Argument CCmdUI *pCmdUI.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnUpdateStatus(CCmdUI *pCmdUI)
{
    pCmdUI->Enable();	//Enable the pane
	CString strPage;
	//Format the string with the status message
	strPage.Format( "%s", m_csStatus); 
	pCmdUI->SetText(strPage);	//Set the pane text
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::OnUpdateUtility(CCmdUI *pCmdUI)
//
// Sample Call: Not called by the user.
//
// Description: Updates the status line in the third status bar pane.
//	- Enables the pane.
//	- Sets the text string in the status pane based on m_csBytes.
//
// Input/Output data elements:
//	- Input: Argument CCmdUI *pCmdUI.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::OnUpdateUtility(CCmdUI *pCmdUI)
{
    pCmdUI->Enable();	//Enable the pane
	pCmdUI->SetText(m_csBytes);	//Set the pane text
}

/////////////////////////////////////////////////////////////////////////////
// void CMainView::UpdateControls()
//
// Sample Call: UpdateControls();
//
// Description: Forces frame items to update, as sometimes they 
// won't in a blocking thread.
//	- Gets a pointer to the main frame window.
//	- Calls OnUpdateCmdUI() to update the status bar.
//
// Input/Output data elements: None.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
void CMainView::UpdateControls()
{
	CMainFrame *pMainFrame = (CMainFrame *)AfxGetApp()->m_pMainWnd;
	pMainFrame->m_wndStatusBar.OnUpdateCmdUI(pMainFrame, TRUE);
}
