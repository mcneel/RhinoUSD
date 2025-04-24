#include "stdafx.h"
#include "ExportUSDPlugIn.h"
#include "../UsdShared/ON_Helpers.h"
#include "../UsdShared/UsdShared.h"
#include "UsdExportOptions.h"
#include "UsdExportPacket.h"

int WriteUSDFile(const wchar_t* filename,
                  bool usda,
                  CRhinoDoc& doc,
                  const CRhinoFileWriteOptions& options,
                  bool scripting,
                  UsdExportOptions& usdOptions)
{
  // TODO : Document why this is necessary
#if defined(ON_RUNTIME_APPLE)
  std::vector<std::string> searchPath;
  NSString* resources = [[NSBundle mainBundle] resourcePath];
  ON_String usd_resource_path = [resources ONString];
  usd_resource_path += "/usd";
  pxr::PlugRegistry::GetInstance().RegisterPlugins(std::string(usd_resource_path.Array()));
#endif

  CRhinoWaitCursor hourglass;
  ON_wString backupname;
  
  int mesh_ui_style = CExportUSDPlugIn::ThePlugin().m_saved_mesh_ui_style;
  ON_MeshParameters mp = CExportUSDPlugIn::ThePlugin().m_saved_mp;

  ON_ClassArray<UsdPacket> packets;
  ON_ClassArray<UsdPacket> meshPackets;
  ON_SimpleArray<const CRhinoObject*> meshObjects;

  CRhinoObjectIterator it(doc.RuntimeSerialNumber(), options);
  for (const CRhinoObject* obj = it.First(); obj; obj = it.Next())
  {
    // We handle all of the NON-Mesh objects first, then do every mesh object at once because it is simpler.
    const ON_Geometry* geometry = obj->Geometry();
    if (nullptr == geometry)
      continue;

    if (!IsValidUsdObject(obj->ObjectType()))
      continue;

    ON::object_type type = GetTypeFromObject(obj);
    if (type == ON::object_type::mesh_object)
    {
      meshObjects.Append(obj);
      meshPackets.Append(UsdPacket(*obj, type));
    }
    else
    {
      packets.Append(UsdPacket(*obj, type));
    }
  }

  if (packets.Count() <= 0 && meshPackets.Count() <= 0)
  {
    RhinoApp().Print(L"No viable objects selected for export.\n");
    return 0;
  }

  const bool useOptionsDictionary = options.OptionsDictionary().Count() > 0;
  if (useOptionsDictionary)
  {
    mesh_ui_style = 4; // no UI // Is 2 not correct?
    const ON_ArchivableDictionary& dict = options.OptionsDictionary();
    GetMeshParametersFromDictionary(dict, mp);
  }

  if (scripting)
  {
    mesh_ui_style = 4;
  }
  
  // Perform Meshing
  ON_ClassArray<CRhinoObjectMesh> mesh_list(meshPackets.Count());
  CRhinoCommand::result rs = RhinoMeshObjects(meshObjects, mp, options.Transformation(), mesh_ui_style, mesh_list);

  if (CRhinoCommand::success != rs) return -1;

  // Save User choices
  if (4 != mesh_ui_style)
  {
    CExportUSDPlugIn::ThePlugin().m_saved_mesh_ui_style = mesh_ui_style;
  }

  CExportUSDPlugIn::ThePlugin().m_saved_mp = mp;
  if (mesh_ui_style < 2)
  {
    // clean up display after interactive meshing.
    doc.Redraw();
  }

  // Push new meshes into mesh packets
  for(int i = 0; i < meshPackets.Count(); i++)
  {
    UsdPacket& meshPacket = meshPackets[i];
    CRhinoObjectMesh& mesh = mesh_list[i];

    UsdPacket& packet = packets.AppendNew();
    packet = meshPacket;
    packet.SetMesh(mesh.m_mesh);
    
    // Transfer Ownership
    mesh.m_mesh = nullptr;
  }

  return WriteUSDFile(filename, doc, packets, usdOptions);
}

