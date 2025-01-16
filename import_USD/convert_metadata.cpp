#include "stdafx.h"
#include "convert_metadata.h"

ON_3dmObjectAttributes* TryGetAttributesFromPrim(UsdPrim& prim)
{

  ON_3dmObjectAttributes* attribs = new ON_3dmObjectAttributes();

  std::string usdName = prim.GetName().GetString();
  // attribs->SetName(onString.To, true);

  // Attribs
  UsdAttributeVector attribVectors = prim.GetAttributes();
  for (UsdAttribute& usdAttrib : attribVectors)
  {
    auto usdAttribName = usdAttrib.GetName().GetString();

    // Metadata
    UsdMetadataValueMap map = usdAttrib.GetAllMetadata();
    for (auto& metaValue : map)
    {

    }

  }

  return attribs;
}

ON_Layer* TryGetLayerFromPrim(UsdPrim& prim)
{
  auto description = prim.GetDescription();
  auto primPath = prim.GetPath();
  auto primString = primPath.GetAsString();

  ON_Layer* onLayer = new ON_Layer();
  onLayer->SetName(L"Test");

  return onLayer;
}

// void TryGetLayersFromStage


