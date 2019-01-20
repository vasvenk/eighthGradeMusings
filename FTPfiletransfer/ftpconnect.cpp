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
// FtpConnect.cpp : implementation file
//

#include "stdafx.h"
#include "TJFTP.h"
#include "FtpConnect.h"
#include <winreg.h>

#include "windows.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFtpConnect dialog


CFtpConnect::CFtpConnect(CWnd* pParent /*=NULL*/)
	: CDialog(CFtpConnect::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFtpConnect)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CFtpConnect::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFtpConnect)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Text(pDX, IDC_COMBO_CONNECT_PROFILE, m_szProfile);
	DDV_MaxChars(pDX, m_szProfile, 40);
	DDX_Text(pDX, IDC_EDIT_CONNECT_HOST, m_szHostName);
	DDV_MaxChars(pDX, m_szHostName, 40);
	DDX_Text(pDX, IDC_EDIT_CONNECT_USER, m_szUserName);
	DDV_MaxChars(pDX, m_szUserName, 40);
	DDX_Text(pDX, IDC_EDIT_CONNECT_PASSWORD, m_szPassword);
	DDV_MaxChars(pDX, m_szPassword, 40);
	DDX_Text(pDX, IDC_EDIT_CONNECT_REMDIR, m_szRemInDir);
	DDV_MaxChars(pDX, m_szRemInDir, 40);
	DDX_Text(pDX, IDC_EDIT_CONNECT_LOCDIR, m_szLocInDir);
	DDV_MaxChars(pDX, m_szLocInDir, 40);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFtpConnect, CDialog)
	//{{AFX_MSG_MAP(CFtpConnect)
	ON_BN_CLICKED(IDOK, OnClickedButtonOK)
	ON_CBN_SELCHANGE(IDC_COMBO_CONNECT_PROFILE, OnSelchangeComboConnectHost)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////
// CFtpConnect message handlers

/////////////////////////////////////////////////////////////////////////////
// BOOL CFtpConnect::OnInitDialog() 
//
// Sample Call: Not called by the user.
//
// Description: Called when the dialog box is initialized - 
// handles all startup actions.
//	- Gets a pointer to the "Profile:" combobox and set the focus to it.
//	- Reads all the subkey folder names and writes them to 
//		the combobox's dropdown list.
//	- Places "Current" entries into their boxes, if valid.  These are 
//		the entries from the last session.
//
// Input/Output data elements: 
//	- Output: Boolean - returns TRUE.
//
// Error handling: None.
//
// Author: Jim Dunne
//
// Date: September 27, 1997
//
BOOL CFtpConnect::OnInitDialog() 
{
	CDialog::OnInitDialog();

	//Get a pointer to the "Profile:" combobox and set the focus to it
	CComboBox *pcb = (CComboBox *)GetDlgItem(IDC_COMBO_CONNECT_PROFILE);
	pcb->SetFocus();

	//Read all the subkey folder names and write them to the combox's
	//dropdown list
	ReadSubKey();

	//Place "Current" entries into their boxes, if valid
	ReadSettings("Current");	//Gets the host info from the last session
	ChngConnectDlg(csProfile,IDC_COMBO_CONNECT_PROFILE);
	ChngConnectDlg(csHost,IDC_EDIT_CONNECT_HOST);
	ChngConnectDlg(csUser,IDC_EDIT_CONNECT_USER);
	ChngConnectDlg(csPass,IDC_EDIT_CONNECT_PASSWORD);
	ChngConnectDlg(csRemInitDir,IDC_EDIT_CONNECT_REMDIR);
	ChngConnectDlg(csLocInitDir,IDC_EDIT_CONNECT_LOCDIR);

	return TRUE;  // return TRUE unless you set the focus to a control
}


