#include "stdafx.h"
#include "ExportUSDPlugIn.h"
#include "../UsdShared/ON_Helpers.h"
#include "../UsdShared/UsdShared.h"

static std::vector<ON_wString> GetLayerNames(const CRhinoObject* obj, const CRhinoDoc& doc)
{
  std::vector<ON_wString> names;
  const CRhinoObjectAttributes& attributes = obj->Attributes();
  int layer_index = attributes.m_layer_index;
 
  const CRhinoLayerTable& layer_table = doc.m_layer_table;
  const CRhinoLayer& layer = layer_table[layer_index];
  ON_wString layerName = UsdShared::RhinoLayerNameToUsd(layer.Name());
  names.push_back(layerName);

  ON_UUID pid(layer.ParentId());
  while (!ON_UuidIsNil(pid))
  {
    layer_index = layer_table.FindLayerFromId(pid, false, false, -1);
    const CRhinoLayer& parentLayer = layer_table[layer_index];
    ON_wString parentLayerName = UsdShared::RhinoLayerNameToUsd(parentLayer.Name());
    names.push_back(parentLayerName);
    ON_UUID id(parentLayer.ParentId());
    pid = id;
  }
  names.insert(names.begin(), L"Geometry");
  names.insert(names.begin(), L"Rhino"); // or "World"
  return names;
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

  UsdExportImport usdEI{};

  //ON_SimpleArray<const CRhinoObject*> rhinoObjects;
  CRhinoObjectIterator it(doc.RuntimeSerialNumber(), options);
  for (CRhinoObject* obj = it.First(); obj; obj = it.Next())
  {
    //rhinoObjects.Append(obj);
    ON_SimpleArray<const ON_Mesh*> meshes;
    int meshCount = obj->GetMeshes(ON::any_mesh, meshes);
    if (meshCount < 1)
      continue;
    std::vector<ON_wString> layerNames = GetLayerNames(obj, doc);
    for (int i = 0; i < meshCount; i++)
    {
      const ON_Mesh* mesh = meshes[i];
      usdEI.AddMesh(mesh, layerNames);
    }
  }


  // Get meshes to export (meshes breps, copies mesh object meshes,
  // deals with instance references that contain meshes and breps,
  // etc.
  //ON_ClassArray<CRhinoObjectMesh> mesh_list;
  //if (CRhinoCommand::success == RhinoMeshObjects(rhinoObjects, mp, options.Transformation(), mesh_ui_style, mesh_list))
  //{
  //  if (mesh_ui_style >= 0 && mesh_ui_style <= 1)
  //    CExportUSDPlugIn::ThePlugin().m_saved_mesh_ui_style = mesh_ui_style;
  //  CExportUSDPlugIn::ThePlugin().m_saved_mp = mp;
  //}
  //else
  //  return -1;

  //doc.Redraw(); // clean up display after interactive meshing.

  //if (mesh_list.Count() <= 0)
  //{
  //  RhinoApp().Print(L"No meshes selected to export.\n");
  //  return 0;
  //}

  //UsdStageRefPtr usdModel = UsdStage::CreateInMemory();

  //for (int i = 0; i < mesh_list.Count(); i++)
  //{
  //  CRhinoObjectMesh& objectMesh = mesh_list[i];
  //  std::vector<ON_wString> layerNames = GetLayerNames(objectMesh.m_mesh_object, doc);
  //  usdEI.AddMesh(objectMesh.m_mesh, layerNames);
  //}

  if (!usdEI.AnythingToSave())
    return 0;

  // use an ON_String to convert from unicode to mbcs which is what usd wants
  ON_wString usdPath(filename);
  usdEI.Save(usdPath);
  return 1;
}

