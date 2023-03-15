#pragma once
#include "stdafx.h"
#include "write_usd.h"
#include "../UsdShared/ON_Helpers.h"
#include "../UsdShared/UsdShared.h"
#include <iostream>

static std::vector<ON_wString> GetLayerNames(const ON_ModelGeometryComponent* gc, const ONX_Model& model)
{
  std::vector<ON_wString> names;
  const ON_3dmObjectAttributes* attr = gc->Attributes(nullptr);
  ON_ModelComponentReference layerCmpRef = model.LayerFromAttributes(*attr);
  if (layerCmpRef.IsEmpty())
    return names;
  const ON_Layer* layer = ON_Layer::Cast(layerCmpRef.ModelComponent());
  ON_wString layerName = UsdShared::RhinoLayerNameToUsd(layer->Name());
  names.push_back(layerName);
  
  ON_UUID pid(layer->ParentId());
  while (!ON_UuidIsNil(pid))
  {
    const ON_ModelComponentReference parentLayerCmpRef = model.LayerFromId(pid);
    if (parentLayerCmpRef.IsEmpty())
      break;
    const ON_Layer* parentLayer = ON_Layer::Cast(parentLayerCmpRef.ModelComponent());
    ON_wString parentLayerName = UsdShared::RhinoLayerNameToUsd(parentLayer->Name());
    names.insert(names.begin(),parentLayerName);
    ON_UUID id(parentLayer->ParentId());
    pid = id;
  }
  names.insert(names.begin(), L"Geometry");
  names.insert(names.begin(), L"Rhino"); // or "World"
  return names;
}

bool WriteUSDFile(const wchar_t* rhinoFilename, const wchar_t* usdFilename)
{
  ONX_Model rhinoModel;
  if (!rhinoModel.Read(rhinoFilename))
    return false;

  const ON_ComponentManifestItem* manifest_item = rhinoModel.Manifest().FirstItem(ON_ModelComponent::Type::ModelGeometry);

  UsdExportImport usdEI{};

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
    //ON_wString layerNamesPath = ON_Helpers::StringVectorToPath(layerNames);
    //UsdShared::SetUsdLayersAsXformable(layerNames, usdModel);

    const ON_Brep* brep = ON_Brep::Cast(geometry);
    if (brep)
    {
      ON_SimpleArray<const ON_Mesh*> meshes;
      brep->GetMesh(ON::any_mesh, meshes);
      for (int i = 0; i < meshes.Count(); i++)
      {
        usdEI.AddMesh(meshes[i], layerNames);
      }
      continue;
    }
    const ON_Mesh* mesh = ON_Mesh::Cast(geometry);
    if (mesh)
    {
      usdEI.AddMesh(mesh, layerNames);
    }
  }

  if (!usdEI.AnythingToSave())
    return false;

  ON_String usdPath(usdFilename);
  usdEI.Save(usdPath);
  return true;
}
