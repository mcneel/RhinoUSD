#include "stdafx.h"
#if defined( CORE_BUILD )
#include "../../../SDK/inc/rhinoSdkPlugInDeclare.h"
#else
#include "rhinoSdkPlugInDeclare.h"
#endif

#include "ExportUSDPlugIn.h"
#include "Resource.h""
#include "UsdExportOptions.h"
#include "write_usd.h"

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

RHINO_PLUG_IN_UPDATE_URL( L"http://www2.rhino3d.com/sr/plugin.asp?id=36803D82-097C-4B55-8F42-E4D44764C9DD" );

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
  ft.FileTypeDescription(L"USD (*.usdz, *.usda, *.usd)");
  ft.AddFileTypeExtension(L"usdz");
  ft.AddFileTypeExtension(L"usda");
  ft.AddFileTypeExtension(L"usd");
  ft.SetDisplayOptionsDialog(true);

  extensions.Append(ft);
}

UsdExportOptions Options;

void CExportUSDPlugIn::LoadProfile(LPCTSTR lpszSection, CRhinoProfileContext& pc)
{
   int blocksValue = (int)Options.DefaultBlocks;
  ON_wString defaultLayerValue = Options.DefaultDefaultLayer;
  ON_wString modelNameValue = Options.DefaultModelName;
  bool forceMeshesValue = Options.DefaultForceMeshes;
  bool includeUserStringsValue = Options.DefaultIncludeUserStrings;

  if (pc.LoadProfileInt(lpszSection, L"blocks", &blocksValue, (int)Options.DefaultBlocks))
    Options.Blocks = (BlockHandling)(blocksValue);

  if (pc.LoadProfileString(lpszSection, L"default-layer", defaultLayerValue, Options.DefaultDefaultLayer))
    Options.DefaultLayer = defaultLayerValue;

  if (pc.LoadProfileString(lpszSection, L"model-name", modelNameValue, Options.DefaultModelName))
    Options.ModelName = modelNameValue;

  if (pc.LoadProfileBool(lpszSection, L"force-meshes", &forceMeshesValue, Options.DefaultForceMeshes))
    Options.ForceMeshes = forceMeshesValue;

  if (pc.LoadProfileBool(lpszSection, L"include-user-strings", &includeUserStringsValue, Options.DefaultIncludeUserStrings))
    Options.IncludeUserStrings = includeUserStringsValue;
}

void CExportUSDPlugIn::SaveProfile(LPCTSTR lpszSection, CRhinoProfileContext& pc)
{
  pc.SaveProfileString(lpszSection, L"model-name", Options.ModelName);
  pc.SaveProfileString(lpszSection, L"default-layer", Options.DefaultLayer);
  pc.SaveProfileInt(lpszSection, L"blocks", (int)Options.Blocks);
  pc.SaveProfileBool(lpszSection, L"force-meshes", Options.ForceMeshes);
  pc.SaveProfileBool(lpszSection, L"user-strings", Options.IncludeUserStrings);
}

void CExportUSDPlugIn::DisplayOptionsDialog(HWND parent, const CRhinoFileType& fileType)
{
  CRhParameterDictionary args;
  args.SetUuid(L"plugin-id", PlugInID());
  args.SetWindowHandle(L"hwnd", parent);

  bool scripting = RhinoApp().IsHeadless();
  HandleUserInput(scripting, args, Options);
}

CExportUSDPlugIn& CExportUSDPlugIn::ThePlugin()
{
  return thePlugIn;
}

int CExportUSDPlugIn::WriteFile(const wchar_t* filename, int index, CRhinoDoc& doc, const CRhinoFileWriteOptions& options)
{
  bool scripting = RhinoApp().IsHeadless() || options.UseBatchMode();
  int mesh_ui_style = CExportUSDPlugIn::ThePlugin().m_saved_mesh_ui_style;

  auto opts = options.OptionsDictionary();
  bool useOptionsDictionary = options.OptionsDictionary().Count() > 0;

  // Scripting
  if (useOptionsDictionary)
  {
    mesh_ui_style = 4;
    PushFileWriteOptionsToUsdOptions(options);
  }
  else if (scripting)
  {
    mesh_ui_style = 4;

    // Headed/headless/batch mode
    CRhParameterDictionary args;
    args.SetUuid(L"plugin-id", PlugInID());
    args.SetInt(L"doc", doc.RuntimeSerialNumber());
    args.SetBool(L"scripting", scripting);

    HandleUserInput(scripting, args, Options);
  }

  return WriteUSDFile(filename, 1 == index, doc, options, scripting, Options, mesh_ui_style);
}

void CExportUSDPlugIn::PushFileWriteOptionsToUsdOptions(const CRhinoFileWriteOptions& options)
{
  const ON_ArchivableDictionary dictionary = options.OptionsDictionary();

  int blocksValue;
  ON_wString defaultLayerValue;
  ON_wString modelNameValue;
  bool forceMeshesValue;
  bool includeUserStringsValue;

  if (dictionary.TryGetInt32(L"blocks", blocksValue))
    Options.Blocks = (BlockHandling)blocksValue;

  if (dictionary.TryGetString(L"default-layer", defaultLayerValue))
    Options.DefaultLayer = defaultLayerValue;

  if (dictionary.TryGetString(L"model-name", modelNameValue))
    Options.ModelName = modelNameValue;

  if (dictionary.TryGetBool(L"force-meshes", forceMeshesValue))
    Options.ForceMeshes = forceMeshesValue;

  if (dictionary.TryGetBool(L"include-user-strings", includeUserStringsValue))
    Options.IncludeUserStrings = includeUserStringsValue;

}
