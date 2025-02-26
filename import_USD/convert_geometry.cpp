#include "stdafx.h"
#include "convert_geometry.h"
#include "convert_metadata.h"

ON_Plane* TryGetPlane(UsdPrim& prim)
{
  // if (ON_Plane* plane = GetPlane(prim)) return plane;

  return nullptr;
}


// TODO : Capsule vs Capsule_1?
ON_Mesh* GetCapsule(UsdPrim& prim)
{
  // UsdGeomCapsule
  return nullptr;
}

ON_RevSurface* GetCone(UsdPrim& prim) { return nullptr; }

ON_Brep* GetBox(UsdPrim& prim)
{
  
  return nullptr;
}

// TODO : Cylinder and Cylinder_1?
ON_RevSurface* GetCylinder(UsdPrim& prim)
{
  // UsdGeomCylinder
  
  return nullptr;
}

ON_Curve* GetCurves(UsdPrim& prim)
{
  // UsdGeomBasisCurves
  return nullptr;
}

ON_NurbsCurve* GetNurbs(UsdPrim& prim)
{
  if (UsdGeomNurbsCurves nurbs = UsdGeomNurbsCurves(prim))
  {
    auto onNurbs = new ON_NurbsCurve();
    /*
    auto knots = GetValueFromAttribute<VtArray<double>>(nurbs.GetKnotsAttr());
    auto orders = GetValueFromAttribute<VtArray<int>>(nurbs.GetOrderAttr());
    auto points = GetValueFromAttribute<VtArray<GfVec3f>>(nurbs.GetPointsAttr());

    for (int i = 0; i < points.capacity(); i++)
    {
      auto point = points[i];
      double x = (double)point[0];
      double y = (double)point[1];
      double z = (double)point[2];

      const ON_3dPoint* onPoint = new ON_3dPoint(x, y, z);
      // TODO : Set Curve
      // onNurbs->SetCV(i, onPoint);
    }

    for (int i = 0; i < knots.capacity(); i++)
    {
      auto knot = knots[i];
      auto order = orders[i];

      onNurbs->SetKnot(order, knot);
    }
    */

    return onNurbs;
  }

  return nullptr;
} // UsdGeomNurbsCurves

// ON_BezierCurve
ON_Curve* GetHermite(UsdPrim& prim) { return nullptr; } // UsdGeomHermiteCurves

ON_Mesh* GetMesh(UsdPrim& prim)
{
  if (UsdGeomMesh mesh = UsdGeomMesh(prim))
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

ON_NurbsSurface* GetNurbsPatch(UsdPrim& prim) { return nullptr; } // UsdGeomNurbsPatch

// TODO : Consider ON_3dPointArray also.
ON_PointCloud* GetPoints(UsdPrim& prim)
{
  // UsdGeomPointBased
  return nullptr;
}

ON_Mesh* GetTetrahedralMesh(UsdPrim& prim)
{
  // UsdGeomTetMesh
  return nullptr;
}

ON_RevSurface* GetSphere(UsdPrim& prim)
{
  if (auto sphere = UsdGeomSphere(prim))
  {
    auto radiusAttrib = sphere.GetRadiusAttr();
    if (radiusAttrib.HasValue())
    {
      VtValue radiusValue;
      radiusAttrib.Get(&radiusValue);
      double radius = radiusValue.Get<double>();
      const ON_3dPoint* point = new ON_3dPoint(0, 0, 0);
      auto sphere = new ON_Sphere(*point, radius);
      return sphere->RevSurfaceForm(false);
    }
  }

  return nullptr;
}

// ON_ ? ? Get ? ? (UsdPrim & prim) {} // For UsdVolVolume <- Unsure what this is yet

ON_Matrix* TryGetTransform(UsdGeomGprim& gPrim)
{
  GfMatrix4d usdTransform = gPrim.ComputeLocalToWorldTransform(UsdTimeCode::Default());
  double* m = usdTransform.data();
  auto matrix = new ON_Matrix(4, 4, *m, true);

  return matrix;
}

ON_Geometry* TryGetPrimGeometry(UsdPrim& prim)
{
  UsdGeomGprim geom = UsdGeomGprim(prim);
  if (&geom == nullptr) return nullptr;

  // TODO : This feels. Slow.
  if (ON_Mesh* mesh = GetCapsule(prim)) return mesh;
  if (ON_RevSurface* cone = GetCone(prim)) return cone;
  if (ON_Brep* box = GetBox(prim)) return box;
  if (ON_RevSurface* cylinder = GetCylinder(prim)) return cylinder;
  if (ON_Curve* curve = GetCurves(prim)) return curve;
  if (ON_NurbsCurve* nurbscurve = GetNurbs(prim)) return nurbscurve;
  if (ON_Curve* beziercurve = GetHermite(prim)) return beziercurve;
  if (ON_Mesh* mesh = GetMesh(prim)) return mesh;
  if (ON_NurbsSurface* nurbssurface = GetNurbsPatch(prim)) return nurbssurface;
  if (ON_PointCloud* pointcloud = GetPoints(prim)) return pointcloud;
  if (ON_Mesh* mesh = GetTetrahedralMesh(prim)) return mesh;
  if (ON_RevSurface* sphere = GetSphere(prim)) return sphere;

  return nullptr;
}
