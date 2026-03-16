#include "stdafx.h"
#include "../../../RhinoUiHooks.h"
#include "UsdExportOptions.h"
#include "../UsdShared/UsdShared.h"

// True when user chnaged the Options, False on any other result
bool HandleUserInput(UsdExportOptions& options)
{
  CRhParameterDictionary args;

  // Plugin Settings
  args.SetInt(L"blocks", (int)options.Blocks);
  args.SetString(L"root-layer", options.RootLayer);
  args.SetString(L"model-name", options.ModelName);
  args.SetBool(L"force-meshes", options.ForceMeshes);
  args.SetBool(L"include-user-strings", options.IncludeUserStrings);
  args.SetBool(L"scripting", options.Headless);
  args.SetBool(L"space-replacement", options.SpaceReplacement);

  if (!RhExecuteNamedCallback(L"ShowExportUsdDialog", args)) return false;
  bool userChoseOk;
  if (!args.GetBool(L"success", userChoseOk)) return false;
  if (!userChoseOk) return false;

  int blocksValue;
  ON_wString rootLayerValue;
  ON_wString modelNameValue;
  bool forceMeshesValue;
  bool includeUserStringsValue;
  ON_wString splaceReplacementValue;

  if (args.GetInt(L"blocks", blocksValue))
  {
    options.Blocks = (BlockHandling)blocksValue;
  }

  if (args.GetString(L"root-layer", rootLayerValue))
  {
    options.RootLayer = UsdShared::RhinoLayerNameToUsd(rootLayerValue);
  }

  if (args.GetString(L"model-name", modelNameValue))
  {
    options.ModelName = UsdShared::RhinoLayerNameToUsd(modelNameValue, L"");
  }

  if (args.GetBool(L"force-meshes", forceMeshesValue))
  {
    options.ForceMeshes = forceMeshesValue;
  }

  if (args.GetBool(L"include-user-strings", includeUserStringsValue))
  {
    options.IncludeUserStrings = includeUserStringsValue;
  }
  
  if (args.GetString(L"space-replacement", splaceReplacementValue))
  {
    options.SpaceReplacement = splaceReplacementValue;
  }

  return true;
}
