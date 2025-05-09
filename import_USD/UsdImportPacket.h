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

private:
  std::shared_ptr<const ON_Layer> layer = nullptr;
  ON_Geometry* geometry = nullptr;
  ON_Matrix* transform = nullptr;

};
