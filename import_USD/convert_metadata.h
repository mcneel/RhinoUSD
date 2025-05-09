#pragma once
#include "stdafx.h"

ON_3dmObjectAttributes* TryGetAttributesFromPrim(pxr::UsdPrim& prim);

ON_Layer* TryGetLayerFromPrim(pxr::UsdPrim& prim);

template <typename T>
T GetValueFromAttribute(pxr::UsdAttribute& attribute)
{
  pxr::VtValue attributeValue;
  // TOOD : Bool check
  attribute.Get(&attributeValue);
  
  return attributeValue.Get<T>();
}
