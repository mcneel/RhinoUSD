#pragma once

class ON_wString;
class ON_Mesh;

using namespace pxr;

class UsdExportImport
{
public:
  UsdExportImport();
  ON_wString AddMesh(const ON_Mesh* mesh, const std::vector<ON_wString>& layerNames);
  void AddAndBindMaterial(const ON_Material* material, const std::vector<ON_wString>& layerNames, const ON_wString meshPath);
  void AddAndBindPbrMaterial(const ON_PhysicallyBasedMaterial* pbrMaterial, const std::vector<ON_wString>& layerNames, const ON_wString meshPath);
  bool AnythingToSave();
  void Save(const ON_wString& fileName);
private:
  void __addAndBindMat(const pxr::GfVec3f& diffuseColor, float opacity, float roughness, float metallic, const std::vector<ON_wString>& layerNames, const ON_wString meshPath);
  UsdStageRefPtr stage;
  int currentMeshIndex;
  int currentMaterialIndex;
  int currentShaderIndex;
  pxr::TfToken tokPreviewSurface;
  pxr::TfToken tokSurface;
  pxr::TfToken tokDiffuseColor;
  pxr::TfToken tokOpacity;
  pxr::TfToken tokRoughness;
  pxr::TfToken tokMetallic;
};

namespace UsdShared
{
  bool IsAcceptableUsdCharacter(wchar_t c);
  ON_wString RhinoLayerNameToUsd(const ON_wString& rhLayerName);
  ON_wString WriteUSDMesh(UsdStageRefPtr usdModel, const ON_Mesh* mesh, ON_wString& path, int index);
  void SetUsdLayersAsXformable(const std::vector<ON_wString>& layerNames, UsdStageRefPtr stage);
}
