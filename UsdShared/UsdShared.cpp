#include "stdafx.h"
#include "UsdShared.h"
#include "ON_Helpers.h"
#include "iostream""

using namespace pxr;

UsdExportImport::UsdExportImport() :
  currentMeshIndex(0),
  currentMaterialIndex(0),
  currentShaderIndex(0),
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

ON_wString UsdExportImport::AddMesh(const ON_Mesh* mesh, const std::vector<ON_wString>& layerNames)
{
  ON_Mesh meshCopy(*mesh);
  ON_Helpers::RotateYUp(&meshCopy);

  UsdShared::SetUsdLayersAsXformable(layerNames, stage);
  ON_wString layerNamesPath = ON_Helpers::StringVectorToPath(layerNames);
  ON_wString meshPath = UsdShared::WriteUSDMesh(stage, &meshCopy, layerNamesPath, currentMeshIndex);
  currentMeshIndex++;
  return meshPath;
}

void UsdExportImport::__addAndBindMat(
  const ON_wString& namePrefix,
  const pxr::GfVec3f& diffuseColor,
  float opacity,
  float roughness,
  float metallic,
  float oior,
  float rior,
  float alpha,
  float clearcoat,
  float anisotropic,
  float sheen,
  float sheenTint,
  pxr::GfVec3f emission,
  float specular,
  float specularTint,
  const std::vector<ON_wString>& layerNames,
  const ON_wString meshPath)
{
  std::string strMeshPath = ON_Helpers::ON_wStringToStdString(meshPath);
  pxr::SdfPath mp(strMeshPath);
  pxr::UsdPrim mesh = stage->GetPrimAtPath(mp);

  ON_wString layerNamesPath = ON_Helpers::StringVectorToPath(layerNames);
  ON_wString name;
  name.Format(L"/%smaterial%d", namePrefix, currentMaterialIndex++);
  name = layerNamesPath + name;
  std::string stdStrName = ON_Helpers::ON_wStringToStdString(name);
  pxr::UsdShadeMaterial usdMaterial = pxr::UsdShadeMaterial::Define(stage, pxr::SdfPath(stdStrName));


  ON_wString shaderName;
  shaderName.Format(L"/shader%d", currentShaderIndex++);
  shaderName = layerNamesPath + shaderName;
  std::string stdStrShaderName = ON_Helpers::ON_wStringToStdString(shaderName);
  pxr::UsdShadeShader shader = pxr::UsdShadeShader::Define(stage, pxr::SdfPath(stdStrShaderName));
  shader.CreateIdAttr(pxr::VtValue(tokPreviewSurface));
  usdMaterial.CreateSurfaceOutput().ConnectToSource(shader.ConnectableAPI(), tokSurface);

  shader.CreateInput(tokDiffuseColor, pxr::SdfValueTypeNames->Color3f).Set(diffuseColor);
  //if (opacity < 0.01)
  //  opacity = 0.5;
  
  //pxr::TfToken tokOpacityThreshold("opacityThreshold");
  //shader.CreateInput(tokOpacityThreshold, pxr::SdfValueTypeNames->Float).Set(0.0);

  pxr::TfToken tokUseSpecularWorkflow("useSpecularWorkflow");
  shader.CreateInput(tokUseSpecularWorkflow, pxr::SdfValueTypeNames->Int).Set(1);

  shader.CreateInput(tokOpacity, pxr::SdfValueTypeNames->Float).Set(opacity);

  if (roughness != -1.0)
    shader.CreateInput(tokRoughness, pxr::SdfValueTypeNames->Float).Set(roughness);
  if (metallic != -1.0)
    // from what I understand this is either 0 (dielectric) and 1 (metallic)
    shader.CreateInput(tokMetallic, pxr::SdfValueTypeNames->Float).Set(/*metallic*/0.0);

  // trying to get the 2 transparent sphere's in Andy's file to show up
  // the file, some_common_material_cases.3dm, can be found here: https://mcneel.myjetbrains.com/youtrack/issue/RH-73726

  // opacity IOR
  //if (oior != -1.0)
  //{
  //  pxr::TfToken tokIor("ior");
  //  shader.CreateInput(tokIor, pxr::SdfValueTypeNames->Float).Set(oior);
  //}

  // reflective IOR
  if (rior != -1.0)
  {
    pxr::TfToken tokIor("ior");
    shader.CreateInput(tokIor, pxr::SdfValueTypeNames->Float).Set(rior);
  }


  //if (alpha != -1.0)
  //{
  //  pxr::TfToken tokAlpha("?");
  //  shader.CreateInput(tokAlpha, pxr::SdfValueTypeNames->Float).Set(alpha);
  //}

  if (clearcoat != -1.0)
  {
    pxr::TfToken tokClearcoat("clearcoat");
    shader.CreateInput(tokClearcoat, pxr::SdfValueTypeNames->Float).Set(clearcoat);
  }

  if (anisotropic != -1.0)
  {
    pxr::TfToken tokAnisotropic("?");
    shader.CreateInput(tokAnisotropic, pxr::SdfValueTypeNames->Float).Set(anisotropic);
  }

  //if (sheen != -1.0)
  //{
  //  pxr::TfToken tokSheen("?");
  //  shader.CreateInput(tokSheen, pxr::SdfValueTypeNames->Float).Set(sheen);
  //}

  //if (sheenTint != -1.0)
  //{
  //  pxr::TfToken tokSheenTint("?");
  //  shader.CreateInput(tokSheenTint, pxr::SdfValueTypeNames->Float).Set(sheenTint);
  //}

  if (namePrefix == L"on_pbr_")
  {
    pxr::TfToken tokEmission("emissiveColor");
    shader.CreateInput(tokEmission, pxr::SdfValueTypeNames->Color3f).Set(emission);
  }

  //if (specular != -1.0)
  //{
  //  pxr::TfToken tokSpecular("?");
  //  shader.CreateInput(tokSpecular, pxr::SdfValueTypeNames->Float).Set(specular);
  //}

  //if (specularTint != -1.0)
  //{
  //  pxr::TfToken tokSpecularTint("?");
  //  shader.CreateInput(tokSpecularTint, pxr::SdfValueTypeNames->Float).Set(specularTint);
  //}

  mesh.ApplyAPI<pxr::UsdShadeMaterialBindingAPI>();
  pxr::UsdShadeMaterialBindingAPI(mesh).Bind(usdMaterial);
}

