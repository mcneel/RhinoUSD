#pragma once

#include <string>
#include <vector>

class ON_wString;

namespace ON_Helpers 
{
  std::string ON_wStringToStdString(const ON_wString& onwstr);
  ON_wString ON_TextureTYPE_ToString(const ON_Texture::TYPE type);
  ON_wString StringVectorToPath(const std::vector<ON_wString>& names);
  ON_wString ON_UUID_to_ON_wString(const ON_UUID& uuid);
  std::string ON_UUID_to_StdString(const ON_UUID& uuid);
  void RotateYUp(ON_Mesh* mesh);
  void RotateGeometryYUp(ON_Geometry* geom);
}
