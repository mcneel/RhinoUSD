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
  // CRITICAL: DO NOT CALL RHINO SDK FUNCTIONS HERE!
  CWinApp::InitInstance();
	return TRUE;
}

int CImportUSDApp::ExitInstance()
{
  // CRITICAL: DO NOT CALL RHINO SDK FUNCTIONS HERE!
  return CWinApp::ExitInstance();
}