void UsdExportImport::AddAndBindMaterial(const ON_Material* material, const std::vector<ON_wString>& layerNames, const ON_wString meshPath)
{
  float r(material->Diffuse().FractionRed());
  float b(material->Diffuse().FractionBlue());
  float g(material->Diffuse().FractionGreen());
  pxr::GfVec3f diffColor(r, b, g);

  float opacity(1.0 - material->Transparency());

  float r(-1.0);
  float m(-1.0);

  pxr::GfVec3f emission(-1.0, -1.0, -1.0);

  ON_wString namePrefix(L"on_");
  __addAndBindMat(namePrefix, diffColor, opacity, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, emission, -1.0, -1.0, layerNames, meshPath);
}

void UsdExportImport::AddAndBindPbrMaterial(const ON_PhysicallyBasedMaterial* pbrMaterial, const std::vector<ON_wString>& layerNames, const ON_wString meshPath)
{
  ON_4fColor color = pbrMaterial->BaseColor();
  pxr::GfVec3f diffColor(color.Red(), color.Green(), color.Blue());
  float o(pbrMaterial->Opacity());
  float r(pbrMaterial->Roughness());
  float m(pbrMaterial->Metallic());
  float oior(pbrMaterial->OpacityIOR());
  float rior(pbrMaterial->ReflectiveIOR());
  float alpha(pbrMaterial->Alpha());
  float clearcoat(pbrMaterial->Clearcoat());
  float anisotropic(pbrMaterial->Anisotropic());
  float sheen(pbrMaterial->Sheen());
  float sheenTint(pbrMaterial->SheenTint());
  ON_4fColor e(pbrMaterial->Emission());
  pxr::GfVec3f emission(e.Red(), e.Green(), e.Blue());
  float specular(pbrMaterial->Specular());
  float specularTint(pbrMaterial->SpecularTint());
  pbrMaterial->FindTexture()
  pbrMaterial->FindTexture()
  ON_wString namePrefix(L"on_pbr_");
  __addAndBindMat(namePrefix, diffColor, o, r, m, oior, rior, alpha, clearcoat, anisotropic, sheen, sheenTint, emission, specular, specularTint, layerNames, meshPath);
}

bool UsdExportImport::AnythingToSave()
{
  return currentMeshIndex > 0 || currentMaterialIndex > 0;
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

ON_wString UsdShared::WriteUSDMesh(UsdStageRefPtr usdModel, const ON_Mesh* mesh, ON_wString& path, int index)
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
