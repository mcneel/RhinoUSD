#include "stdafx.h"
#include "iostream"
#include <fstream>
#include <chrono>

#include "../UsdShared/UsdShared.h"
#include "../UsdShared/ON_Helpers.h"

#include "UsdExportOptions.h"
#include "UsdExportImport.h"
#include "write_usd.h"

using namespace pxr;
using namespace std;

UsdExportImport::UsdExportImport(const ON_wString& fileName, double metersPerUnit, const UsdExportOptions& options, CRhinoDoc& doc) :
  UsdOptions(options),
  Doc(doc),

  usdFullFileName(fileName),
  metersPerUnit(metersPerUnit),
  currentMeshIndex(0),
  //currentMaterialIndex(0),
  currentShaderIndex(0),
  currentNurbsCurveIndex(0),
  currentBlockIndex(0),
  tokPreviewSurface("UsdPreviewSurface"),
  tokSurface("surface"),

  // UsdPreviewSurface inputs
  tokDiffuseColor("diffuseColor"),
  tokEmissiveColor("emmisiveColor"),
  tokUseSpecularWorkflow("useSpecularWorkflow"),
  tokMetallic("metallic"),
  tokRoughness("roughness"),
  tokClearcoat("clearcoat"),
  tokOpacity("opacity"),
  tokIor("ior"),
  tokDisplacement("displacement"),
  tokOcclusion("occlusion"),
  Blocks(ON_SimpleArray<ON_wString>(10))
{
  CreateUsdFile();
  SetDefaultPrim();

  // Set the Z up direction for Rhino
  pxr::TfToken upAxis = pxr::UsdGeomTokens->y; // z;
  if (!pxr::UsdGeomSetStageUpAxis(stage, upAxis))
  {
    std::cout << "could not set StageUpAxis";
  };

  if (!pxr::UsdGeomSetStageMetersPerUnit(stage, metersPerUnit))
  {
    std::cout << "could not set StageMetersPerUnit";
  }

}

const ON_wString TempFolder("TEMP_USD");

void UsdExportImport::CreateUsdFile()
{
  ON_wString tempPath;
  if (CRhinoFileUtilities::GetTemporaryPath(tempPath))
  {
    ON_wString tempFolder = ON_FileSystemPath::CombinePaths(tempPath, false, TempFolder, false, false);

    UUID uuid;
    ON_wString fileName;
    ON_CreateUuid(uuid);
    ON_UuidToString(uuid, fileName);

    ON_wString extension = ON_FileSystemPath::FileNameExtensionFromPath(usdFullFileName);
    fileName += extension;

    tempUsdFilePath = ON_FileSystemPath::CombinePaths(tempFolder, false, fileName, true, false);
    
    // TODO : USDZ is odd and should be written after temp files. USDZ needs a different API.
    stage = UsdStage::CreateNew(ON_Helpers::ON_wString_to_StdString(tempUsdFilePath));
  }
}

void UsdExportImport::WriteObject(std::shared_ptr<UsdPacket>& packet, const UsdExportOptions& usdOptions)
{
  switch (packet->Type())
  {
    case ON::object_type::curve_object:
      if (!AddCurve(packet, usdOptions)) return;
      break;

    case ON::object_type::instance_reference:
      if (!AddBlock(packet, usdOptions)) return;
      break;

    default:
      if (!AddMesh(packet, usdOptions)) return;
      break;
  }
}

