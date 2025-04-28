#include "stdafx.h"
#include "ExportUSDPlugIn.h"
#include "../UsdShared/ON_Helpers.h"
#include "../UsdShared/UsdShared.h"
#include "UsdExportOptions.h"
#include "UsdExportPacket.h"

static bool MeshPackets(ON_ClassArray<UsdPacket>& meshPackets,
  ON_ClassArray<UsdPacket>& packets,
  ON_SimpleArray<const CRhinoObject*> meshObjects,
  ON_Xform transform,
  ON_MeshParameters mp,
  int mesh_ui_style);

static void GetMeshParametersFromDictionary(const ON_ArchivableDictionary& dict, ON_MeshParameters& params);

static bool IsValidUsdObject(ON::object_type type);

int WriteUSDFile(const wchar_t* filename,
  bool usda,
  CRhinoDoc& doc,
  const CRhinoFileWriteOptions& options,
  bool scripting,
  UsdExportOptions& usdOptions,
   int mesh_ui_style);

int WriteUSDFile(const wchar_t* filename,
  CRhinoDoc& doc,
  ON_ClassArray<UsdPacket>& packets,
  const UsdExportOptions& usdOptions);

ON::object_type GetTypeFromObject(const CRhinoObject* obj);
