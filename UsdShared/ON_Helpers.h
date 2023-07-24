#pragma once

#include <string>
#include <vector>

class ON_wString;

namespace ON_Helpers 
{
  std::string ON_wString_to_StdString(const ON_wString& onwstr);
  ON_wString ON_TextureTYPE_to_ON_wString(const ON_Texture::TYPE type);
  ON_wString ON_wString_vector_to_ON_wString_path(const std::vector<ON_wString>& names);
  ON_wString ON_UUID_to_ON_wString(const ON_UUID& uuid);
  std::string ON_UUID_to_StdString(const ON_UUID& uuid);
  void RotateYUp(ON_Mesh* mesh);
  void RotateGeometryYUp(ON_Geometry* geom);
  template <class TYPE> TYPE DegreesToRadians(TYPE deg) { return deg * ON_PI / 180.0; }
  template <class TYPE> TYPE RadiansToDegrees(TYPE rad) { return rad / ON_PI * 180.0; }
}
