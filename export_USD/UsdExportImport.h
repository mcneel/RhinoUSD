#pragma once
#include "stdafx.h"
#include "iostream"
#include <fstream>

#include "UsdExportOptions.h"
#include "../UsdShared/UsdPacket.h"

using namespace pxr;

// Move this to it's own header/cpp
class UsdExportImport
{
public:
  UsdExportImport(const ON_wString& fileName, double metersPerUnit, const UsdExportOptions& options, CRhinoDoc& doc);
  ON_wString AddMesh(const ON_Mesh* mesh, const ON_wString meshName, const std::vector<ON_wString>& layerNames, const std::map<int, ON_TextureCoordinates>& tcs);
  //void AddAndBindMaterial(const ON_Material* material, const std::vector<ON_wString>& layerNames, const ON_wString meshPath);
  void AddMaterialWithTexturesIfNotAlreadyAdded(unsigned int docSerNo, const ON_UUID& matId, const ON_wString& matName, const ON_PhysicallyBasedMaterial* pbrMaterial, const ON_ObjectArray<ON_Texture>& textures);
  void BindPbrMaterialToMesh(const ON_UUID& matId, const ON_wString meshPath);
  void AddNurbsCurve(const ON_NurbsCurve* nurbsCurve, const std::vector<ON_wString>& layerNames);
  void AddNurbsSurface(const ON_NurbsSurface* nurbsSurface, const std::vector<ON_wString>& layerNames);
  bool AddBlock(const std::shared_ptr<UsdPacket> packet, const UsdExportOptions& usdOptions);
  void GetInstancePackets(const CRhinoInstanceDefinition* definition, ON_ClassArray<std::shared_ptr<UsdPacket>>& packets);
  bool GetPacketsFromCRhinoObjects(ObjectArray& objects, const CRhinoFileWriteOptions& fileOptions, ON_ClassArray<std::shared_ptr<UsdPacket>>& packets, int mesh_ui_style = -1);
  const ON_Mesh& GetMeshFromSubD(ON_SubD& subD, const ON_MeshParameters mp);
  bool AnythingToSave();
  void Save(/*const ON_wString& fileName*/);

  void WriteObject(std::shared_ptr<UsdPacket>& packet, const UsdExportOptions& usdOptions);
  bool AddCurve(const std::shared_ptr<UsdPacket> packet, const UsdExportOptions& usdOptions);
  bool AddMesh(std::shared_ptr<UsdPacket> packet, const UsdExportOptions& usdOptions);

  std::vector<ON_wString> GetLayerNames(const std::shared_ptr<UsdPacket> packet);

  void SetDefaultPrim();

  const UsdExportOptions& UsdOptions;

private:
  CRhinoDoc& Doc;

  // pxr stuff
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
