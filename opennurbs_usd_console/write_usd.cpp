#include "stdafx.h"
#include "opennurbs/opennurbs_textiterator.h"
//#include "opennurbs/opennurbs_system.h"
#include "write_usd.h"
#include <iostream>

using namespace pxrInternal_v0_23__pxrReserved__;

static std::string ON_wStringToStdString(const ON_wString& onwstr)
{
  // todo: problems with losing data
  std::wstring wstr(static_cast<const wchar_t*>(onwstr));
  std::string str(wstr.begin(), wstr.end());
  return str;
}

static bool WriteUSDMesh(UsdStageRefPtr usdModel, const ON_Mesh* mesh, ON_wString& path, int index)
{
  if (nullptr == mesh)
    return false;

  ON_wString name;
  name.Format(L"/mesh%d", index);
  name = path + name;
  std::string stdStrName = ON_wStringToStdString(name);
  UsdGeomMesh usdMesh = UsdGeomMesh::Define(usdModel, SdfPath(stdStrName));

  pxr::VtArray<pxr::GfVec3f> points;
  for (int i = 0; i < mesh->m_V.Count(); i++)
  {
    const ON_3fPoint& rhinoPt = mesh->m_V[i];
    pxr::GfVec3f pt(rhinoPt.x, rhinoPt.y, rhinoPt.z);
    points.push_back(pt);
  }
  usdMesh.GetPointsAttr().Set(points);

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
      std::cout << usdClr << "--" << colors.size() << std::endl;
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

static void SetUsdLayersAsXformable(const std::vector<ON_wString>& layerNames, UsdStageRefPtr stage)
{
  ON_wString path;
  for (ON_wString name : layerNames)
  {
    path = path + L"/" + name;
    std::string stdStrPath = ON_wStringToStdString(path);

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

    std::cout << "layer: " << stdStrPath << std::endl; //debug
		pxr::UsdGeomXform nextLayerXform = pxr::UsdGeomXform::Define(stage, pxr::SdfPath(stdStrPath));
  }
}

static bool IsAcceptableUsdCharacter(wchar_t c)
{
  if (ON_wString::IsDecimalDigit(c))
    return true;
  if (L'_' == c)
    return true;
  if ((c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z'))
    return true;
  return false;
}

static ON_wString RhinoLayerNameToUsd(const ON_wString& rhLayerName) 
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

static std::vector<ON_wString> GetLayerNames(const ON_ModelGeometryComponent* gc, const ONX_Model& model)
{
  std::vector<ON_wString> names;
  const ON_3dmObjectAttributes* attr = gc->Attributes(nullptr);
  ON_ModelComponentReference layerCmpRef = model.LayerFromAttributes(*attr);
  if (layerCmpRef.IsEmpty())
    return names;
  const ON_Layer* layer = ON_Layer::Cast(layerCmpRef.ModelComponent());
  ON_wString layerName = RhinoLayerNameToUsd(layer->Name());
  names.push_back(layerName);
  
  ON_UUID pid(layer->ParentId());
  while (!ON_UuidIsNil(pid))
  {
    const ON_ModelComponentReference parentLayerCmpRef = model.LayerFromId(pid);
    if (parentLayerCmpRef.IsEmpty())
      break;
    const ON_Layer* parentLayer = ON_Layer::Cast(parentLayerCmpRef.ModelComponent());
    ON_wString parentLayerName = RhinoLayerNameToUsd(parentLayer->Name());
    names.insert(names.begin(),parentLayerName);
    ON_UUID id(parentLayer->ParentId());
    pid = id;
  }
  names.insert(names.begin(), L"Geometry");
  names.insert(names.begin(), L"Rhino"); // or "World"
  return names;
}

static ON_wString GetLayerNamesPath(const std::vector<ON_wString>& names)
{
  ON_wString path;
  for (ON_wString name : names) {
    path = path + L"/" + name;
  }
  return path;
}

static ON_wString GetLayerNamesPath(const ON_ModelGeometryComponent* gc, const ONX_Model& model)
{
  std::vector<ON_wString> names = GetLayerNames(gc, model);
  return GetLayerNamesPath(names);
}

bool WriteUSDFile(const wchar_t* rhinoFilename, const wchar_t* usdFilename)
{
  ONX_Model rhinoModel;
  if (!rhinoModel.Read(rhinoFilename))
    return false;

  //auto unitSystem = rhinoModel.m_settings.m_ModelUnitsAndTolerances.m_unit_system;
  //float scaleFactor = (float)ON::UnitScale(unitSystem, ON::LengthUnitSystem::Centimeters);

  UsdStageRefPtr usdModel = UsdStage::CreateInMemory();
  // Set the Z up direction for Rhino
  pxr::TfToken upAxis = pxr::UsdGeomTokens->z;
  pxr::UsdGeomSetStageUpAxis(usdModel, upAxis);

  const ON_ComponentManifestItem* manifest_item = rhinoModel.Manifest().FirstItem(ON_ModelComponent::Type::ModelGeometry);
  int currentIndex = 0;
  while (manifest_item)
  {
    ON_UUID objectId = manifest_item->Id();
    manifest_item = rhinoModel.Manifest().NextItem(manifest_item);

    ON_ModelComponentReference component = rhinoModel.ComponentFromId(ON_ModelComponent::Type::ModelGeometry, objectId);
    const ON_ModelGeometryComponent* geometryComponent = ON_ModelGeometryComponent::Cast(component.ModelComponent());
    const ON_Geometry* geometry = geometryComponent ? geometryComponent->Geometry(nullptr) : nullptr;
    if (nullptr == geometry)
      continue;

    std::vector<ON_wString> layerNames = GetLayerNames(geometryComponent, rhinoModel);
    ON_wString layerNamesPath = GetLayerNamesPath(layerNames);
    SetUsdLayersAsXformable(layerNames, usdModel);

    const ON_Brep* brep = ON_Brep::Cast(geometry);
    if (brep)
    {
      ON_SimpleArray<const ON_Mesh*> meshes;
      brep->GetMesh(ON::any_mesh, meshes);
      for (int i = 0; i < meshes.Count(); i++)
      {
        if (WriteUSDMesh(usdModel, meshes[i], layerNamesPath, currentIndex))
          currentIndex++;
      }
      continue;
    }
    const ON_Mesh* mesh = ON_Mesh::Cast(geometry);
    if (mesh)
    {
      if (WriteUSDMesh(usdModel, mesh, layerNamesPath, currentIndex))
        currentIndex++;
      continue;
    }
  }

  if (0 == currentIndex)
    return false;

  ON_String usdPath(usdFilename);
  usdModel->Export(usdPath.Array());
  return true;
}
