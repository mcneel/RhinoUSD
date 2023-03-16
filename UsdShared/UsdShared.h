#pragma once

class ON_wString;
class ON_Mesh;

using namespace pxr;

class UsdExportImport
{
public:
  UsdExportImport();
  void AddMesh(const ON_Mesh* mesh, const std::vector<ON_wString>& layerNames);
  bool AnythingToSave();
  void Save(const ON_wString& fileName);
private:
  UsdStageRefPtr stage;
  int currentMeshIndex;
};

namespace UsdShared
{
  bool IsAcceptableUsdCharacter(wchar_t c);
  ON_wString RhinoLayerNameToUsd(const ON_wString& rhLayerName);
  bool WriteUSDMesh(UsdStageRefPtr usdModel, const ON_Mesh* mesh, ON_wString& path, int index);
  void SetUsdLayersAsXformable(const std::vector<ON_wString>& layerNames, UsdStageRefPtr stage);
}
