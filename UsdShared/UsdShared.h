#pragma once

class ON_wString;
class ON_Mesh;

using namespace pxr;

class UsdExportImport
{
public:
  UsdExportImport(const ON_wString& fileName, double metersPerUnit);
  ON_wString AddMesh(const ON_Mesh* mesh, const ON_wString meshName, const std::vector<ON_wString>& layerNames, const std::map<int, ON_TextureCoordinates>& tcs);
  //void AddAndBindMaterial(const ON_Material* material, const std::vector<ON_wString>& layerNames, const ON_wString meshPath);
  void AddMaterialWithTexturesIfNotAlreadyAdded(unsigned int docSerNo, const ON_UUID& matId, const ON_wString& matName, const ON_PhysicallyBasedMaterial* pbrMaterial, const ON_ObjectArray<ON_Texture>& textures);
  void BindPbrMaterialToMesh(const ON_UUID& matId,const ON_wString meshPath);
  void AddNurbsCurve(const ON_NurbsCurve* nurbsCurve, const std::vector<ON_wString>& layerNames);
  void AddNurbsSurface(const ON_NurbsSurface* nurbsSurface, const std::vector<ON_wString>& layerNames);
  bool AnythingToSave();
  void Save(/*const ON_wString& fileName*/);
private:
  //std::vector<std::tuple<pxr::TfToken, ON_Texture::TYPE, std::string>> usd_texture_pbr_mapping;
  std::vector<ON_wString> filesInExport;
  // ON_UUID cannot be used as the key to a std::map
  std::map<std::string, ON_wString> materialsAddedToScene;
  const ON_wString usdFullFileName;
  double metersPerUnit;
  pxr::TfToken TextureTypeToUsdPbrPropertyTfToken(ON_Texture::TYPE& type);
  UsdStageRefPtr stage;
  int currentMeshIndex;
  //int currentMaterialIndex;
  int currentShaderIndex;
  int currentNurbsCurveIndex;
  pxr::TfToken tokPreviewSurface;
  pxr::TfToken tokSurface;

  // UsdPreviewSurfaceInputs
  pxr::TfToken tokDiffuseColor;
  pxr::TfToken tokEmissiveColor;
  pxr::TfToken tokUseSpecularWorkflow;
  //pxr::TfToken tokSpecularColor;
  pxr::TfToken tokMetallic;
  pxr::TfToken tokRoughness;
  pxr::TfToken tokClearcoat;
  //pxr::TfToken tokClearcoatRoughness;
  pxr::TfToken tokOpacity;
  // Andy says to ignore opacity threshold
  //pxr::TfToken tokOpacityThreshold;
  pxr::TfToken tokIor;

  // Andy: "This is related to bump. We might have to think carefully about this"
  //pxr::TfToken tokNormal;

  pxr::TfToken tokDisplacement;
  pxr::TfToken tokOcclusion;
};

namespace UsdShared
{
  ON_wString PathWithoutExtension(const ON_wString& fullFileName);
  void CopyFileTo(const ON_wString& fullFileName, const ON_wString& destination);
  void CreateUsdzFile(const ON_wString& fullFileNameNoExtension, const std::vector<ON_wString>& filesToInclude);
  bool IsAcceptableUsdCharacter(wchar_t c);
  ON_wString RhinoLayerNameToUsd(const ON_wString& rhLayerName);
  void SetUsdLayersAsXformable(const std::vector<ON_wString>& layerNames, UsdStageRefPtr stage);
}
