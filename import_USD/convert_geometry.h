#pragma once
#include "stdafx.h"

namespace ConvertGeometry
{

  std::shared_ptr<ON_Geometry> TryGetPrimGeometry(pxr::UsdPrim& prim);

  std::shared_ptr<const ON_Matrix> TryGetTransform(pxr::UsdGeomGprim& gPrim);

}
