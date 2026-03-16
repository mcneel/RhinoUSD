#pragma once
#include "stdafx.h"

using namespace std;

enum class BlockHandling : int { SeparateFiles = 0, Ignore = 1, InsideFile = 2 };

class UsdExportOptions
{
public:
  UsdExportOptions()
    : ModelName(DefaultModelName),
    RootLayer(DefaultRootLayer),
    Blocks(DefaultBlocks),
    ForceMeshes(DefaultForceMeshes),
    IncludeUserStrings(DefaultIncludeUserStrings),
    Headless(false),
    SpaceReplacement(L"-")
  {

  }

  ON_wString DefaultModelName = L"";
  ON_wString DefaultRootLayer = L"World";
  BlockHandling DefaultBlocks = BlockHandling::SeparateFiles;
  bool DefaultForceMeshes = false;
  bool DefaultIncludeUserStrings = true;
  ON_wString DefaultSpaceReplacement = L"-";

  // If Model Name is empty that layer/prim isn't included
  ON_wString ModelName;
  ON_wString RootLayer;
  BlockHandling Blocks;
  bool ForceMeshes;
  bool IncludeUserStrings;
  ON_wString SpaceReplacement = L"-";

  // Non-User Settings
  bool Headless;

  // Meshing Settings
  ON_MeshParameters MeshingParams;

};

bool HandleUserInput(UsdExportOptions& options);
