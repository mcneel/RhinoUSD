#include "stdafx.h"
#include "UsdExportOptions.h"
#pragma once

int WriteUSDFile(const wchar_t* filename, bool usda, CRhinoDoc& doc, const CRhinoFileWriteOptions& options, UsdExportOptions& usdOptions);
