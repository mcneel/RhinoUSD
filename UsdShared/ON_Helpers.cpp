#include "stdafx.h"
#include "ON_Helpers.h"

std::string ON_Helpers::ON_wStringToStdString(const ON_wString& onwstr)
{
  // todo: problems with losing data
  std::wstring wstr(static_cast<const wchar_t*>(onwstr));
  std::string str(wstr.begin(), wstr.end());
  return str;
}

ON_wString ON_Helpers::StringVectorToPath(const std::vector<ON_wString>& names)
{
  ON_wString path;
  for (ON_wString name : names) {
    path = path + L"/" + name;
  }
  return path;
}

void ON_Helpers::RotateYUp(ON_Mesh* mesh)
{
  ON_Xform rotate_y_up;
  double ninetyDegrees = ON_PI / -2.0;
  rotate_y_up.Rotation(ninetyDegrees, ON_3dVector::XAxis, ON_3dPoint::Origin);
  mesh->Transform(rotate_y_up);
}

void ON_Helpers::RotateGeometryYUp(ON_Geometry* geom)
{
  ON_Xform rotate_y_up;
  double ninetyDegrees = ON_PI / -2.0;
  rotate_y_up.Rotation(ninetyDegrees, ON_3dVector::XAxis, ON_3dPoint::Origin);
  geom->Transform(rotate_y_up);
}
