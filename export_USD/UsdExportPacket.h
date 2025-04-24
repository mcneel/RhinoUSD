#pragma once
#include "stdafx.h"

// Small packet of information for Usd Export
// Allows for bundling converted geometry with the original object which has lots of data
class UsdPacket
{
public:
  const CRhinoObject* RhinoObject;
  // Geometry will likely be different to the original
  // Especially in the case of meshing
  const ON_Geometry* NewGeometry;
  // Type is type of NewGeometry for quick switching.
  // To start most items will be Meshes
  ON::object_type Type;

  UsdPacket(const CRhinoObject* rhinoObject, const ON_Geometry* newGeometry, ON::object_type type)
    : RhinoObject(rhinoObject), NewGeometry(newGeometry), Type(type)
  {
  }

  UsdPacket(const CRhinoObject* rhinoObject, ON::object_type type)
    : RhinoObject(rhinoObject), NewGeometry(nullptr), Type(type)
  {
  }

};
