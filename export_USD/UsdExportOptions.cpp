#include "stdafx.h"
#include "../../../RhinoUiHooks.h"
#include "UsdExportOptions.h"


// True when user chnaged the Options, False on any other result
bool HandleUserInput(bool scripting, CRhParameterDictionary& args, UsdExportOptions& options)
{
  // Plugin Settings
  args.SetInt(L"blocks", (int)options.Blocks);
  args.SetString(L"default-layer", options.DefaultLayer);
  args.SetString(L"model-name", options.ModelName);
  args.SetBool(L"force-meshes", options.ForceMeshes);
  args.SetBool(L"include-user-strings", options.IncludeUserStrings);

  if (!RhExecuteNamedCallback(L"ShowExportUsdDialog", args)) return false;
  bool userChoseOk;
  if (!args.GetBool(L"success", userChoseOk)) return false;
  if (!userChoseOk) return false;

  int blocksValue;
  ON_wString defaultLayerValue;
  ON_wString modelNameValue;
  bool forceMeshesValue;
  bool includeUserStringsValue;

  if (args.GetInt(L"blocks", blocksValue))
  {
    options.Blocks = (BlockHandling)blocksValue;
  }

  if (args.GetString(L"default-layer", defaultLayerValue))
  {
    options.DefaultLayer = defaultLayerValue;
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
