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
// csock.cpp : implementation file
//

#include "stdafx.h"
#include "TJFTP.h"
#include "csock.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWinsock::CWinsock(WORD wVersionRequired /*=MAKEWORD(1,1)*/)
//
// Sample Call: Not called by user. wVersionRequested set in csock.h
// initialization of wVersionRequested.
//
// Description: CWinsock constructor.  Passes in desired Winsock version
// as wVersionRequested.
//
// Input/Output data elements: None.
//	- Input: Argument WORD wVersionrequested.
//
// Error handling:
//
// Author:	Jim Dunne
//
// Date:	Sept 26, 1997
//
CWinsock::CWinsock(WORD wVersionRequired /*=MAKEWORD(1,1)*/)
{
	// Initialize Variables
	m_wVersionRequired = wVersionRequired;
	m_nLastError =0;
}

CWinsock::~CWinsock()
{
}

BEGIN_MESSAGE_MAP(CWinsock, CWnd)
	//{{AFX_MSG_MAP(CWinsock)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinsock message handlers

/////////////////////////////////////////////////////////////////////////////
// int CWinsock::Startup()
//
// Sample Call: Startup();
//
// Description: Starts Winsock with WSAStartup().
//
// Input/Output data elements: 
//	- Output: Returns integer m_nStatus.
//
// Error handling: 
//	- If there is an error with WSAStartup, returns 
//		CWINSOCK_WINSOCK_ERROR as m_nStatus.  
//	- If the version is incorrect,  displays a 
//		"Wrong Winsock Version" error dialog box.
//
// Author:	Jim Dunne
//
// Date:	July 29, 1997
//
int CWinsock::Startup()
{
	int nStatus = CWINSOCK_NOERROR;
	//Error messages are put into m_nLastError
	m_nLastError = WSAStartup(m_wVersionRequired, &m_wsaData);
	//There was an error - return it
	if (m_nLastError != 0)
		nStatus = CWINSOCK_WINSOCK_ERROR;
	
	else
	{	//Check for the correct Winsock version
		if ((LOBYTE(m_wsaData.wVersion) != LOBYTE(m_wVersionRequired)) ||
			(HIBYTE(m_wsaData.wVersion) != HIBYTE(m_wVersionRequired)))
		{
			char m_lpszError[100];
			wsprintf(m_lpszError, "Winsock Version Wrong");
			AfxMessageBox(m_lpszError);
		}
	}
	return nStatus;
}

/////////////////////////////////////////////////////////////////////////////
// int CWinsock::Shutdown()
//
// Sample Call: Shutdown();
//
// Description: Shuts down the Winsock sub-system. 
//
// Input/Output data elements: 
//	- Output: Returns integer m_nStatus.
//
// Error handling: 
//	- If there is an error with WSACleanup(), returns 
//		CWINSOCK_WINSOCK_ERROR as m_nStatus.
//
// Author:	Jim Dunne
//
// Date:	July 29, 1997
//
int CWinsock::Shutdown()
{	//Assume no error
	int m_nStatus = CWINSOCK_NOERROR;
	//If there was an error, return it
	if (WSACleanup() != 0)
	{ 
		m_nLastError = WSAGetLastError();
		m_nStatus = CWINSOCK_WINSOCK_ERROR;
	}
	return m_nStatus;
}


/////////////////////////////////////////////////////////////////////////////
// CStreamSocket
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CStreamSocket::CStreamSocket(CWnd *pParentWnd, UINT uMsg)
//
// Sample Call: new CStreamSocket(this, WM_USER_STREAM);
//
// Description: Constructs the CStreamSocket object and 
// initializes member variables. 
//
// Input/Output data elements: 
//	- Input: Arguments CWnd *pParentWnd, UINT uMsg.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	July 29, 1997
//
CStreamSocket::CStreamSocket(CWnd *pParentWnd, UINT uMsg)
{
	m_pParentWnd = pParentWnd;
	ASSERT(m_pParentWnd != NULL);
	m_uMsg = uMsg;
	ASSERT(m_uMsg != 0);
	InitVars();
}

/////////////////////////////////////////////////////////////////////////////
// CStreamSocket::~CStreamSocket()
// CStreamSocket destructor
CStreamSocket::~CStreamSocket()
{
}

/////////////////////////////////////////////////////////////////////////////
// void CStreamSocket::InitVars(BOOL bInitLastError/*= TRUE*/)
//
// Sample Call: InitVars(FALSE);
//
// Description: Initializes class member variables 
//
// Input/Output data elements: 
//	- Input: Argument BOOL bInitLastError.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	July 29, 1997
//
void CStreamSocket::InitVars(BOOL bInitLastError/*= TRUE*/)
{
	if(bInitLastError)
		m_nLastError = 0;
	
	m_s = INVALID_SOCKET;
	m_sAccept = INVALID_SOCKET;
	memset(&m_sinLocal, 0, sizeof(SOCKADDR_IN));
	memset(&m_sinRemote, 0, sizeof(SOCKADDR_IN));
	m_bServer = FALSE;

	m_sData = INVALID_SOCKET;
	memset(&m_sinLocalData, 0, sizeof(SOCKADDR_IN));
	memset(&m_sinRemoteData, 0, sizeof(SOCKADDR_IN));
}

/////////////////////////////////////////////////////////////////////////////
// int CStreamSocket::CreateSocket(int nLocalPort)
//
// Sample Call: CreateSocket(21);
//
// Description: This version of the CreateSocket() function takes a
// port number, in host order, as input.  A port number
// should only be specified if the socket is to be bound
// to a certain port.  If you don't care which port is
// assigned to the socket, just call CreateSocket() without
// any parameter, causing CreateSocket(NULL) to be called.
//
// Input/Output data elements: 
//	- Input: Argument int nLocalPort.
//	- Output: Argument CreateSocket(pszLocalService);.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	July 29, 1997
//
int CStreamSocket::CreateSocket(int nLocalPort)
{
  //If this version of the function is being called,
  //a valid port number must be specified
  if (nLocalPort <= 0)
    return CWINSOCK_PROGRAMMING_ERROR;

  //Convert the port number into a string and
  //call the version of CreateSocket() which
  //accepts a string
  char pszLocalService[18];
  _itoa(nLocalPort, pszLocalService, 10);
  return CreateSocket(pszLocalService);
}

