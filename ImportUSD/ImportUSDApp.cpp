#include "stdafx.h"
#include "ImportUSDApp.h"

// CImportUSDApp

BEGIN_MESSAGE_MAP(CImportUSDApp, CWinApp)
END_MESSAGE_MAP()

// The one and only CImportUSDApp object
CImportUSDApp theApp;

// CImportUSDApp initialization

BOOL CImportUSDApp::InitInstance()
{
	CWinApp::InitInstance();
	return TRUE;
}

int CImportUSDApp::ExitInstance()
{
  return CWinApp::ExitInstance();
}
