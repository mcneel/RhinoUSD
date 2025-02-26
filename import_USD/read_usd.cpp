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
  ON_Matrix* transform = nullptr;
  // This will cause the object to be hidden if true
  bool isVisible = true;
  // Metadata I don't know what else to do with
  ON_ClassArray<ON_UserString> userStrings = {};

  PrimDataCollection() = default;

  PrimDataCollection(const PrimDataCollection&) {} // Copy Constructor
  PrimDataCollection& operator=(const PrimDataCollection& x) {} // Copy Assignment

};

bool AddPrimDataToDoc(CRhinoDoc& doc, PrimDataCollection& data)
{
  if (data.geometry == nullptr) return false;

  auto attribs = new ON_3dmObjectAttributes();
  attribs->SetVisible(data.isVisible);

  const ON_Matrix matrix = *data.transform;
  ON_Xform xform(matrix);
  data.geometry->Transform(xform);

  if (auto revSurface = ON_RevSurface::Cast(data.geometry))
  {
    auto surfObj = new CRhinoSurfaceObject(*attribs);
    surfObj->SetSurface(revSurface);
    doc.AddObject(surfObj);
    
    delete revSurface;
  }
  else if (auto brep = ON_Brep::Cast(data.geometry))
  {
    doc.AddBrepObject(*brep, attribs);
  }
  else if (auto mesh = ON_Mesh::Cast(data.geometry))
  {
    doc.AddMeshObject(*mesh, attribs);
  }
  else if (auto curve = ON_Curve::Cast(data.geometry))
  {
    doc.AddCurveObject(*curve, attribs);
  }
  else if (auto surface = ON_NurbsSurface::Cast(data.geometry))
  {
    doc.AddSurfaceObject(*surface, attribs);
  }
  else if (auto pointCloud = ON_PointCloud::Cast(data.geometry))
  { 
    doc.AddPointCloudObject(pointCloud->PointCount(), pointCloud->m_P, attribs);
  }
  else
  {
    return false;
  }

  return true;
}

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
    if (ON_Geometry* onGeom = TryGetPrimGeometry(prim))
    {
      collection.geometry = onGeom;
      if (UsdGeomGprim geom = UsdGeomGprim(prim))
      {
        auto visibleAttribute = geom.GetVisibilityAttr();
        // bool visible = GetValueFromAttribute<bool>(visibleAttribute);
        bool visible = false;
        collection.isVisible = visible;

        // TODO : What to do if the Geometry is a Time Varying Transform? I'd assume get the first?
        //        Maybe settings could specify a time frame?
        auto usdTransform = geom.ComputeLocalToWorldTransform(UsdTimeCode::Default());
        auto onTransform = TryGetTransform(geom);
        collection.transform = onTransform;
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

bool TryAddToDocument(CRhinoDoc& doc, ON_Geometry* geom, ON_3dmObjectAttributes* attribs)
{
  if (ON_Mesh* mesh = ON_Mesh::Cast(geom))
  {
    doc.AddMeshObject(*mesh, attribs);
    return true;
  }

  if (ON_RevSurface* surface = ON_RevSurface::Cast(geom))
  {
    doc.AddSurfaceObject(*surface, attribs);
    return true;
  }

  if (ON_Brep* brep = ON_Brep::Cast(geom))
  {
    doc.AddBrepObject(*brep, attribs);
    return true;
  }

  if (ON_Curve* curve = ON_Curve::Cast(geom))
  {
    doc.AddCurveObject(*curve, attribs);
    return true;
  }

  // TODO : Will above prevent this?
  if (ON_NurbsCurve* nurbscurve = ON_NurbsCurve::Cast(geom))
  {
    doc.AddCurveObject(*nurbscurve, attribs);
    return true;
  }

  if (ON_NurbsSurface* nurbssurface = ON_NurbsSurface::Cast(geom))
  {
    doc.AddSurfaceObject(*nurbssurface, attribs);
    return true;
  }

  if (ON_PointCloud* pointcloud = ON_PointCloud::Cast(geom))
  {
    // TODO : More Complex than above
    // doc.AddPointCloudObject(*pointcloud, attribs);
    return true;
  }

  return false;
}

bool ReadUSDFile(const wchar_t* filename, CRhinoDoc& doc, const CRhinoFileReadOptions& options)
{
  // use an ON_String to convert from unicode to mbcs which is what usd wants
  ON_String usdPath(filename);
  UsdStageRefPtr usdModel = UsdStage::Open(usdPath.Array());
  
  // TODO : Layers
  auto layerStack = usdModel->GetLayerStack();
  size_t st = layerStack.size();

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

    if (ON_Geometry* geom = TryGetPrimGeometry(prim))
    {
      ON_3dmObjectAttributes* attribs = TryGetAttributesFromPrim(prim);
      TryAddToDocument(doc, geom, attribs);
    }

  }

  return true;
}