bool UsdExportImport::AddMesh(std::shared_ptr<UsdPacket> packet, const UsdExportOptions& usdOptions)
{
  ON_Mesh* mesh = packet->Mesh();

  if (!mesh) return false;
  if (packet->Type() != ON::object_type::mesh_object) return false;

  const CRhinoDoc* doc = packet->Object().Document();
  if (!doc) return false;

  std::map<int, ON_TextureCoordinates> textureCoordinatesByMappingChannel;
  // this has to be done first, before meshes vertices are read to be exported
  // because setting the texture coordinates can modify the mesh vertices
  UsdShared::SetTextureCoordinatesOnMesh(packet->Object(), mesh, doc, textureCoordinatesByMappingChannel);

  //todo: check if the m_mesh includes the changed vertices made by the SetTexttureCoordinatesOnMesh call above. If not the object has to be re-read.
  const ON_wString meshName = packet->Object().Attributes().Name();

  std::vector<ON_wString> layerNames = GetLayerNames(packet);
  // AddMeshMaterial();

  ON_Mesh meshCopy(*mesh);
  ON_Helpers::RotateYUp(&meshCopy);

  UsdShared::SetUsdLayersAsXformable(layerNames, stage);
  ON_wString layerNamesPath = ON_Helpers::ON_wString_vector_to_ON_wString_path(layerNames);

  ON_wString meshPath;
  if (meshName.IsEmpty())
    meshPath.Format(L"/Mesh%d", currentMeshIndex++);
  else
  {
    // RhinoLayerNameToUsd function should be renamed to something like On_wStringToValidUsd[Name|String|Path] ...
    ON_wString validMeshName = UsdShared::RhinoLayerNameToUsd(meshName);
    meshPath.Format(L"/%s_Mesh%d", validMeshName.Array(), currentMeshIndex++);
  }
  meshPath = layerNamesPath + meshPath;
  std::string stdStrName = ON_Helpers::ON_wString_to_StdString(meshPath);
  UsdGeomMesh usdMesh = UsdGeomMesh::Define(stage, SdfPath(stdStrName));

  if (!mesh->IsClosed()) {
    usdMesh.CreateDoubleSidedAttr(pxr::VtValue(true), true);
  }

  pxr::VtArray<pxr::GfVec3f> points;
  for (int i = 0; i < meshCopy.m_V.Count(); i++)
  {
    const ON_3fPoint& rhinoPt = meshCopy.m_V[i];
    pxr::GfVec3f pt(rhinoPt.x, rhinoPt.y, rhinoPt.z);
    points.push_back(pt);
  }
  usdMesh.CreatePointsAttr().Set(points);

  pxr::VtArray<int> faceVertexCounts;
  pxr::VtArray<int> faceVertexIndices;
  for (int i = 0; i < meshCopy.m_F.Count(); i++)
  {
    const ON_MeshFace& face = meshCopy.m_F[i];
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

  if (meshCopy.HasVertexNormals())
  {
    pxr::VtArray<pxr::GfVec3f> normals;
    normals.resize(meshCopy.m_N.Count());
    for (int i = 0; i < meshCopy.m_N.Count(); i++)
    {
      ON_3fVector v = meshCopy.m_N[i];
      normals[i] = pxr::GfVec3f(v.x, v.y, v.z);
    }
    usdMesh.CreateNormalsAttr(pxr::VtValue(normals));
  }

  if (meshCopy.HasVertexColors())
  {
    pxr::VtArray<pxr::GfVec3f> colors;
    int colorsCount = meshCopy.m_C.Count();
    for (int i = 0; i < colorsCount; i++)
    {
      ON_Color clr = meshCopy.m_C[i];
      GfVec3f usdClr((float)clr.FractionRed(), (float)clr.FractionGreen(), (float)clr.FractionBlue());
      //std::cout << usdClr << "--" << colors.size() << std::endl;
      colors.push_back(usdClr);
    }
    UsdAttribute cattr = usdMesh.CreateDisplayColorAttr();
    cattr.Set(colors);
  }

  // texture coordinates
  //if (mesh->HasTextureCoordinates())
  //{
  //  //usdMesh.ApplyAPI<pxr::UsdGeomPrimvarsAPI>();
  //  int tcCnt = mesh->m_TC.Count(); //not sure if m_S should be used instead.
  //  for (int i = 0; i < tcCnt; i++)
  //  {
  //    ON_TextureCoordinates tc = mesh->m_TC[i];

  //    auto primvar = usdMesh.GetPrimvar(pxr::TfToken("primvars:st"));
  //    pxr::VtVec2fArray uvValues;
  //    //pxr::VtArray<GfVec2f> uvArray;
  //    if (primvar.Get<pxr::VtVec2fArray>(&uvValues))
  //    {
  //      //pxr::UsdGeomPrimvar pv = pxr::UsdGeomPrimvarsAPI(usdMesh).CreatePrimvar(pxr::TfToken("st"), pxr::SdfValueTypeNames->TexCoord2fArray);
  //	    //pxr::UsdGeomPrimvar pv = usdMesh.CreateAttribute(pxr::TfToken("primvars:st", pxr::TfToken::Immortal), pxr::SdfValueTypeNames->TexCoord2fArray);
  //	    pxr::UsdGeomPrimvar pv = usdMesh.CreateAttribute(pxr::TfToken("primvars:st", pxr::TfToken::Immortal), pxr::SdfValueTypeNames->Float2Array);
  //	    pv.Set(uvValues);
  //	    pv.SetInterpolation(pxr::TfToken("vertex"));
  //    }

  //	  //pxr::UsdGeomPrimvar attr2 = usdMesh.CreatePrimvar(pxr::TfToken("st"), pxr::SdfValueTypeNames->TexCoord2fArray);
  //	  //attr = meshPrim.CreateAttribute(pxr::TfToken("primvars:st", pxr::TfToken::Immortal), pxr::SdfValueTypeNames->Float2Array);
  //
  //	  //attr2.Set(uvArray);
  //	  //attr2.SetInterpolation(pxr::TfToken("vertex"));
  //  }
  //}

  // texture coordinates
  for (auto& tc : textureCoordinatesByMappingChannel)
  {
    // let's just use the 1st one in the array for now
    int mc_id = tc.first;
    const ON_TextureCoordinates* firstTc = &tc.second;
    //if (tcs.size() > 1)
    //  // todo: support multiple channels or report that some were skipped.
    if (firstTc != nullptr)
    {
      ON_SimpleArray<ON_3fPoint> uvwPoints = firstTc->m_T;
      int ayCnt = firstTc->m_T.Count();
      //pseudo: if uvwPoints.Any(p => p.W != 0) then report that 3rd dimension is ignored
      // i guess that W is always ignored

      pxr::VtArray<pxr::GfVec2f> uvArray;
      uvArray.resize(ayCnt); //todo: assert: ayCnt should be the same as the number of vertices on the mesh
      for (int i = 0; i < ayCnt; i++)
      {
        uvArray[i] = pxr::GfVec2f(uvwPoints[i].x, uvwPoints[i].y);
      }

      ON_String sTokenName;
      sTokenName.Format("st%u", mc_id);
      const char* tokenName = sTokenName;
      pxr::UsdGeomPrimvar texCoords = pxr::UsdGeomPrimvarsAPI(usdMesh).CreatePrimvar(pxr::TfToken(tokenName), pxr::SdfValueTypeNames->TexCoord2fArray, pxr::UsdGeomTokens->vertex);
      //texCoords.SetInterpolation(pxr::TfToken("vertex")); //already set in CreatePrimvar
      texCoords.Set(uvArray);
    }
  }

  VtVec3fArray extents(2);
  ON_BoundingBox bbox = meshCopy.BoundingBox();
  extents[0].Set((float)bbox.m_min.x, (float)bbox.m_min.y, (float)bbox.m_min.z);
  extents[1].Set((float)bbox.m_max.x, (float)bbox.m_max.y, (float)bbox.m_max.z);
  usdMesh.GetExtentAttr().Set(extents);

  UsdPrim prim = usdMesh.GetPrim();
  if (usdOptions.IncludeUserStrings && prim.IsValid())
  {
    UsdShared::AddUserDataToPrim(packet, &prim);
  }

  return true;
}

bool UsdExportImport::AddCurve(const std::shared_ptr<UsdPacket> packet, const UsdExportOptions& usdOptions)
{
  if (packet->Type() != ON::object_type::curve_object) return false;

  const ON_Geometry* geometry = packet->Object().Geometry();
  if (!geometry) return false;

  // TODO : Support other types of Curves
  const ON_NurbsCurve* nurbsCurve = ON_NurbsCurve::Cast(geometry);
  if (!nurbsCurve) return false;

  std::vector<ON_wString> layerNames = GetLayerNames(packet);

  ON_wString layerNamesPath = ON_Helpers::ON_wString_vector_to_ON_wString_path(layerNames);

  if (!nurbsCurve)
    return false;

  ON_NurbsCurve nc(*nurbsCurve);
  ON_Helpers::RotateGeometryYUp(&nc);

  ON_wString name;
  name.Format(L"nurbsCurve%d", currentNurbsCurveIndex++);
  name.Format(L"/NurbsCurve%d", currentNurbsCurveIndex++);
  name = layerNamesPath + name;
  std::string stdStrName = ON_Helpers::ON_wString_to_StdString(name);
  pxr::UsdGeomNurbsCurves usdNc = pxr::UsdGeomNurbsCurves::Define(stage, pxr::SdfPath(stdStrName));

  int degree = nurbsCurve->Degree();
  //pxr::VtValue order(degree + 1);
  pxr::VtArray<int> order;
  order.resize(1);
  order[0] = degree + 1;
  usdNc.CreateOrderAttr(pxr::VtValue(order));

  int ctrlPtsCount = nurbsCurve->m_cv_count;
  pxr::VtArray<pxr::GfVec3f> ctrlPts;
  ctrlPts.resize(ctrlPtsCount);
  for (int i = 0; i < ctrlPtsCount; i++)
  {
    ON_3dPoint cp;
    if (nurbsCurve->GetCV(i, cp))
    {
      ctrlPts[i] = pxr::GfVec3f((float)cp.x, (float)cp.y, (float)cp.z);
    }
  }
  usdNc.CreatePointsAttr(pxr::VtValue(ctrlPts));

  pxr::VtArray<int> crvVertexCount;
  crvVertexCount.resize(1);
  crvVertexCount[0] = ctrlPtsCount;
  usdNc.CreateCurveVertexCountsAttr(pxr::VtValue(crvVertexCount));

  std::vector<double> stdKnots;
  int knotCount = nurbsCurve->KnotCount();
  for (int i = 0; i < knotCount; i++)
  {
    double k = nurbsCurve->m_knot[i];
    stdKnots.push_back(k);

    // add 2 superfluous knots, one at each extremity as almost every 3rd party format requires it
    if (i == 0 || i == knotCount - 1)
      stdKnots.push_back(k);
  }
  pxr::VtArray<double> knots;
  knots.resize(stdKnots.size());
  for (int i = 0; i < stdKnots.size(); i++)
    knots[i] = stdKnots[i];
  usdNc.CreateKnotsAttr(pxr::VtValue(knots));

  UsdPrim prim = usdNc.GetPrim();
  if (usdOptions.IncludeUserStrings && prim.IsValid())
  {
    UsdShared::AddUserDataToPrim(packet, &prim);
  }

  return true;
}

// TODO : This is very hard to read and honestly not that good
// TODO : Does this work recursively?
// TOOD : Create a Folder for these files! (Should this offer nesting?)
bool UsdExportImport::AddBlock(const std::shared_ptr<UsdPacket> packet, const UsdExportOptions& usdOptions)
{
  if (packet->Type() != ON::object_type::instance_reference) return false;
  if (usdOptions.Blocks == BlockHandling::Ignore) return false;
  
  CRhinoDoc* doc = packet->Object().Document();
  if (!doc) return false;
  
  const ON_Geometry* geometry = packet->Object().Geometry();
  const ON_InstanceRef* reference = ON_InstanceRef::Cast(geometry);
  
  ON_UUID refId = reference->m_instance_definition_uuid;
  
  int index = doc->m_instance_definition_table.FindInstanceDefinition(refId, true);
  if (index < 0) return false;
  
  const CRhinoInstanceDefinition* definition = doc->m_instance_definition_table[index];
  if (Blocks.Search(definition->Name()) > -1)
  {
    return true;
  }
  
  ON_wString rootDirectory = ON_FileSystemPath::DirectoryFromPath(usdFullFileName);
  ON_wString fileExtension = ON_FileSystemPath::FileNameExtensionFromPath(usdFullFileName);
  
  ON_wString blockFileName(definition->Name());
  blockFileName += fileExtension;
  
  ON_wString blockFilePath = ON_FileSystemPath::CombinePaths(rootDirectory, false, blockFileName, true, false);
  
  ON_ClassArray<std::shared_ptr<UsdPacket>> packets(0);
  GetInstancePackets(definition, packets);
  
  if (usdOptions.Blocks == BlockHandling::SeparateFiles)
  {
    int returnValue = WriteUSDFile(blockFilePath.Array(), *doc, packets, usdOptions);
    if (returnValue < 0) return false;
  }
  else
  {
    for (std::shared_ptr<UsdPacket> packet : packets)
    {
      WriteObject(packet, usdOptions);
    }
  }
  
  Blocks.Append(definition->Name());
  
  std::vector<ON_wString> layerNames = GetLayerNames(packet);
  ON_wString blockPrimPath = ON_Helpers::ON_wString_vector_to_ON_wString_path(layerNames);
  ON_wString blockPrimRefPath(blockPrimPath);
  
  ON_wString blockPrimName;
  blockPrimName.Format(L"/BlockInstance%d", currentBlockIndex++);
  blockPrimPath += blockPrimName;
  
  // TOOD : Make Component
  // https://openusd.org/release/glossary.html#usdglossary-assetinfo
  UsdGeomXform instanceForm = UsdGeomXform::Define(stage, SdfPath(ON_Helpers::ON_wString_to_StdString(blockPrimPath)));
  
  UsdPrim prim = instanceForm.GetPrim();
  
  if (usdOptions.Blocks == BlockHandling::SeparateFiles)
  {
    const pxr::SdfPath path(ON_Helpers::ON_wString_to_StdString(blockPrimRefPath));
    const std::string refString(ON_Helpers::ON_wString_to_StdString(blockFilePath));
    
    // Set Kind -> Causes issues
    //  pxr::UsdEditTarget().MapToSpecPath(path);
    //  pxr::UsdModelAPI modelApi = pxr::UsdModelAPI();
    //  modelApi.SetKind(pxr::KindTokens->assembly);
    
    prim.SetInstanceable(true);
    
    pxr::UsdReferences references = prim.GetReferences();
    references.AddReference(ON_Helpers::ON_wString_to_StdString(blockFileName), path);
  }

  // https://openusd.org/release/glossary.html#usdglossary-assetinfo
  // https://github.com/ColinKennedy/USD-Cookbook/tree/master/features/asset_info
  pxr::VtDictionary vtDict(4);
  vtDict.SetValueAtPath("identifier", pxr::VtValue(ON_Helpers::ON_wString_to_StdString(blockFileName)));
  vtDict.SetValueAtPath("name", pxr::VtValue(ON_Helpers::ON_wString_to_StdString(definition->Name())));
  vtDict.SetValueAtPath("version", pxr::VtValue(ON_Helpers::ON_UUID_to_StdString(refId)));
  // TODO : Include embedded block paths?
  // vtDict.SetValueAtPath("payloadAssetDependencies", pxr::VtValue());
  prim.SetAssetInfo(vtDict);
  prim.SetAssetInfoByKey(pxr::TfToken("id"), pxr::VtValue("example"));
//  
//  pxr::SdfSubLayerProxy stack = stage->GetRootLayer()->GetSubLayerPaths();
//  stack.push_back(ON_Helpers::ON_wString_to_StdString(blockFileName));
//  

  // TODO : Set Transform!
  // reference->m_xform
  // pxr::UsdGeomXformOp op = instanceForm.AddTransformOp();

  if (usdOptions.IncludeUserStrings && prim.IsValid())
  {
    UsdShared::AddUserDataToPrim(packet, &prim);
  }

  return true;
}

// TODO : Use Smart Pointers
void UsdExportImport::GetInstancePackets(const CRhinoInstanceDefinition* definition, ON_ClassArray<std::shared_ptr<UsdPacket>>& packets)
{
  ObjectArray objects;
  definition->GetObjects(objects);

  const CRhinoFileWriteOptions fileOptions;

  GetPacketsFromCRhinoObjects(objects, fileOptions, packets);
}

bool UsdExportImport::GetPacketsFromCRhinoObjects(ObjectArray& objects, const CRhinoFileWriteOptions& fileOptions, ON_ClassArray<std::shared_ptr<UsdPacket>>& packets, int mesh_ui_style)
{
  CRhinoWaitCursor hourglass;
  ON_wString backupname;

  ON_ClassArray<std::shared_ptr<UsdPacket>> meshPackets;
  ON_SimpleArray<const CRhinoObject*> meshObjects;
  for (const CRhinoObject* obj : objects)
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
      meshPackets.Append(std::make_shared<UsdPacket>(*obj, type));
    }
    else
    {
      packets.Append(std::make_unique<UsdPacket>(*obj, type));
    }
  }

  if (packets.Count() <= 0 && meshPackets.Count() <= 0)
  {
    return false;
  }

  ON_MeshParameters params = UsdOptions.MeshingParams;
  if (!MeshPackets(meshPackets, packets, meshObjects, fileOptions.Transformation(), params, mesh_ui_style)) return false;
  if (mesh_ui_style < 2 && mesh_ui_style > 0)
  {
    Doc.Redraw(); // clean up display after interactive meshing.
  }

  // TODO : Add meshPackets to packets

  return false;
}


