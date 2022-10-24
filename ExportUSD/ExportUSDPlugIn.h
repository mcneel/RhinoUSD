#pragma once

class CExportUSDPlugIn : public CRhinoFileExportSystemPlugIn
{
public:
  CExportUSDPlugIn() = default;
  ~CExportUSDPlugIn() = default;

  // Required overrides
  const wchar_t* PlugInName() const override;
  const wchar_t* LocalPlugInName() const override;
  const wchar_t* PlugInVersion() const override;
  GUID PlugInID() const override;
  
  // Additional overrides
  BOOL OnLoadPlugIn() override;
  void OnUnloadPlugIn() override;

  // File export plug-in overrides
  void AddFileType(ON_ClassArray<CRhinoFileType>& extensions, const CRhinoFileWriteOptions& options) override;
  int WriteFile(const wchar_t* filename, int index, CRhinoDoc& doc, const CRhinoFileWriteOptions& options) override;
};
