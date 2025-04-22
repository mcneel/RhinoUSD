#pragma once
#include "Resource.h" // main symbols

using namespace std;

enum BlockHandling { SeparateFiles = 0, Ignore = 1, InsideFile = 2 };

class UsdExportOptions
{
public:
  // If Model Name is empty that layer/prim isn't included
  ON_wString ModelName = L"";
  
  // Default is L"World"
  ON_wString DefaultLayer = L"World";
  
  // How should blocks be handled?
  BlockHandling Blocks;
  
  // Make all Geometry Meshes
  bool ForceMeshes = false;
  
  // Include User Strings
  bool IncludeUserStrings = true;
};
