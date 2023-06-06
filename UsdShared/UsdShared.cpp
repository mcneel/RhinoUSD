#include "stdafx.h"
#include "UsdShared.h"
#include "ON_Helpers.h"
#include "iostream"
#include <fstream>

using namespace pxr;

UsdExportImport::UsdExportImport(const ON_wString& fn) :
  usdFullFileName(fn),
  currentMeshIndex(0),
  currentMaterialIndex(0),
  currentShaderIndex(0),
  currentNurbsCurveIndex(0),
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
  tokOcclusion("occlusion")
{
  stage = UsdStage::CreateInMemory();
  //stage = UsdStage::CreateNew(<some path>);

  // Set the Z up direction for Rhino
  pxr::TfToken upAxis = pxr::UsdGeomTokens->y; // z;
  if (!pxr::UsdGeomSetStageUpAxis(stage, upAxis))
  {
    std::cout << "nope";
  };
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

    case ON_Texture::TYPE::bump_texture: { return tokDiffuseColor; break; }

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
    default: {return pxr::TfToken(""); break; }
  }
}

ON_wString UsdExportImport::AddMesh(const ON_Mesh* mesh, const std::vector<ON_wString>& layerNames, const std::map<int, const ON_TextureCoordinates*>& tcs)
{
  ON_Mesh meshCopy(*mesh);
  ON_Helpers::RotateYUp(&meshCopy);

  UsdShared::SetUsdLayersAsXformable(layerNames, stage);
  ON_wString layerNamesPath = ON_Helpers::StringVectorToPath(layerNames);
  ON_wString meshPath = UsdShared::WriteUSDMesh(stage, &meshCopy, layerNamesPath, currentMeshIndex, tcs);
  currentMeshIndex++;
  return meshPath;
}

