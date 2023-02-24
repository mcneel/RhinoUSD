#include "stdafx.h"
#include "rhinoSdkPlugInDeclare.h"
#include "import_USDPlugIn.h"
#include "Resource.h"

// The plug-in object must be constructed before any plug-in classes derived
// from CRhinoCommand. The #pragma init_seg(lib) ensures that this happens.
#pragma warning(push)
#pragma warning(disable : 4073)
#pragma init_seg(lib)
#pragma warning(pop)

// Rhino plug-in declaration
RHINO_PLUG_IN_DECLARE

// Rhino plug-in name
// Provide a short, friendly name for this plug-in.
RHINO_PLUG_IN_NAME(L"Import USD");
RHINO_PLUG_IN_ID(L"7C5D9A51-4AF7-4CE1-8661-30FE69473C6E");
RHINO_PLUG_IN_VERSION(__DATE__ "  " __TIME__)
RHINO_PLUG_IN_DESCRIPTION(L"Import USD and USDA file formats");

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
RHINO_PLUG_IN_UPDATE_URL(L"http://www2.rhino3d.com/sr/plugin.asp?id=7C5D9A51-4AF7-4CE1-8661-30FE69473C6E");

// The one and only Cimport_USDPlugIn object
static class Cimport_USDPlugIn thePlugIn;

Cimport_USDPlugIn::Cimport_USDPlugIn()
{
	m_plugin_version = RhinoPlugInVersion();
}

const wchar_t* Cimport_USDPlugIn::PlugInName() const
{
	return RhinoPlugInName();
}

const wchar_t* Cimport_USDPlugIn::PlugInVersion() const
{
	return m_plugin_version;
}

GUID Cimport_USDPlugIn::PlugInID() const
{
	// Description:
	//   Plug-in unique identifier. The identifier is used by Rhino to
	//   manage the plug-ins.
	return ON_UuidFromString(RhinoPlugInId());
}

void Cimport_USDPlugIn::AddFileType(ON_ClassArray<CRhinoFileType>& extensions, const CRhinoFileReadOptions& options)
{
  CRhinoFileType ft;
  ft.SetFileTypePlugInID(PlugInID());
  ft.FileTypeDescription(L"USD (*.usd, *.usda)");
  ft.AddFileTypeExtension(L"usd");
  ft.AddFileTypeExtension(L"usda");
  extensions.Append(ft);
}

BOOL Cimport_USDPlugIn::ReadFile(const wchar_t* filename, int index, CRhinoDoc& doc, const CRhinoFileReadOptions& options)
{
  UNREFERENCED_PARAMETER(index);
  if (ReadUSDFile(filename, doc, options))
    return TRUE;
	return FALSE;
}
