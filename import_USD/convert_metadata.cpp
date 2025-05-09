#include <iostream>
#include "stdafx.h"
#include "convert_metadata.h"

std::shared_ptr<const ON_3dmObjectAttributes> ConvertMetadata::TryGetAttributesFromPrim(pxr::UsdPrim& prim)
{

  ON_3dmObjectAttributes attribs;

  std::string usdName = prim.GetName().GetString();
  // attribs->SetName(onString.To, true);

  // Attribs
  pxr::UsdAttributeVector attribVectors = prim.GetAttributes();
  for (pxr::UsdAttribute& usdAttrib : attribVectors)
  {
    auto usdAttribName = usdAttrib.GetName().GetString();

    // Metadata
    pxr::UsdMetadataValueMap map = usdAttrib.GetAllMetadata();
    for (auto& metaValue : map)
    {
      // TODO : Set attribs
    }

  }

  std::shared_ptr<const ON_3dmObjectAttributes> constAttribs = std::make_shared<ON_3dmObjectAttributes>(attribs);
  return constAttribs;
}

std::shared_ptr<ON_Layer> ConvertMetadata::TryGetLayerFromPrim(pxr::UsdPrim& prim)
{
  auto description = prim.GetDescription();
  auto primPath = prim.GetPath();
  auto primString = primPath.GetAsString();

  std::shared_ptr<ON_Layer> onLayer = std::make_shared<ON_Layer>();
  onLayer->SetName(L"Test");

  return onLayer;
}
