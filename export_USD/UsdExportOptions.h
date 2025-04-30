#pragma once
#include "stdafx.h"

using namespace std;

enum class BlockHandling : int { SeparateFiles = 0, Ignore = 1, InsideFile = 2 };

class UsdExportOptions
{
public:
  // If Model Name is empty that layer/prim isn't included
  ON_wString ModelName = DefaultModelName;
  ON_wString RootLayer = DefaultRootLayer;
  BlockHandling Blocks = DefaultBlocks;
  bool ForceMeshes = DefaultForceMeshes;
  bool IncludeUserStrings = DefaultIncludeUserStrings;

  // Non-User Settings
  bool Headless = false;

  // Meshing Settings
  ON_MeshParameters MeshingParams;
  

  ON_wString DefaultModelName = L"";
  ON_wString DefaultRootLayer = L"World";
  BlockHandling DefaultBlocks = BlockHandling::SeparateFiles;
  bool DefaultForceMeshes = false;
  bool DefaultIncludeUserStrings = true;

};

bool HandleUserInput(UsdExportOptions& options);
