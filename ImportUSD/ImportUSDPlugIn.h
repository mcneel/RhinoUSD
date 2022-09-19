#pragma once

class CImportUSDPlugIn : public CRhinoFileImportPlugIn
{
public:
  CImportUSDPlugIn();
  ~CImportUSDPlugIn() = default;

  // Required overrides
  const wchar_t* PlugInName() const override;
  const wchar_t* PlugInVersion() const override;
  GUID PlugInID() const override;
  
  // Additional overrides
  BOOL OnLoadPlugIn() override;
  void OnUnloadPlugIn() override;

  // File import overrides
  void AddFileType(ON_ClassArray<CRhinoFileType>& extensions, const CRhinoFileReadOptions& options) override;
  BOOL ReadFile(const wchar_t* filename, int index, CRhinoDoc& doc, const CRhinoFileReadOptions& options) override;

private:
  ON_wString m_plugin_version;
};

// Return a reference to the one and only CImportUSDPlugIn object
CImportUSDPlugIn& ImportUSDPlugIn();
