#include "stdafx.h"
#include "pxr/usd/usd/primRange.h"
#include "convert_mesh.h"

bool ReadUSDFile(const wchar_t* filename, CRhinoDoc& doc, const CRhinoFileReadOptions& options)
{
  // use an ON_String to convert from unicode to mbcs which is what usd wants
  ON_String usdPath(filename);
  UsdStageRefPtr usdModel = UsdStage::Open(usdPath.Array());

  // Traverse all prims in the stage
  UsdPrimRange primRange = usdModel->TraverseAll();
  for (auto& prim : primRange)
  {
    if (ON_Mesh* mesh = TryGetMeshFromPrim(prim))
    {
      doc.AddMeshObject(*mesh);
    }
  }

  return true;
}
