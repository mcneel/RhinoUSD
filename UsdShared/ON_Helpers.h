#pragma once

#include <string>
#include <vector>

class ON_wString;

namespace ON_Helpers 
{
  std::string ON_wStringToStdString(const ON_wString& onwstr);
  ON_wString StringVectorToPath(const std::vector<ON_wString>& names);
  void RotateYUp(ON_Mesh* mesh);
  void RotateGeometryYUp(ON_Geometry* geom);
}