const ON_Mesh& UsdExportImport::GetMeshFromSubD(ON_SubD& subD, const ON_MeshParameters mp)
{
  int mesh_density = 5;
  ON_SubDDisplayParameters limit_mesh_parameters = ON_SubDDisplayParameters::CreateFromDisplayDensity(mp.MeshDensity());
  return *subD.GetSurfaceMesh(limit_mesh_parameters, nullptr);
}

void UsdShared::AddUserDataToPrim(const std::shared_ptr<UsdPacket> packet, pxr::UsdPrim* prim)
{
  const CRhinoObjectAttributes& attributes = packet->Object().Attributes();

  ON_ClassArray<ON_UserString> user_strings;
  attributes.GetUserStrings(user_strings);

  /* TODO : Do these 2 need to be supported?
  ON_ClassArray<ON_UserString> object_user_strings;
  packet->Object().GetUserStrings(object_user_strings);

  if (packet->Object().Geometry())
  {
    ON_ClassArray<ON_UserString> geoemtry_user_strings;
    packet->Object().Geometry()->GetUserStrings(geoemtry_user_strings);
  }
  */

  for (const ON_UserString& user_string : user_strings)
  {
    const ON_wString& keyString(user_string.m_key);
    ON_String utf8_key(keyString);
    const std::string utf8_string(utf8_key.Array());
    TfToken key(utf8_string);

    const ON_wString& valueString(user_string.m_string_value);
    ON_String utf8_value(valueString);
    const std::string utf8_value_string(utf8_value.Array());
    VtValue value(utf8_value_string);

    prim->SetCustomDataByKey(key, value);
  }

  // attributes.GetUserData()
}