/////////////////////////////////////////////////////////////////////////////
// void CFtpConnect::OnClickedButtonOK() 
//
// Sample Call: Not called by the user.
//
// Description: Called when the user presses the "OK" button.
//	- Gets a pointer to the "Profile:" combobox.
//	- Calls UpdateData() to get the data from all the edit boxes.
//	- Writes the current settings to the  "Current" Registry subkey - 
//		saves info for next session.
//	- Also writes the current selection's settings to it's own folder.
//
// Input/Output data elements: None.
//
// Error handling: 
//	- If no profile name has been specified, shows an error dialog box.
//	- If no address has been specified, shows an error dialog box.
//
// Author:	Jim Dunne
//
// Date:	September 27, 1997
//
void CFtpConnect::OnClickedButtonOK() 
{	//Get a pointer to the combobox
	CComboBox *pcb = (CComboBox *)GetDlgItem(IDC_COMBO_CONNECT_PROFILE);
	//Get the data from the edit boxes
	UpdateData(TRUE);
	//if no profile name has been specified
	if (m_szProfile.GetLength() == NULL)
	{
		AfxMessageBox("Enter a Profile name first");
		return;
	}
	//If no address has been entered
	if (m_szHostName.GetLength() == NULL)
	{
		AfxMessageBox("Enter a Host Name or IP Address first");
		return;
	}

	//If no remote directory has been entered
	if (m_szRemInDir.GetLength() == NULL)
		m_szRemInDir = "/";

	//Write "Current" settings - save info for next session
	WriteSettings("Hosts\\Current");

	//Also write current selection's settings to it's folder
	wsprintf(m_lpszEntry, "Hosts\\%s", m_szProfile);
	WriteSettings(m_lpszEntry);
		
	CDialog::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// void CFtpConnect::OnSelchangeComboConnectHost() 
//
// Sample Call: Not called by the user.
//
// Description: Called when the user changes the selection in 
// the combobox - pulls up and displays info for that selection.
//	- Gets a pointer to the "Profile:" combobox.
//	- Gets the dropdown list index of the new selection and then it's text.
//	- Reads in all the settings from that profile's Registry folder.
//	- Writes the settings to their respective edit boxes.
//
// Input/Output data elements: None.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	September 27, 1997
//
void CFtpConnect::OnSelchangeComboConnectHost() 
{	// Get index of new selection and then it's text
	CComboBox *pcb = (CComboBox *)GetDlgItem(IDC_COMBO_CONNECT_PROFILE);
    int index = pcb->SendMessage(CB_GETCURSEL, (WORD)0, 0L);
    pcb->SendMessage(CB_GETLBTEXT, (WORD)index, (LONG)m_lpszEntry);
	//Read all the settings
	ReadSettings(m_lpszEntry);
	//Write the settings to their respective edit boxes
	ChngConnectDlg(csHost,IDC_EDIT_CONNECT_HOST);
	ChngConnectDlg(csUser,IDC_EDIT_CONNECT_USER);
	ChngConnectDlg(csPass,IDC_EDIT_CONNECT_PASSWORD);
	ChngConnectDlg(csRemInitDir,IDC_EDIT_CONNECT_REMDIR);
	ChngConnectDlg(csLocInitDir,IDC_EDIT_CONNECT_LOCDIR);
}

/////////////////////////////////////////////////////////////////////////////
// void CFtpConnect::ChngConnectDlg(CString csString, int i)
//
// Sample Call: ChngConnectDlg(csProfile,IDC_COMBO_CONNECT_PROFILE);
//
// Description: Puts information into the specified edit box.
//	- If csString is not NULL, writes the string to the box, 
//	else writes "" to the box.
//
// Input/Output data elements: 
//	- Input: Arguments CString csString, int i.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 27, 1997
//
void CFtpConnect::ChngConnectDlg(CString csString, int i)
{
			if(csString !=  "NULL")
				SetDlgItemText(i, csString);
			else
				SetDlgItemText(i, "");
}

/////////////////////////////////////////////////////////////////////////////
// void CFtpConnect::WriteSettings(LPSTR lpStr)
//
// Sample Call: WriteSettings("Hosts\\Current");
//
// Description: Writes information to the lpStr Registry key 
// using WriteProfileString().
//
// Input/Output data elements: 
//	- Input: Argument LPSTR lpStr.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 27, 1997
//
void CFtpConnect::WriteSettings(LPSTR lpStr)
{
	//Write current settings
	AfxGetApp()->WriteProfileString(lpStr, "Profile", m_szProfile);
	AfxGetApp()->WriteProfileString(lpStr, "Host Address", m_szHostName);
	AfxGetApp()->WriteProfileString(lpStr, "User", m_szUserName);
	AfxGetApp()->WriteProfileString(lpStr, "Pass", m_szPassword);
	AfxGetApp()->WriteProfileString(lpStr, "Remote Directory", m_szRemInDir);
	AfxGetApp()->WriteProfileString(lpStr, "Local Directory", m_szLocInDir);
}

/////////////////////////////////////////////////////////////////////////////
// void CFtpConnect::ReadSettings(LPSTR lpStr)
//
// Sample Call: ReadSettings("Current");
//
// Description: Puts information into the specified edit box.
//	- Formats a string with the path to the Registry subkey - 
//		lpStr is added to the end of the path.
//	- Reads settings from the subkey with GetProfileString().
//
// Input/Output data elements: 
//	- Input: Argument LPSTR lpStr.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 27, 1997
//
void CFtpConnect::ReadSettings(LPSTR lpStr)
{
	wsprintf(m_lpszMessage, "Hosts\\%s", lpStr);
	csProfile = AfxGetApp()->GetProfileString(m_lpszMessage,
		"Profile", "NULL");
	csHost = AfxGetApp()->GetProfileString(m_lpszMessage,
		"Host Address", "NULL");
	csUser = AfxGetApp()->GetProfileString(m_lpszMessage,
		"User", "NULL");
	csPass = AfxGetApp()->GetProfileString(m_lpszMessage,
		"Pass", "NULL");
	csRemInitDir = AfxGetApp()->GetProfileString(m_lpszMessage,
		"Remote Directory", "NULL");
	csLocInitDir = AfxGetApp()->GetProfileString(m_lpszMessage,
		"Local Directory", "NULL");
}

/////////////////////////////////////////////////////////////////////////////
// void CFtpConnect::ReadSubKey()
//
// Sample Call: ReadSubKey();
//
// Description: Reads Profile names from the folder names and puts 
// them into the combobox's dropdown list.
//	- Gets a pointer to the "Profile:" combobox.
//	- Opens the "Hosts" Registry subkey.
//	- Reads all the subkey names and adds them to the combobox's 
//		dropdown list, with the exception of "Current", which 
//		is not a profile name.
//
// Input/Output data elements: None.
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 27, 1997
//
void CFtpConnect::ReadSubKey()
{
	LONG retCode;
	char KeyName[50];
	int i;
	CComboBox *pcb = (CComboBox *)GetDlgItem(IDC_COMBO_CONNECT_PROFILE);
	HKEY key;

	if(::RegOpenKeyEx(HKEY_CURRENT_USER,
		"Software\\Top Jimmy Software\\TJFTP\\Hosts",
		0, KEY_READ, &key) == ERROR_SUCCESS)
	{
		for(i=0, retCode=ERROR_SUCCESS; retCode==ERROR_SUCCESS; i++)
		{
			if( (retCode=RegEnumKey(key, i, KeyName, 50))==ERROR_SUCCESS)
			{
				if(lstrcmp(KeyName,"Current") != 0)
				{
				pcb->AddString(KeyName);//Put entries into combo list
				}
			}
		}
			RegCloseKey(key);
	}
}

