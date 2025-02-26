#include "stdafx.h"
#include "pxr/usd/usd/primRange.h"

using namespace pxr;
using namespace pxrInternal_v0_23__pxrReserved__;

ON_3dmObjectAttributes* TryGetAttributesFromPrim(UsdPrim& prim);

ON_Layer* TryGetLayerFromPrim(UsdPrim& prim);

template <typename T>
T GetValueFromAttribute(UsdAttribute& attribute)
{
  VtValue attributeValue;
  // TOOD : Bool check
  attribute.Get(&attributeValue);
  
  return attributeValue.Get<T>();
}
