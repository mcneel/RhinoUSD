#include "stdafx.h"
#include "UsdShared.h"
#include "ON_Helpers.h"
#include "iostream"

using namespace pxr;

UsdExportImport::UsdExportImport() :
  currentMeshIndex(0),
  currentMaterialIndex(0),
  currentShaderIndex(0),
  currentNurbsCurveIndex(0),
  tokPreviewSurface("UsdPreviewSurface"),
  tokSurface("surface"),
  tokDiffuseColor("diffuseColor"),
  tokOpacity("opacity"),
  tokRoughness("roughness"),
  tokMetallic("metallic")
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
    //case ON_Texture::TYPE::pbr_base_color_texture:
    case ON_Texture::TYPE::diffuse_texture: { return tokDiffuseColor; break; }
    case ON_Texture::TYPE::bump_texture: { return tokDiffuseColor; break; }
    //case ON_Texture::TYPE::transparency_texture:
    case ON_Texture::TYPE::opacity_texture: { return tokOpacity; break; }
    case ON_Texture::TYPE::pbr_subsurface_texture: { return TfToken("pbr_subsurface_texture"); break; }
    case ON_Texture::TYPE::pbr_subsurface_scattering_texture: { return TfToken("pbr_subsurface_scattering_texture"); break; }
    case ON_Texture::TYPE::pbr_subsurface_scattering_radius_texture: { return TfToken("pbr_subsurface_scattering_radius_texture"); break; }
    case ON_Texture::TYPE::pbr_metallic_texture: { return TfToken("metallic"); break; }
    case ON_Texture::TYPE::pbr_specular_texture: { return TfToken("pbr_specular_texture"); break; }
    case ON_Texture::TYPE::pbr_specular_tint_texture: { return TfToken("pbr_specular_tint_texture"); break; }
    case ON_Texture::TYPE::pbr_roughness_texture: { return TfToken("pbr_roughness_texture"); break; }
    case ON_Texture::TYPE::pbr_anisotropic_texture: { return TfToken("pbr_anisotropic_texture"); break; }
    case ON_Texture::TYPE::pbr_anisotropic_rotation_texture: { return TfToken("pbr_anisotropic_rotation_texture"); break; }
    case ON_Texture::TYPE::pbr_sheen_texture: { return TfToken("pbr_sheen_texture"); break; }
    case ON_Texture::TYPE::pbr_sheen_tint_texture: { return TfToken("pbr_sheen_tint_texture"); break; }
    case ON_Texture::TYPE::pbr_clearcoat_texture: { return TfToken("pbr_clearcoat_texture"); break; }
    case ON_Texture::TYPE::pbr_clearcoat_roughness_texture: { return TfToken("pbr_clearcoat_roughness_texture"); break; }
    case ON_Texture::TYPE::pbr_opacity_ior_texture: { return TfToken("pbr_opacity_ior_texture"); break; }
    case ON_Texture::TYPE::pbr_opacity_roughness_texture: { return TfToken("pbr_opacity_roughness_texture"); break; }
    case ON_Texture::TYPE::pbr_emission_texture: { return TfToken("pbr_emission_texture"); break; }
    case ON_Texture::TYPE::pbr_ambient_occlusion_texture: { return TfToken("pbr_ambient_occlusion_texture"); break; }
    //case ON_Texture::TYPE::pbr_smudge_texture: { return TfToken("pbr_smudge_texture"); break; }
    case ON_Texture::TYPE::pbr_displacement_texture: { return TfToken("pbr_displacement_texture"); break; }
    case ON_Texture::TYPE::pbr_clearcoat_bump_texture: { return TfToken("pbr_clearcoat_bump_texture"); break; }
    case ON_Texture::TYPE::pbr_alpha_texture: { return TfToken("pbr_alpha_texture"); break; }
    case ON_Texture::TYPE::emap_texture: { return TfToken("emap_texture"); break; }
    default: {return tokDiffuseColor; break; }
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
  shaderName.Format(L"%s/shader%d", mesh_name, currentShaderIndex++);
  //shaderName = layerNamesPath + shaderName;
  std::string stdStrShaderName = ON_Helpers::ON_wStringToStdString(shaderName);
  pxr::UsdShadeShader shader = pxr::UsdShadeShader::Define(stage, pxr::SdfPath(stdStrShaderName));
  shader.CreateIdAttr(pxr::VtValue(tokPreviewSurface));
  usdMaterial.CreateSurfaceOutput().ConnectToSource(shader.ConnectableAPI(), tokSurface);

  ON_4fColor color = pbrMaterial->BaseColor();
  pxr::GfVec3f diffuseColor(color.Red(), color.Green(), color.Blue());
  shader.CreateInput(tokDiffuseColor, pxr::SdfValueTypeNames->Color3f).Set(diffuseColor);
  //if (opacity < 0.01)
  //  opacity = 0.5;
  
  //pxr::TfToken tokOpacityThreshold("opacityThreshold");
  //shader.CreateInput(tokOpacityThreshold, pxr::SdfValueTypeNames->Float).Set(0.0);

  pxr::TfToken tokUseSpecularWorkflow("useSpecularWorkflow");
  shader.CreateInput(tokUseSpecularWorkflow, pxr::SdfValueTypeNames->Int).Set(1);
  
  float opacity(pbrMaterial->Opacity());
  shader.CreateInput(tokOpacity, pxr::SdfValueTypeNames->Float).Set(opacity);

  float roughness(pbrMaterial->Roughness());
  if (roughness != -1.0)
    shader.CreateInput(tokRoughness, pxr::SdfValueTypeNames->Float).Set(roughness);

  float metallic(pbrMaterial->Metallic());
  if (metallic != -1.0)
    // from what I understand this is either 0 (dielectric) and 1 (metallic)
    shader.CreateInput(tokMetallic, pxr::SdfValueTypeNames->Float).Set(/*metallic*/0.0);

  // what about opacity IOR?
  // reflective IOR
  float rior(pbrMaterial->ReflectiveIOR());
  if (rior != -1.0)
  {
    pxr::TfToken tokIor("ior");
    shader.CreateInput(tokIor, pxr::SdfValueTypeNames->Float).Set(rior);
  }

  //float alpha(pbrMaterial->Alpha());
  //if (alpha != -1.0)
  //{
  //  pxr::TfToken tokAlpha("?");
  //  shader.CreateInput(tokAlpha, pxr::SdfValueTypeNames->Float).Set(alpha);
  //}

  float clearcoat(pbrMaterial->Clearcoat());
  if (clearcoat != -1.0)
  {
    pxr::TfToken tokClearcoat("clearcoat");
    shader.CreateInput(tokClearcoat, pxr::SdfValueTypeNames->Float).Set(clearcoat);
  }

  float anisotropic(pbrMaterial->Anisotropic());
  if (anisotropic != -1.0)
  {
    pxr::TfToken tokAnisotropic("?");
    shader.CreateInput(tokAnisotropic, pxr::SdfValueTypeNames->Float).Set(anisotropic);
  }

  //float sheen(pbrMaterial->Sheen());
  //if (sheen != -1.0)
  //{
  //  pxr::TfToken tokSheen("?");
  //  shader.CreateInput(tokSheen, pxr::SdfValueTypeNames->Float).Set(sheen);
  //}

  //float sheenTint(pbrMaterial->SheenTint());
  //if (sheenTint != -1.0)
  //{
  //  pxr::TfToken tokSheenTint("?");
  //  shader.CreateInput(tokSheenTint, pxr::SdfValueTypeNames->Float).Set(sheenTint);
  //}

  ON_4fColor e(pbrMaterial->Emission());
  pxr::GfVec3f emission(e.Red(), e.Green(), e.Blue());
  pxr::TfToken tokEmission("emissiveColor");
  shader.CreateInput(tokEmission, pxr::SdfValueTypeNames->Color3f).Set(emission);

  //float specular(pbrMaterial->Specular());
  //if (specular != -1.0)
  //{
  //  pxr::TfToken tokSpecular("?");
  //  shader.CreateInput(tokSpecular, pxr::SdfValueTypeNames->Float).Set(specular);
  //}

  //float specularTint(pbrMaterial->SpecularTint());
  //if (specularTint != -1.0)
  //{
  //  pxr::TfToken tokSpecularTint("?");
  //  shader.CreateInput(tokSpecularTint, pxr::SdfValueTypeNames->Float).Set(specularTint);
  //}


  ON_wString stReaderName;
  stReaderName.Format(L"%s/stReader%d", mesh_name, currentMaterialIndex - 1);
  //stReaderName = layerNamesPath + stReaderName;
  auto stReader = pxr::UsdShadeShader::Define(stage, pxr::SdfPath(ON_Helpers::ON_wStringToStdString(stReaderName)));
  stReader.CreateIdAttr(pxr::VtValue("UsdPrimvarReader_float2"));

  pxr::UsdShadeInput stInput = usdMaterial.CreateInput(TfToken("frame:stPrimvarName"), SdfValueTypeNames->Token);
  stInput.Set("st");

  stReader.CreateInput(TfToken("varname"), SdfValueTypeNames->Token).ConnectToSource(stInput);
  
  const int texture_count = textures.Count();
  for (int i = 0; i < texture_count; i++)
  {
    ON_Texture t = textures[i];
    ON_Texture::TYPE tt = t.m_type;
    //todo: copy file to export directory
    std::string filePath = ON_Helpers::ON_wStringToStdString(t.m_image_file_reference.FullPath());
    pxr::TfToken pbrParam = this->TextureTypeToUsdPbrPropertyTfToken(tt);
    ON_wString textureName;
    textureName.Format(L"%s/texture_%d", mesh_name, tt);
    //todo: diffuseTextureSampler should be renamed to textureSampler
    pxr::UsdShadeShader diffuseTextureSampler = UsdShadeShader::Define(stage, pxr::SdfPath(ON_Helpers::ON_wStringToStdString(textureName)));
    diffuseTextureSampler.CreateIdAttr(pxr::VtValue("UsdUVTexture"));
    diffuseTextureSampler.CreateInput(TfToken("file"), pxr::SdfValueTypeNames->Asset).Set(filePath);
    //todo: if (t.m_mapping_channel_id <> 1 /*or 0*/) append id to "st"
    diffuseTextureSampler.CreateInput(TfToken("st"), pxr::SdfValueTypeNames->Float2).ConnectToSource(stReader.ConnectableAPI(), TfToken("result"));
    //todo: "rgb" is probably only for colors like diffuseColor. What should it be for other props?
    diffuseTextureSampler.CreateOutput(TfToken("rgb"), pxr::SdfValueTypeNames->Float3);
    shader.CreateInput(TfToken("diffuseColor"), pxr::SdfValueTypeNames->Color3f).ConnectToSource(diffuseTextureSampler.ConnectableAPI(), TfToken("rgb"));
  }


  mesh.ApplyAPI<pxr::UsdShadeMaterialBindingAPI>();
  pxr::UsdShadeMaterialBindingAPI(mesh).Bind(usdMaterial);
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

void UsdExportImport::Save(const ON_wString& fileName)
{
  std::string fn = ON_Helpers::ON_wStringToStdString(fileName);
  stage->Export(fn);
  //stage->Save(); paired with UsdStage::CreateNew(pathname)?
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
    int mc_id = tcs.begin()->first;
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
    path = path + L"/" + name;
    std::string stdStrPath = ON_Helpers::ON_wStringToStdString(path);

    // make sure the layer is activated
    pxr::UsdPrim existingPrim;
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
