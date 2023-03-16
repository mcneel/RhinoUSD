#include "stdafx.h"
#include "write_usd.h"
#include <iostream>


int main()
{
  ON_FileIterator fit;
  fit.Initialize(L"./opennurbs/example_files/V7", L"*logo_nurbs.3dm");
  for (bool haveItem = fit.FirstItem(); haveItem; haveItem = fit.NextItem())
  {
    if (fit.CurrentItemIsFile())
    {
      ON_wString filename_3dm = fit.CurrentItemFullPathName();
      ON_wString filename = ON_FileSystemPath::FileNameFromPath(filename_3dm, false);
      ON_wString baseDirectory = ON_FileSystemPath::CombinePaths(ON_FileSystemPath::CurrentDirectory(false), false, L"output", false, false);
      ON_wString usdaPath = ON_FileSystemPath::CombinePaths(baseDirectory, false, filename + L".usda", true, false);
      ON_wString usdPath = ON_FileSystemPath::CombinePaths(baseDirectory, false, filename + L".usd", true, false);
      
      std::cout << "Converting " << ON_String(filename) << "\n";
      bool success = WriteUSDFile(filename_3dm, usdaPath);
      if (success)
        std::cout << "  writing .usda (success)\n";
      else
        std::cout << "  writing .usda (fail)\n";

      success = WriteUSDFile(filename_3dm, usdPath);
      if (success)
        std::cout << "  writing .usd (success)\n";
      else
        std::cout << "  writing .usd (fail)\n";
    }
  }
}
