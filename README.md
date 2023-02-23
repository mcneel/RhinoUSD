# RhinoUSD
This repository contains the source code and Visual Studio solution (.sln) to build **RhinoUSD**, Rhino file export and import plug-in for Pixar's [USD](https://graphics.pixar.com/usd/release/index.html) file format.

The solution contains the following projects:

- **opennurbs_usd_console** - C++ console application that uses the open source version of opennurbs
- **ExportUSD** - A Rhino C++ plug-in that exports .USD files. The project links with the static USD library. 
- **ImportUSD** - A Rhino C++ plug-in that imports .USD files. The project links with the static USD library.

