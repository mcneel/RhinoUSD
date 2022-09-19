#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h" // main symbols

class CImportUSDApp : public CWinApp
{
public:
	CImportUSDApp() = default;
	BOOL InitInstance() override;
	int ExitInstance() override;
	DECLARE_MESSAGE_MAP()
};
