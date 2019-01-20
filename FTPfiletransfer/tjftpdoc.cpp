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
// TJFTPDoc.cpp : implementation of the CTJFTPDoc class
//

#include "stdafx.h"
#include "TJFTP.h"

#include "TJFTPDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTJFTPDoc

IMPLEMENT_DYNCREATE(CTJFTPDoc, CDocument)

BEGIN_MESSAGE_MAP(CTJFTPDoc, CDocument)
	//{{AFX_MSG_MAP(CTJFTPDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_SEND_MAIL, OnFileSendMail)
	ON_UPDATE_COMMAND_UI(ID_FILE_SEND_MAIL, OnUpdateFileSendMail)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTJFTPDoc construction/destruction

CTJFTPDoc::CTJFTPDoc()
{
	// TODO: add one-time construction code here

}

CTJFTPDoc::~CTJFTPDoc()
{
}

BOOL CTJFTPDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CTJFTPDoc serialization

void CTJFTPDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTJFTPDoc diagnostics

#ifdef _DEBUG
void CTJFTPDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTJFTPDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTJFTPDoc commands
