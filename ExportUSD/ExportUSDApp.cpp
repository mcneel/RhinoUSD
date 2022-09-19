#include "stdafx.h"
#include "ExportUSDApp.h"

// CExportUSDApp

BEGIN_MESSAGE_MAP(CExportUSDApp, CWinApp)
END_MESSAGE_MAP()

// The one and only CExportUSDApp object
CExportUSDApp theApp;

// CExportUSDApp initialization

BOOL CExportUSDApp::InitInstance()
{
	CWinApp::InitInstance();
	return TRUE;
}

int CExportUSDApp::ExitInstance()
{
  return CWinApp::ExitInstance();
}
