#pragma once

class Cimport_USDPlugIn : public CRhinoFileImportPlugIn
{
public:
  Cimport_USDPlugIn();
  ~Cimport_USDPlugIn() = default;

  // Plug-in name display string. This name is displayed by Rhino when
  // loading the plug-in, in the plug-in help menu, and in the Rhino
  // interface for managing plug-ins. 
  const wchar_t* PlugInName() const override;
  
  // Plug-in version display string. This name is displayed by Rhino
  // when loading the plug-in and in the Rhino interface for 
  // managing plug-ins.
  const wchar_t* PlugInVersion() const override;
  
  // Plug-in unique identifier. The identifier is used by Rhino for
  // managing plug-ins.
  GUID PlugInID() const override;
  
  // Called by Rhino when displaying the open file dialog.
  // Add supported file type extensions here.
  void AddFileType(ON_ClassArray<CRhinoFileType>& extensions, const CRhinoFileReadOptions& options) override;
  
  // Called by Rhino to read document geometry from an external file.
  BOOL ReadFile(const wchar_t* filename, int index, CRhinoDoc& doc, const CRhinoFileReadOptions& options) override;
private:
  ON_wString m_plugin_version;
};

bool ReadUSDFile(const wchar_t* filename, CRhinoDoc& doc, const CRhinoFileReadOptions& options);



