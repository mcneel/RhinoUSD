# RhinoUSD
This repository contains the source code and Visual Studio solution (.sln) to build **RhinoUSD**, Rhino file export and import plug-in for Pixar's [USD](https://graphics.pixar.com/usd/release/index.html) file format.

The repository contains the following projects:

- **opennurbs_usd_console** - C++ console application that uses the open source version of opennurbs
- **export_USD** - A Rhino C++ plug-in that exports .USD files 
- **import_USD** - A Rhino C++ plug-in that imports .USD files


# Building
### Pre-requisites
- export_USD and import_USD require the [Rhino C++ SDK](https://www.rhino3d.com/download/Rhino-SDK/7.0/latest/) be installed.
- Visual Studio 2022
- A local copy of the Rhino source code
> NOTE: This project cannot be currently built or debugged without having access to the Rhino Source Code

# Relevant Documentation
If you're interested in debugging, understanding and even contributing to this repo, having a basic understanding of USD will go a long way to helping you understand the source code.

- USD Developer Docs - https://openusd.org/release/api/index.html
    - USD Scene and relevant types - https://openusd.org/release/api/usd_page_front.html
    - USD Package Overview - https://openusd.org/release/api/_usd__overview_and_purpose.html
- Visual Overview - https://www.youtube.com/watch?v=zRyKI55qEZY
