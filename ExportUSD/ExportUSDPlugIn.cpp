#include "StdAfx.h"
#include "rhinoSdkPlugInDeclare.h"
#include "ExportUSDPlugIn.h"
#include "Resource.h"

#pragma warning(push)
#pragma warning(disable : 4073)
#pragma init_seg(lib)
#pragma warning(pop)

RHINO_PLUG_IN_DECLARE
RHINO_PLUG_IN_NAME(L"ExportUSD");
RHINO_PLUG_IN_ID(L"36803D82-097C-4B55-8F42-E4D44764C9DD");
RHINO_PLUG_IN_VERSION(__DATE__ "  " __TIME__)
RHINO_PLUG_IN_DESCRIPTION(L"Export Pixar USD");
RHINO_PLUG_IN_ICON_RESOURCE_ID(IDI_ICON);

RHINO_PLUG_IN_DEVELOPER_ORGANIZATION(L"Robert McNeel & Associates");
RHINO_PLUG_IN_DEVELOPER_ADDRESS(L"146 North Canal Street, Suite 320\r\nSeattle, WA 98103");
RHINO_PLUG_IN_DEVELOPER_COUNTRY(L"United States");
RHINO_PLUG_IN_DEVELOPER_PHONE(L"(206) 545-7000");
RHINO_PLUG_IN_DEVELOPER_FAX(L"(206) 545-7321");
RHINO_PLUG_IN_DEVELOPER_EMAIL(L"tech@mcneel.com");
RHINO_PLUG_IN_DEVELOPER_WEBSITE(L"https://www.rhino3d.com/");
RHINO_PLUG_IN_UPDATE_URL(L"https://www.rhino3d.com/");

// The one and only CExportUSDPlugIn object
static class CExportUSDPlugIn thePlugIn;

// CExportUSDPlugIn definition

CExportUSDPlugIn& ExportUSDPlugIn()
{
	// Return a reference to the one and only CExportUSDPlugIn object
	return thePlugIn;
}

CExportUSDPlugIn::CExportUSDPlugIn()
{
	m_plugin_version = RhinoPlugInVersion();
}

// Required overrides

const wchar_t* CExportUSDPlugIn::PlugInName() const
{
	return RhinoPlugInName();
}

const wchar_t* CExportUSDPlugIn::PlugInVersion() const
{
	return m_plugin_version;
}

GUID CExportUSDPlugIn::PlugInID() const
{
	return ON_UuidFromString(RhinoPlugInId());
}

// Additional overrides

BOOL CExportUSDPlugIn::OnLoadPlugIn()
{
	return TRUE;
}

void CExportUSDPlugIn::OnUnloadPlugIn()
{
}

// File export overrides

void CExportUSDPlugIn::AddFileType(ON_ClassArray<CRhinoFileType>& extensions, const CRhinoFileWriteOptions& options)
{
	UNREFERENCED_PARAMETER(options);
	CRhinoFileType ft(PlugInID(), L"Pixar USD (*.usd)", L"usd");
	extensions.Append(ft);
}

BOOL CExportUSDPlugIn::WriteFile(const wchar_t* filename, int index, CRhinoDoc& doc, const CRhinoFileWriteOptions& options)
{
	UNREFERENCED_PARAMETER(filename);
	UNREFERENCED_PARAMETER(index);
	UNREFERENCED_PARAMETER(doc);
	UNREFERENCED_PARAMETER(options);

	// TODO: Add file export code here.

	return TRUE;
}
