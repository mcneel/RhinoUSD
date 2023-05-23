#pragma once

class ON_wString;
class ON_Mesh;

using namespace pxr;

class UsdExportImport
{
public:
  UsdExportImport(const ON_wString& fileName);
  ON_wString AddMesh(const ON_Mesh* mesh, const std::vector<ON_wString>& layerNames, const std::map<int, const ON_TextureCoordinates*>& tcs);
  //void AddAndBindMaterial(const ON_Material* material, const std::vector<ON_wString>& layerNames, const ON_wString meshPath);
  void AddAndBindPbrMaterialAndTextures(const ON_PhysicallyBasedMaterial* pbrMaterial, const ON_ObjectArray<ON_Texture>& textures, const std::vector<ON_wString>& layerNames, const ON_wString meshPath);
  void AddNurbsCurve(const ON_NurbsCurve* nurbsCurve, const std::vector<ON_wString>& layerNames);
  void AddNurbsSurface(const ON_NurbsSurface* nurbsSurface, const std::vector<ON_wString>& layerNames);
  bool AnythingToSave();
  void Save(/*const ON_wString& fileName*/);
private:
  const std::string usdFileName;
  pxr::TfToken TextureTypeToUsdPbrPropertyTfToken(ON_Texture::TYPE& type);
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
  std::string PathFromFullFileName(const std::string& fileName);
  std::string FileNameFromFullFileName(const std::string& fileName);
  void CopyFileTo(const std::string& fullFileName, const std::string& destination);
  bool IsAcceptableUsdCharacter(wchar_t c);
  ON_wString RhinoLayerNameToUsd(const ON_wString& rhLayerName);
  ON_wString WriteUSDMesh(UsdStageRefPtr usdModel, const ON_Mesh* mesh, ON_wString& path, int index, const std::map<int, const ON_TextureCoordinates*>& tcs);
  void SetUsdLayersAsXformable(const std::vector<ON_wString>& layerNames, UsdStageRefPtr stage);
}
