#include "StdAfx.h"
#include "rhinoSdkPlugInDeclare.h"
#include "ImportUSDPlugIn.h"
#include "Resource.h"

#pragma warning(push)
#pragma warning(disable : 4073)
#pragma init_seg(lib)
#pragma warning(pop)

RHINO_PLUG_IN_DECLARE
RHINO_PLUG_IN_NAME(L"ImportUSD");
RHINO_PLUG_IN_ID(L"7C5D9A51-4AF7-4CE1-8661-30FE69473C6E");
RHINO_PLUG_IN_VERSION(__DATE__ "  " __TIME__)
RHINO_PLUG_IN_DESCRIPTION(L"Import Pixar USD");
RHINO_PLUG_IN_ICON_RESOURCE_ID(IDI_ICON);

RHINO_PLUG_IN_DEVELOPER_ORGANIZATION(L"Robert McNeel & Associates");
RHINO_PLUG_IN_DEVELOPER_ADDRESS(L"146 North Canal Street, Suite 320\r\nSeattle, WA 98103");
RHINO_PLUG_IN_DEVELOPER_COUNTRY(L"United States");
RHINO_PLUG_IN_DEVELOPER_PHONE(L"(206) 545-7000");
RHINO_PLUG_IN_DEVELOPER_FAX(L"(206) 545-7321");
RHINO_PLUG_IN_DEVELOPER_EMAIL(L"tech@mcneel.com");
RHINO_PLUG_IN_DEVELOPER_WEBSITE(L"https://www.rhino3d.com/");
RHINO_PLUG_IN_UPDATE_URL(L"https://www.rhino3d.com/");

// The one and only CImportUSDPlugIn object
static class CImportUSDPlugIn thePlugIn;

// CImportUSDPlugIn definition

CImportUSDPlugIn& ImportUSDPlugIn()
{
	return thePlugIn;
}

CImportUSDPlugIn::CImportUSDPlugIn()
{
	m_plugin_version = RhinoPlugInVersion();
}

// Required overrides

const wchar_t* CImportUSDPlugIn::PlugInName() const
{
	return RhinoPlugInName();
}

const wchar_t* CImportUSDPlugIn::PlugInVersion() const
{
	return m_plugin_version;
}

GUID CImportUSDPlugIn::PlugInID() const
{
	return ON_UuidFromString(RhinoPlugInId());
}

// Additional overrides

BOOL CImportUSDPlugIn::OnLoadPlugIn()
{
	return TRUE;
}

void CImportUSDPlugIn::OnUnloadPlugIn()
{
}

// File import overrides

void CImportUSDPlugIn::AddFileType(ON_ClassArray<CRhinoFileType>& extensions, const CRhinoFileReadOptions& options)
{
	UNREFERENCED_PARAMETER(options);
	CRhinoFileType ft(PlugInID(), L"Pixar USD (*.usd)", L"usd");
	extensions.Append(ft);
}

BOOL CImportUSDPlugIn::ReadFile(const wchar_t* filename, int index, CRhinoDoc& doc, const CRhinoFileReadOptions& options)
{
	UNREFERENCED_PARAMETER(filename);
	UNREFERENCED_PARAMETER(index);
	UNREFERENCED_PARAMETER(doc);
	UNREFERENCED_PARAMETER(options);

	// TODO: Add file import code here.

	return TRUE;
}
