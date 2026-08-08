#pragma once

#include <string>
#include <vector>

class ON_wString;

namespace ON_Helpers 
{
  std::string ON_wString_to_StdString(const ON_wString& onwstr);
  ON_wString StdString_to_ON_wString(const std::string& str);
  ON_wString ON_TextureTYPE_to_ON_wString(const ON_Texture::TYPE type);
  ON_wString ON_wString_vector_to_ON_wString_path(const std::vector<ON_wString>& names);
  ON_wString ON_UUID_to_ON_wString(const ON_UUID& uuid);
  std::string ON_UUID_to_StdString(const ON_UUID& uuid);
  void RotateGeometryYUp(ON_Geometry* geom);
  template <class TYPE> TYPE DegreesToRadians(TYPE deg) { return deg * ON_PI / 180.0; }
  template <class TYPE> TYPE RadiansToDegrees(TYPE rad) { return rad / ON_PI * 180.0; }

  // Returns:
  //  The rotation that takes Rhino's Z-up world into the Y-up world we author
  //  into USD. This is the same rotation RotateYUp() and RotateGeometryYUp()
  //  bake into the geometry they are handed.
  const ON_Xform YUpRotation();

  // Description:
  //  Converts a Rhino transformation into the equivalent USD transformation,
  //  expressed in the Y-up frame that the exported geometry lives in.
  const pxr::GfMatrix4d ConvertToYUp(const ON_Xform& xForm);

  // Description:
  //  Converts a Rhino transformation to a USD matrix.
  const pxr::GfMatrix4d Convert(const ON_Xform& xForm);
}
