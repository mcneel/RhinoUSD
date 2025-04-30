#include "stdafx.h"
#include "../../../RhinoUiHooks.h"
#include "UsdExportOptions.h"

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

  if (!RhExecuteNamedCallback(L"ShowExportUsdDialog", args)) return false;
  bool userChoseOk;
  if (!args.GetBool(L"success", userChoseOk)) return false;
  if (!userChoseOk) return false;

  int blocksValue;
  ON_wString rootLayerValue;
  ON_wString modelNameValue;
  bool forceMeshesValue;
  bool includeUserStringsValue;

  if (args.GetInt(L"blocks", blocksValue))
  {
    options.Blocks = (BlockHandling)blocksValue;
  }

  if (args.GetString(L"root-layer", rootLayerValue))
  {
    // TODO : Validate that root layer is not null or empty
    options.RootLayer = rootLayerValue;
  }

  if (args.GetString(L"model-name", modelNameValue))
  {
    options.ModelName = modelNameValue;
  }

  if (args.GetBool(L"force-meshes", forceMeshesValue))
  {
    options.ForceMeshes = forceMeshesValue;
  }

  if (args.GetBool(L"include-user-strings", includeUserStringsValue))
  {
    options.IncludeUserStrings = includeUserStringsValue;
  }

  return true;
}
