#include <iostream>
#include "stdafx.h"
#include "convert_metadata.h"
#include "../UsdShared/ON_Helpers.h"

std::shared_ptr<const ON_3dmObjectAttributes> ConvertMetadata::TryGetAttributesFromPrim(pxr::UsdPrim& prim)
{
  ON_3dmObjectAttributes attribs;

  std::string usdName = prim.GetName().GetString();
  attribs.SetName(ON_Helpers::StdString_to_ON_wString(usdName), true);

  // Attribs
  pxr::UsdAttributeVector attribVectors = prim.GetAttributes();
  for (pxr::UsdAttribute& usdAttrib : attribVectors)
  {
    auto usdAttribName = usdAttrib.GetName().GetString();

    // Metadata
    pxr::UsdMetadataValueMap map = usdAttrib.GetAllMetadata();
    for (auto& metaValue : map)
    {
      // TODO : Set attribs
    }
  }
  
  // Custom Data
  pxr::VtDictionary dict = prim.GetCustomData();
  for (const auto& property : dict)
  {
    const ON_wString name = ON_Helpers::StdString_to_ON_wString(property.first);
    if (property.second.CanCast<std::string>())
    {
      std::string strValue = property.second.Get<std::string>();
      const ON_wString value = ON_Helpers::StdString_to_ON_wString(strValue);
      attribs.SetUserString(name, value);
    }
  }

  return std::make_shared<ON_3dmObjectAttributes>(attribs);
}

std::shared_ptr<ON_Layer> ConvertMetadata::TryGetLayerFromPrim(pxr::UsdPrim& prim)
{
  auto description = prim.GetDescription();
  auto primPath = prim.GetPath();
  auto primString = primPath.GetAsString();

  std::shared_ptr<ON_Layer> onLayer = std::make_shared<ON_Layer>();
  onLayer->SetName(L"Test");

  return onLayer;
}

// TODO : Which Layer is the starting layer? May be a good item to include in IMPORT OPTIONS
const int ConvertMetadata::GetOrCreateLayerIndex(CRhinoDoc& doc, pxr::UsdPrim& prim)
{
  const pxr::SdfPath path = prim.GetPath();
  const ON_wString pathString = ON_Helpers::StdString_to_ON_wString(path.GetString());
  
  // TODO : Remove the first :: or /
  ON_wString layerFullPath(pathString);
  layerFullPath.Replace(L"/", ON_Layer::NamePathSeparator);
  
  const ON_wString constPath(layerFullPath);
  return ConvertMetadata::GetOrCreateLayerIndex(doc, constPath);
}
  
const int ConvertMetadata::GetOrCreateLayerIndex(CRhinoDoc& doc, const ON_wString& layerFullPath)
{
  int index = doc.m_layer_table.FindLayerFromFullPathName(layerFullPath, -1);
  if (index >= 0)
  {
    return index;
  }
  
  ON_Layer newLayer;
  ON_wString newLayerName;
  
  int colonIndex = layerFullPath.ReverseFind(L":");
  if (colonIndex > -1)
  {
    const ON_wString parentFullPath = layerFullPath.SubString(0, colonIndex -1);
    int parentLayerIndex = ConvertMetadata::GetOrCreateLayerIndex(doc, parentFullPath);
    newLayer.SetParentId(doc.m_layer_table[index].Id());
    
    newLayerName = layerFullPath.SubString(colonIndex);
  }
  else
  {
    // Root Layer, none of the path exists
    newLayerName = layerFullPath;
  }
  
  newLayer.SetName(newLayerName);
  
  const ON_Layer constLayer(newLayer);
  index = doc.m_layer_table.AddLayer(constLayer);
  return index;
}
