#pragma once
#include "stdafx.h"

namespace ConvertMetadata
{

  std::shared_ptr<const ON_3dmObjectAttributes> TryGetAttributesFromPrim(pxr::UsdPrim& prim);

  std::shared_ptr<ON_Layer> TryGetLayerFromPrim(pxr::UsdPrim& prim);

  template <typename T>
  T GetValueFromAttribute(pxr::UsdAttribute attribute)
  {
    pxr::VtValue attributeValue;
    // TOOD : Bool check
    attribute.Get(&attributeValue);
    
    return attributeValue.Get<T>();
  }

}
