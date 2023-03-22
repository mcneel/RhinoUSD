#pragma once

class ON_wString;
class ON_Mesh;

using namespace pxr;

class UsdExportImport
{
public:
  UsdExportImport();
  void AddMesh(const ON_Mesh* mesh, const std::vector<ON_wString>& layerNames);
  void AddMaterial(const ON_Material* material, const std::vector<ON_wString>& layerNames);
  void AddPbrMaterial(const ON_PhysicallyBasedMaterial* pbrMaterial, const std::vector<ON_wString>& layerNames);
  bool AnythingToSave();
  void Save(const ON_wString& fileName);
private:
  void __addMat(const pxr::GfVec3f& diffuseColor, float opacity, const std::vector<ON_wString>& layerNames);
  UsdStageRefPtr stage;
  int currentMeshIndex;
  int currentMaterialIndex;
  int currentShaderIndex;
  pxr::TfToken tokPreviewSurface;
  pxr::TfToken tokSurface;
  pxr::TfToken tokDiffuseColor;
  pxr::TfToken tokOpacity;
};

namespace UsdShared
{
  bool IsAcceptableUsdCharacter(wchar_t c);
  ON_wString RhinoLayerNameToUsd(const ON_wString& rhLayerName);
  bool WriteUSDMesh(UsdStageRefPtr usdModel, const ON_Mesh* mesh, ON_wString& path, int index);
  void SetUsdLayersAsXformable(const std::vector<ON_wString>& layerNames, UsdStageRefPtr stage);
}
