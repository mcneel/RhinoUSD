#include "stdafx.h"

#include "../UsdShared/ON_Helpers.h"
#include "../UsdShared/UsdShared.h"
#include "../UsdShared/UsdPacket.h"

#include "ExportUSDPlugIn.h"
#include "write_usd.h"
#include "UsdExportOptions.h"
#include "UsdExportImport.h"

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

int GetPackets(CRhinoDoc& doc, const CRhinoFileWriteOptions& fileOptions, UsdExportOptions& usdOptions, ON_ClassArray<UsdPacket>& packets)
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

  ON_ClassArray<UsdPacket> meshPackets;
  ON_SimpleArray<const CRhinoObject*> meshObjects;
  CRhinoObjectIterator it(doc.RuntimeSerialNumber(), fileOptions);
  for (const CRhinoObject* obj = it.First(); obj; obj = it.Next())
  {
    // We handle all of the NON-Mesh objects first, then do every mesh object at once because it is simpler.
    const ON_Geometry* geometry = obj->Geometry();
    if (nullptr == geometry)
      continue;
    
    if (!UsdShared::IsValidUsdObject(obj->ObjectType()))
      continue;

    ON::object_type type = UsdShared::GetTypeFromObject(obj);
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

  int mesh_ui_style = CExportUSDPlugIn::ThePlugin().m_saved_mesh_ui_style;
  if (usdOptions.Headless)
    mesh_ui_style = 4;

  if (!MeshPackets(meshPackets, packets, meshObjects, fileOptions.Transformation(), usdOptions.MeshingParams, mesh_ui_style)) return -1;
  if (mesh_ui_style < 2 && mesh_ui_style > 0)
  {
    doc.Redraw(); // clean up display after interactive meshing.
  }

  return 1;
}

int WriteUSDFile(const wchar_t* filename,
  CRhinoDoc& doc,
  ON_ClassArray<UsdPacket>& packets,
  const UsdExportOptions& usdOptions)
{
  double metersPerUnit(doc.ModelUnits().MetersPerUnit(ON_DBL_QNAN));

  const ON_wString fn(filename);
  UsdExportImport usdEI(fn, metersPerUnit, usdOptions, doc);
  for (UsdPacket& packet : packets)
  {
    usdEI.WriteObject(packet, usdOptions);
  }

  if (!usdEI.AnythingToSave())
    return 0;

  usdEI.Save();
  return 1;
}

bool MeshPackets(ON_ClassArray<UsdPacket>& meshPackets,
  ON_ClassArray<UsdPacket>& packets,
  ON_SimpleArray<const CRhinoObject*> meshObjects,
  ON_Xform transform,
  ON_MeshParameters& mp,
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