void UsdExportImport::AddAndBindPbrMaterialAndTextures(const ON_PhysicallyBasedMaterial* pbrMaterial, const ON_ObjectArray<ON_Texture>& textures, const std::vector<ON_wString>& layerNames, const ON_wString meshPath)
{
  std::string strMeshPath = ON_Helpers::ON_wStringToStdString(meshPath);
  pxr::SdfPath mp(strMeshPath);
  pxr::UsdPrim mesh = stage->GetPrimAtPath(mp);

  ON_wString layerNamesPath = ON_Helpers::StringVectorToPath(layerNames);
  ON_wString mesh_name;
  mesh_name.Format(L"/material%d", currentMaterialIndex++);
  mesh_name = layerNamesPath + mesh_name;
  std::string stdStrMeshName = ON_Helpers::ON_wStringToStdString(mesh_name);
  pxr::UsdShadeMaterial usdMaterial = pxr::UsdShadeMaterial::Define(stage, pxr::SdfPath(stdStrMeshName));




  ON_wString shaderName;
  shaderName.Format(L"%s/shader%d", mesh_name.Array(), currentShaderIndex++);
  //shaderName = layerNamesPath + shaderName;
  std::string stdStrShaderName = ON_Helpers::ON_wStringToStdString(shaderName);
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
  float metallic(pbrMaterial->Metallic());
  shader.CreateInput(tokMetallic, pxr::SdfValueTypeNames->Float).Set(metallic);

  float roughness(pbrMaterial->Roughness());
  shader.CreateInput(tokRoughness, pxr::SdfValueTypeNames->Float).Set(roughness);

  float clearcoat(pbrMaterial->Clearcoat());
  shader.CreateInput(tokClearcoat, pxr::SdfValueTypeNames->Float).Set(clearcoat);

  //@todo: "clearcoatRoughness"
  
  float opacity(pbrMaterial->Opacity());
  shader.CreateInput(tokOpacity, pxr::SdfValueTypeNames->Float).Set(opacity);

  // "opacityThreshold" : Andy says to ignore

  // there is also ReflectiveIOR() but Andy says that OpacityIOR is the correct one to use
  float rior(pbrMaterial->OpacityIOR());
  shader.CreateInput(tokIor, pxr::SdfValueTypeNames->Float).Set(rior);

  //@todo: "normal" : Andy says: "This is related to bump. We might have to think carefully about this"

  // even though there is no pbr input mapped to this yet there could be a texture: ON_Texture::TYPE::pbr_displacement_texture
  //@todo: "displacement"

  // even though there is no pbr input mapped to this yet there could be a texture: ON_Texture::TYPE::pbr_ambient_occlusion_texture
  //@todo: "occlusion"

  // UsdPreviewSurface inputs - END


  usdMaterial.CreateSurfaceOutput().ConnectToSource(shader.ConnectableAPI(), tokSurface);

  ON_wString stReaderName;
  stReaderName.Format(L"%s/stReader%d", mesh_name.Array(), currentMaterialIndex - 1);
  //stReaderName = layerNamesPath + stReaderName;
  auto stReader = pxr::UsdShadeShader::Define(stage, pxr::SdfPath(ON_Helpers::ON_wStringToStdString(stReaderName)));
  stReader.CreateIdAttr(pxr::VtValue(pxr::TfToken("UsdPrimvarReader_float2")));

  
  const int texture_count = textures.Count();
  for (int i = 0; i < texture_count; i++)
  {
    ON_Texture t = textures[i];
    ON_Texture::TYPE tt = t.m_type;
    ON_wString textureFullFileName = t.m_image_file_reference.FullPath();
    filesInExport.push_back(textureFullFileName);

    pxr::TfToken pbrParam = this->TextureTypeToUsdPbrPropertyTfToken(tt);
    if (pbrParam.IsEmpty()) {
      // skip this texture
      continue;
    }
    ON_wString textureFullName;
    ON_wString ttStr(ON_Helpers::ON_TextureTYPE_ToString(tt));
    textureFullName.Format(L"%s/texture_%s", mesh_name.Array(), ttStr.Array());
    ON_wString textureName;
    textureName.Format(L"texture_%s", ttStr.Array());
    std::string stdTextureName = ON_Helpers::ON_wStringToStdString(textureName);

    pxr::UsdShadeShader usdUVTextureSampler = UsdShadeShader::Define(stage, pxr::SdfPath(ON_Helpers::ON_wStringToStdString(textureFullName)));
    usdUVTextureSampler.CreateIdAttr(pxr::VtValue(pxr::TfToken("UsdUVTexture")));

    std::string textureFileName = "./" + ON_Helpers::ON_wStringToStdString(ON_FileSystemPath::FileNameFromPath(textureFullFileName, true));
    usdUVTextureSampler.CreateInput(TfToken("file"), pxr::SdfValueTypeNames->Asset).Set(pxr::SdfAssetPath(textureFileName));

    //todo: if (t.m_mapping_channel_id <> 1 /*or 0*/) append id to "st"
    usdUVTextureSampler.CreateInput(TfToken("st"), pxr::SdfValueTypeNames->Float2).ConnectToSource(stReader.ConnectableAPI(), TfToken("result"));
    //todo: "rgb" is probably only for colors like diffuseColor. What should it be for other props?
    usdUVTextureSampler.CreateOutput(TfToken("rgb"), pxr::SdfValueTypeNames->Float3);
    //todo: same here. typeNames->Color3f is correct for diffuseColor but not for most other pbrParam
    shader.CreateInput(pbrParam, pxr::SdfValueTypeNames->Color3f).ConnectToSource(usdUVTextureSampler.ConnectableAPI(), TfToken("rgb"));

    // primvar for texture mapping coordinates
    //pxr::UsdShadeInput stInput = usdMaterial.CreateInput(TfToken("frame:stPrimvarName"), SdfValueTypeNames->Token);
    pxr::UsdShadeInput stInput = usdMaterial.CreateInput(TfToken(stdTextureName), SdfValueTypeNames->String);
    stInput.Set("st");
    stReader.CreateInput(TfToken("varname"), SdfValueTypeNames->String).ConnectToSource(stInput);
  }


  mesh.ApplyAPI<pxr::UsdShadeMaterialBindingAPI>();
  pxr::UsdGeomMesh usdMesh = pxr::UsdGeomMesh(mesh);
  pxr::UsdShadeMaterialBindingAPI(usdMesh).Bind(usdMaterial);
}

