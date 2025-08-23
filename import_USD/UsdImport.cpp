#include "stdafx.h"
#include <string>

#include "../UsdShared/ON_Helpers.h"

#include "convert_geometry.h"
#include "convert_metadata.h"
#include "UsdImportPacket.h"
#include "UsdImport.h"

bool UsdImport::ReadFile()
{
  const ON_wString wPath(m_filename);
  const std::string filePath = ON_Helpers::ON_wString_to_StdString(wPath);
  Model = pxr::UsdStage::Open(filePath);
  
  // TODO : Layers
  auto layerStack = Model->GetLayerStack();
  size_t st = layerStack.size();
  
  // TODO : Use Stage Direction to set Rhino WorldXY

  // Traverse all prims in the stage
  // NOTE : This is ALL prims, including ones we likely don't want,
  //        e.g abstract, hidden, etc! Ensure to filter correctly.

  // NOTE : How does the traverse travel? Is it to the first leaf and recursively down?
  pxr::UsdPrimRange primRange = Model->Traverse(); // Use TraverseAll if it feels like we're missing something
  // usdModel->Flatten(); // <- Hmm
  pxr::UsdPrim root = Model->GetDefaultPrim();
  // TraversePrimTree(root, m_doc, PrimDataCollection());

  ON_Layer* previousLayer;
  for (pxr::UsdPrim prim : primRange)
  {
    // TODO : This also checks for ancestors
    if (prim.IsAbstract()) continue;
    if (!prim.IsValid()) continue;

    bool hidden = prim.IsHidden();

    auto description = prim.GetDescription();
    auto name = prim.GetDisplayName();
    
    int layerIndex = ConvertMetadata::GetOrCreateLayerIndex(m_doc, prim);

    pxr::TfTokenVector properties = prim.GetPropertyNames();
    for (auto& property : properties)
    {
      auto propertyString = property.GetString();
      int t = 7;
    }

    if (std::shared_ptr<ON_Geometry> geom = ConvertGeometry::TryGetPrimGeometry(prim))
    {
      std::shared_ptr<const ON_3dmObjectAttributes> attribs = ConvertMetadata::TryGetAttributesFromPrim(prim);
      TryAddToDocument(geom, attribs);
    }

  }
  
  return true;
}

bool UsdImport::AddPrimDataToDoc(UsdImportPacket& data)
{
  ON_Geometry* geom = data.GetGeometry().get();
  if (geom == nullptr) return false;

  ON_3dmObjectAttributes initAttribs;
  initAttribs.SetVisible(data.isVisible);
  
  const ON_3dmObjectAttributes attribs(initAttribs);

  const ON_Xform xForm = data.GetXForm();
  geom->Transform(xForm);

  if (auto brep = ON_Brep::Cast(geom))
  {
    m_doc.AddBrepObject(*brep, &attribs);
  }
  else if (auto mesh = ON_Mesh::Cast(geom))
  {
    m_doc.AddMeshObject(*mesh, &attribs);
  }
  else if (auto curve = ON_Curve::Cast(geom))
  {
    m_doc.AddCurveObject(*curve, &attribs);
  }
  else if (auto surface = ON_NurbsSurface::Cast(geom))
  {
    m_doc.AddSurfaceObject(*surface, &attribs);
  }
  else if (auto pointCloud = ON_PointCloud::Cast(geom))
  {
    m_doc.AddPointCloudObject(pointCloud->PointCount(), pointCloud->m_P, &attribs);
  }
  else
  {
    return false;
  }

  return true;
}

void UsdImport::TraversePrimTree(pxr::UsdPrim& root, UsdImportPacket& collection)
{
  for (auto prim : root.GetChildren())
  {
    if (prim.IsAbstract()) continue;
    if (prim.IsValid()) continue;

    // TODO : How to get Layers?

    // It's Geometry!
    // Are classes that inherit this going to return true?
    std::shared_ptr<ON_Geometry> onGeom = ConvertGeometry::TryGetPrimGeometry(prim);
    if (onGeom != nullptr)
    {
      collection.SetGeometry(onGeom);
      if (pxr::UsdGeomGprim geom = pxr::UsdGeomGprim(prim))
      {
        auto visibleAttribute = geom.GetVisibilityAttr();
        // bool visible = GetValueFromAttribute<bool>(visibleAttribute);
        bool visible = false;
        collection.isVisible = visible;

        // TODO : What to do if the Geometry is a Time Varying Transform? I'd assume get the first?
        //        Maybe settings could specify a time frame?
        auto usdTransform = geom.ComputeLocalToWorldTransform(pxr::UsdTimeCode::Default());
        auto onTransform = ConvertGeometry::TryGetTransform(geom);
        collection.SetTransform(onTransform);
      }
    }

    for (std::pair<pxr::TfToken, pxr::VtValue> metaData : prim.GetAllMetadata())
    {
      pxr::TfToken key = metaData.first;
      pxr::VtValue value = metaData.second;

      ON_UserString onUString;
      onUString.m_key = ON_Helpers::StdString_to_ON_wString(key.GetString());

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
        onUString.m_string_value = ON_Helpers::StdString_to_ON_wString(value.GetTypeName());
      }
        
      collection.userStrings.Append(onUString);
    }

  }

}

bool UsdImport::TryAddToDocument(std::shared_ptr<ON_Geometry> geomPtr, std::shared_ptr<const ON_3dmObjectAttributes> attribsPtr)
{
  ON_Geometry* geom = geomPtr.get();
  const ON_3dmObjectAttributes* attribs = attribsPtr.get();
  
  if (ON_Mesh* mesh = ON_Mesh::Cast(geom))
  {
    m_doc.AddMeshObject(*mesh, attribs);
    return true;
  }

  if (ON_RevSurface* surface = ON_RevSurface::Cast(geom))
  {
    m_doc.AddSurfaceObject(*surface, attribs);
    return true;
  }

  if (ON_Brep* brep = ON_Brep::Cast(geom))
  {
    m_doc.AddBrepObject(*brep, attribs);
    return true;
  }

  if (ON_Curve* curve = ON_Curve::Cast(geom))
  {
    m_doc.AddCurveObject(*curve, attribs);
    return true;
  }

  // TODO : Will above prevent this?
  if (ON_NurbsCurve* nurbscurve = ON_NurbsCurve::Cast(geom))
  {
    m_doc.AddCurveObject(*nurbscurve, attribs);
    return true;
  }

  if (ON_NurbsSurface* nurbssurface = ON_NurbsSurface::Cast(geom))
  {
    m_doc.AddSurfaceObject(*nurbssurface, attribs);
    return true;
  }

  if (ON_PointCloud* pointcloud = ON_PointCloud::Cast(geom))
  {
    // TODO : More Complex than above
    // m_doc.AddPointCloudObject(*pointcloud, attribs);
    return true;
  }

  return false;
}
