#include "stdafx.h"
#include "pxr/usd/usd/primRange.h"
#include "convert_geometry.h"
#include "convert_metadata.h"

bool ReadUSDFile(const wchar_t* filename, CRhinoDoc& doc, const CRhinoFileReadOptions& options)
{
  // use an ON_String to convert from unicode to mbcs which is what usd wants
  ON_String usdPath(filename);
  UsdStageRefPtr usdModel = UsdStage::Open(usdPath.Array());

  // TODO : Use Stage Direction to set Rhino WorldXY

  // Traverse all prims in the stage
  UsdPrimRange primRange = usdModel->TraverseAll();

  ON_Layer* previousLayer = new ON_Layer();
  for (auto& prim : primRange)
  {
    auto description = prim.GetDescription();
    auto name = prim.GetDisplayName();

    ON_Layer* onLayer = TryGetLayerFromPrim(prim);
    if (previousLayer->Id() != ON_UUID())
    {
      onLayer->SetParentId(previousLayer->Id());
    }

    int layerIndex = doc.m_layer_table.CreateLayer(*onLayer, 0, 0, 0);
    // auto layeraaa = doc.m_layer_table[layerIndex];
    // previousLayer = doc.m_layer_table[layerIndex];

    TfTokenVector properties = prim.GetPropertyNames();
    for (auto& property : properties)
    {
      auto propertyString = property.GetString();
      int t = 7;
    }

    if (ON_Mesh* mesh = TryGetMeshFromPrim(prim))
    {
      auto attribs = TryGetAttributesFromPrim(prim);
      doc.AddMeshObject(*mesh, attribs);
    }
  }

  return true;
}
