#include "StdAfx.h"
#include "ImportUSDPlugIn.h"
#include "Resource.h"

#pragma warning(push)
#pragma warning(disable : 4073)
#pragma init_seg(lib)
#pragma warning(pop)

// rhinoSdkPlugInDeclare.h defines the RHINO_PLUG_IN_DECLARE macro
#include "../../../SDK/inc/rhinoSdkPlugInDeclare.h"

// Rhino plug-in declarations
RHINO_PLUG_IN_DECLARE
RHINO_PLUG_IN_NAME(L"Import Pixar USD");
RHINO_PLUG_IN_ID(L"7C5D9A51-4AF7-4CE1-8661-30FE69473C6E");
RHINO_PLUG_IN_VERSION(RHINO_VERSION_NUMBER_STRING)
RHINO_PLUG_IN_UPDATE_URL(L"http://www.rhino3d.com/download");

// The one and only CImportUSDPlugIn object
static class CImportUSDPlugIn thePlugIn;

// CImportUSDPlugIn definition

const wchar_t* CImportUSDPlugIn::PlugInName() const
{
	return RhinoPlugInName();
}

const wchar_t* CImportUSDPlugIn::LocalPlugInName() const
{
  return RhLocalizeString( L"Import Pixar USD", 55139);
}

const wchar_t* CImportUSDPlugIn::PlugInVersion() const
{
  static wchar_t plugin_version[128] = { 0 };
  if (0 == plugin_version[0])
    ON_ConvertUTF8ToWideChar(false, RhinoPlugInVersion(), 0, plugin_version, sizeof(plugin_version) / sizeof(plugin_version[0]) - 1, 0, 0, 0, 0);
  return plugin_version;
}

GUID CImportUSDPlugIn::PlugInID() const
{
  static GUID plugin_id = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
  if (0 == plugin_id.Data1 && ON_UuidIsNil(plugin_id))
    plugin_id = ON_UuidFromString(RhinoPlugInId());
  return plugin_id;
}

BOOL CImportUSDPlugIn::OnLoadPlugIn()
{
	return TRUE;
}

void CImportUSDPlugIn::OnUnloadPlugIn()
{
}

void CImportUSDPlugIn::AddFileType(ON_ClassArray<CRhinoFileType>& extensions, const CRhinoFileReadOptions& options)
{
  CRhinoFileType ft;
  ft.SetFileTypePlugInID(PlugInID());
  ft.FileTypeDescription(RhLocalizeString( L"Pixar USD (*.usd)", 55140));
  ft.AddFileTypeExtension(RHSTR_LIT(L"usd"));
  extensions.Append(ft);
}

BOOL32 CImportUSDPlugIn::ReadFile(const wchar_t* filename, int index, CRhinoDoc& doc, const CRhinoFileReadOptions& options)
{
  if (nullptr == filename || 0 == filename[0] || !CRhinoFileUtilities::FileExists(filename))
    return FALSE;

	// TODO: Add file import code here.

	return TRUE;
}
