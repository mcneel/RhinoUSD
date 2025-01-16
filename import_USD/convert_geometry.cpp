#include "stdafx.h"
#include "convert_geometry.h"

ON_Mesh* TryGetMeshFromPrim(UsdPrim& prim)
{
  UsdGeomMesh mesh = UsdGeomMesh(prim);
  if (mesh)
  {
    VtArray<int> faceVertexCounts;
    VtArray<int> faceVertexIndices;
    VtArray<GfVec3f> normals;
    VtArray<GfVec3f> points;
    // VtArray<GfVec2f> textCoords;

    mesh.GetFaceVertexCountsAttr().Get(&faceVertexCounts);
    mesh.GetFaceVertexIndicesAttr().Get(&faceVertexIndices);
    mesh.GetNormalsAttr().Get(&normals);
    mesh.GetPointsAttr().Get(&points);

    const int vertexCount = (int)points.size();
    const int faceCount = (int)faceVertexCounts.size();

    if (vertexCount < 3) return nullptr;
    if (faceCount < 1) return nullptr;

    ON_Mesh* rhinoMesh = new ON_Mesh(faceCount, vertexCount, true, false);

    for (int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
    {
      GfVec3d usdPoint = points[vertexIndex];
      ON_3fPoint meshPoint(usdPoint[0], usdPoint[1], usdPoint[2]);

      GfVec3d usdNormal = normals[vertexIndex];
      ON_3fVector meshNormal(usdNormal[0], usdNormal[1], usdNormal[2]);

      rhinoMesh->m_V.Append(meshPoint);
      rhinoMesh->m_N.Append(meshNormal);
    }

    int qqq = 0;
    for (int faceIndex = 0; faceIndex < faceCount; faceIndex++)
    {
      int faceVertexCount = faceVertexCounts[faceIndex];
      ON_MeshFace f;
      for (int i = 0; i < faceVertexCount; i++)
      {
        f.vi[i] = faceVertexIndices[qqq++];
      }

      // Tells ON_Mesh this face is a Triangle
      if (3 == faceVertexCount)
      {
        f.vi[3] = f.vi[2];
      }
      rhinoMesh->m_F.Append(f);
    }

    ON_BoundingBox bbox = rhinoMesh->BoundingBox();
    rhinoMesh->ComputeFaceNormals();

    return rhinoMesh;
  }

  return nullptr;
}
