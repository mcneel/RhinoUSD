#include "stdafx.h"
#include "ExportUSDApp.h"

// CExportUSDApp

BEGIN_MESSAGE_MAP(CExportUSDApp, CWinApp)
END_MESSAGE_MAP()

// The one and only CExportUSDApp object
static class CExportUSDApp theApp;

// CExportUSDApp initialization

BOOL CExportUSDApp::InitInstance()
{
	// CRITICAL: DO NOT CALL RHINO SDK FUNCTIONS HERE!
	CWinApp::InitInstance();
	return TRUE;
}

int CExportUSDApp::ExitInstance()
{
	// CRITICAL: DO NOT CALL RHINO SDK FUNCTIONS HERE!
	return CWinApp::ExitInstance();
}