pxr::TfToken UsdExportImport::TextureTypeToUsdPbrPropertyTfToken(ON_Texture::TYPE& type)
{
  switch (type)
  {
    //case ON_Texture::TYPE::bitmap_texture:
    //case ON_Texture::TYPE::diffuse_texture:
  case ON_Texture::TYPE::pbr_base_color_texture: { return tokDiffuseColor; break; }

  case ON_Texture::TYPE::pbr_emission_texture: { return tokEmissiveColor; break; }

                                             // Andy say "don't use either of these" which makes sense because Rhino will never support specular workflow.
                                             //case ON_Texture::TYPE::pbr_specular_texture: { return TfToken("pbr_specular_texture"); break; }
                                             //case ON_Texture::TYPE::pbr_specular_tint_texture: { return TfToken("pbr_specular_tint_texture"); break; }

  case ON_Texture::TYPE::pbr_metallic_texture: { return tokMetallic; break; }

  case ON_Texture::TYPE::pbr_roughness_texture: { return tokRoughness; break; }

  case ON_Texture::TYPE::pbr_clearcoat_texture: { return tokClearcoat; break; }

                                              //case ON_Texture::TYPE::opacity_texture:
  case ON_Texture::TYPE::transparency_texture: { return tokOpacity; break; }

  case ON_Texture::TYPE::pbr_opacity_ior_texture: { return tokIor; break; }

                                                //case ON_Texture::TYPE::bump_texture: { return tokDiffuseColor; break; }

  case ON_Texture::TYPE::pbr_displacement_texture: { return tokDisplacement; break; }

  case ON_Texture::TYPE::pbr_ambient_occlusion_texture: { return tokOcclusion; break; }

                                                      // don't know what to do with the rest of these.
                                                      //case ON_Texture::TYPE::pbr_subsurface_texture: { return TfToken("pbr_subsurface_texture"); break; }
                                                      //case ON_Texture::TYPE::pbr_subsurface_scattering_texture: { return TfToken("pbr_subsurface_scattering_texture"); break; }
                                                      //case ON_Texture::TYPE::pbr_subsurface_scattering_radius_texture: { return TfToken("pbr_subsurface_scattering_radius_texture"); break; }
                                                      //case ON_Texture::TYPE::pbr_anisotropic_texture: { return TfToken("pbr_anisotropic_texture"); break; }
                                                      //case ON_Texture::TYPE::pbr_anisotropic_rotation_texture: { return TfToken("pbr_anisotropic_rotation_texture"); break; }
                                                      //case ON_Texture::TYPE::pbr_sheen_texture: { return TfToken("pbr_sheen_texture"); break; }
                                                      //case ON_Texture::TYPE::pbr_sheen_tint_texture: { return TfToken("pbr_sheen_tint_texture"); break; }
                                                      //case ON_Texture::TYPE::pbr_clearcoat_roughness_texture: { return TfToken("pbr_clearcoat_roughness_texture"); break; }
                                                      //case ON_Texture::TYPE::pbr_opacity_roughness_texture: { return TfToken("pbr_opacity_roughness_texture"); break; }
                                                      //case ON_Texture::TYPE::pbr_smudge_texture: { return TfToken("pbr_smudge_texture"); break; }
                                                      //case ON_Texture::TYPE::pbr_clearcoat_bump_texture: { return TfToken("pbr_clearcoat_bump_texture"); break; }
                                                      //case ON_Texture::TYPE::pbr_alpha_texture: { return tokAlpha; break; }
                                                      //case ON_Texture::TYPE::emap_texture: { return TfToken("emap_texture"); break; }
  default: { return pxr::TfToken(""); break; }
  }
}

