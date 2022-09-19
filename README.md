# RhinoUSD
This repository contains the source code and Visual Studio solution (.sln) to build **RhinoUSD**, Rhino file export and import plug-in for Pixar's [USD](https://graphics.pixar.com/usd/release/index.html) file format.

The solution contains the following projects:

- **ExportUSD** - A Rhino C++ plug-in that exports .USD files. The project links with the static USD library. 
- **ImportUSD** - A Rhino C++ plug-in that imports .USD files. The project links with the static USD library.

In order to build this solution, you will need to build static library version of USD . Microsoft's C/C++ package manager, [vcpkg](https://vcpkg.io/en/index.html), makes building OpenVDB is relatively easy. And, vcpkg is cross-platform.

## Building Windows

Here is how to build USD on Windows:

1.) **Install vcpkg**

1.1.) **Clone the vcpkg repo**

```
git clone https://github.com/microsoft/vcpkg
```

1.2.) **Run the bootstrap script to build vcpkg**

```
.\vcpkg\bootstrap-vcpkg.bat
```

2.) **Copy the custom triplet**

A vcpkg triplet contains the rules for building a package. 

Copy the `x64-windows-usd.cmake` file from the solution's **triplets** folder into your vcpkg **triplets** folder. 

For example, if you cloned vcpkg here:

```
C:\dev\github>
```

then copy `x64-windows-usd.cmake` here:

```
C:\dev\github\vcpkg\triplets
```

3.) **Install USD**

Install OpenVDB and all of its dependencies using our custom triplet.

```
.\vcpkg\vcpkg install usd:x64-windows-usd
```

Now grab some coffee and sit back - this will take a while (~15 minutes on my laptop)..

4.) **Export USD**

Now that you've installed and built the USD package, export it to a NuGet package so others who don't have vcpkg and USD installed can still use it.

4.1) Run the following vcpkg command:

```
.\vcpkg export usd:x64-windows-usd --nuget
```

After a minute or two, you'll see something like the following echoed to the command window:

```
NuGet package exported at: C:\dev\github\other\usd\vcpkg-export-20220919-112512.1.0.0.nupkg

With a project open, go to Tools->NuGet Package Manager->Package Manager Console and paste:
    Install-Package vcpkg-export-20220919-112512 -Source "C:\dev\github\other\usd"
```

4.2.) **Launch Visual Studio** and open `RhinoUSD.sln`. 

4.3.) From **Visual Studio**, pick `Tools > NuGet Package Manager > Package Manager Console`.

4.4.) In the **Package Manager Console**.

4.5.) Run the following Package Manager command for both the **ExportUSD** and **ImportUSD** projects:

```
Install-Package vcpkg-export-20220919-112512 -Source "<path_to_nuget_package>"
```

4.6.) Installing the NuGet package will add a new `packages.config` file to the project. If this isn't already add to GitHub, make sure to add it.

4.7.) Done!

