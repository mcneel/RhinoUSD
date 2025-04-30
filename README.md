# RhinoUSD
This repository contains the source code and Visual Studio solution (.sln) to build **RhinoUSD**, Rhino file export and import plug-in for Pixar's [USD](https://graphics.pixar.com/usd/release/index.html) file format.


The solution contains the following projects:

- **opennurbs_usd_console** - C++ console application that uses the open source version of opennurbs
- **export_USD** - A Rhino C++ plug-in that exports .USD files 
- **import_USD** - A Rhino C++ plug-in that imports .USD files


export_USD and import_USD require the Rhino C++ SDK be installed.
https://www.rhino3d.com/download/Rhino-SDK/7.0/latest/