ON_wString UsdExportImport::AddMesh(const ON_Mesh* mesh, const ON_wString meshName, const std::vector<ON_wString>& layerNames, const std::map<int, ON_TextureCoordinates>& tcs)
{
  ON_Mesh meshCopy(*mesh);
  ON_Helpers::RotateYUp(&meshCopy);

  UsdShared::SetUsdLayersAsXformable(layerNames, stage);
  ON_wString layerNamesPath = ON_Helpers::ON_wString_vector_to_ON_wString_path(layerNames);

  ON_wString meshPath;
  if (meshName.IsEmpty())
    meshPath.Format(L"/mesh%d", currentMeshIndex++);
  else
  {
    // RhinoLayerNameToUsd function should be renamed to something like On_wStringToValidUsd[Name|String|Path] ...
    ON_wString validMeshName = UsdShared::RhinoLayerNameToUsd(meshName);
    meshPath.Format(L"/%s_mesh%d", validMeshName.Array(), currentMeshIndex++);
  }
  meshPath = layerNamesPath + meshPath;
  std::string stdStrName = ON_Helpers::ON_wString_to_StdString(meshPath);
  UsdGeomMesh usdMesh = UsdGeomMesh::Define(stage, SdfPath(stdStrName));

  if (!mesh->IsClosed()) {
    usdMesh.CreateDoubleSidedAttr(pxr::VtValue(true), true);
  }

  pxr::VtArray<pxr::GfVec3f> points;
  for (int i = 0; i < meshCopy.m_V.Count(); i++)
  {
    const ON_3fPoint& rhinoPt = meshCopy.m_V[i];
    pxr::GfVec3f pt(rhinoPt.x, rhinoPt.y, rhinoPt.z);
    points.push_back(pt);
  }
  usdMesh.CreatePointsAttr().Set(points);

  pxr::VtArray<int> faceVertexCounts;
  pxr::VtArray<int> faceVertexIndices;
  for (int i = 0; i < meshCopy.m_F.Count(); i++)
  {
    const ON_MeshFace& face = meshCopy.m_F[i];
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

  if (meshCopy.HasVertexNormals())
  {
    pxr::VtArray<pxr::GfVec3f> normals;
    normals.resize(meshCopy.m_N.Count());
    for (int i = 0; i < meshCopy.m_N.Count(); i++)
    {
      ON_3fVector v = meshCopy.m_N[i];
      normals[i] = pxr::GfVec3f(v.x, v.y, v.z);
    }
    usdMesh.CreateNormalsAttr(pxr::VtValue(normals));
  }

  if (meshCopy.HasVertexColors())
  {
    pxr::VtArray<pxr::GfVec3f> colors;
    int colorsCount = meshCopy.m_C.Count();
    for (int i = 0; i < colorsCount; i++)
    {
      ON_Color clr = meshCopy.m_C[i];
      GfVec3f usdClr((float)clr.FractionRed(), (float)clr.FractionGreen(), (float)clr.FractionBlue());
      //std::cout << usdClr << "--" << colors.size() << std::endl;
      colors.push_back(usdClr);
    }
    UsdAttribute cattr = usdMesh.CreateDisplayColorAttr();
    cattr.Set(colors);
  }

  // texture coordinates
  //if (mesh->HasTextureCoordinates())
  //{
  //  //usdMesh.ApplyAPI<pxr::UsdGeomPrimvarsAPI>();
  //  int tcCnt = mesh->m_TC.Count(); //not sure if m_S should be used instead.
  //  for (int i = 0; i < tcCnt; i++)
  //  {
  //    ON_TextureCoordinates tc = mesh->m_TC[i];

  //    auto primvar = usdMesh.GetPrimvar(pxr::TfToken("primvars:st"));
  //    pxr::VtVec2fArray uvValues;
  //    //pxr::VtArray<GfVec2f> uvArray;
  //    if (primvar.Get<pxr::VtVec2fArray>(&uvValues))
  //    {
  //      //pxr::UsdGeomPrimvar pv = pxr::UsdGeomPrimvarsAPI(usdMesh).CreatePrimvar(pxr::TfToken("st"), pxr::SdfValueTypeNames->TexCoord2fArray);
  //	    //pxr::UsdGeomPrimvar pv = usdMesh.CreateAttribute(pxr::TfToken("primvars:st", pxr::TfToken::Immortal), pxr::SdfValueTypeNames->TexCoord2fArray);
  //	    pxr::UsdGeomPrimvar pv = usdMesh.CreateAttribute(pxr::TfToken("primvars:st", pxr::TfToken::Immortal), pxr::SdfValueTypeNames->Float2Array);
  //	    pv.Set(uvValues);
  //	    pv.SetInterpolation(pxr::TfToken("vertex"));
  //    }

  //	  //pxr::UsdGeomPrimvar attr2 = usdMesh.CreatePrimvar(pxr::TfToken("st"), pxr::SdfValueTypeNames->TexCoord2fArray);
  //	  //attr = meshPrim.CreateAttribute(pxr::TfToken("primvars:st", pxr::TfToken::Immortal), pxr::SdfValueTypeNames->Float2Array);
  //
  //	  //attr2.Set(uvArray);
  //	  //attr2.SetInterpolation(pxr::TfToken("vertex"));
  //  }
  //}

  // texture coordinates
  for (auto& tc : tcs)
  {
    // let's just use the 1st one in the array for now
    int mc_id = tc.first;
    const ON_TextureCoordinates* firstTc = &tc.second;
    //if (tcs.size() > 1)
    //  // todo: support multiple channels or report that some were skipped.
    if (firstTc != nullptr)
    {
      ON_SimpleArray<ON_3fPoint> uvwPoints = firstTc->m_T;
      int ayCnt = firstTc->m_T.Count();
      //pseudo: if uvwPoints.Any(p => p.W != 0) then report that 3rd dimension is ignored
      // i guess that W is always ignored

      pxr::VtArray<pxr::GfVec2f> uvArray;
      uvArray.resize(ayCnt); //todo: assert: ayCnt should be the same as the number of vertices on the mesh
      for (int i = 0; i < ayCnt; i++)
      {
        uvArray[i] = pxr::GfVec2f(uvwPoints[i].x, uvwPoints[i].y);
      }

      ON_String sTokenName;
      sTokenName.Format("st%u", mc_id);
      const char* tokenName = sTokenName;
      pxr::UsdGeomPrimvar texCoords = pxr::UsdGeomPrimvarsAPI(usdMesh).CreatePrimvar(pxr::TfToken(tokenName), pxr::SdfValueTypeNames->TexCoord2fArray, pxr::UsdGeomTokens->vertex);
      //texCoords.SetInterpolation(pxr::TfToken("vertex")); //already set in CreatePrimvar
      texCoords.Set(uvArray);
    }
  }

  VtVec3fArray extents(2);
  ON_BoundingBox bbox = meshCopy.BoundingBox();
  extents[0].Set((float)bbox.m_min.x, (float)bbox.m_min.y, (float)bbox.m_min.z);
  extents[1].Set((float)bbox.m_max.x, (float)bbox.m_max.y, (float)bbox.m_max.z);
  usdMesh.GetExtentAttr().Set(extents);

  return meshPath;
}

void UsdExportImport::AddMaterialWithTexturesIfNotAlreadyAdded(unsigned int docSerNo, const ON_UUID& matId, const ON_wString& matName, const ON_PhysicallyBasedMaterial* pbrMaterial, const ON_ObjectArray<ON_Texture>& textures)
{
  std::string matIdStr(ON_Helpers::ON_UUID_to_StdString(matId));
  if (materialsAddedToScene.count(matIdStr) == 1)
    return;

  ON_wString matIdOnStr = UsdShared::RhinoLayerNameToUsd(ON_Helpers::ON_UUID_to_ON_wString(matId));

  ON_wString material_name;
  material_name.Format(L"material_%s_%s", matName.Array(), matIdOnStr.Array());
  material_name = UsdShared::RhinoLayerNameToUsd(material_name);

  const ON_wString matPath = L"/Rhino/Materials";
  ON_wString full_material_name;
  full_material_name.Format(L"%s/%s", matPath.Array(), material_name.Array());

  pxr::UsdShadeMaterial usdMaterial = pxr::UsdShadeMaterial::Define(stage, pxr::SdfPath(ON_Helpers::ON_wString_to_StdString(full_material_name)));
  materialsAddedToScene[matIdStr] = full_material_name;

  ON_wString shaderName;
  shaderName.Format(L"%s/shader%d", full_material_name.Array(), currentShaderIndex++);
  std::string stdStrShaderName = ON_Helpers::ON_wString_to_StdString(shaderName);
  pxr::UsdShadeShader shader = pxr::UsdShadeShader::Define(stage, pxr::SdfPath(stdStrShaderName));
  shader.CreateIdAttr(pxr::VtValue(tokPreviewSurface));


  // UsdPreviewSurface inputs - BEGIN
  // https://openusd.org/release/spec_usdpreviewsurface.html

  ON_4fColor color = pbrMaterial->BaseColor();
  pxr::GfVec3f diffuseColor(color.Red(), color.Green(), color.Blue());
  shader.CreateInput(tokDiffuseColor, pxr::SdfValueTypeNames->Color3f).Set(diffuseColor);

  ON_4fColor e(pbrMaterial->Emission());
  pxr::GfVec3f emission(e.Red(), e.Green(), e.Blue());
  shader.CreateInput(tokEmissiveColor, pxr::SdfValueTypeNames->Color3f).Set(emission);

  // hard coded to 0. Andy: "speculay workflow will never be supported in Rhino"
  shader.CreateInput(tokUseSpecularWorkflow, pxr::SdfValueTypeNames->Int).Set(0);

  // "specularColor" would only be set if useSpecularWorkflow was 1 which 
  // never occurs because it's hard coded to 0

  // only set metallic if useSpecularWorkflow is 0 which it always is as it
  // is hardcoded
  float metallic = (float)(pbrMaterial->Metallic());
  shader.CreateInput(tokMetallic, pxr::SdfValueTypeNames->Float).Set(metallic);

  float roughness = (float)(pbrMaterial->Roughness());
  shader.CreateInput(tokRoughness, pxr::SdfValueTypeNames->Float).Set(roughness);

  float clearcoat = (float)(pbrMaterial->Clearcoat());
  shader.CreateInput(tokClearcoat, pxr::SdfValueTypeNames->Float).Set(clearcoat);

  //@todo: "clearcoatRoughness"

  float opacity = (float)(pbrMaterial->Opacity());
  shader.CreateInput(tokOpacity, pxr::SdfValueTypeNames->Float).Set(opacity);

  // "opacityThreshold" : Andy says to ignore

  // there is also ReflectiveIOR() but Andy says that OpacityIOR is the correct one to use
  float rior = (float)(pbrMaterial->OpacityIOR());
  shader.CreateInput(tokIor, pxr::SdfValueTypeNames->Float).Set(rior);

  //@todo: "normal" : Andy says: "This is related to bump. We might have to think carefully about this"

  // even though there is no pbr input mapped to this yet there could be a texture: ON_Texture::TYPE::pbr_displacement_texture
  //@todo: "displacement"

  // even though there is no pbr input mapped to this yet there could be a texture: ON_Texture::TYPE::pbr_ambient_occlusion_texture
  //@todo: "occlusion"

  // UsdPreviewSurface inputs - END


  usdMaterial.CreateSurfaceOutput().ConnectToSource(shader.ConnectableAPI(), tokSurface);

  std::map<int, pxr::UsdShadeShader> stReaders;

  const int texture_count = textures.Count();
  for (int i = 0; i < texture_count; i++)
  {
    ON_Texture t = textures[i];
    ON_Texture::TYPE tt = t.m_type;
    ON_wString textureFullFileName = t.m_image_file_reference.FullPath();
    const wchar_t* tffnPtr = textureFullFileName.Array();
    CRhinoFileUtilities::FindFile(docSerNo, tffnPtr, textureFullFileName);
    filesInExport.push_back(textureFullFileName);

    pxr::TfToken pbrParam = this->TextureTypeToUsdPbrPropertyTfToken(tt);
    if (pbrParam.IsEmpty()) {
      // skip this texture
      continue;
    }
    ON_wString textureFullName;
    ON_wString ttStr(ON_Helpers::ON_TextureTYPE_to_ON_wString(tt));
    textureFullName.Format(L"%s/texture_%s", full_material_name.Array(), ttStr.Array());
    ON_wString textureName;
    textureName.Format(L"texture_%s", ttStr.Array());
    std::string stdTextureName = ON_Helpers::ON_wString_to_StdString(textureName);

    pxr::UsdShadeShader usdUVTextureSampler = UsdShadeShader::Define(stage, pxr::SdfPath(ON_Helpers::ON_wString_to_StdString(textureFullName)));
    usdUVTextureSampler.CreateIdAttr(pxr::VtValue(pxr::TfToken("UsdUVTexture")));

    std::string textureFileName = "./" + ON_Helpers::ON_wString_to_StdString(ON_FileSystemPath::FileNameFromPath(textureFullFileName, true));
    usdUVTextureSampler.CreateInput(TfToken("file"), pxr::SdfValueTypeNames->Asset).Set(pxr::SdfAssetPath(textureFileName));

    // Mapping channel is always strictly positive (zero is sometimes used as the default but it should be one).
    const int mappingChannel = t.m_mapping_channel_id < 1 ? 1 : t.m_mapping_channel_id;
    // Get primvar reader for the mapping channel
    auto strIt = stReaders.find(t.m_mapping_channel_id);
    if (strIt == stReaders.end())
    {
      ON_wString stReaderName;
      stReaderName.Format(L"%s/stReader%u", full_material_name.Array(), mappingChannel);
      stReaders[mappingChannel] = pxr::UsdShadeShader::Define(stage, pxr::SdfPath(ON_Helpers::ON_wString_to_StdString(stReaderName)));
      stReaders[mappingChannel].CreateIdAttr(pxr::VtValue(pxr::TfToken("UsdPrimvarReader_float2")));
      strIt = stReaders.find(mappingChannel);
    }
    pxr::UsdShadeShader& stReader = strIt->second;

    ON_2dVector v = t.Repeat();
    double scalex = v.x;
    double scaley = v.y;
    double rotation = t.Rotation();

    if (scalex != 1.0 || scaley != 1.0 || rotation != 0.0) {
      ON_wString transformFullName;
      transformFullName.Format(L"%s/transform2d", textureFullName.Array());
      pxr::UsdShadeShader transform2d = UsdShadeShader::Define(stage, pxr::SdfPath(ON_Helpers::ON_wString_to_StdString(transformFullName)));
      transform2d.CreateIdAttr(pxr::VtValue(pxr::TfToken("UsdTransform2d")));
      transform2d.CreateInput(TfToken("in"), SdfValueTypeNames->Float2).ConnectToSource(stReader.ConnectableAPI(), TfToken("result"));
      pxr::UsdShadeInput rotationInput = transform2d.CreateInput(TfToken("rotation"), SdfValueTypeNames->Float);
      rotationInput.Set((float)rotation);
      pxr::GfVec2f scaleVec((float)scalex, (float)scaley);
      pxr::UsdShadeInput scaleInput = transform2d.CreateInput(TfToken("scale"), SdfValueTypeNames->Float2);
      scaleInput.Set(scaleVec);
      usdUVTextureSampler.CreateInput(TfToken("st"), pxr::SdfValueTypeNames->Float2).ConnectToSource(transform2d.ConnectableAPI(), TfToken("result"));
    }
    else
    {
      usdUVTextureSampler.CreateInput(TfToken("st"), pxr::SdfValueTypeNames->Float2).ConnectToSource(stReader.ConnectableAPI(), TfToken("result"));
    }

    //todo: if (t.m_mapping_channel_id <> 1 /*or 0*/) append id to "st"
    //todo: "rgb" is probably only for colors like diffuseColor. What should it be for other props?
    usdUVTextureSampler.CreateOutput(TfToken("rgb"), pxr::SdfValueTypeNames->Float3);
    //todo: same here. typeNames->Color3f is correct for diffuseColor but not for most other pbrParam
    shader.CreateInput(pbrParam, pxr::SdfValueTypeNames->Color3f).ConnectToSource(usdUVTextureSampler.ConnectableAPI(), TfToken("rgb"));
  }

  // primvar for texture mapping coordinates
  for (auto& stReaderIt : stReaders)
  {
    ON_String strInputName;
    strInputName.Format("stPrimvarName%u", stReaderIt.first);
    pxr::UsdShadeInput stInput = usdMaterial.CreateInput(TfToken(strInputName), SdfValueTypeNames->String);
    ON_String strPrimVarname;
    strPrimVarname.Format("st%u", stReaderIt.first);
    stInput.Set((const char*)strPrimVarname);
    stReaderIt.second.CreateInput(TfToken("varname"), SdfValueTypeNames->String).ConnectToSource(stInput);
  }
}

void UsdExportImport::BindPbrMaterialToMesh(const ON_UUID& matId, const ON_wString meshPath)
{
  std::string strMeshPath = ON_Helpers::ON_wString_to_StdString(meshPath);
  pxr::SdfPath mp(strMeshPath);
  pxr::UsdPrim mesh = stage->GetPrimAtPath(mp);

  std::string matIdStr(ON_Helpers::ON_UUID_to_StdString(matId));
  pxr::UsdPrim material = stage->GetPrimAtPath(pxr::SdfPath(ON_Helpers::ON_wString_to_StdString(materialsAddedToScene.at(matIdStr))));
  pxr::UsdShadeMaterial usdMaterial = pxr::UsdShadeMaterial(material);

  mesh.ApplyAPI<pxr::UsdShadeMaterialBindingAPI>();
  pxr::UsdGeomMesh usdMesh = pxr::UsdGeomMesh(mesh);
  pxr::UsdShadeMaterialBindingAPI(usdMesh).Bind(usdMaterial);
}

void UsdExportImport::AddNurbsCurve(const ON_NurbsCurve* nurbsCurve, const std::vector<ON_wString>& layerNames)
{
  ON_wString layerNamesPath = ON_Helpers::ON_wString_vector_to_ON_wString_path(layerNames);

  if (nullptr == nurbsCurve)
    return;

  ON_NurbsCurve nc(*nurbsCurve);
  ON_Helpers::RotateGeometryYUp(&nc);

  ON_wString name;
  name.Format(L"nurbsCurve%d", currentNurbsCurveIndex++);
  name = layerNamesPath + name;
  std::string stdStrName = ON_Helpers::ON_wString_to_StdString(name);
  pxr::UsdGeomNurbsCurves usdNc = pxr::UsdGeomNurbsCurves::Define(stage, pxr::SdfPath(stdStrName));

  int degree = nurbsCurve->Degree();
  //pxr::VtValue order(degree + 1);
  pxr::VtArray<int> order;
  order.resize(1);
  order[0] = degree + 1;
  usdNc.CreateOrderAttr(pxr::VtValue(order));

  int ctrlPtsCount = nurbsCurve->m_cv_count;
  pxr::VtArray<pxr::GfVec3f> ctrlPts;
  ctrlPts.resize(ctrlPtsCount);
  for (int i = 0; i < ctrlPtsCount; i++)
  {
    ON_3dPoint cp;
    if (nurbsCurve->GetCV(i, cp))
    {
      ctrlPts[i] = pxr::GfVec3f((float)cp.x, (float)cp.y, (float)cp.z);
    }
  }
  usdNc.CreatePointsAttr(pxr::VtValue(ctrlPts));

  pxr::VtArray<int> crvVertexCount;
  crvVertexCount.resize(1);
  crvVertexCount[0] = ctrlPtsCount;
  usdNc.CreateCurveVertexCountsAttr(pxr::VtValue(crvVertexCount));

  std::vector<double> stdKnots;
  int knotCount = nurbsCurve->KnotCount();
  for (int i = 0; i < knotCount; i++)
  {
    double k = nurbsCurve->m_knot[i];
    stdKnots.push_back(k);

    // add 2 superfluous knots, one at each extremity as almost every 3rd party format requires it
    if (i == 0 || i == knotCount - 1)
      stdKnots.push_back(k);
  }
  pxr::VtArray<double> knots;
  knots.resize(stdKnots.size());
  for (int i = 0; i < stdKnots.size(); i++)
    knots[i] = stdKnots[i];
  usdNc.CreateKnotsAttr(pxr::VtValue(knots));
}

void UsdExportImport::AddNurbsSurface(const ON_NurbsSurface* nurbsSurface, const std::vector<ON_wString>& layerNames)
{
  //ON_wString layerNamesPath = ON_Helpers::StringVectorToPath(layerNames);

  //if (nullptr == nurbsSurface)
  //  return;
  //
  //ON_NurbsSurface ns(*nurbsSurface);
  //ON_Helpers::RotateGeometryYUp(&ns);

  //ON_wString name;
  //name.Format(L"nurbsCurve%d", currentNurbsCurveIndex++);
  //name = layerNamesPath + name;
  //std::string stdStrName = ON_Helpers::ON_wStringToStdString(name);

  //pxr::UsdGeomNurbsPatch usdNurbsSurface = pxr::UsdGeomNurbsPatch::Define(stage, pxr::SdfPath(stdStrName));

  //// continue ...
}

bool UsdExportImport::AnythingToSave()
{
  return currentMeshIndex > 0 ||
        !materialsAddedToScene.empty() ||
        currentNurbsCurveIndex > 0 ||
        currentBlockIndex > 0;
}

void UsdExportImport::SetDefaultPrim()
{
  ON_wString rootPath("/");
  rootPath += UsdOptions.RootLayer;
  
  const pxr::SdfPath path(ON_Helpers::ON_wString_to_StdString(UsdOptions.RootLayer));
  const pxr::UsdPrim prim = stage->DefinePrim(path);
  
  stage->SetDefaultPrim(prim);
  // stage->GetRootLayer()->SetDefaultPrim(pxr::TfToken(ON_Helpers::ON_wString_to_StdString(UsdOptions.RootLayer)));
}

void UsdExportImport::SetAuthorMetadata()
{
  pxr::UsdPrim defaultPrim = stage->GetDefaultPrim();
  
  if (pxr::UsdAttribute dateAttribute = defaultPrim.CreateAttribute(pxr::TfToken("date"), pxr::SdfValueTypeNames->String))
  {
    const std::chrono::system_clock::time_point& now = std::chrono::system_clock::now();
    const std::time_t tt = std::chrono::system_clock::to_time_t(now);
      
    dateAttribute.Set(pxr::VtValue(std::ctime(&tt)));
  }
}

void UsdExportImport::Save()
{
  stage->Save();
  UsdShared::CopyFileTo(tempUsdFilePath, usdFullFileName);
  
  ON_FileSystem::RemoveFile(tempUsdFilePath.Array());
  return;
}

std::vector<ON_wString> UsdExportImport::GetLayerNames(const std::shared_ptr<UsdPacket> packet)
{
  std::vector<ON_wString> names;

  CRhinoDoc* doc = packet->Object().Document();
  if (!doc) return names;

  const CRhinoObjectAttributes& attributes = packet->Object().Attributes();
  int layer_index = attributes.m_layer_index;

  const CRhinoLayerTable& layer_table = doc->m_layer_table;
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
  if (!UsdOptions.ModelName.IsEmpty())
  {
    names.insert(names.begin(), UsdOptions.ModelName);
  }

  names.insert(names.begin(), UsdOptions.RootLayer);
  return names;
}