void UsdExportImport::AddNurbsCurve(const ON_NurbsCurve* nurbsCurve, const std::vector<ON_wString>& layerNames)
{
  ON_wString layerNamesPath = ON_Helpers::StringVectorToPath(layerNames);

  if (nullptr == nurbsCurve)
    return;
  
  ON_NurbsCurve nc(*nurbsCurve);
  ON_Helpers::RotateGeometryYUp(&nc);

  ON_wString name;
  name.Format(L"nurbsCurve%d", currentNurbsCurveIndex++);
  name = layerNamesPath + name;
  std::string stdStrName = ON_Helpers::ON_wStringToStdString(name);
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
      ctrlPts[i] = pxr::GfVec3f(cp.x, cp.y, cp.z);
    }
  }
  usdNc.CreatePointsAttr(pxr::VtValue(ctrlPts));

  pxr::VtArray<int> crvVertexCount;
  crvVertexCount.resize(1);
  crvVertexCount[0] = ctrlPtsCount;
  usdNc.CreateCurveVertexCountsAttr(pxr::VtValue(crvVertexCount));

  //pxr::VtArray<pxr::VtValue> knots;
  pxr::VtArray<double> knots;
  //std::vector<double> stdKnots;
  int knotCount = nurbsCurve->KnotCount();
  knots.resize(knotCount);
  for (int i = 0; i < knotCount; i++)
  {
    double k = nurbsCurve->m_knot[i]; // nurbsCurve->Knot()[i];
    knots[i] = k; // pxr::VtValue(k);
    //stdKnots.push_back(k);
  }
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
  return currentMeshIndex > 0 || currentMaterialIndex > 0 || currentNurbsCurveIndex > 0;
}

void UsdExportImport::Save()
{
  if (ON_FileSystemPath::FileNameExtensionFromPath(usdFullFileName) == L".usdz")
  {
    ON_wString fullFileNameWithoutExtension = UsdShared::PathWithoutExtension(usdFullFileName);
    ON_wString usdaFileName = fullFileNameWithoutExtension + ".usda";
    stage->Export(ON_Helpers::ON_wStringToStdString(usdaFileName));
    UsdShared::CreateUsdzFile(fullFileNameWithoutExtension, filesInExport);
    ON_FileSystem::RemoveFile(usdaFileName.Array());
  }
  else
  {
    stage->Export(ON_Helpers::ON_wStringToStdString(usdFullFileName));
    //ON_wString copyToPath = UsdShared::PathFromFullFileName(usdFileName);
    ON_wString usdFileName = ON_FileSystemPath::FileNameFromPath(usdFullFileName, true);
    ON_wString copyToPath = ON_FileSystemPath::RemoveFileName(usdFullFileName, &usdFileName);
    for (ON_wString fullFileName : filesInExport)
    {
      UsdShared::CopyFileTo(fullFileName, copyToPath);
    }
  }
}

//todo: I'm sure there's a copy file function that's already available somewhere
void UsdShared::CopyFileTo(const ON_wString& fullFileName, const ON_wString& destination)
{
  ON_wString fileName = ON_FileSystemPath::FileNameExtensionFromPath(fullFileName);
  ON_wString destFullFileName = destination + fileName;
  std::ifstream  src(ON_Helpers::ON_wStringToStdString(fullFileName), std::ios::binary);
  std::ofstream  dst(ON_Helpers::ON_wStringToStdString(destFullFileName),   std::ios::binary);
  dst << src.rdbuf();
}

