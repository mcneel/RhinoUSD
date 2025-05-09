#include "stdafx.h"
#include "convert_geometry.h"
#include "convert_metadata.h"

using namespace pxr;

std::shared_ptr<ON_Plane> TryGetPlane(pxr::UsdPrim& prim)
{
  // if (ON_Plane* plane = GetPlane(prim)) return plane;

  return nullptr;
}


// TODO : Capsule vs Capsule_1?
std::shared_ptr<ON_Mesh> GetCapsule(pxr::UsdPrim& prim)
{
  // UsdGeomCapsule
  return nullptr;
}

std::shared_ptr<ON_RevSurface> GetCone(pxr::UsdPrim& prim) { return nullptr; }

std::shared_ptr<ON_Brep> GetBox(pxr::UsdPrim& prim)
{
  
  return nullptr;
}

// TODO : Cylinder and Cylinder_1?
std::shared_ptr<ON_RevSurface> GetCylinder(pxr::UsdPrim& prim)
{
  // UsdGeomCylinder
  
  return nullptr;
}

std::shared_ptr<ON_Curve> GetCurves(pxr::UsdPrim& prim)
{
  // UsdGeomBasisCurves
  return nullptr;
}

std::shared_ptr<ON_NurbsCurve> GetNurbs(pxr::UsdPrim& prim)
{
  if (UsdGeomNurbsCurves nurbs = UsdGeomNurbsCurves(prim))
  {
    auto knots = ConvertMetadata::GetValueFromAttribute<VtArray<double>>(nurbs.GetKnotsAttr());
    auto orders = ConvertMetadata::GetValueFromAttribute<VtArray<int>>(nurbs.GetOrderAttr());
    auto points = ConvertMetadata::GetValueFromAttribute<VtArray<GfVec3f>>(nurbs.GetPointsAttr());

    std::shared_ptr<ON_NurbsCurve> onNurbs = std::make_shared<ON_NurbsCurve>();
    for (int i = 0; i < points.capacity(); i++)
    {
      auto point = points[i];
      double x = (double)point[0];
      double y = (double)point[1];
      double z = (double)point[2];

      const ON_3dPoint onPoint(x, y, z);
      // TODO : Set Curve
      // onNurbs->SetCV(i, onPoint);
    }

    for (int i = 0; i < knots.capacity(); i++)
    {
      auto knot = knots[i];
      auto order = orders[i];

      onNurbs->SetKnot(order, knot);
    }

    return onNurbs;
  }

  return nullptr;
} // UsdGeomNurbsCurves

// ON_BezierCurve
std::shared_ptr<ON_Curve> GetHermite(pxr::UsdPrim& prim) { return nullptr; } // UsdGeomHermiteCurves

std::shared_ptr<ON_Mesh> GetMesh(pxr::UsdPrim& prim)
{
  if (UsdGeomMesh mesh = UsdGeomMesh(prim))
  {
    pxr::VtArray<int> faceVertexCounts;
    pxr::VtArray<int> faceVertexIndices;
    pxr::VtArray<GfVec3f> normals;
    pxr::VtArray<GfVec3f> points;
    // VtArray<GfVec2f> textCoords;

    mesh.GetFaceVertexCountsAttr().Get(&faceVertexCounts);
    mesh.GetFaceVertexIndicesAttr().Get(&faceVertexIndices);
    mesh.GetNormalsAttr().Get(&normals);
    mesh.GetPointsAttr().Get(&points);

    const int vertexCount = (int)points.size();
    const int faceCount = (int)faceVertexCounts.size();

    if (vertexCount < 3) return nullptr;
    if (faceCount < 1) return nullptr;

    std::shared_ptr<ON_Mesh> rhinoMesh = std::make_shared<ON_Mesh>(faceCount, vertexCount, true, false);
    for (int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
    {
      pxr::GfVec3d usdPoint = points[vertexIndex];
      ON_3fPoint meshPoint(usdPoint[0], usdPoint[1], usdPoint[2]);

      pxr::GfVec3d usdNormal = normals[vertexIndex];
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

std::shared_ptr<ON_NurbsSurface> GetNurbsPatch(pxr::UsdPrim& prim) { return nullptr; } // UsdGeomNurbsPatch

// TODO : Consider ON_3dPointArray also.
std::shared_ptr<ON_PointCloud> GetPoints(pxr::UsdPrim& prim)
{
  // UsdGeomPointBased
  return nullptr;
}

std::shared_ptr<ON_Mesh> GetTetrahedralMesh(pxr::UsdPrim& prim)
{
  // UsdGeomTetMesh
  return nullptr;
}

std::shared_ptr<ON_RevSurface> GetSphere(pxr::UsdPrim& prim)
{
  if (auto sphere = UsdGeomSphere(prim))
  {
    auto radiusAttrib = sphere.GetRadiusAttr();
    if (radiusAttrib.HasValue())
    {
      pxr::VtValue radiusValue;
      radiusAttrib.Get(&radiusValue);
      double radius = radiusValue.Get<double>();
      ON_3dPoint point(0, 0, 0);
      ON_Sphere sphere(point, radius);
      auto rev = sphere.RevSurfaceForm(false);
      
    }
  }

  return nullptr;
}

// ON_ ? ? Get ? ? (UsdPrim & prim) {} // For UsdVolVolume <- Unsure what this is yet

std::shared_ptr<const ON_Matrix> ConvertGeometry::TryGetTransform(pxr::UsdGeomGprim& gPrim)
{
  pxr::GfMatrix4d usdTransform = gPrim.ComputeLocalToWorldTransform(UsdTimeCode::Default());
  double* m = usdTransform.data();

  return std::make_shared<ON_Matrix>(4, 4, *m, true);
}

std::shared_ptr<ON_Geometry> ConvertGeometry::TryGetPrimGeometry(pxr::UsdPrim& prim)
{
  if (std::shared_ptr<ON_Mesh> mesh = GetCapsule(prim)) return mesh;
  if (std::shared_ptr<ON_RevSurface> cone = GetCone(prim)) return cone;
  if (std::shared_ptr<ON_Brep> box = GetBox(prim)) return box;
  if (std::shared_ptr<ON_RevSurface> cylinder = GetCylinder(prim)) return cylinder;
  if (std::shared_ptr<ON_Curve> curve = GetCurves(prim)) return curve;
  if (std::shared_ptr<ON_NurbsCurve> nurbscurve = GetNurbs(prim)) return nurbscurve;
  if (std::shared_ptr<ON_Curve> beziercurve = GetHermite(prim)) return beziercurve;
  if (std::shared_ptr<ON_Mesh> mesh = GetMesh(prim)) return mesh;
  if (std::shared_ptr<ON_NurbsSurface> nurbssurface = GetNurbsPatch(prim)) return nurbssurface;
  if (std::shared_ptr<ON_PointCloud> pointcloud = GetPoints(prim)) return pointcloud;
  if (std::shared_ptr<ON_Mesh> mesh = GetTetrahedralMesh(prim)) return mesh;
  if (std::shared_ptr<ON_RevSurface> sphere = GetSphere(prim)) return sphere;

  return nullptr;
}
