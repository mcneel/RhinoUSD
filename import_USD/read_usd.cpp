#include "stdafx.h"
#include "../UsdShared/ON_Helpers.h"

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
    
    int ptsSz = points.size();
    int vtxCountsSz = faceVertexCounts.size();
    int vtxIdxsSz = faceVertexIndices.size();

    if (ptsSz == 0)
      return false; //invalid mesh

    // assert sum of faceVertexCounts == faceVertexIndex.size()
    int totalCounts = std::accumulate(faceVertexCounts.begin(), faceVertexCounts.end(), 0);
    if (totalCounts != faceVertexIndices.size())
      return false;
    
    ON_Mesh mesh(faceVertexCounts.size(), ptsSz, false, false);

    for (int i = 0; i < ptsSz; i++)
    {
      GfVec3f& usd_pt = points[i];
      const ON_3dPoint on_pt(usd_pt[0], usd_pt[1], usd_pt[2]);
      mesh.SetVertex(i, on_pt);
    }

    double vi = 0;
    for (int i = 0; i < vtxCountsSz; i++)
    {
      int vc = faceVertexCounts[i];
      if (vc < 3 || vc > 4)
        return false;
      int idx1 = vi++;
      int idx2 = vi++;
      int idx3 = vi++;
      if (vc == 3)
        mesh.SetTriangle(i, faceVertexIndices[idx1], faceVertexIndices[idx2], faceVertexIndices[idx3]);
      else if (vc == 4)
      {
        int idx4 = vi++;
        mesh.SetQuad(i, faceVertexIndices[idx1], faceVertexIndices[idx2], faceVertexIndices[idx3], faceVertexIndices[idx4]);
      }
    }

    if (mesh.IsValid())
    {
      ON_Helpers::RotateZUp(&mesh);
      if (!mesh.HasVertexNormals())
        mesh.ComputeVertexNormals();
      doc.AddMeshObject(mesh);
    }
  }
  return true;
}
