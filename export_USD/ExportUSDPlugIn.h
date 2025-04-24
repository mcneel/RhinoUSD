#pragma once
#include "stdafx.h"
#include "UsdExportOptions.h"

class CExportUSDPlugIn : public CRhinoFileExportSystemPlugIn
{
public:
  static CExportUSDPlugIn& ThePlugin();
public:
  CExportUSDPlugIn();
  ~CExportUSDPlugIn() = default;

  // Required overrides
  const wchar_t* PlugInName() const override;
  const wchar_t* PlugInVersion() const override;
  GUID PlugInID() const override;
  
  // File export plug-in overrides
  void AddFileType(ON_ClassArray<CRhinoFileType>& extensions, const CRhinoFileWriteOptions& options) override;
  int WriteFile(const wchar_t* filename, int index, CRhinoDoc& doc, const CRhinoFileWriteOptions& options) override;

  void LoadProfile(LPCTSTR lpszSection, CRhinoProfileContext& pc) override;
  void SaveProfile(LPCTSTR lpszSection, CRhinoProfileContext& pc) override;

  void DisplayOptionsDialog(HWND parent, const CRhinoFileType& fileType) override;

  void PushFileWriteOptionsToUsdOptions(const CRhinoFileWriteOptions& options);

private:
  ON_wString m_plugin_version;

public:
  int m_saved_mesh_ui_style = 0;
  ON_MeshParameters m_saved_mp;
};

int WriteUSDFile(const wchar_t* filename,
                  bool usda,
                  CRhinoDoc& doc,
                  const CRhinoFileWriteOptions& options,
                  bool scripting,
                  UsdExportOptions& usdOptions);

static void GetMeshParametersFromDictionary(const ON_ArchivableDictionary& dict, ON_MeshParameters& params);
