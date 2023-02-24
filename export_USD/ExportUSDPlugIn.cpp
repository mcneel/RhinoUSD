#include "stdafx.h"
#include "rhinoSdkPlugInDeclare.h"
#include "ExportUSDPlugIn.h"
#include "Resource.h"
using namespace pxrInternal_v0_23__pxrReserved__;

#pragma warning(push)
#pragma warning(disable : 4073)
#pragma init_seg(lib)
#pragma warning(pop)

// Rhino plug-in declaration
RHINO_PLUG_IN_DECLARE

// Rhino plug-in name
// Provide a short, friendly name for this plug-in.
RHINO_PLUG_IN_NAME(L"Export USD");
RHINO_PLUG_IN_ID(L"36803D82-097C-4B55-8F42-E4D44764C9DD");
RHINO_PLUG_IN_VERSION(__DATE__ "  " __TIME__)
RHINO_PLUG_IN_DESCRIPTION(L"Export USD and USDA file formats");

// Rhino plug-in icon resource id
// Provide an icon resource this plug-in.
// Icon resource should contain 16, 24, 32, 48, and 256-pixel image sizes.
RHINO_PLUG_IN_ICON_RESOURCE_ID(IDI_ICON);

RHINO_PLUG_IN_DEVELOPER_ORGANIZATION(L"Robert McNeel & Associates");
RHINO_PLUG_IN_DEVELOPER_ADDRESS(L"3670 Woodland Park Avenue North\015\012Seattle WA 98103");
RHINO_PLUG_IN_DEVELOPER_COUNTRY(L"United States");
RHINO_PLUG_IN_DEVELOPER_PHONE(L"206-545-6877");
RHINO_PLUG_IN_DEVELOPER_FAX(L"206-545-7321");
RHINO_PLUG_IN_DEVELOPER_EMAIL(L"tech@mcneel.com");
RHINO_PLUG_IN_DEVELOPER_WEBSITE(L"http://www.rhino3d.com");
RHINO_PLUG_IN_UPDATE_URL( L"http://www2.rhino3d.com/sr/plugin.asp?id=plugin-uuid-goes-here" );

// The one and only CExportUSDPlugIn object
static class CExportUSDPlugIn thePlugIn;

CExportUSDPlugIn::CExportUSDPlugIn()
{
	m_plugin_version = RhinoPlugInVersion();
}

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
	// Description:
	//   Plug-in unique identifier. The identifier is used by Rhino to
	//   manage the plug-ins.
	return ON_UuidFromString(RhinoPlugInId());
}

void CExportUSDPlugIn::AddFileType(ON_ClassArray<CRhinoFileType>& extensions, const CRhinoFileWriteOptions& options)
{
	CRhinoFileType ft;
	ft.SetFileTypePlugInID(PlugInID());
	ft.FileTypeDescription(L"USD (*.usd, *.usda)");
  ft.AddFileTypeExtension(L"usd");
  ft.AddFileTypeExtension(L"usda");
  extensions.Append(ft);
}

CExportUSDPlugIn& CExportUSDPlugIn::ThePlugin()
{
  return thePlugIn;
}

int CExportUSDPlugIn::WriteFile(const wchar_t* filename, int index, CRhinoDoc& doc, const CRhinoFileWriteOptions& options)
{
  return WriteUSDFile(filename, 1 == index, doc, options);
}
