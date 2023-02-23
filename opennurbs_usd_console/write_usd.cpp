#include "stdafx.h"
#include "write_usd.h"

using namespace pxrInternal_v0_23__pxrReserved__;

static bool WriteUSDMesh(UsdStageRefPtr usdModel, const ON_Mesh* mesh, int index)
{
  if (nullptr == mesh)
    return false;

  ON_String name;
  name.Format("/mesh%d", index);
  UsdGeomMesh usdMesh = UsdGeomMesh::Define(usdModel, SdfPath(name.Array()));

  pxr::VtArray<pxr::GfVec3f> points;
  for (int i = 0; i < mesh->m_V.Count(); i++)
  {
    const ON_3fPoint& rhinoPt = mesh->m_V[i];
    pxr::GfVec3f pt(rhinoPt.x, rhinoPt.y, rhinoPt.z);
    points.push_back(pt);
  }
  usdMesh.GetPointsAttr().Set(points);

  pxr::VtArray<int> faceVertexCounts;
  pxr::VtArray<int> faceVertexIndices;
  for (int i = 0; i < mesh->m_F.Count(); i++)
  {
    const ON_MeshFace& face = mesh->m_F[i];
    faceVertexIndices.push_back(face.vi[0]);
    faceVertexIndices.push_back(face.vi[1]);
    faceVertexIndices.push_back(face.vi[2]);
    if (face.IsTriangle())
    {
      faceVertexCounts.push_back(3);
    }
    else
    {
      faceVertexCounts.push_back(4);
      faceVertexIndices.push_back(face.vi[3]);
    }
  }


  usdMesh.GetFaceVertexCountsAttr().Set(faceVertexCounts);
  usdMesh.GetFaceVertexIndicesAttr().Set(faceVertexIndices);

  VtVec3fArray extents(2);
  ON_BoundingBox bbox = mesh->BoundingBox();
  extents[0].Set((float)bbox.m_min.x, (float)bbox.m_min.y, (float)bbox.m_min.z);
  extents[1].Set((float)bbox.m_max.x, (float)bbox.m_max.y, (float)bbox.m_max.z);
  usdMesh.GetExtentAttr().Set(extents);
  return true;
}

bool WriteUSDFile(const wchar_t* rhinoFilename, const wchar_t* usdFilename)
{
  ONX_Model rhinoModel;
  if (!rhinoModel.Read(rhinoFilename))
    return false;

  UsdStageRefPtr usdModel = UsdStage::CreateInMemory();

  const ON_ComponentManifestItem* manifest_item = rhinoModel.Manifest().FirstItem(ON_ModelComponent::Type::ModelGeometry);
  int currentIndex = 0;
  while (manifest_item)
  {
    ON_UUID objectId = manifest_item->Id();
    manifest_item = rhinoModel.Manifest().NextItem(manifest_item);

    ON_ModelComponentReference component = rhinoModel.ComponentFromId(ON_ModelComponent::Type::ModelGeometry, objectId);
    const ON_ModelGeometryComponent* geometryComponent = ON_ModelGeometryComponent::Cast(component.ModelComponent());
    const ON_Geometry* geometry = geometryComponent ? geometryComponent->Geometry(nullptr) : nullptr;
    if (nullptr == geometry)
      continue;

    const ON_Brep* brep = ON_Brep::Cast(geometry);
    if (brep)
    {
      ON_SimpleArray<const ON_Mesh*> meshes;
      brep->GetMesh(ON::any_mesh, meshes);
      for (int i = 0; i < meshes.Count(); i++)
      {
        if (WriteUSDMesh(usdModel, meshes[i], currentIndex))
          currentIndex++;
      }
      continue;
    }
    const ON_Mesh* mesh = ON_Mesh::Cast(geometry);
    if (mesh)
    {
      if (WriteUSDMesh(usdModel, mesh, currentIndex))
        currentIndex++;
      continue;
    }
  }

  if (0 == currentIndex)
    return false;

  ON_String usdPath(usdFilename);
  usdModel->Export(usdPath.Array());
  return true;
}
