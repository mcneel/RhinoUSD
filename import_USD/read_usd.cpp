#include "stdafx.h"
#include "pxr/usd/usd/primRange.h"
#include "convert_geometry.h"
#include "convert_metadata.h"
#include <string>

class UsdReadSettings
{
  bool includeIsHidden;

  // Ideas / To-Do's etc.
  // bool includeLayers;
};

class PrimDataCollection
{
// private:

public:
  ON_Layer* layer = nullptr;
  ON_Geometry* geometry = nullptr;
  // This will cause the object to be hidden if true
  bool isVisible = true;
  // Metadata I don't know what else to do with
  ON_ClassArray<ON_UserString> userStrings = {};

  PrimDataCollection() = default;

  PrimDataCollection(const PrimDataCollection&) {} // Copy Constructor
  PrimDataCollection& operator=(const PrimDataCollection& x) {} // Copy Assignment

};

ON_wString GetOnFromOldString(std::string string)
{
  std::wstring wstring(string.begin(), string.end());

  ON_UserString onUString;
  ON_wString ons(static_cast<const wchar_t*>(wstring.c_str()));
  return ons;
}

static void TraversePrimTree(UsdPrim& root, CRhinoDoc& doc, PrimDataCollection& collection)
{
  for (UsdPrim prim : root.GetChildren())
  {
    if (prim.IsAbstract()) continue;
    if (prim.IsValid()) continue;

    // TODO : How to get Layers?

    // It's Geometry!
    // Are classes that inherit this going to return true?
    if (UsdGeomGprim geom = UsdGeomGprim(prim))
    {
      // bool visible = geom.GetVisibilityAttr()
      
      if (!geom.TransformMightBeTimeVarying())
      {
        auto transform = geom.ComputeLocalToWorldTransform(UsdTimeCode::Default());
      }
      else
      {
        // TODO : What to do if the Geometry is a Time Varying Transform? I'd assume get the first?
                  // Maybe settings could specify a time frame?
      } 
    }

    for (std::pair<TfToken, VtValue> metaData : prim.GetAllMetadata())
    {
      TfToken key = metaData.first;
      VtValue value = metaData.second;

      ON_UserString onUString;
      onUString.m_key = GetOnFromOldString(key.GetString());

      // TODO : Improve this.
      if (value.CanCast<std::wstring>())
      {
        auto valuewString = value.Cast<std::wstring>().Get<std::wstring>();
        ON_wString valueonwString(static_cast<const wchar_t*>(valuewString.c_str()));
        onUString.m_string_value = valueonwString;
      }
      else
      {
        // TODO : Better Fallback
        onUString.m_string_value = GetOnFromOldString(value.GetTypeName());
      }
        
      collection.userStrings.Append(onUString);
    }

  }

}

bool ReadUSDFile(const wchar_t* filename, CRhinoDoc& doc, const CRhinoFileReadOptions& options)
{
  // use an ON_String to convert from unicode to mbcs which is what usd wants
  ON_String usdPath(filename);
  UsdStageRefPtr usdModel = UsdStage::Open(usdPath.Array());
  auto layerStack = usdModel->GetLayerStack();
  size_t st = layerStack.size();
  for (auto layer : layerStack)
  {
    ;
    int i = 2;
  }

  // TODO : Use Stage Direction to set Rhino WorldXY

  // Traverse all prims in the stage
  // NOTE : This is ALL prims, including ones we likely don't want,
  //        e.g abstract, hidden, etc! Ensure to filter correctly.

  // NOTE : How does the traverse travel? Is it to the first leaf and recursively down?
  UsdPrimRange primRange = usdModel->Traverse(); // Use TraverseAll if it feels like we're missing something
  // usdModel->Flatten(); // <- Hmm
  UsdPrim root = usdModel->GetDefaultPrim();
  TraversePrimTree(root, doc, PrimDataCollection());

  ON_Layer* previousLayer = new ON_Layer();
  for (UsdPrim prim : primRange)
  {
    // TODO : This also checks for ancestors
    if (prim.IsAbstract()) continue;
    if (prim.IsValid()) continue;

    bool hidden = prim.IsHidden();

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
