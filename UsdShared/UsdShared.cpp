#pragma once

#include "stdafx.h"
#include "UsdShared.h"
#include "ON_Helpers.h"

using namespace pxr;

UsdExportImport::UsdExportImport() :
  currentMeshIndex(0),
  currentMaterialIndex(0),
  currentShaderIndex(0),
  tokPreviewSurface("UsdPreviewSurface"),
  tokSurface("surface"),
  tokDiffuseColor("diffuseColor"),
  tokOpacity("opacity")
{
  stage = UsdStage::CreateInMemory();
  // Set the Z up direction for Rhino
  pxr::TfToken upAxis = pxr::UsdGeomTokens->z;
  pxr::UsdGeomSetStageUpAxis(stage, upAxis);
}

void UsdExportImport::AddMesh(const ON_Mesh* mesh, const std::vector<ON_wString>& layerNames)
{
  UsdShared::SetUsdLayersAsXformable(layerNames, stage);
  ON_wString layerNamesPath = ON_Helpers::StringVectorToPath(layerNames);
  if (UsdShared::WriteUSDMesh(stage, mesh, layerNamesPath, currentMeshIndex))
    currentMeshIndex++;
}

void UsdExportImport::__addMat(const pxr::GfVec3f& diffuseColor, float opacity, const std::vector<ON_wString>& layerNames)
{
  ON_wString layerNamesPath = ON_Helpers::StringVectorToPath(layerNames);
  ON_wString name;
  name.Format(L"/material%d", currentMaterialIndex++);
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
  shader.CreateInput(tokOpacity, pxr::SdfValueTypeNames->Float).Set(opacity);
}

void UsdExportImport::AddMaterial(const ON_Material* material, const std::vector<ON_wString>& layerNames)
{
  float r(material->Diffuse().FractionRed());
  float b(material->Diffuse().FractionBlue());
  float g(material->Diffuse().FractionGreen());
  pxr::GfVec3f diffColor(r, b, g);

  float opacity(material->Transparency());

  __addMat(diffColor, opacity, layerNames);
}

void UsdExportImport::AddPbrMaterial(const ON_PhysicallyBasedMaterial* pbrMaterial, const std::vector<ON_wString>& layerNames)
{
  ON_4fColor color = pbrMaterial->BaseColor();
  pxr::GfVec3f diffColor(color.Red(), color.Green(), color.Blue());
  float o(pbrMaterial->Opacity());
  __addMat(diffColor, o, layerNames);
}

bool UsdExportImport::AnythingToSave()
{
  return currentMeshIndex > 0 || currentMaterialIndex > 0;
}

void UsdExportImport::Save(const ON_wString& fileName)
{
  std::string fn = ON_Helpers::ON_wStringToStdString(fileName);
  stage->Export(fn);
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

bool UsdShared::WriteUSDMesh(UsdStageRefPtr usdModel, const ON_Mesh* mesh, ON_wString& path, int index)
{
  if (nullptr == mesh)
    return false;

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
  return true;
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