void UsdShared::CreateUsdzFile(const ON_wString& fullFileNameNoExtension, const std::vector<ON_wString>& filesToInclude)
{
  ON_wString usdaFullFileName = fullFileNameNoExtension + ".usda";
  ON_wString usdaFileName = ON_FileSystemPath::FileNameFromPath(usdaFullFileName, true);
  ON_wString usdzFullFileName = fullFileNameNoExtension + ".usdz";
  UsdZipFileWriter writer = UsdZipFileWriter::CreateNew(ON_Helpers::ON_wStringToStdString(usdzFullFileName));
  // usda has to be added before textures
  writer.AddFile(ON_Helpers::ON_wStringToStdString(usdaFullFileName), ON_Helpers::ON_wStringToStdString(usdaFileName));
  for (ON_wString fullFileName : filesToInclude)
  {
    ON_wString fileName = ON_FileSystemPath::FileNameFromPath(fullFileName, true);
    writer.AddFile(ON_Helpers::ON_wStringToStdString(fullFileName), ON_Helpers::ON_wStringToStdString(fileName));
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

ON_wString UsdShared::RhinoLayerNameToUsd(const ON_wString& rhLayerName) 
{
  if (rhLayerName.Length() < 1)
    return L"Default";

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

ON_wString UsdShared::WriteUSDMesh(UsdStageRefPtr usdModel, const ON_Mesh* mesh, ON_wString& path, int index, const std::map<int, const ON_TextureCoordinates*>& tcs)
{
  if (nullptr == mesh)
    return "";

  ON_wString name;
  name.Format(L"/mesh%d", index);
  name = path + name;
  std::string stdStrName = ON_Helpers::ON_wStringToStdString(name);
  UsdGeomMesh usdMesh = UsdGeomMesh::Define(usdModel, SdfPath(stdStrName));

  pxr::VtArray<pxr::GfVec3f> points;
  for (int i = 0; i < mesh->m_V.Count(); i++)
  {
    const ON_3fPoint& rhinoPt = mesh->m_V[i];
    pxr::GfVec3f pt(rhinoPt.x, rhinoPt.y, rhinoPt.z);
    points.push_back(pt);
  }
  usdMesh.CreatePointsAttr().Set(points);

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

  if (mesh->HasVertexNormals())
  {
    pxr::VtArray<pxr::GfVec3f> normals;
    normals.resize(mesh->m_N.Count());
    for (int i = 0; i < mesh->m_N.Count(); i++)
    {
      ON_3fVector v = mesh->m_N[i];
      normals[i] = pxr::GfVec3f(v.x, v.y, v.z);
    }
    usdMesh.CreateNormalsAttr(pxr::VtValue(normals));
  }

  if (mesh->HasVertexColors())
  {
    pxr::VtArray<pxr::GfVec3f> colors;
    int colorsCount = mesh->m_C.Count();
    for (int i = 0; i < colorsCount; i++)
    {
      ON_Color clr = mesh->m_C[i];
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
  if (!tcs.empty())
  {
    // let's just use the 1st one in the array for now
    //int mc_id = tcs.begin()->first;
    const ON_TextureCoordinates* firstTc = tcs.begin()->second;
    //if (tcs.size() > 1)
    //  // todo: support multiple channels or report that some were skipped.
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

    pxr::UsdGeomPrimvar texCoords = pxr::UsdGeomPrimvarsAPI(usdMesh).CreatePrimvar(pxr::TfToken("st"), pxr::SdfValueTypeNames->TexCoord2fArray, pxr::UsdGeomTokens->vertex);
		//texCoords.SetInterpolation(pxr::TfToken("vertex")); //already set in CreatePrimvar
    texCoords.Set(uvArray);
  }

  VtVec3fArray extents(2);
  ON_BoundingBox bbox = mesh->BoundingBox();
  extents[0].Set((float)bbox.m_min.x, (float)bbox.m_min.y, (float)bbox.m_min.z);
  extents[1].Set((float)bbox.m_max.x, (float)bbox.m_max.y, (float)bbox.m_max.z);
  usdMesh.GetExtentAttr().Set(extents);
  return name;
}

void UsdShared::SetUsdLayersAsXformable(const std::vector<ON_wString>& layerNames, UsdStageRefPtr stage)
{
  ON_wString path;
  for (ON_wString name : layerNames)
  {
    // make sure the layer is activated
    pxr::UsdPrim existingPrim;

    path = path + L"/" + name;
    std::string stdStrPath = ON_Helpers::ON_wStringToStdString(path);
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