/////////////////////////////////////////////////////////////////////////////
// int CStreamSocket::CreateSocket(LPSTR pszLocalService/*= NULL*/)
//
// Sample Call: CreateSocket();
//
// Description: Create a hidden window that will receive asynchronous messages
// from WinSock.  Also creates a socket and optionally binds it to
// a name if the socket is a server socket. This version of the CreateSocket()
// function takes a
// string containing a service name or port number.
// A parameter should only be specified if the socket is to be
// bound to a certain port.  If you don't care which port is
// assigned to the socket, just call CreateSocket() without
// any parameter, causing CreateSocket(NULL) to be called.
//
// Input/Output data elements: 
//	- Input: Argument LPSTR pszLocalService.
//	- Output: Returns integer nStatus.
//
// Error handling:
//	 - If any socket errors are encountered, closesocket() is called 
//		to close the socket, and then DestroyWindow() is called 
//		to destroy the hidden window..
//
// Author:	Jim Dunne
//
// Date:	July 29, 1997
//
int CStreamSocket::CreateSocket(LPSTR pszLocalService/*= NULL*/)
{
  int nStatus = CWINSOCK_NOERROR;

  while (1)
  {
    //Make sure the socket isn't already created.
    //If the socket handle is valid, return from this
    //function right away so the existing parameters of
    //the object are not tampered with.
    if (m_s != INVALID_SOCKET)
      return CWINSOCK_PROGRAMMING_ERROR;

    InitVars();

    //Create the hidden window
    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = 100;
    rect.bottom = 100;
    if (Create(NULL, NULL, WS_OVERLAPPEDWINDOW, rect, m_pParentWnd, 0) == 0)
    {
      nStatus = CWINSOCK_WINDOWS_ERROR;
      break;
    }

    //Create the socket
    m_s = socket(AF_INET, SOCK_STREAM, 0);
    if (m_s == INVALID_SOCKET)
    {
      m_nLastError = WSAGetLastError();
      nStatus = CWINSOCK_WINSOCK_ERROR;
      DestroyWindow();
      break;
    }

    //If pszLocalService is not NULL, this is a server socket
    //that will accept data on the specified port.
    if (pszLocalService != NULL)
    {	//This socket is bound to a port number
		//so set the server flag
		m_bServer = TRUE;

		//Assign the address family
		m_sinLocal.sin_family = AF_INET;

		//Assign the service port (may have to do a database lookup
		//if a service port number was not specified)
		m_sinLocal.sin_port = htons(atoi(pszLocalService));
		if (m_sinLocal.sin_port == 0)
		{
			LPSERVENT pSent = getservbyname(pszLocalService, "tcp");
			if (pSent == NULL)
			{
				m_nLastError = WSAGetLastError();
				nStatus = CWINSOCK_WINSOCK_ERROR;
				closesocket(m_s);
				DestroyWindow();
				break;
			}
			m_sinLocal.sin_port = pSent->s_port;
		}
	
		//Assign the IP address
		m_sinLocal.sin_addr.s_addr = htonl(INADDR_ANY);

		//Bind the server socket to the name containing the port
		if (bind(m_s, (LPSOCKADDR)&m_sinLocal, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
		{
			m_nLastError = WSAGetLastError();
			nStatus = CWINSOCK_WINSOCK_ERROR;
			closesocket(m_s);
			DestroyWindow();
			break;
		}
	}

	//Start asynchronous event notification
	long lEvent;

	if (m_bServer)
		lEvent = FD_READ | FD_WRITE | FD_ACCEPT | FD_CLOSE;
	else
		lEvent = FD_READ | FD_WRITE | FD_CONNECT | FD_CLOSE;

	if(WSAAsyncSelect(m_s, m_hWnd, CWINSOCK_EVENT_NOTIFICATION, lEvent) == 
		SOCKET_ERROR)
	{
		m_nLastError = WSAGetLastError();
		nStatus = CWINSOCK_WINSOCK_ERROR;
		closesocket(m_s);
		DestroySocket();
		break;
	}

	//If this is a server, listen for client connections
	if (m_bServer)
	{
		if(listen(m_s, 3) == SOCKET_ERROR)
		{	 
			m_nLastError = WSAGetLastError();
			nStatus = CWINSOCK_WINSOCK_ERROR;
			closesocket(m_s);
			DestroySocket();
			break;
		}
	}

	break;
	}

	//If anything failed in this function, set the
	//socket variables appropriately
	if(nStatus != CWINSOCK_NOERROR)
		InitVars(FALSE);

	return nStatus;
}

/////////////////////////////////////////////////////////////////////////////
// int CStreamSocket::CreateDataSocket(int nLocalPort)
//
// Sample Call: CreateDataSocket(21);
//
// Description: This version of the CreateSocket() function takes a 
// port number, in host order, as input.  A port number should only be 
// specified if the socket is to be bound to a certain port. 
//	- Checks to see if a valid port number has been specified.  If not, 
//		returns CWINSOCK_PROGRAMMING_ERROR.
//	- Converts the port number into a string and calls the version of 
//		CreateSocket() which accepts a string.
//
// Input/Output data elements:
//	- Input: Argument int nLocalPort.
//	- Output: Returns CreateSocket(pszLocalService).
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	July 29, 1997
//
int CStreamSocket::CreateDataSocket(int nLocalPort)
{
	//If this version of the function is being called,
	//a valid port number must be specified
	if (nLocalPort <= 0)
		return CWINSOCK_PROGRAMMING_ERROR;

	//Convert the port number into a string and
	//call the version of CreateSocket() which
	//accepts a string
	char pszLocalService[18];
	_itoa(nLocalPort, pszLocalService, 10);
	return CreateSocket(pszLocalService);
}
/////////////////////////////////////////////////////////////////////////////
// int CStreamSocket::CreateDataSocket(LPSTR pszLocalService/*= NULL*/)
//
// Sample Call: CreateDataSocket(21);
//
// Description: Creates a hidden window that will receive asynchronous 
// messages from WinSock.  Also creates a socket and optionally binds it 
// to a name if the socket is a server socket. 
//	- Makes sure the socket isn't already created - if so, returns 
//		CWINSOCK_PROGRAMMING_ERROR.
//	- Creates the hidden message window.
//	- Creates the socket.
//	- Assigns the address family as AF_INET.
//	- Assigns the service port.
//	- Assigns the IP address.
//	- Binds the server socket to the name containing the port.
//	- Gets the port number.
//	- Starts the asynchronous event notification.
//	- Listens for client connections.
//
// Input/Output data elements:
//	- Input: Argument LPSTR pszLocalService.
//	- Output: Returns an integer Winsock status variable.
//
// Error handling:
//	- If any socket errors are encountered, closesocket() is called to close 
//	the socket, and then DestroyWindow() is called to destroy the hidden window.
//
// Author:	Jim Dunne
//
// Date:	July 29, 1997
//
int CStreamSocket::CreateDataSocket(LPSTR pszLocalService/*= NULL*/)
{
	int nStatus = CWINSOCK_NOERROR;
	
	while (1)
	{
	//Make sure the socket isn't already created.
	//If the socket handle is valid, return from this
	//function right away so the existing parameters of
	//the object are not tampered with.
	if (m_sData != INVALID_SOCKET)
		return CWINSOCK_PROGRAMMING_ERROR;

	//Create the hidden window
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = 100;
	rect.bottom = 100;

	if(Create(NULL, NULL, WS_OVERLAPPEDWINDOW, rect, m_pParentWnd, 0) == 0)
	{
		nStatus = CWINSOCK_WINDOWS_ERROR;
		break;
	}

	//Create the socket
	m_sData = socket(AF_INET, SOCK_STREAM, 0);
	if(m_sData == INVALID_SOCKET)
	{
		AfxMessageBox("Invalid Data Socket");
		m_nLastError = WSAGetLastError();
		nStatus = CWINSOCK_WINSOCK_ERROR;
		DestroyWindow();
		break;
	}

	//Assign the address family
	m_sinLocalData.sin_family = AF_INET;

	//Let winsock assign a port
	m_sinLocalData.sin_port = 0;

	//Assign the IP address
	m_sinLocalData.sin_addr.s_addr = htonl(INADDR_ANY);

	//Bind the server socket to the name containing the port
	if (bind(m_sData, (LPSOCKADDR)&m_sinLocalData,
		sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		m_nLastError = WSAGetLastError();
		nStatus = CWINSOCK_WINSOCK_ERROR;
		closesocket(m_sData);
		DestroyWindow();
		break;
	}

	//Get the port number
	int naddrAssignedLen = sizeof(m_sinaddrAssigned);
	getsockname(m_sData, (LPSOCKADDR)&m_sinaddrAssigned,
		&naddrAssignedLen);

	//Start asynchronous event notification
	long lEvent;

	lEvent = FD_READ | FD_WRITE | FD_ACCEPT | FD_CLOSE;

	if (WSAAsyncSelect(m_sData, m_hWnd, CWINSOCK_EVENT_NOTIFICATION, lEvent) ==
		SOCKET_ERROR)
	{
		m_nLastError = WSAGetLastError();
		nStatus = CWINSOCK_WINSOCK_ERROR;
		closesocket(m_sData);
		DestroySocket();
		break;
	}

	// Listen for client connections
	if (listen(m_sData, 3) == SOCKET_ERROR)
	{	
		m_nLastError = WSAGetLastError();
		nStatus = CWINSOCK_WINSOCK_ERROR;
		closesocket(m_sData);
		DestroySocket();
		break;
	}
	break;
	}

	//If anything failed in this function, set the
	//socket variables appropriately
	if(nStatus != CWINSOCK_NOERROR)
		InitVars(FALSE);

	return nStatus;
}

/////////////////////////////////////////////////////////////////////////////
// int CStreamSocket::DestroySocket()
//
// Sample Call: DestroySocket();
//
// Description: Closes the socket, removes any queued data, and destroys 
// the hidden window. 
//	- Makes sure the socket is valid.
//	- Removes any data in the write queue.
//	- Removes any data in the read queue.
//	- Closes the socket and initialize variables.
//	- Destroys the hidden window.
//
// Input/Output data elements:
//	- Output: Returns an integer Winsock status variable.
//
// Error handling:
//	- If the socket is invalid, returns CWINSOCK_PROGRAMMING_ERROR.
//
// Author:	Jim Dunne
//
// Date:	July 29, 1997
//
int CStreamSocket::DestroySocket()
{
	int nStatus = CWINSOCK_NOERROR;

	//Make sure the socket is valid
	if (m_s == INVALID_SOCKET)
		nStatus = CWINSOCK_PROGRAMMING_ERROR;
	else
	{	//Remove any data in the write queue
		while(!m_listWrite.IsEmpty())
		{
			LPSTREAMDATA pStreamData = (LPSTREAMDATA)m_listWrite.RemoveHead();
			LPVOID pData = pStreamData->pData;
			delete pStreamData;

			m_pParentWnd->PostMessage(m_uMsg, CWINSOCK_ERROR_WRITING,
				(LPARAM)pData);
		}

		//Remove any data in the read queue
		while(!m_listRead.IsEmpty())
		{
			LPSTREAMDATA pStreamData = (LPSTREAMDATA)m_listRead.RemoveHead();
			free(pStreamData->pData);
			delete pStreamData;
		}

		//Close the socket and initialize variables
		closesocket(m_s);
		InitVars();

		//Destroy the hidden window
		DestroyWindow();
	}

return nStatus;
}

/////////////////////////////////////////////////////////////////////////////
// int CStreamSocket::DestroyDataSocket()
//
// Sample Call: DestroyDataSocket();
//
// Description: Closes the socket, removes any queued data, and 
// destroys the hidden window. 
//	- Makes sure the socket is valid.
//	- Removes any data in the write queue.
//	- Removes any data in the read queue.
//	- Closes the socket and initialize variables.
//	- Destroys the hidden window.
//
// Input/Output data elements:
//	- Output: Returns an integer Winsock status variable.
//
// Error handling:
//	- If the socket is invalid, returns CWINSOCK_PROGRAMMING_ERROR.
//
// Author:	Jim Dunne
//
// Date:	July 29, 1997
//
int CStreamSocket::DestroyDataSocket()
{
	int nStatus = CWINSOCK_NOERROR;

	//Make sure the socket is valid
	if(m_sData == INVALID_SOCKET)
		nStatus = CWINSOCK_PROGRAMMING_ERROR;
	else
	{
		//Remove any data in the write queue
		while(!m_listWrite.IsEmpty())
		{
			LPSTREAMDATA pStreamData = (LPSTREAMDATA)m_listWrite.RemoveHead();
			LPVOID pData = pStreamData->pData;
			delete pStreamData;

			m_pParentWnd->PostMessage(m_uMsg, CWINSOCK_ERROR_WRITING,
				(LPARAM)pData);
		}

		//Remove any data in the read queue
		while (!m_listRead.IsEmpty())
		{
			LPSTREAMDATA pStreamData = (LPSTREAMDATA)m_listRead.RemoveHead();
			free(pStreamData->pData);
			delete pStreamData;
		}

		//Close the socket and initialize variables
		closesocket(m_sData);
		InitVars();

		//Destroy the hidden window
		DestroyWindow();
	}

	return nStatus;
}

/////////////////////////////////////////////////////////////////////////////
// int CStreamSocket::Connect(LPSTR pszRemoteName, int nRemotePort)
//
// Sample Call: Connect(m_pszServer, 21);
//
// Description: Connects the client socket to a server specified by the 
// name and port. Converts the port number into a string and calls the 
// version of Connect() which accepts a string service name or number. 
//
// Input/Output data elements:
//	- Output: Returns Connect(pszRemoteName, pszRemoteService).
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	July 29, 1997
//
int CStreamSocket::Connect(LPSTR pszRemoteName, int nRemotePort)
{
	//Convert the port number into a string and
	//call the version of Connect() which accepts
	//a string service name or number
	char pszRemoteService[18];
	_itoa(nRemotePort, pszRemoteService, 10);
	return Connect(pszRemoteName, pszRemoteService);
}

/////////////////////////////////////////////////////////////////////////////
// int CStreamSocket::Connect(LPSTR pszRemoteName, LPSTR pszRemoteService)
//
// Sample Call: Connect(pszRemoteName, pszRemoteService);
//
// Description: Connects the client socket to a server specified by the 
// name and port. Converts the port number into a string and calls the 
// version of Connect() which accepts a string service name or number. 
//
// Input/Output data elements:
//	- Output: Returns Connect(pszRemoteName, pszRemoteService).
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	July 29, 1997
//
int CStreamSocket::Connect(LPSTR pszRemoteName, LPSTR pszRemoteService)
{
	int nStatus = CWINSOCK_NOERROR; // error status
	LPHOSTENT pHent;                // pointer to host entry structure
	LPSERVENT pSent;                // pointer to service entry structure
	SOCKADDR_IN sinRemote;          // Internet address of destination

	while (1)
	{
		//Assign the address family
		sinRemote.sin_family = AF_INET;

		//Assign the service port (may have to do a database lookup
		//if a service port number was not specified)
		sinRemote.sin_port = htons(atoi(pszRemoteService));
		if (sinRemote.sin_port == 0)
		{
			pSent = getservbyname(pszRemoteService, "tcp");
			if (pSent == NULL)
			{
				m_nLastError = WSAGetLastError();
				nStatus = CWINSOCK_WINSOCK_ERROR;
				break;
			}
			sinRemote.sin_port = pSent->s_port;
		}

		//Assign the IP address (may have to do a database lookup
		//if a dotted decimal IP address was not specified)
		sinRemote.sin_addr.s_addr = inet_addr(pszRemoteName);
		if (sinRemote.sin_addr.s_addr == INADDR_NONE)
		{
			pHent = gethostbyname(pszRemoteName);
			if (pHent == NULL)
			{
				m_nLastError = WSAGetLastError();
				nStatus = CWINSOCK_WINSOCK_ERROR;
				break;
			}
			sinRemote.sin_addr.s_addr = *(u_long *)pHent->h_addr;
		}

		//Call the version of Connect() that takes an
		//Internet address structure
		return Connect(&sinRemote);
	}

  return nStatus;
}

/////////////////////////////////////////////////////////////////////////////
// int CStreamSocket::Connect(LPSOCKADDR_IN psinRemote)
//
// Sample Call: Connect(&sinRemote);
//
// Description: Connects the client socket to a server specified 
// by the Internet address. 
//	- Makes sure we are a client.
//	- Copies the Internet address of the remote server to connect to.
//	- Attempts the asynchronous connect().
//	- Calls the version of Connect() that takes an Internet address structure.
//
// Input/Output data elements:
//	- Input: Argument LPSOCKADDR_IN psinRemote.
//	- Output: Returns an integer Winsock status variable.
//
// Error handling:
//	- If not a client, returns CWINSOCK_PROGRAMMING_ERROR.
//	- If connect() fails, returns the WSAGetLastError result.
//
// Author:	Jim Dunne
//
// Date:	July 29, 1997
//
int CStreamSocket::Connect(LPSOCKADDR_IN psinRemote)
{
	int nStatus = CWINSOCK_NOERROR;

	while(1)
	{
		//Only clients should call connect
		if(m_bServer)
		{
			nStatus = CWINSOCK_PROGRAMMING_ERROR;
			break;
		}

		//Copy the Internet address of the remote server to connect to
		memcpy(&m_sinRemote, psinRemote, sizeof(SOCKADDR_IN));

		//Attempt the asynchronous connect
		if(connect(m_s, (LPSOCKADDR)&m_sinRemote, sizeof(SOCKADDR_IN)) ==
			SOCKET_ERROR)
		{
			m_nLastError = WSAGetLastError();

			if (m_nLastError == WSAEWOULDBLOCK)
				m_nLastError = 0;
			else
				nStatus = CWINSOCK_WINSOCK_ERROR;
			break;
		}

		break;
	}

	return nStatus;
}

/////////////////////////////////////////////////////////////////////////////
// int CStreamSocket::Accept(CStreamSocket *pStreamSocket)
//
// Sample Call: Accept(m_pStreamSocket);
//
// Description: Accepts a connection request from a client.
//	- Makes sure a valid CStreamSocket object pointer was passed in.
//	- Makes sure the socket isn't already created.
//	- Creates a hidden message window.
//	- Accepts the client connection.
//	- Starts the asynchronous event notification.
//	- If anything fails in the function, sets the socket 
//		variables appropriately.
//	- Notifies the parent window if the connection was 
//		accepted successfully.
//
// Input/Output data elements:
//	- Input: Argument CStreamSocket *pStreamSocket.
//	- Output: Returns an integer Winsock status variable.
//
// Error handling: 
//	- If the CStreamSocket object pointer isn't valid, returns 
//		CWINSOCK_PROGRAMMING_ERROR and breaks.
//	- If the hidden window creation fails, returns 
//		CWINSOCK_WINDOWS_ERROR and breaks.
//	- If the pStreamSocket is invalid, calls WSAGetLastError, returns 
//		CWINSOCK_WINSOCK_ERROR, destroys the pStreamSocket 
//		message window, and breaks.
//	- If the WSAAsyncSelect() call fails, returns CWINSOCK_WINSOCK_ERROR, 
//		closes and destroys the pStreamSocket socket.
//	- If anything fails in the function, returns CWINSOCK_WINSOCK_ERROR, 
//		and intializes the variables to FALSE.
//
// Author:	Jim Dunne
//
// Date:	July 29, 1997
//
int CStreamSocket::Accept(CStreamSocket *pStreamSocket)
{
int nStatus = CWINSOCK_NOERROR;

	while (1)
	{
		//Must have valid CStreamSocket object pointer passed in
		if (pStreamSocket == NULL)
		{
			ASSERT(0);
			nStatus = CWINSOCK_PROGRAMMING_ERROR;
			break;
		}

		//Make sure the socket isn't already created.
		//If the socket handle is valid, return from this
		//function right away so the existing parameters of
		//the object are not tampered with.
		if (pStreamSocket->m_s != INVALID_SOCKET)
			return CWINSOCK_PROGRAMMING_ERROR;

		//Create the hidden window
	    RECT rect;
	    rect.left = 0;
	    rect.top = 0;
	    rect.right = 100;
	    rect.bottom = 100;
	    if (pStreamSocket->Create(NULL, NULL, WS_OVERLAPPEDWINDOW, rect, 
			pStreamSocket->m_pParentWnd, 0) == 0)
		{
			nStatus = CWINSOCK_WINDOWS_ERROR;
			break;
		}

		//Accept the client connection
		pStreamSocket->m_s = accept(m_sData, NULL, NULL);
		if (pStreamSocket->m_s == INVALID_SOCKET)
		{
			m_nLastError = WSAGetLastError();
			nStatus = CWINSOCK_WINSOCK_ERROR;
			pStreamSocket->DestroyWindow();
			break;
		}

		//Start asynchronous event notification
		long lEvent;
		lEvent = FD_READ | FD_WRITE | FD_CONNECT | FD_CLOSE;
		if(WSAAsyncSelect(pStreamSocket->m_s, pStreamSocket->m_hWnd, 
			CWINSOCK_EVENT_NOTIFICATION, lEvent) == SOCKET_ERROR)
		{
			m_nLastError = WSAGetLastError();
			nStatus = CWINSOCK_WINSOCK_ERROR;
			closesocket(pStreamSocket->m_s);
			pStreamSocket->DestroySocket();
			break;
		}

		break;
	}

	//If anything failed in this function, set the
	//socket variables appropriately
	if(nStatus == CWINSOCK_WINSOCK_ERROR)
		pStreamSocket->InitVars(FALSE);
	else if(nStatus == CWINSOCK_NOERROR)
		//Notify the parent if the connection was accepted successfully
		pStreamSocket->m_pParentWnd->PostMessage(pStreamSocket->m_uMsg, 
			CWINSOCK_YOU_ARE_CONNECTED);

	return nStatus;
}

/////////////////////////////////////////////////////////////////////////////
// int CStreamSocket::Write(int nLen, LPVOID pData)
//
// Sample Call: Write(strlen(m_pszBuf), m_pszBuf);
//
// Description: Writes data to the socket.
//	- Dynamically allocates an LPSTREAMDATA structure to hold the data 
//		pointer and the data's length.
//	- Fills the structure with the data and its length.
//	- Adds the data to the data list.
//	- Triggers the FD_WRITE handler to try to send.
//
// Input/Output data elements:
//	- Input: Arguments int nLen, LPVOID pData.
//	- Output: Returns an integer Winsock status variable.
//
// Error handling: 
//	- If the LPSTREAMDATA object can't be created, returns 
//		CWINSOCK_WINDOWS_ERROR and breaks.
//
// Author:	Jim Dunne
//
// Date:	July 29, 1997
//
int CStreamSocket::Write(int nLen, LPVOID pData)
{
	CWaitCursor cWait;
	int nStatus = CWINSOCK_NOERROR;

	while (1)
	{
		//Dynamically allocate a structure to hold the
		//data pointer and the data's length
		LPSTREAMDATA pStreamData = new STREAMDATA;
		if(pStreamData == NULL)
		{
			nStatus = CWINSOCK_WINDOWS_ERROR;
			break;
		}

		pStreamData->pData = pData;
		pStreamData->nLen = nLen;

		//Add the data to the list
		m_listWrite.AddTail(pStreamData);

		//Trigger the FD_WRITE handler to try to send
		PostMessage(CWINSOCK_EVENT_NOTIFICATION, m_s, WSAMAKESELECTREPLY(FD_WRITE, 0));
		break;
	}

	return nStatus;
}

/////////////////////////////////////////////////////////////////////////////
// LPVOID CStreamSocket::Read(LPINT pnLen)
//
// Sample Call: Read(&nLen);
//
// Description: Read data that has been received by the socket.
//	- Creates an LPVOID data buffer.
//	- Checks to see if there is data to retrieve.
//	- Removes the stream data from the list and puts it into the 
//		LPVOID buffer.
//
// Input/Output data elements:
//	- Input: Argument LPINT pnLen.
//	- Output: Returns an LPVOID buffer containing the data.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	July 29, 1997
//
LPVOID CStreamSocket::Read(LPINT pnLen)
{
	CWaitCursor cWait;
	LPVOID pData = NULL;

	//Check to see if there is data to retrieve
	if(!m_listRead.IsEmpty())
	{
		//Remove the stream data from the list
		LPSTREAMDATA pStreamData = (LPSTREAMDATA)m_listRead.RemoveHead();
		pData = pStreamData->pData;

		*pnLen = pStreamData->nLen;
		delete pStreamData;
	}

	return pData;
}

//Message map
BEGIN_MESSAGE_MAP(CStreamSocket, CWnd)
  //{{AFX_MSG_MAP(CStreamSocket)
  //}}AFX_MSG_MAP
  ON_MESSAGE(CWINSOCK_EVENT_NOTIFICATION, OnWinSockEvent)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// LONG CStreamSocket::OnWinSockEvent(WPARAM wParam, LPARAM lParam)
//
// Sample Call: WSAAsyncSelect(m_sData, m_hWnd, 
//	CWINSOCK_EVENT_NOTIFICATION, lEvent);
//
// Description: Called when there is an asynchronous event on the socket.
//	- Checks for an error by typecasting lParam to WSAGETSELECTERROR.
//	- Switch statement handles the error type actions:
//		- case FD_READ: A read event - calls HandleRead().
//		- case FD_WRITE: A write event - calls handleWrite().
//		- case FD_ACCEPT: Tells the parent window that a client would like 
//			to connect to the server socket - sends a 
//			CWINSOCK_READY_TO_ACCEPT_CONNECTION message to the parent.
//		-  case FD_CLOSE: 
//	- Checks for more data queued on the socket.
//	- Fakes the close event to try again by sending a 
//		CWINSOCK_EVENT_NOTIFICATION message.
//	- Tells the parent window that the socket is closed by sending a 
//		CWINSOCK_LOST_CONNECTION message.
//
// Input/Output data elements:
//	- Input: Arguments WPARAM wParam, LPARAM lParam.
//	- Output: Returns zero.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	July 29, 1997
//
LONG CStreamSocket::OnWinSockEvent(WPARAM wParam, LPARAM lParam)
{
	CWaitCursor cWait;
	//Check for an error
	if(WSAGETSELECTERROR(lParam) != 0)
		return 0L;

	//What event are we being notified of?
	switch (WSAGETSELECTEVENT(lParam))
	{
		case FD_READ:
			return HandleRead(wParam, lParam);
			break;
		case FD_WRITE:
			return HandleWrite(wParam, lParam);
			break;
		case FD_ACCEPT:
			//Tell the parent window that a client would like to connect
			//to the server socket
			m_pParentWnd->PostMessage(m_uMsg, CWINSOCK_READY_TO_ACCEPT_CONNECTION);
			break;
		case FD_CONNECT:
			//Tell the parent window that the socket has connected
			m_pParentWnd->PostMessage(m_uMsg, CWINSOCK_YOU_ARE_CONNECTED);
			break;
		case FD_CLOSE:
			//Check for more data queued on the socket
			//(don't tell the application that the socket is closed
			//until all data has been read and notification has been posted)
			if(HandleRead(wParam, lParam))
			{
				//Fake the close event to try again
				PostMessage(CWINSOCK_EVENT_NOTIFICATION, wParam, lParam);
				break;
			}
      
			//Tell the parent window that the socket is closed
			m_pParentWnd->PostMessage(m_uMsg, CWINSOCK_LOST_CONNECTION);
			break;
		default:
			//This should never happen
			ASSERT(0);
			break;
	}

	return 0L;
}

/////////////////////////////////////////////////////////////////////////////
// LONG CStreamSocket::HandleRead(WPARAM wParam, LPARAM lParam)
//
// Sample Call: HandleRead(WPARAM wParam, LPARAM lParam);
//
// Description: Called when there is an asynchronous read event on the socket.
//	- Allocates memory for incoming data.
//	- If data was NULL:
//	- Frees anything that was allocated.
//	- Tells the parent that a possible data read failed.
//	- Fakes the event to try again with an FD_READ message.
//	- Receives the data.
//		- If there was a socket error: 
//			- Frees the memory allocated for incoming data.
//			- If the error is just that the read would block, doesn't do 
//			anything; as another FD_READ will come along soon.  Otherwise, 
//			tells the parent that a data read failed.
//	- Makes sure some data was read:
//		- If not, frees the memory for the incoming data and breaks.
//	- Adds the data to the data list.
//	- Tell the parent that the data has been read.
//
// Input/Output data elements:
//	- Input: Arguments WPARAM wParam, LPARAM lParam.
//	- Output: Returns one if data was read, else zero.
//
// Error handling:
//	- If there is an error reading data, the memory allocated to hold the 
//	incoming data is released, and the parent window is notified 
//	of a possible failed read.
//
// Author:	Jim Dunne
//
// Date:	July 29, 1997
//
LONG CStreamSocket::HandleRead(WPARAM wParam, LPARAM lParam)
{
	CWaitCursor cWait;
	while(1)
	{
		//Allocate memory for incoming data
		LPVOID pData = malloc(READ_BUF_LEN);
		LPSTREAMDATA pStreamData = new STREAMDATA;
		if((pData == NULL) || (pStreamData == NULL))
		{	//Free anything that was allocated
			if(pData != NULL)
				free(pData);
			pData = NULL;
			if(pStreamData != NULL)
				delete pStreamData;
			pStreamData = NULL;

			//Tell the parent that a possible data read failed
			m_pParentWnd->PostMessage(m_uMsg, 
				CWINSOCK_ERROR_READING);

			//Fake the event to try again
			PostMessage(CWINSOCK_EVENT_NOTIFICATION, m_s,
				WSAMAKESELECTREPLY(FD_READ, 0));

			break;
		}

		//Receive data
		int nBytesRead = recv(m_s, (LPSTR)pData, READ_BUF_LEN, 0);

		if(nBytesRead == SOCKET_ERROR)
		{
			//Free memory for incoming data
			free(pData);
			pData = NULL;
			delete pStreamData;
			pStreamData = NULL;

			//If the error is just that the read would block,
			//don't do anything; we'll get another FD_READ soon
			m_nLastError = WSAGetLastError();
			if (m_nLastError == WSAEWOULDBLOCK)
				m_nLastError = 0;
			else
				//Tell the parent that a data read failed
				m_pParentWnd->PostMessage(m_uMsg, 
					CWINSOCK_ERROR_READING);
			break;
		}

		//Make sure some data was read
		if(nBytesRead == 0)
		{
			//Free memory for incoming data
			free(pData);
			pData = NULL;
			delete pStreamData;
			pStreamData = NULL;

			break;
		}

		//Add the data to the list
		pStreamData->pData = pData;
		pStreamData->nLen = nBytesRead;
		m_listRead.AddTail(pStreamData);

		//Tell the parent that data has been read
		m_pParentWnd->PostMessage(m_uMsg, CWINSOCK_DONE_READING, 
			(LPARAM)m_listRead.GetCount());

		//1 is returned if there is data so 
		//CStreamSocket::OnWinSockEvent()'s FD_CLOSE handler will 
		//know when the socket can really be closed
		return 1L;
   
		break;
	}

	return 0L;
}

/////////////////////////////////////////////////////////////////////////////
// LONG CStreamSocket::HandleWrite(WPARAM wParam, LPARAM lParam)
//
// Sample Call: HandleWrite(WPARAM wParam, LPARAM lParam);
//
// Description: Called when there is an asynchronous write event 
// on the socket.
//	- Creates data buffers.
//	- Checks to see if there is any data to send.
//	- If another buffer is not sending, get data and data length 
//	from the write queue.
//	- Sends the data.
//		- If there was a socket error: 
//			- If the error is just that the read would block, doesn't 
//			do anything, as another FD_READ will come along soon.  Otherwise, 
//			tells the parent that a data write failed.
//		- If data was sent, checks to see if all the bytes were sent. 
//		If not,  adjusts the buffer values accordingly.
//	- If the data was completely sent or there was a real error, removes 
//	the data from the queue.
//	- If there is more data to send, triggers the FD_WRITE handler.
//
// Input/Output data elements:
//	- Input: Arguments WPARAM wParam, LPARAM lParam.
//	- Output: Returns zero.
//
// Error handling:
//	- If there is an error writing data, sends a CWINSOCK_ERROR_WRITING 
//	message to the parent and removes the data from the queue.
//
// Author:	Jim Dunne
//
// Date:	July 29, 1997
//
LONG CStreamSocket::HandleWrite(WPARAM wParam, LPARAM lParam)
{
	CWaitCursor cWait;
	LPSTREAMDATA pStreamData;            // pointer to stream data structure
	LPVOID pData;                        // pointer to buffer to send
	int nLen;                            // total length of buffer to send
	static LPVOID pDataRemaining = NULL; // pointer into buffer to send
	static int nLenRemaining = 0;        // number of bytes left to send

	while(1)
	{
		//Check to see if there is any data to send
		if(m_listWrite.IsEmpty())
			break;

		//If we are not in the middle of another buffer send,
		//get data and data length from the write queue
		pStreamData = (LPSTREAMDATA)m_listWrite.GetHead(); // not RemoveHead()
		pData = pStreamData->pData;
		nLen = pStreamData->nLen;

		if(pDataRemaining == NULL)
		{
			pDataRemaining = pData;
			nLenRemaining = nLen;
		}

		//Send the data
		BOOL bRemove = FALSE;      // remove data from queue?
		int nBytesSent = send(m_s, (LPCSTR)pDataRemaining, nLenRemaining, 0);
		if(nBytesSent == SOCKET_ERROR)
		{
			//If the error is just that the send would block,
			//don't do anything; we'll get another FD_WRITE soon
			m_nLastError = WSAGetLastError();
			if (m_nLastError == WSAEWOULDBLOCK)
				m_nLastError = 0;
			else
			{
				bRemove = TRUE;
				m_pParentWnd->PostMessage(m_uMsg, CWINSOCK_ERROR_WRITING, 
					(LPARAM)pData);
			}
		}
		else
		{
			//If data was sent, we must still check to see
			//if all the bytes were sent
			if (nBytesSent == nLenRemaining)
			{
				bRemove = TRUE;
				m_pParentWnd->PostMessage(m_uMsg, CWINSOCK_DONE_WRITING, 
					(LPARAM)pData);
			}
			else
			{
				//The complete buffer was not sent so adjust
				//these values accordingly
				pDataRemaining = (LPVOID)((LPCSTR)pDataRemaining + nBytesSent);
				nLenRemaining = nLenRemaining - nBytesSent;
			}
		}

		//If the data was completely sent or there was
		//a real error, remove the data from the queue
		if(bRemove)
		{
			delete pStreamData;
			m_listWrite.RemoveHead();
			pDataRemaining = NULL;
			nLenRemaining = 0;
		}

		//If there is more data to send, trigger this FD_WRITE handler
		if(!m_listWrite.IsEmpty())
			PostMessage(CWINSOCK_EVENT_NOTIFICATION, m_s, 
				WSAMAKESELECTREPLY(FD_WRITE, 0));

		break;
	}

	return 0L;
}

/////////////////////////////////////////////////////////////////////////////
// int CStreamSocket::GetPeerName(LPSOCKADDR_IN psinRemote)
//
// Sample Call: GetPeerName(&sin);
//
// Description: Copies the Internet address of the other end of the 
// socket connection into the pointer provided.  Useful for server to 
// use after an Accept().
//	- Makes sure the listening socket doesn't call this function.
//	- Calls getpeername().
//
// Input/Output data elements: 
//	- Input argument LPSOCKADDR_IN psinRemote. 
//	- Output: Returns an integer Winsock status variable.
//
// Error handling:
//	- If there is an error calling getpeername(), calls WSAGetLastError 
//	and returns CWINSOCK_WINSOCK_ERROR.
//
// Author:	Jim Dunne
//
// Date:	July 29, 1997
int CStreamSocket::GetPeerName(LPSOCKADDR_IN psinRemote)
{
	int nStatus = CWINSOCK_NOERROR;
	int nLen = sizeof(SOCKADDR_IN);

	//Make sure the listening socket doesn't call this function
	if(m_bServer)
		nStatus = CWINSOCK_PROGRAMMING_ERROR;
	else if(getpeername(m_s, (LPSOCKADDR)psinRemote, &nLen) == SOCKET_ERROR)
	{
		m_nLastError = WSAGetLastError();
		nStatus = CWINSOCK_WINSOCK_ERROR;
	}

	return nStatus;
}

/////////////////////////////////////////////////////////////////////////////
// LPSTR CWinsock::ErrorBox(int nError, LPSTR pszMessage)
//
// Sample Call: ErrorBox(WSAGetLastError(), "");
//
// Description: Reports Winsock errors back to CMainView.  
// Takes WSAGetLastError() as nError, and a text string as 
// pszMessage.  Returns the corresponding Winsock error 
// message to CMainView for printing to the screen.
//	- Formats a string to take the error message.
//	- Adds the text of the error message to the end of the error string.
//	- If pszMessage exists, adds it to the end of the error message.
//
// Input/Output data elements: 
//	- Input: Arguments int nError, LPSTR pszMessage.
//	- Output: Returns the error message as a string pointer.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	July 29, 1997
//
LPSTR CWinsock::ErrorBox(int nError, LPSTR pszMessage/*= NULL*/)
{
  //Format m_lpszError to take Error mesage 
	wsprintf(m_lpszError, "WinSock error %d: ", nError);

  switch (nError)
  { //Put error message into m_lpszError
    case WSAEINTR:
      lstrcat(m_lpszError, "Interrupted system call");
      break;
    case WSAEBADF:
      lstrcat(m_lpszError, "Bad file number");
      break;
    case WSAEACCES:
      lstrcat(m_lpszError, "Permission denied");
      break;
    case WSAEFAULT:
      lstrcat(m_lpszError, "Bad address");
      break;
    case WSAEINVAL:
      lstrcat(m_lpszError, "Invalid argument");
      break;
    case WSAEMFILE:
      lstrcat(m_lpszError, "Too many open files");
      break;
    case WSAEWOULDBLOCK:
      lstrcat(m_lpszError, "Operation would block");
      break;
    case WSAEINPROGRESS:
      lstrcat(m_lpszError, "Operation now in progress");
      break;
    case WSAEALREADY:
      lstrcat(m_lpszError, "Operation already in progress");
      break;
    case WSAENOTSOCK:
      lstrcat(m_lpszError, "Socket operation on non-socket");
      break;
    case WSAEDESTADDRREQ:
      lstrcat(m_lpszError, "Destination address required");
      break;
    case WSAEMSGSIZE:
      lstrcat(m_lpszError, "Message too long");
      break;
    case WSAEPROTOTYPE:
      lstrcat(m_lpszError, "Protocol wrong type for socket");
      break;
    case WSAENOPROTOOPT:
      lstrcat(m_lpszError, "Protocol not available");
      break;
    case WSAEPROTONOSUPPORT:
      lstrcat(m_lpszError, "Protocol not supported");
      break;
    case WSAESOCKTNOSUPPORT:
      lstrcat(m_lpszError, "Socket type not supported");
      break;
    case WSAEOPNOTSUPP:
      lstrcat(m_lpszError, "Operation not supported on socket");
      break;
    case WSAEPFNOSUPPORT:
      lstrcat(m_lpszError, "Protocol family not supported");
      break;
    case WSAEAFNOSUPPORT:
      lstrcat(m_lpszError, "Address family not supported by protocol family");
      break;
    case WSAEADDRINUSE:
      lstrcat(m_lpszError, "Address already in use");
      break;
    case WSAEADDRNOTAVAIL:
      lstrcat(m_lpszError, "Can't assign requested address");
      break;
    case WSAENETDOWN:
      lstrcat(m_lpszError, "Network is down");
      break;
    case WSAENETUNREACH:
      lstrcat(m_lpszError, "Network is unreachable");
      break;
    case WSAENETRESET:
      lstrcat(m_lpszError, "Network dropped connection on reset");
      break;
    case WSAECONNABORTED:
      lstrcat(m_lpszError, "Software caused connection abort");
      break;
    case WSAECONNRESET:
      lstrcat(m_lpszError, "Connection reset by peer");
      break;
    case WSAENOBUFS:
      lstrcat(m_lpszError, "No buffer space available");
      break;
    case WSAEISCONN:
      lstrcat(m_lpszError, "Socket is already connected");
      break;
    case WSAENOTCONN:
      lstrcat(m_lpszError, "Socket is not connected");
      break;
    case WSAESHUTDOWN:
      lstrcat(m_lpszError, "Can't send after socket shutdown");
      break;
    case WSAETOOMANYREFS:
      lstrcat(m_lpszError, "Too many references: can't splice");
      break;
    case WSAETIMEDOUT:
      lstrcat(m_lpszError, "Connection timed out");
      break;
    case WSAECONNREFUSED:
      lstrcat(m_lpszError, "Connection refused");
      break;
    case WSAELOOP:
      lstrcat(m_lpszError, "Too many levels of symbolic links");
      break;
    case WSAENAMETOOLONG:
      lstrcat(m_lpszError, "File name too long");
      break;
    case WSAEHOSTDOWN:
      lstrcat(m_lpszError, "Host is down");
      break;
    case WSAEHOSTUNREACH:
      lstrcat(m_lpszError, "No route to host");
      break;
    case WSAENOTEMPTY:
      lstrcat(m_lpszError, "Directory not empty");
      break;
    case WSAEPROCLIM:
      lstrcat(m_lpszError, "Too many processes");
      break;
    case WSAEUSERS:
      lstrcat(m_lpszError, "Too many users");
      break;
    case WSAEDQUOT:
      lstrcat(m_lpszError, "Disc quota exceeded");
      break;
    case WSAESTALE:
      lstrcat(m_lpszError, "Stale NFS file handle");
      break;
    case WSAEREMOTE:
      lstrcat(m_lpszError, "Too many levels of remote in path");
      break;
#ifdef _WIN32
    case WSAEDISCON:
      lstrcat(m_lpszError, "Disconnect");
      break;
#endif
    case WSASYSNOTREADY:
      lstrcat(m_lpszError, "Network sub-system is unusable");
      break;
    case WSAVERNOTSUPPORTED:
      lstrcat(m_lpszError, "WinSock DLL cannot support this application");
      break;
    case WSANOTINITIALISED:
      lstrcat(m_lpszError, "WinSock not initialized");
      break;
    case WSAHOST_NOT_FOUND:
      lstrcat(m_lpszError, "Host not found");
      break;
    case WSATRY_AGAIN:
      lstrcat(m_lpszError, "Non-authoritative host not found");
      break;
    case WSANO_RECOVERY:
      lstrcat(m_lpszError, "Non-recoverable error");
      break;
    case WSANO_DATA:
      lstrcat(m_lpszError, "Valid name, no data record of requested type");
      break;
    default:
      lstrcpy(m_lpszError, "Not a WinSock error");
      break;
  }

  int n = lstrlen(m_lpszError);
  if (pszMessage != NULL)
    n += lstrlen(pszMessage);
  if ((pszMessage != NULL) && (n < ERROR_BUF_LEN))
    lstrcat(m_lpszError, pszMessage);

  return (m_lpszError);
}
