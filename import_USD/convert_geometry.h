#include "stdafx.h"
#include "pxr/usd/usd/primRange.h"

using namespace pxr;

ON_Mesh* TryGetMeshFromPrim(UsdPrim& prim);

ON_Geometry* TryGetPrimGeometry(UsdPrim& prim);

ON_Matrix* TryGetTransform(UsdGeomGprim& gPrim);
