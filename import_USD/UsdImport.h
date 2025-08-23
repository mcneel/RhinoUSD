#pragma once
#include "stdafx.h"

#include "convert_geometry.h"
#include "convert_metadata.h"
#include "UsdImportPacket.h"

class UsdImport
{
public:
  
  UsdImport(const wchar_t* fileName, CRhinoDoc& doc, const CRhinoFileReadOptions& options)
    : m_doc(doc), m_options(options), m_filename(fileName)
  {
    
  }
  
  ~UsdImport()
  {
    // No delete needed
  }
  
  void TraversePrimTree(pxr::UsdPrim& root, UsdImportPacket& collection);
  bool AddPrimDataToDoc(UsdImportPacket& data);
  bool TryAddToDocument(std::shared_ptr<ON_Geometry> geom, std::shared_ptr<const ON_3dmObjectAttributes> attribs);
  bool ReadFile();

  
private:
  CRhinoDoc& m_doc;
  
  const CRhinoFileReadOptions& m_options;
  
  const wchar_t* m_filename;
  
  pxr::UsdStageRefPtr Model;
  
};
