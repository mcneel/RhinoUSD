#include "stdafx.h"
#include "UsdExportOptions.h"
#include "UsdExportPacket.h"
#pragma once

int GetPackets(CRhinoDoc& doc, const CRhinoFileWriteOptions& options, UsdExportOptions& usdOptions, ON_ClassArray<UsdPacket>& packets);

int WriteUSDFile(const wchar_t* filename, CRhinoDoc& doc, ON_ClassArray<UsdPacket>& packets, const UsdExportOptions& usdOptions);
