#include "stdafx.h"
#if defined( CORE_BUILD )
#include "../../../SDK/inc/rhinoSdkPlugInDeclare.h"
#else
#include "rhinoSdkPlugInDeclare.h"
#endif

#include "ExportUSDPlugIn.h"
#include "Resource.h"

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
	ft.FileTypeDescription(L"USD (*.usdc, *.usda, *.usd, *.usdz)");
  ft.AddFileTypeExtension(L"usdc");
  ft.AddFileTypeExtension(L"usda");
  ft.AddFileTypeExtension(L"usd");
  ft.AddFileTypeExtension(L"usdz");
  ft.SetDisplayOptionsDialog(true);

  extensions.Append(ft);
}

CExportUSDPlugIn& CExportUSDPlugIn::ThePlugin()
{
  return thePlugIn;
}

int CExportUSDPlugIn::WriteFile(const wchar_t* filename,
                                int index,
                                CRhinoDoc& doc,
                                const CRhinoFileWriteOptions& options)
{
	ExportOptions.Headless = RhinoApp().IsHeadless() || options.UseBatchMode();

  int mesh_ui_style = CExportUSDPlugIn::ThePlugin().m_saved_mesh_ui_style;
  
  // user has input some options via the AIP
  bool useOptionsDictionary = options.OptionsDictionary().Count() > 0;
  if (useOptionsDictionary)
  {
    mesh_ui_style = 4;
    PushFileWriteOptionsToUsdOptions(options);
		ExportOptions.Headless = true;
  }
  else if (ExportOptions.Headless)
  {
    mesh_ui_style = 4;
		HandleUserInput(ExportOptions);
  }

  ON_ClassArray<std::shared_ptr<UsdPacket>> packets;
  if (GetPackets(doc, options, ExportOptions, packets) <= 0) return -1;
  
  int result = WriteUSDFile(filename, doc, packets, ExportOptions);
  
  SaveFiles();
  
  return result;
}

bool CExportUSDPlugIn::SaveFiles()
{
  if (UsdExportImport::Exported.Count() <= 0) return false;
  
  UsdFilePathPair baseFilePair = UsdExportImport::Exported[0];
  const ON_wString extension = ON_FileSystemPath::FileNameExtensionFromPath(baseFilePair.Real);
  if (extension.EqualOrdinal(L".usdz", true))
  {
    pxr::UsdZipFileWriter writer = pxr::UsdZipFileWriter::CreateNew(ON_Helpers::ON_wString_to_StdString(baseFilePair.Real));
    
    for (UsdFilePathPair filePair : UsdExportImport::Exported)
    {
      ON_wString tempFilePath = filePair.Temporary;
      ON_wString fileName = ON_FileSystemPath::FileNameFromPath(tempFilePath, true);
      writer.AddFile(ON_Helpers::ON_wString_to_StdString(tempFilePath), ON_Helpers::ON_wString_to_StdString(fileName));
      ON_FileSystem::RemoveFile(filePair.Temporary);
    }
    
    for (ON_wString materialFilePath : UsdExportImport::FilesInExport)
    {
      ON_wString materialFileName = ON_FileSystemPath::FileNameFromPath(materialFilePath, true);
      UsdShared::GetValidMaterialName(materialFileName);
      std::string result = writer.AddFile(ON_Helpers::ON_wString_to_StdString(materialFilePath),
                                          ON_Helpers::ON_wString_to_StdString(materialFileName));
    }
    
    writer.Save();
    
    UsdExportImport::Exported.Empty();
    
    return true;
  }
  else if (extension.EqualOrdinal(L".usd", true) ||
           extension.EqualOrdinal(L".usda", true) ||
           extension.EqualOrdinal(L".usdc", true))
  {
    for (UsdFilePathPair filePair : UsdExportImport::Exported)
    {
      CRhinoFileUtilities::MoveFile(filePair.Temporary, filePair.Real);
    }
    
    for (ON_wString originalMaterialFilePath : UsdExportImport::FilesInExport)
    {
      ON_wString exportDir = ON_FileSystemPath::DirectoryFromPath(baseFilePair.Real);
      ON_wString materialFileName = ON_FileSystemPath::FileNameFromPath(originalMaterialFilePath, true);
      UsdShared::GetValidMaterialName(materialFileName);
      ON_wString materialNewPath = ON_FileSystemPath::CombinePaths(exportDir, false, materialFileName, true, false);
      
      CRhinoFileUtilities::CopyFile(originalMaterialFilePath, materialNewPath, false);
    }
    
    UsdExportImport::Exported.Empty();
    
    return true;
  }
  
  UsdExportImport::Exported.Empty();
  
  return false;
}

