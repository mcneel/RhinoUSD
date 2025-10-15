#pragma once
#include "stdafx.h"

//
//  UsdImportPacket.h
//  import_USD
//
//  Created by Callum Sykes on 2025-05-06.
//

class UsdImportPacket
{
public:
  UsdImportPacket() = default;
//
//  UsdImportPacket(const PrimDataCollection&) {} // Copy Constructor
//  UsdImportPacket& operator=(const PrimDataCollection& x) {} // Copy Assignment
  
  // This will cause the object to be hidden if true
  bool isVisible = true;
  // Metadata I don't know what else to do with
  ON_ClassArray<ON_UserString> userStrings = {};
  
  void SetGeometry(std::shared_ptr<ON_Geometry> geometry);
  void SetLayer(std::shared_ptr<const ON_Layer> layer);
  void SetTransform(std::shared_ptr<const ON_Matrix> matrix);
  
  std::shared_ptr<ON_Geometry> GetGeometry() const;
  std::shared_ptr<const ON_Layer> GetLayer() const;
  std::shared_ptr<const ON_Matrix> GetTransform() const;
  
  const ON_Xform GetXForm() const;

private:
  std::shared_ptr<const ON_Layer> m_layer = nullptr;
  std::shared_ptr<ON_Geometry> m_geometry = nullptr;
  std::shared_ptr<const ON_Matrix> m_transform = nullptr;

};
