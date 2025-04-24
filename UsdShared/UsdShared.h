#pragma once
#include "stdafx.h"
#include "UsdExportPacket.h"
#include "UsdExportOptions.h"

class ON_wString;
class ON_Mesh;

using namespace pxr;

class UsdExportImport
{
public:
  UsdExportImport(const ON_wString& fileName, double metersPerUnit);
  void WriteObject(UsdPacket& packet, const UsdExportOptions& usdOptions);
  //void AddAndBindMaterial(const ON_Material* material, const std::vector<ON_wString>& layerNames, const ON_wString meshPath);
  void AddMaterialWithTexturesIfNotAlreadyAdded(unsigned int docSerNo, const ON_UUID& matId, const ON_wString& matName, const ON_PhysicallyBasedMaterial* pbrMaterial, const ON_ObjectArray<ON_Texture>& textures);
  void BindPbrMaterialToMesh(const ON_UUID& matId,const ON_wString meshPath);
  
  void AddNurbsSurface(const ON_NurbsSurface* nurbsSurface, const std::vector<ON_wString>& layerNames);
  void AddUserDataToPrim(const UsdPacket& packet, pxr::UsdPrim* prim);

  bool AnythingToSave();
  void Save(/*const ON_wString& fileName*/);

  bool AddMesh(UsdPacket& packet, const UsdExportOptions& usdOptions);
  bool AddCurve(const UsdPacket& packet, const UsdExportOptions& usdOptions);
	bool AddBlock(const UsdPacket& packet, const UsdExportOptions& usdOptions);

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

  std::vector<ON_wString> GetLayerNames(const UsdPacket& packet, const UsdExportOptions& usdOptions);
};

namespace UsdShared
{
  ON_wString PathWithoutExtension(const ON_wString& fullFileName);
  void CopyFileTo(const ON_wString& fullFileName, const ON_wString& destination);
  void CreateUsdzFile(const ON_wString& fullFileNameNoExtension, const std::vector<ON_wString>& filesToInclude);
  bool IsAcceptableUsdCharacter(wchar_t c);
  ON_wString RhinoLayerNameToUsd(const ON_wString& rhLayerName);
  void SetUsdLayersAsXformable(const std::vector<ON_wString>& layerNames, UsdStageRefPtr stage);
  void SetStringMap(std::multimap<const ON_UUID, const ON_wString>& sm);
  void WorkoutTextureCoordinates(const int mapping_channel_id, const std::map<int, const ON_TextureCoordinates*>& mappingCoordinatesOnMesh, std::vector<const ON_TextureCoordinates>& tcs);
  void SetTextureCoordinatesOnMesh(const CRhinoObject& obj, ON_Mesh* pMesh, const CRhinoDoc* doc, std::map<int, ON_TextureCoordinates>& tcs);
}
