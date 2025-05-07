#include "stdafx.h"
#include "iostream"
#include <fstream>

#include "UsdShared.h"
#include "ON_Helpers.h"

using namespace pxr;

//todo: I'm sure there's a copy file function that's already available somewhere
void UsdShared::CopyFileTo(const ON_wString& fullFileName, const ON_wString& destination)
{
  ON_wString fileName = ON_FileSystemPath::FileNameFromPath(fullFileName, true);
  ON_wString destFullFileName = destination + fileName;
  std::ifstream  src(ON_Helpers::ON_wString_to_StdString(fullFileName), std::ios::binary);
  std::ofstream  dst(ON_Helpers::ON_wString_to_StdString(destFullFileName),   std::ios::binary);
  dst << src.rdbuf();
}

void UsdShared::CreateUsdzFile(const ON_wString& fullFileNameNoExtension, const std::vector<ON_wString>& filesToInclude)
{
  ON_wString usdaFullFileName = fullFileNameNoExtension + ".usda";
  ON_wString usdaFileName = ON_FileSystemPath::FileNameFromPath(usdaFullFileName, true);
  ON_wString usdzFullFileName = fullFileNameNoExtension + ".usdz";
  UsdZipFileWriter writer = UsdZipFileWriter::CreateNew(ON_Helpers::ON_wString_to_StdString(usdzFullFileName));
  // usda has to be added before textures
  writer.AddFile(ON_Helpers::ON_wString_to_StdString(usdaFullFileName), ON_Helpers::ON_wString_to_StdString(usdaFileName));
  for (ON_wString fullFileName : filesToInclude)
  {
    ON_wString fileName = ON_FileSystemPath::FileNameFromPath(fullFileName, true);
    writer.AddFile(ON_Helpers::ON_wString_to_StdString(fullFileName), ON_Helpers::ON_wString_to_StdString(fileName));
  }
  writer.Save();
}

ON_wString UsdShared::PathWithoutExtension(const ON_wString& fullFileName)
{
  // I didn't see an obvious ON_FileSystemPath way to do this.
  //todo: add typical checks
  return fullFileName.SubString(0, fullFileName.ReverseFind('.'));
}

bool UsdShared::IsAcceptableUsdCharacter(wchar_t c)
{
  if (ON_wString::IsDecimalDigit(c))
    return true;
  if (L'_' == c)
    return true;
  if ((c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z'))
    return true;
  return false;
}

ON_wString UsdShared::RhinoLayerNameToUsd(const ON_wString& rhLayerName, const ON_wString& defaultName)
{
  if (rhLayerName.Length() < 1)
    return defaultName;

  ON_wString rc;
  wchar_t c = rhLayerName[0];
  if (ON_wString::IsDecimalDigit(c))
    rc += ON_wString::Underscore;

  for (int i = 0; i < rhLayerName.Length(); i++)
  {
    c = rhLayerName[i];
    if (IsAcceptableUsdCharacter(c))
      rc += c;
    else
      rc += ON_wString::Underscore;
  }

  return rc;
}

void UsdShared::SetUsdLayersAsXformable(const std::vector<ON_wString>& layerNames, UsdStageRefPtr stage)
{
  ON_wString path;
  for (ON_wString name : layerNames)
  {
    // make sure the layer is activated
    pxr::UsdPrim existingPrim;

    path = path + L"/" + name;
    std::string stdStrPath = ON_Helpers::ON_wString_to_StdString(path);
    existingPrim = stage->GetPrimAtPath(pxr::SdfPath(stdStrPath));
    if (existingPrim)
    {
        if (!existingPrim.IsActive())
        {
            existingPrim.ClearActive();
        }
    }

    //std::cout << "layer: " << stdStrPath << std::endl; //debug
		pxr::UsdGeomXform nextLayerXform = pxr::UsdGeomXform::Define(stage, pxr::SdfPath(stdStrPath));
  }
}

bool UsdShared::IsValidUsdObject(ON::object_type type)
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
      
    default:
      return true;
  }
}

ON::object_type UsdShared::GetTypeFromObject(const CRhinoObject* obj)
{
  switch (obj->ObjectType())
  {
    // Supported Objects
    case ON::object_type::curve_object:
    // case ON::object_type::instance_reference: // TODO : Impliment
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
      
    default:
      return ON::object_type::mesh_object;
  }
}

void UsdShared::SetTextureCoordinatesOnMesh(const CRhinoObject& obj, ON_Mesh* pMesh, const CRhinoDoc* doc, std::map<int, ON_TextureCoordinates>& tcs)
{
  // instead of int as the map key use ON_UUID as a string: ON_UuidToString() and ON_UuidFromString()
  // Jussi: Pass int the default renderer guid
  const ON_MappingRef* pMR = obj.Attributes().m_rendering_attributes.MappingRef(RhinoApp().GetDefaultRenderApp());

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
    // Store a copy of the cached texture coordinate set. Original set gets destroyed if ON_Mesh::m_TC array needs to be reallocated.
    if (nullptr != pTCs)
      tcs[1] = *pTCs;
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
      int txMpIdx = doc->m_texture_mapping_table.FindTextureMapping(mc.m_mapping_id);
      if (txMpIdx != -1)
      {
        const ON_TextureMapping& mapping = doc->m_texture_mapping_table[txMpIdx];
        const ON_Xform local_xform = mc.m_object_xform;
        //side effect: changes the mesh vertices
        pMesh->SetTextureCoordinates(mapping, &local_xform);
      }
    }
    for (int i = 0; i < count; i++)
    {
      const ON_MappingChannel& mc = pMR->m_mapping_channels[i];
      int txMpIdx = doc->m_texture_mapping_table.FindTextureMapping(mc.m_mapping_id);
      if (txMpIdx != -1)
      {
        const ON_TextureMapping& mapping = doc->m_texture_mapping_table[txMpIdx];
        const ON_Xform local_xform = mc.m_object_xform;
        // Jussi: No lazy evaluaion: previously cached values might be out-of-date
        const ON_TextureCoordinates* pTCs = pMesh->SetCachedTextureCoordinatesEx(mapping, &local_xform, false, true);
        ASSERT(pTCs != nullptr && pTCs->m_T.Count() == pMesh->VertexCount());
        //ON_wString uuidStr;
        //ON_UuidToString(/*mc.m_mapping_id*/mapping.Id(), uuidStr);
        //tcs[uuidStr] = pTCs;
        // Store a copy of the cached texture coordinate set. Original set gets destroyed if ON_Mesh::m_TC array needs to be reallocated.
        if (nullptr != pTCs)
          tcs[mc.m_mapping_channel_id] = *pTCs;
      }
    }
  }
}

static void SetBoundingBox(UsdGeomBoundable& boundable, ON_Geometry& obj)
{
  VtVec3fArray extents(2);
  ON_BoundingBox bbox = obj.BoundingBox();
  extents[0].Set((float)bbox.m_min.x, (float)bbox.m_min.y, (float)bbox.m_min.z);
  extents[1].Set((float)bbox.m_max.x, (float)bbox.m_max.y, (float)bbox.m_max.z);
  boundable.GetExtentAttr().Set(extents);
}
