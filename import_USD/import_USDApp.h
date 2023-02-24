// import_USD.h : main header file for the import_USD DLL.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h" // main symbols

// Cimport_USDApp
// See import_USDApp.cpp for the implementation of this class
//

class Cimport_USDApp : public CWinApp
{
public:
  // CRITICAL: DO NOT CALL RHINO SDK FUNCTIONS HERE!
  // Only standard MFC DLL instance construction belongs here. 
  // All other significant initialization should take place in
  // Cimport_USDPlugIn::OnLoadPlugIn().
	Cimport_USDApp() = default;

  // CRITICAL: DO NOT CALL RHINO SDK FUNCTIONS HERE!
  // Only standard MFC DLL instance initialization belongs here. 
  // All other significant initialization should take place in
  // Cimport_USDPlugIn::OnLoadPlugIn().
	BOOL InitInstance() override;
  
  // CRITICAL: DO NOT CALL RHINO SDK FUNCTIONS HERE!
  // Only standard MFC DLL instance clean up belongs here. 
  // All other significant cleanup should take place in either
  // Cimport_USDPlugIn::OnSaveAllSettings() or
  // Cimport_USDPlugIn::OnUnloadPlugIn().  
	int ExitInstance() override;
  
	DECLARE_MESSAGE_MAP()
};
