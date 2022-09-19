#pragma once

class CExportUSDPlugIn : public CRhinoFileExportPlugIn
{
public:
  CExportUSDPlugIn();
  ~CExportUSDPlugIn() = default;

  // Required overrides
  const wchar_t* PlugInName() const override;
  const wchar_t* PlugInVersion() const override;
  GUID PlugInID() const override;
  
  // Additional overrides
  BOOL OnLoadPlugIn() override;
  void OnUnloadPlugIn() override;

  // File export overrides
  void AddFileType(ON_ClassArray<CRhinoFileType>& extensions, const CRhinoFileWriteOptions& options) override;
  BOOL WriteFile(const wchar_t* filename, int index, CRhinoDoc& doc, const CRhinoFileWriteOptions& options) override;

private:
  ON_wString m_plugin_version;
};

// Return a reference to the one and only CExportUSDPlugIn object
CExportUSDPlugIn& ExportUSDPlugIn();
