#pragma once

class CExportUSDPlugIn : public CRhinoFileExportPlugIn
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

private:
  ON_wString m_plugin_version;

public:
  int m_saved_mesh_ui_style = 0;
  ON_MeshParameters m_saved_mp;
};

int WriteUSDFile(const wchar_t* filename, bool usda, CRhinoDoc& doc, const CRhinoFileWriteOptions& options);
