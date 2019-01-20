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
// csock.h : header file
//
// return values for most member functions
#define CWINSOCK_NOERROR            (0)
#define CWINSOCK_WINDOWS_ERROR      (1)
#define CWINSOCK_WINSOCK_ERROR      (2)
#define CWINSOCK_PROGRAMMING_ERROR  (3)
#define ERROR_BUF_LEN (1000)

// wParam of messages sent to application windows
#define CWINSOCK_DONE_WRITING               (1) // lParam = pointer to data
#define CWINSOCK_ERROR_WRITING              (2) // lParam = pointer to data
#define CWINSOCK_DONE_READING               (3) // lParam = # data chunks in queue
#define CWINSOCK_ERROR_READING              (4)
#define CWINSOCK_READY_TO_ACCEPT_CONNECTION (5)
#define CWINSOCK_YOU_ARE_CONNECTED          (6)
#define CWINSOCK_LOST_CONNECTION            (7)

// message to be posted to the socket window for async events
#define CWINSOCK_EVENT_NOTIFICATION (WM_USER + 2)
#define WM_USER_STREAM (WM_USER + 3)

// buffer sizes
#define READ_BUF_LEN (1023)
#define WRITE_BUF_LEN (4096)

// structure used for stream socket read/write queue
typedef struct tagSTREAMDATA
{
  LPVOID pData;
  int    nLen;
} STREAMDATA, FAR * LPSTREAMDATA;

/////////////////////////////////////////////////////////////////////////////
// CWinsock window

class CWinsock : public CWnd
{
// Construction
protected:
CWinsock(WORD wVersionRequired = MAKEWORD(1,1));           // protected constructor used by dynamic creation
int Startup();
int Shutdown();
WORD m_wVersionRequired;
WSADATA m_wsaData;

// Attributes
public:
friend class CMainView;
BOOL m_bWinSockOK;			//TRUE if Winsock Startup Succeeded
int m_nWinSockError;		// Winsock Error code
int m_nLastError;
int m_nStatus;
char m_lpszWSMessage[100];

// Operations
public:
LPSTR CWinsock::ErrorBox(int nError, LPSTR pszMessage/*= NULL*/);
char m_lpszError[ERROR_BUF_LEN];

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWinsock)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWinsock();

	// Generated message map functions
protected:
	//{{AFX_MSG(CWinsock)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CStreamSocket
//
class CStreamSocket : public CWnd
{
private:
	friend class CMainView;

	CWnd *m_pParentWnd;      // window to receive event notification	
	UINT m_uMsg;             // message to send to m_pParentWnd on event
	SOCKET m_s;              // socket handle
	SOCKET m_sAccept;        // socket handle
	SOCKET m_sData;
	SOCKADDR_IN m_sinLocal;  // name bound to socket m_s
	SOCKADDR_IN m_sinRemote; // name on other side of m_s
	SOCKADDR_IN m_sinaddrAssigned;

	int m_nLastError;        // last WinSock error
	BOOL m_bServer;          // TRUE if socket m_s is bound to a name
	BOOL m_bServerData;          // TRUE if socket m_sData is bound to a name
	CPtrList m_listWrite;    // data waiting to be sent
	CPtrList m_listRead;     // data read

	SOCKADDR_IN m_sinLocalData;  // name bound to socket m_s
	SOCKADDR_IN m_sinRemoteData; // name on other side of m_s

public:
  CStreamSocket(CWnd *pParentWnd, UINT uMsg);
  virtual ~CStreamSocket();
  int CreateSocket(int nLocalPort);
  int CreateSocket(LPSTR pszLocalService = NULL);
  int CreateDataSocket(int nLocalPort);
  int CreateDataSocket(LPSTR pszLocalService = NULL);
  int DestroySocket();
  int DestroyDataSocket();
  int Connect(LPSTR pszRemoteName, int nRemotePort);
  int Connect(LPSTR pszRemoteName, LPSTR pszRemoteService);
  int Connect(LPSOCKADDR_IN psinRemote);
  int Accept(CStreamSocket *pStreamSocket);
  int Write(int nLen, LPVOID pData);
  LPVOID Read(LPINT pnLen);
  int GetPeerName(LPSOCKADDR_IN psinRemote);
  int LastError() { return m_nLastError; }

private:
  void InitVars(BOOL bInitLastError = TRUE);
  LONG HandleRead(WPARAM wParam, LPARAM lParam);
  LONG HandleWrite(WPARAM wParam, LPARAM lParam);

  // message map functions
protected:
  //{{AFX_MSG(CStreamSocket)
  //}}AFX_MSG
  LONG OnWinSockEvent(WPARAM wParam, LPARAM lParam);
  DECLARE_MESSAGE_MAP()
};
