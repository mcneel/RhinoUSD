#pragma once
#include "Resource.h" // main symbols

using namespace std;

enum BlockHandling { SeparateFiles = 0, Ignore = 1, InsideFile = 2 };

class UsdExportOptions
{
public:
  // If Model Name is empty that layer/prim isn't included
  ON_wString ModelName;
  
  // Default is L"World"
  ON_wString DefaultLayer;
  
  // How should blocks be handled?
  BlockHandling Blocks;
  
  // Make all Geometry Meshes
  bool ForceMeshes;
  
  // Include User Strings
  bool IncludeUserStrings;


  ON_wString DefaultModelName = L"";
  ON_wString DefaultDefaultLayer = L"World";
  BlockHandling DefaultBlocks = (BlockHandling)0;
  bool DefaultForceMeshes = false;
  bool DefaultIncludeUserStrings = true;

};
