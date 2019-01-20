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
//*	topjimmy@topjimmy.net									*
//* www.topjimmy.net/tjs									*
//***********************************************************
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "TJFTP.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_INDICATOR_STATUS,
	ID_INDICATOR_RECEIVE,
	ID_INDICATOR_UTILITY,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

/////////////////////////////////////////////////////////////////////////////
// int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
//
// Sample Call: Not called by the user.
//
// Description: Called when the application frame is created.  
// Specifies toolbars, status bars, window size, etc.
//	- Creates the status bar and sets the size of the 
//		panes with SetPaneInfo().
//
// Input/Output data elements:
//	- Input: Argument LPCREATESTRUCT lpCreateStruct.
//	- Output: Returns an integer value.
//
// Error handling: 
//	- If CFrameWnd::OnCreate() fails, returns -1.
//
// Author:	Jim Dunne
//
// Date:	Sept 27, 1997
//
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetPaneInfo(0,ID_INDICATOR_STATUS, SBPS_NORMAL, 250);
	m_wndStatusBar.SetPaneInfo(1,ID_INDICATOR_RECEIVE, SBPS_NORMAL, 175);
	m_wndStatusBar.SetPaneInfo(2,ID_INDICATOR_UTILITY, SBPS_NORMAL, 125);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
//
// Sample Call: Not called by the user.
//
// Description: Called before the application frame is created. 
//	- Sets the application frame to 1/3 the screen size and centers it.
//	- Specifies the frame style by setting cs.style values.
//
// Input/Output data elements:
//	- Input: Argument CREATESTRUCT& cs.
//	- Output: Boolean, returns  CFrameWnd::PreCreateWindow(cs).
//
// Error handling: None.
//
// Author:	Jim Dunne
//
// Date:	Sept 27, 1997
//
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
     // Size the window to 1/3 screen size and center it 
	cs.cy = ::GetSystemMetrics(SM_CYSCREEN)/1.279; 
	cs.cx = ::GetSystemMetrics(SM_CXSCREEN)/1.271; 
    cs.y = 0; 
    cs.x = 0;

	cs.style = WS_OVERLAPPED | WS_THICKFRAME | WS_SYSMENU
		| WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE;

	return CFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
