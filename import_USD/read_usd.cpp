#include "stdafx.h"

using namespace pxrInternal_v0_23__pxrReserved__;

bool ReadUSDFile(const wchar_t* filename, CRhinoDoc& doc, const CRhinoFileReadOptions& options)
{
  // use an ON_String to convert from unicode to mbcs which is what usd wants
  ON_String usdPath(filename);
  UsdStageRefPtr usdModel = UsdStage::Open(usdPath.Array());

  std::vector<UsdPrim> meshes;

  /*UsdPrimRange*/auto range = usdModel->TraverseAll();
  std::copy_if(range.begin(), range.end(), std::back_inserter(meshes), [](UsdPrim const& prim) {return prim.IsA<UsdGeomMesh>(); });

  std::vector<UsdPrim>::iterator iter = meshes.begin();
  for (iter; iter < meshes.end(); iter++)
  {
    UsdPrim meshPrim = *iter;
    UsdGeomMesh usdMesh = UsdGeomMesh(meshPrim);
    VtArray<int> faceVertexCounts;
    VtArray<int> faceVertexIndices;
    VtArray<GfVec3f> points;
    //VtArray<GfVec3f> normals;

    UsdAttribute fvc = usdMesh.GetFaceVertexCountsAttr();
    if (fvc.ValueMightBeTimeVarying())
      return false;
    else
      fvc.Get(&faceVertexCounts, 0);

    UsdAttribute fvi = usdMesh.GetFaceVertexIndicesAttr();
    if (fvi.ValueMightBeTimeVarying())
      return false;
    else
      fvi.Get(&faceVertexIndices, 0);

    usdMesh.GetPointsAttr().Get(&points);
    //usdMesh.GetNormalsAttr().Get(&normals);
    
    if (points.size() == 0)
      return false; //invalid mesh
    
    ON_Mesh mesh(faceVertexCounts.size(), points.size(), false, false);

    for (int i = 0; i < points.size(); i++)
    {
      GfVec3f& usd_pt = points[i];
      const ON_3dPoint on_pt(usd_pt[0], usd_pt[1], usd_pt[2]);
      mesh.SetVertex(i, on_pt);
    }

    if (faceVertexCounts.size() != faceVertexIndices.size())
      return false; //invalid mesh;

    for (int i = 0; i < faceVertexCounts.size(); i++)
    {
      VtArray<int>& a = faceVertexCounts;
      if (a.size() == 3)
        mesh.SetTriangle(i, a[0], a[1], a[2]);
      else if (a.size() == 4)
        mesh.SetQuad(i, a[0], a[1], a[2], a[3]);
      else
        return false; // invalid mesh
    }
  }

  return false;
}
