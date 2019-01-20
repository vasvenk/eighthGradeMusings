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

// FtpConnect.h : header file
//
#define USER_INPUT_BUF_LEN   (100)
//#define HKEY_CURRENT_USER    "HKEY_CURRENT_USER"
/////////////////////////////////////////////////////////////////////////////
// CFtpConnect dialog

class CFtpConnect : public CDialog
{
// Construction
public:
	CFtpConnect(CWnd* pParent = NULL);   // standard constructor

	void CFtpConnect::ChngConnectDlg(CString csString, int i);
	void CFtpConnect::WriteSettings(LPSTR lpStr);
	void CFtpConnect::ReadSubKey();
	void CFtpConnect::ReadSettings(LPSTR lpStr);

	friend class CMainView;
// Dialog Data
	//{{AFX_DATA(CFtpConnect)
	enum { IDD = IDD_DIALOG_CONNECT };
	CString m_szProfile;
	CString m_szHostName;
	CString m_szUserName;
	CString m_szPassword;
	CString m_szRemInDir;
	CString m_szLocInDir;

		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFtpConnect)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	char m_lpszEntry[100];
	char m_lpszMessage[100];

	CString csProfile;
	CString csHost;
	CString csUser;
	CString csPass;
	CString csRemInitDir;
	CString csLocInitDir;

	BOOL m_bSameAddressFlag;

	// Generated message map functions
	//{{AFX_MSG(CFtpConnect)
	virtual void OnClickedButtonOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnDropdownComboConnectHost();
	afx_msg void OnSelchangeComboConnectHost();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
