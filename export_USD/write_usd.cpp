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

static void SetStringMap(std::multimap<const ON_UUID, const ON_wString>& sm)
{
  auto pr1 = std::pair<const ON_UUID, const ON_wString>(ON_nil_uuid, L"Hello");
  //sm.insert(pr1);
}

static void WorkoutTextureCoordinates(
  const int mapping_channel_id,
  const std::map<int, const ON_TextureCoordinates*>& mappingCoordinatesOnMesh,
  std::vector<const ON_TextureCoordinates>& tcs
)
{
  //auto dfltMc = static_cast<ON_Texture::MAPPING_CHANNEL>(mapping_channel_id);
  //if (true /*is default*/)
  //  if (dfltMc == ON_Texture::MAPPING_CHANNEL::tc_channel) // deprecated
  //    dfltMc = ON_Texture::MAPPING_CHANNEL::default_channel;
  if (ON_Texture::IsBuiltInMappingChannel(mapping_channel_id)) {
    auto mc_type = ON_Texture::BuiltInMappingChannelFromUnsigned(mapping_channel_id);
    ON_TextureMapping mapping;
    switch (mc_type)
    {
      case ON_Texture::MAPPING_CHANNEL::tc_channel:
      case ON_Texture::MAPPING_CHANNEL::default_channel: { mapping.SetSurfaceParameterMapping(); }
      //case ON_Texture::MAPPING_CHANNEL::screen_based_channel: { mapping.setmapping}
      //case ON_Texture::MAPPING_CHANNEL::wcs_channel: { return 2; }
      //case ON_Texture::MAPPING_CHANNEL::wcs_box_channel: { return 3; }
      //case ON_Texture::MAPPING_CHANNEL::environment_map_box_channel: { return 4; }
      //case ON_Texture::MAPPING_CHANNEL::environment_map_light_probe_channel: { return 5; }
      //case ON_Texture::MAPPING_CHANNEL::environment_map_spherical_channel: { return 6; }
      //case ON_Texture::MAPPING_CHANNEL::environment_map_cube_map_channel: { return 7; }
      //case ON_Texture::MAPPING_CHANNEL::environment_map_vcross_cube_map_channel: { return 8; }
      //case ON_Texture::MAPPING_CHANNEL::environment_map_hcross_cube_map_channel: { return 9; }
      //case ON_Texture::MAPPING_CHANNEL::environment_map_hemispherical_channel: { return 10; }
      //case ON_Texture::MAPPING_CHANNEL::environment_map_emap_channel: { return 11; }
      default: { ASSERT(false); mapping.SetSurfaceParameterMapping(); }
    }
    //auto a = mapping.GetTextureCoordinates()
  }
  // this function doesn't do anything yet.
}

static void SetTextureCoordinatesOnMesh(CRhinoObjectMesh& meshObj, const CRhinoDoc& doc, std::map<int, const ON_TextureCoordinates*>& tcs)
{
  // instead of int as the map key use ON_UUID as a string: ON_UuidToString() and ON_UuidFromString()
  const CRhinoObject* obj = meshObj.m_parent_object;
  ON_Mesh* pMesh = meshObj.m_mesh;
  const ON_MappingRef* pMR = obj->Attributes().m_rendering_attributes.MappingRef(ON_nil_uuid);

  const int count = pMR == nullptr ? 0 : pMR->m_mapping_channels.Count();

  if (count == 0)
  {
    ON_TextureMapping mapping;
    mapping.SetSurfaceParameterMapping();
    // in this case you don't need to do the seam check thing because
    // surface parameter mapping cannot create a seam
    // in this case SetTextureCoordinates doesn't need to be called
    const ON_TextureCoordinates* pTCs = pMesh->SetCachedTextureCoordinatesEx(mapping, &ON_Xform::IdentityTransformation);
    //int idx = mapping.Index(); // zero? probably 1
    //auto pr = std::pair<int, const ON_TextureCoordinates*>(idx, pTCs);
    //ON_wString uuidStr;
    //ON_UuidToString(mapping.Id(), uuidStr);
    //auto pr = std::pair<ON_wString, const ON_TextureCoordinates*>(uuidStr, pTCs);
    //tcs.insert(pr);
    tcs[1] = pTCs;
  }
  else
  {
    // SetTextureCoordinates is obsolete but still needs to be called 
    // before calling SetCachedTextureCoordinatesEx 
    // because it will create all the necessary vertices on the mesh
    // that are needed to properly apply the texture coordinates.
    for (int i = 0; i < count; i++)
    {
      const ON_MappingChannel& mc = pMR->m_mapping_channels[i];
      // mapping_id is what we can use to find the 
      int txMpIdx = doc.m_texture_mapping_table.FindTextureMapping(mc.m_mapping_id);
      if (txMpIdx != -1)
      {
        const ON_TextureMapping& mapping = doc.m_texture_mapping_table[txMpIdx];
        const ON_Xform local_xform = mc.m_object_xform;
        //side effect: changes the mesh vertices
        pMesh->SetTextureCoordinates(mapping, &local_xform);
      }
    }
    for (int i = 0; i < count; i++)
    {
      const ON_MappingChannel& mc = pMR->m_mapping_channels[i];
      int txMpIdx = doc.m_texture_mapping_table.FindTextureMapping(mc.m_mapping_id);
      if (txMpIdx != -1)
      {
        const ON_TextureMapping& mapping = doc.m_texture_mapping_table[txMpIdx];
        const ON_Xform local_xform = mc.m_object_xform;
        const ON_TextureCoordinates* pTCs = pMesh->SetCachedTextureCoordinatesEx(mapping, &local_xform);
        //ON_wString uuidStr;
        //ON_UuidToString(/*mc.m_mapping_id*/mapping.Id(), uuidStr);
        //tcs[uuidStr] = pTCs;
        tcs[mc.m_mapping_channel_id] = pTCs;
      }
    }
  }
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

    std::map<int, const ON_TextureCoordinates*> textureCoordinatesByMappingChannel;
    // this has to be done first, before meshes vertices are read to be exported
    // because setting the texture coordinates can modify the mesh vertices
    SetTextureCoordinatesOnMesh(objectMesh, doc, textureCoordinatesByMappingChannel);

    std::vector<ON_wString> layerNames = GetLayerNames(objectMesh.m_parent_object, doc);
    //todo: check if the m_mesh includes the changed vertices made by the SetTexttureCoordinatesOnMesh call above. If not the object has to be re-read.
    ON_wString meshPath = usdEI.AddMesh(objectMesh.m_mesh, layerNames, textureCoordinatesByMappingChannel);

    const CRhRdkMaterial* pMaterial = objectMesh.m_parent_object->ObjectRdkMaterial(ON_COMPONENT_INDEX::UnsetComponentIndex);
    if (pMaterial)
    {
      ON_Material material = pMaterial->SimulatedMaterial();
      material.ToPhysicallyBased();
      std::shared_ptr<ON_PhysicallyBasedMaterial> pbrMat = material.PhysicallyBased();
      if (pbrMat)
      {
        ON_PhysicallyBasedMaterial& pbr = *pbrMat;
        usdEI.AddAndBindPbrMaterialAndTextures(&pbr, pbrMat->Material().m_textures, layerNames, meshPath);
      }
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

