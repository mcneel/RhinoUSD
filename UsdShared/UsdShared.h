#pragma once
#include "stdafx.h"
#include "UsdPacket.h"

class ON_wString;
class ON_Mesh;

using namespace pxr;

namespace UsdShared
{
  ON_wString PathWithoutExtension(const ON_wString& fullFileName);
  void CopyFileTo(const ON_wString& fullFileName, const ON_wString& destination);
  void CreateUsdzFile(const ON_wString& fullFileNameNoExtension, const std::vector<ON_wString>& filesToInclude);
  bool IsAcceptableUsdCharacter(wchar_t c);
  ON_wString RhinoLayerNameToUsd(const ON_wString& rhLayerName, const ON_wString& defaultName = L"Default");
  void SetUsdLayersAsXformable(const std::vector<ON_wString>& layerNames, UsdStageRefPtr stage);
  bool IsValidUsdObject(ON::object_type type);
  ON::object_type GetTypeFromObject(const CRhinoObject* obj);
  void AddUserDataToPrim(const std::shared_ptr<UsdPacket> packet, pxr::UsdPrim* prim);
  void SetTextureCoordinatesOnMesh(const CRhinoObject& obj, ON_Mesh* pMesh, const CRhinoDoc* doc, std::map<int, ON_TextureCoordinates>& tcs);
}
