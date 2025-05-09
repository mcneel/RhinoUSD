#pragma once
#include "stdafx.h"

std::shared_ptr<const ON_Geometry> TryGetPrimGeometry(pxr::UsdPrim& prim);

std::shared_ptr<const ON_Matrix> TryGetTransform(pxr::UsdGeomGprim& gPrim);
