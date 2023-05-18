#pragma once

class ON_wString;
class ON_Mesh;

using namespace pxr;

class UsdExportImport
{
public:
  UsdExportImport();
  ON_wString AddMesh(const ON_Mesh* mesh, const std::vector<ON_wString>& layerNames, const std::map<int, const ON_TextureCoordinates*>& tcs);
  void AddAndBindMaterial(const ON_Material* material, const std::vector<ON_wString>& layerNames, const ON_wString meshPath);
  void AddAndBindPbrMaterial(const ON_PhysicallyBasedMaterial* pbrMaterial, const std::vector<ON_wString>& layerNames, const ON_wString meshPath);
  void AddNurbsCurve(const ON_NurbsCurve* nurbsCurve, const std::vector<ON_wString>& layerNames);
  void AddNurbsSurface(const ON_NurbsSurface* nurbsSurface, const std::vector<ON_wString>& layerNames);
  bool AnythingToSave();
  void Save(const ON_wString& fileName);
private:
  void __addAndBindMat(
    const ON_wString& namePrefix,
    const pxr::GfVec3f& diffuseColor,
    float opacity,
    float roughness,
    float metallic,
    float oior,
    float rior,
    float alpha,
    float clearcoat,
    float anisotropic,
    float sheen,
    float sheenTint,
    pxr::GfVec3f emission,
    float specular,
    float specularTint,
    const std::vector<ON_wString>& layerNames,
    const ON_wString meshPath
  );
  UsdStageRefPtr stage;
  int currentMeshIndex;
  int currentMaterialIndex;
  int currentShaderIndex;
  int currentNurbsCurveIndex;
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
  ON_wString WriteUSDMesh(UsdStageRefPtr usdModel, const ON_Mesh* mesh, ON_wString& path, int index, const std::map<int, const ON_TextureCoordinates*>& tcs);
  void SetUsdLayersAsXformable(const std::vector<ON_wString>& layerNames, UsdStageRefPtr stage);
}