int WriteUSDFile(const wchar_t* filename,
  CRhinoDoc& doc,
  ON_ClassArray<UsdPacket>& packets,
  const UsdExportOptions& usdOptions)
{
  double metersPerUnit(doc.ModelUnits().MetersPerUnit(ON_DBL_QNAN));

  const ON_wString fn(filename);
  UsdExportImport usdEI(fn, metersPerUnit);
  for (UsdPacket& packet : packets)
  {
    usdEI.WriteObject(packet, usdOptions);
  }

  if (!usdEI.AnythingToSave())
    return 0;

  usdEI.Save();
  return 1;
}

static bool MeshPackets(ON_ClassArray<UsdPacket>& meshPackets,
                      ON_ClassArray<UsdPacket>& packets,
                      ON_SimpleArray<const CRhinoObject*> meshObjects,
                      ON_Xform transform,
                      ON_MeshParameters mp,
                      int mesh_ui_style)
{
  // Perform Meshing
  ON_ClassArray<CRhinoObjectMesh> mesh_list(meshPackets.Count());
  
  CRhinoCommand::result rs = RhinoMeshObjects(meshObjects, mp, transform, mesh_ui_style, mesh_list);
  if (CRhinoCommand::success != rs) return false;

  // Save User choices
  if (4 != mesh_ui_style)
  {
    CExportUSDPlugIn::ThePlugin().m_saved_mesh_ui_style = mesh_ui_style;
  }

  CExportUSDPlugIn::ThePlugin().m_saved_mp = mp;

  // Push new meshes into mesh packets
  for (int i = 0; i < meshPackets.Count(); i++)
  {
    UsdPacket& meshPacket = meshPackets[i];
    CRhinoObjectMesh& mesh = mesh_list[i];

    UsdPacket& packet = packets.AppendNew();
    packet = meshPacket;
    packet.SetMesh(mesh.m_mesh);

    // Transfer Ownership
    mesh.m_mesh = nullptr;
  }

  return true;
}

static void GetMeshParametersFromDictionary(const ON_ArchivableDictionary& dict, ON_MeshParameters& params)
{
  ON_MeshParameters mp;
  if (dict.TryGetMeshParameters(L"MeshingParameters", mp))
    params = mp;
}

static bool IsValidUsdObject(ON::object_type type)
{
  switch (type)
  {
    // No current fallback or just not a good option
    case ON::object_type::unknown_object_type:
    case ON::object_type::point_object: // TODO : Support
    case ON::object_type::pointset_object:
    case ON::object_type::layer_object:
    case ON::object_type::material_object:
    case ON::object_type::light_object: // TODO : Support
    case ON::object_type::annotation_object:
    case ON::object_type::userdata_object:
    case ON::object_type::instance_definition: // TODO : Support
      // case ON::object_type::instance_reference: // TODO : Support
    case ON::object_type::text_dot:
    case ON::object_type::grip_object:
    case ON::object_type::detail_object:
    case ON::object_type::hatch_object: // TODO : Support
    case ON::object_type::morph_control_object:
    case ON::object_type::loop_object:
    case ON::object_type::brepvertex_filter:
    case ON::object_type::polysrf_filter:
    case ON::object_type::edge_filter:
    case ON::object_type::polyedge_filter:
    case ON::object_type::meshvertex_filter:
    case ON::object_type::meshedge_filter:
    case ON::object_type::meshface_filter:
    case ON::object_type::meshcomponent_reference:
    case ON::object_type::cage_object:
    case ON::object_type::phantom_object:
    case ON::object_type::clipplane_object:
      return false;
  }

  return true;
}

static ON::object_type GetTypeFromObject(const CRhinoObject* obj)
{
  switch (obj->ObjectType())
  {
    // Supported Objects
  case ON::object_type::curve_object:
  case ON::object_type::instance_reference:
    return  obj->ObjectType();
    break;

    /* TODO : Support natively
    case ON::object_type::point_object:
      type = ON::object_type::point_object;
      break;

    case ON::object_type::surface_object:
      if (usdOptions.ForceMeshes)
        type = ON::object_type::mesh_object;
      else
        type = ON::object_type::surface_object;
      break;

    case ON::object_type::brep_object:
      if (usdOptions.ForceMeshes)
        type = ON::object_type::mesh_object;
      else
        type = ON::object_type::brep_object;
      break;

    case ON::object_type::subd_object:
      if (usdOptions.ForceMeshes)
        type = ON::object_type::mesh_object;
      else
        type = ON::object_type::subd_object;
      break;
    */
  }

  return ON::object_type::mesh_object;
}