void CExportUSDPlugIn::LoadProfile(LPCTSTR lpszSection, CRhinoProfileContext& pc)
{
  int blocksValue = (int)ExportOptions.DefaultBlocks;
  ON_wString RootLayerValue = ExportOptions.DefaultRootLayer;
  ON_wString modelNameValue = ExportOptions.DefaultModelName;
  bool forceMeshesValue = ExportOptions.DefaultForceMeshes;
  bool includeUserStringsValue = ExportOptions.DefaultIncludeUserStrings;

  if (pc.LoadProfileInt(lpszSection, L"blocks", &blocksValue, (int)ExportOptions.DefaultBlocks))
    ExportOptions.Blocks = (BlockHandling)(blocksValue);

  if (pc.LoadProfileString(lpszSection, L"root-layer", RootLayerValue, ExportOptions.DefaultRootLayer))
    ExportOptions.RootLayer = RootLayerValue;

  if (pc.LoadProfileString(lpszSection, L"model-name", modelNameValue, ExportOptions.DefaultModelName))
    ExportOptions.ModelName = modelNameValue;

  if (pc.LoadProfileBool(lpszSection, L"force-meshes", &forceMeshesValue, ExportOptions.DefaultForceMeshes))
    ExportOptions.ForceMeshes = forceMeshesValue;

  if (pc.LoadProfileBool(lpszSection, L"include-user-strings", &includeUserStringsValue, ExportOptions.DefaultIncludeUserStrings))
    ExportOptions.IncludeUserStrings = includeUserStringsValue;
}

void CExportUSDPlugIn::SaveProfile(LPCTSTR lpszSection, CRhinoProfileContext& pc)
{
  pc.SaveProfileString(lpszSection, L"model-name", ExportOptions.ModelName);
  pc.SaveProfileString(lpszSection, L"root-layer", ExportOptions.RootLayer);
  pc.SaveProfileInt(lpszSection, L"blocks", (int)ExportOptions.Blocks);
  pc.SaveProfileBool(lpszSection, L"force-meshes", ExportOptions.ForceMeshes);
  pc.SaveProfileBool(lpszSection, L"user-strings", ExportOptions.IncludeUserStrings);
}

void CExportUSDPlugIn::DisplayOptionsDialog(HWND parent, const CRhinoFileType& fileType)
{
	// This only runs when a user "clicks" options, and is therefore not headless
	ExportOptions.Headless = false;
  HandleUserInput(ExportOptions);
}

void CExportUSDPlugIn::PushFileWriteOptionsToUsdOptions(const CRhinoFileWriteOptions & fileWriteOptions)
{
  const ON_ArchivableDictionary dictionary = fileWriteOptions.OptionsDictionary();

  ON_MeshParameters mp = CExportUSDPlugIn::ThePlugin().m_saved_mp;
  if (dictionary.TryGetMeshParameters(L"MeshingParameters", mp))
  {
    ExportOptions.MeshingParams = mp;
  }

  int blocksValue;
  ON_wString rootLayerValue;
  ON_wString modelNameValue;
  bool forceMeshesValue;
  bool includeUserStringsValue;

  // Misc Export Settings
  if (dictionary.TryGetInt32(L"blocks", blocksValue))
    ExportOptions.Blocks = (BlockHandling)blocksValue;

  if (dictionary.TryGetString(L"root-layer", rootLayerValue))
    ExportOptions.RootLayer = rootLayerValue;

  if (dictionary.TryGetString(L"model-name", modelNameValue))
    ExportOptions.ModelName = modelNameValue;

  if (dictionary.TryGetBool(L"force-meshes", forceMeshesValue))
    ExportOptions.ForceMeshes = forceMeshesValue;

  if (dictionary.TryGetBool(L"include-user-strings", includeUserStringsValue))
    ExportOptions.IncludeUserStrings = includeUserStringsValue;
}
