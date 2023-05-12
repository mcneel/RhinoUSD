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
#if defined(ON_RUNTIME_APPLE)
  std::vector<std::string> searchPath;
  NSString* resources = [[NSBundle mainBundle] resourcePath];
  ON_String usd_resource_path = [resources ONString];
  usd_resource_path += "/usd";
  pxr::PlugRegistry::GetInstance().RegisterPlugins(std::string(usd_resource_path.Array()));
#endif

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

  UsdExportImport usdEI;

  ON_ClassArray<CRhinoObjectMesh> mesh_list;
  ON_SimpleArray<const CRhinoObject*> objects(256);
  CRhinoObjectIterator it(doc.RuntimeSerialNumber(), options);
  for (CRhinoObject* obj = it.First(); obj; obj = it.Next())
  {
    objects.Append(obj);

    ///////// Disable Nurbs for now - USD only partially supports nurbs curves and surfaces
    //const ON_Geometry* geometry = obj->Geometry();
    //if (nullptr == geometry)
    //  continue;
   
    //std::vector<ON_wString> layerNames = GetLayerNames(obj, doc);

    //const ON_NurbsCurve* nurbsCurve = ON_NurbsCurve::Cast(geometry);
    //if (nurbsCurve)
    //{
    //  usdEI.AddNurbsCurve(nurbsCurve, layerNames);
    //}

    ////const CRhinoBrepObject* pBrep = CRhinoBrepObject::Cast(obj);
    //// try casting from geometry
    //const ON_Brep* brep = ON_Brep::Cast(geometry);
    //if (brep)
    //{
    //  if (brep->IsManifold())
    //  {
    //    for (int i = 0; i < brep->m_S.Count(); i++)
    //    {
    //      ON_NurbsSurface* nurbsSurface = nullptr;
    //      brep->m_S[i]->NurbsSurface(nurbsSurface);
    //      if (nurbsSurface)
    //      {
    //        
    //      }
    //    }
    //  }
    //}
  }

  CRhinoCommand::result rs = RhinoMeshObjects(objects, mp, options.Transformation(), mesh_ui_style, mesh_list);
  //if (CRhinoCommand::success != rs)
  //{
  //  return 0;
  //}
  if (CRhinoCommand::success == rs)
  {
    if (4 != mesh_ui_style)
      CExportUSDPlugIn::ThePlugin().m_saved_mesh_ui_style = mesh_ui_style;
    CExportUSDPlugIn::ThePlugin().m_saved_mp = mp;
  }
  doc.Redraw(); // clean up display after interactive meshing.

  for (int i = 0; i < mesh_list.Count(); i++)
  {
    CRhinoObjectMesh& objectMesh = mesh_list[i];
    if (nullptr == objectMesh.m_parent_object || nullptr == objectMesh.m_mesh)
      continue;

    std::vector<ON_wString> layerNames = GetLayerNames(objectMesh.m_parent_object, doc);
    ON_wString meshPath = usdEI.AddMesh(objectMesh.m_mesh, layerNames);

    const CRhRdkMaterial* pMaterial = objectMesh.m_parent_object->ObjectRdkMaterial(ON_COMPONENT_INDEX::UnsetComponentIndex);
    if (pMaterial)
    {
      const ON_Material& material = pMaterial->SimulatedMaterial();
      if (material.IsPhysicallyBased())
      {
        //Shiny new Pixar approved material.
        std::shared_ptr <ON_PhysicallyBasedMaterial> ppbr = material.PhysicallyBased();
        if (ppbr)
        {
          ON_PhysicallyBasedMaterial& pbr = *ppbr;
          usdEI.AddAndBindPbrMaterial(&pbr, layerNames, meshPath);
          //auto color = pbr.BaseColor();
        }
      }
      else
      {
        //Old skool 1981 material.
        usdEI.AddAndBindMaterial(&material, layerNames, meshPath);
      }

      //int tc = material.m_textures.Count();
      //for (int i = 0; i < tc; i++)
      //{
      //  ON_Texture tx = material.m_textures[i];
      //  ON_Texture::TYPE t = tx.m_type;
      //  ON_wString fp = tx.m_image_file_reference.FullPath();
      //  
      //  RhinoApp().Print(L"texture type: %d\r\n", t);
      //}
      //int ti = material.FindTexture(nullptr, ON_Texture::TYPE::diffuse_texture);
      //int ti = material.FindTexture(nullptr, ON_Texture::TYPE::bump_texture);
      //int ti = material.FindTexture(nullptr, ON_Texture::TYPE::transparency_texture);
      //int ti = material.FindTexture(nullptr, ON_Texture::TYPE::bitmap_texture);
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

