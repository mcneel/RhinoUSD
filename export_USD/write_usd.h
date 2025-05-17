#pragma once
#include "stdafx.h"
#include "UsdExportOptions.h"
#include "../UsdShared/UsdPacket.h"

int GetPackets(CRhinoDoc& doc, const CRhinoFileWriteOptions& fileOptions, const UsdExportOptions& usdOptions, ON_ClassArray<std::shared_ptr<UsdPacket>>& packets);

int WriteUSDFile(const wchar_t* filename, CRhinoDoc& doc, ON_ClassArray<std::shared_ptr<UsdPacket>>, const UsdExportOptions& usdOptions, const ON_wString exportReferenceName = L"");

bool MeshPackets(ON_ClassArray<std::shared_ptr<UsdPacket>>& meshPackets,
  ON_ClassArray<std::shared_ptr<UsdPacket>>& packets,
  ON_SimpleArray<const CRhinoObject*>& meshObjects,
  ON_Xform transform,
  ON_MeshParameters& mp,
  int mesh_ui_style);
