#include "stdafx.h"
#include "ExportUSDPlugIn.h"

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

int WriteUSDFile(const wchar_t* filename, bool usda, CRhinoDoc& doc, const CRhinoFileWriteOptions& options)
{
  CRhinoWaitCursor hourglass;
  ON_wString backupname;

  //if (!doc.IsHeadless())
  //{
  //  // TODO: Add code for getting options from user
  //  if (CRhinoCommand::success != USDExportOptionsUI(false == options.UseBatchMode()))
  //    return -1;
  //}
  
  int mesh_ui_style = CExportUSDPlugIn::ThePlugin().m_saved_mesh_ui_style;
  ON_MeshParameters mp = CExportUSDPlugIn::ThePlugin().m_saved_mp;

  ON_SimpleArray<const CRhinoObject*> rhinoObjects;
  CRhinoObjectIterator it(doc.RuntimeSerialNumber(), options);
  for (CRhinoObject* obj = it.First(); obj; obj = it.Next())
  {
    rhinoObjects.Append(obj);
  }

  // Get meshes to export (meshes breps, copies mesh object meshes,
  // deals with instance references that contain meshes and breps,
  // etc.
  ON_ClassArray<CRhinoObjectMesh> mesh_list;
  if (CRhinoCommand::success == RhinoMeshObjects(rhinoObjects, mp, options.Transformation(), mesh_ui_style, mesh_list))
  {
    if (mesh_ui_style >= 0 && mesh_ui_style <= 1)
      CExportUSDPlugIn::ThePlugin().m_saved_mesh_ui_style = mesh_ui_style;
    CExportUSDPlugIn::ThePlugin().m_saved_mp = mp;
  }
  else
    return -1;

  doc.Redraw(); // clean up display after interactive meshing.

  if (mesh_list.Count() <= 0)
  {
    RhinoApp().Print(L"No meshes selected to export.\n");
    return 0;
  }

  UsdStageRefPtr usdModel = UsdStage::CreateInMemory();

  int currentIndex = 0;
  for (int i = 0; i < mesh_list.Count(); i++)
  {
    CRhinoObjectMesh& objectMesh = mesh_list[i];
    if (WriteUSDMesh(usdModel, objectMesh.m_mesh, currentIndex))
      currentIndex++;
  }

  if (0 == currentIndex)
    return 0;

  // use an ON_String to convert from unicode to mbcs which is what usd wants
  ON_String usdPath(filename);
  if (usdModel->Export(usdPath.Array()))
    return 1;
  return 0;
}

