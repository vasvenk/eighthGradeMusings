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
// mainview.h : interface of the CMainView class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif
#define USER_INPUT_BUF_LEN   (100)
#define UC(b)   (((int)b)&0xff)	//Convert Port, IP integers to strings

#define WM_USER_ASYNC_HOST_LOOKUP	(WM_USER + 1)
#define WM_USER_LISTEN				(WM_USER + 4)
#define WM_USER_ACCEPT				(WM_USER + 5)

#define TJFTP "TJFTP v. 1.0"


class CMainView : public CFormView
{
protected: // create from serialization only
	CMainView();
	DECLARE_DYNCREATE(CMainView)

	friend class CWinsock;
	friend class CMainFrame;
	friend class CStreamSocket;

	char m_lpszMessage[200];
	char szString[512];
	char m_lpszHost[USER_INPUT_BUF_LEN];
	char m_achHostEnt[MAXGETHOSTSTRUCT];
	char m_pszBuf[100];  // buffer to send
	char m_pszServer[512]; // host name or IP address of stream server
	char m_szRemPath[512];
	char m_szStatus[512];
	char m_pszLocLE[512];
	char m_pszRemLE[512];

	CString m_csLocPath;
	CString	m_csLocDir;
	CString csMain;
	CString csHost;
	CString csUser;
	CString csPass;
	CString m_csRemDir;
	CString m_csLeftovers;
	CString m_csStatus;
	CString m_csListOrFile;
	CString m_csRemFileList;
	CString m_csResponse;
	CString m_csLocalFile;
	CString m_csBytes;

	int m_nPlaceOnList;
	int m_nWinSockError;
	int m_nByName;
	int m_nConDialStatus;
	int m_nBytesReceived;
	int m_nFileSize;
	int m_nTotalSent;

	BOOL m_bWinSockOK;
	BOOL m_bLocalFlag;
	BOOL m_bLocMkDir;
	BOOL m_bRemMkDir;
	BOOL m_bSendFile;
	BOOL m_bReadReady;
	BOOL m_bBinary;
	BOOL m_bWriteDone;
	BOOL m_bPieceDone;
	BOOL m_bAuto;
	BOOL m_bConClose;
	BOOL m_bConnect;

	HANDLE m_hAsyncHost;

	PHOSTENT m_pHostEnt;

	IN_ADDR m_in;
	SOCKADDR_IN m_sinaddrDataPort; // name on other side of m_s

	unsigned long m_ulIPAddress;
	unsigned long m_ulIPLocal;

	CImageList m_imglDrives;
	CProgressCtrl m_wndProgress;

	CPtrList m_Retrieve;    // data waiting to be sent
	CFile * m_cfUserFile;
	CStdioFile* m_cfAsciiFile;
	CWinsock * m_pWinsock;
	CMainFrame * pMainFrame;
	CStreamSocket * m_pStream; // Stream socket to receive from
	CStreamSocket * m_pListenSocket; // Stream socket to wait for connections
	CStreamSocket * m_pDataSocket; // Stream socket to receive data

	FILETIME m_ftYear;

	static int CALLBACK CompareFunc (LPARAM , LPARAM , LPARAM);
	static int CALLBACK CompareFuncRem (LPARAM , LPARAM , LPARAM);

	typedef struct tagITEMINFO
	{
	DWORD		dwFileAttributes;
    CString     strFileName; 
    DWORD       nFileSizeLow; 
    FILETIME    ftLastWriteTime; 
	} ITEMINFO;

public:

	//{{AFX_DATA(CMainView)
	enum{ IDD = IDD_TJFTP_FORM };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Attributes
public:
	CTJFTPDoc* GetDocument();
	void CMainView::InitLocList(LPCTSTR pszPath);
	void CMainView::FreeItemMemory();
	void CMainView::FreeItemMemoryRem();
	void CMainView::ParseInfo(CString csLine);
	void CMainView::SendCommand(CString csString1, CString csString2);
	void CMainView::DrawLocList(); 
	void CMainView::PrintToStatus(LPCSTR lpStr, LPCSTR lpStr1);
	void CMainView::PrintToStatusSpace();
	void CMainView::EnableFunctions();
	void CMainView::DisableFunctions();
	void CMainView::KillStream();
	void CMainView::UpdateControls();
	BOOL CMainView::Port();
	BOOL CMainView::PostDirectory();
	BOOL CMainView::ReadResponse();
	BOOL CMainView::AddItem(WIN32_FIND_DATA* pfd);
	BOOL CMainView::AddDriveNode(CString& strDrive);
	BOOL CMainView::AddItemRemList(int nIndex, LPCTSTR lpszRemFile,
		CString csFileSize, FILETIME FileTime, int DirList);

	FILETIME CMainView::TimeConversion(CString csString);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CMainView)
	afx_msg LONG OnStream(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnAccept(WPARAM wParam, LPARAM lParam);
	afx_msg	LONG OnListenSocket(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateStatus(CCmdUI *pCmdUI);
	afx_msg void OnUpdateUtility(CCmdUI *pCmdUI);
	afx_msg LONG OnAsyncHost(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClickedButtonMainConnect();
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetDispInfoRem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkListLocal(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickListRemote(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkListRemote(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeComboLocdir();
	afx_msg void OnClickedButtonMainLocrefresh();
	afx_msg void OnSelchangeComboRemdir();
	afx_msg void OnBeginlabeleditListLocal(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditListLocal(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickedButtonMainRemrefresh();
	afx_msg void OnClickedButtonMainLocmkdir();
	afx_msg void OnClickedButtonMainLocdel();
	afx_msg void OnClickedButtonMainRemmkdir();
	afx_msg void OnBeginlabeleditListRemote(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditListRemote(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickedButtonMainRemdel();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnClickedButtonMainLocsend();
	afx_msg void OnClickedButtonMainRemret();
	afx_msg void OnClickedRadioMainBinary();
	afx_msg void OnClickedRadioMainAscii();
	afx_msg void OnClickedRadioMainAuto();
	afx_msg void OnButtonMainExit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in mainview.cpp
inline CTJFTPDoc* CMainView::GetDocument()
   { return (CTJFTPDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
