#include "stdafx.h"

using namespace pxrInternal_v0_23__pxrReserved__;

bool ReadUSDFile(const wchar_t* filename, CRhinoDoc& doc, const CRhinoFileReadOptions& options)
{
  // use an ON_String to convert from unicode to mbcs which is what usd wants
  ON_String usdPath(filename);
  UsdStageRefPtr usdModel = UsdStage::Open(usdPath.Array());
  // TODO: Figure out how to walk throuh a UsdStage and extract geometry
  return false;
}
