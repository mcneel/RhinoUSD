#include "StdAfx.h"
#include "ExportUSDPlugIn.h"
#include "Resource.h"

#pragma warning(push)
#pragma warning(disable : 4073)
#pragma init_seg(lib)
#pragma warning(pop)

// rhinoSdkPlugInDeclare.h defines the RHINO_PLUG_IN_DECLARE macro
#include "../../../SDK/inc/rhinoSdkPlugInDeclare.h"

// Rhino plug-in declarations
RHINO_PLUG_IN_DECLARE
RHINO_PLUG_IN_NAME(L"Export Pixar USD");
RHINO_PLUG_IN_ID(L"36803D82-097C-4B55-8F42-E4D44764C9DD");
RHINO_PLUG_IN_VERSION(RHINO_VERSION_NUMBER_STRING)
RHINO_PLUG_IN_UPDATE_URL(L"http://www.rhino3d.com/download");

// The one and only CExportUSDPlugIn object
static class CExportUSDPlugIn thePlugIn;

const wchar_t* CExportUSDPlugIn::PlugInName() const
{
	return RhinoPlugInName();
}

const wchar_t* CExportUSDPlugIn::LocalPlugInName() const
{
	return RHSTR(L"Export Pixar USD");
}

const wchar_t* CExportUSDPlugIn::PlugInVersion() const
{
	static wchar_t plugin_version[128] = { 0 };
	if (0 == plugin_version[0])
		ON_ConvertUTF8ToWideChar(false, RhinoPlugInVersion(), 0, plugin_version, sizeof(plugin_version) / sizeof(plugin_version[0]) - 1, 0, 0, 0, 0);
	return plugin_version;
}

GUID CExportUSDPlugIn::PlugInID() const
{
	static GUID plugin_id = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
	if (0 == plugin_id.Data1 && ON_UuidIsNil(plugin_id))
		plugin_id = ON_UuidFromString(RhinoPlugInId());
	return plugin_id;
}

BOOL CExportUSDPlugIn::OnLoadPlugIn()
{
	return TRUE;
}

void CExportUSDPlugIn::OnUnloadPlugIn()
{
}

void CExportUSDPlugIn::AddFileType(ON_ClassArray<CRhinoFileType>& extensions, const CRhinoFileWriteOptions& options)
{
	CRhinoFileType ft;
	ft.SetFileTypePlugInID(PlugInID());
	ft.FileTypeDescription(RHSTR(L"Pixar USD (*.usd)"));
	ft.AddFileTypeExtension(RHSTR_LIT(L"usd"));
	extensions.Append(ft);
}

int CExportUSDPlugIn::WriteFile(const wchar_t* filename, int index, CRhinoDoc& doc, const CRhinoFileWriteOptions& options)
{
	if (nullptr == filename || 0 == filename[0])
		return FALSE;

	// TODO

	return TRUE;
}
