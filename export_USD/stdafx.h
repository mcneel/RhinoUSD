#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN                             // Exclude rarely-used stuff from Windows headers
#endif

// This plug-in is Rhino 6 ready
#define RHINO_V6_READY

// Plug-ins must use the release version of MFC used by Rhino.
// Plug-ins that require debugging information need to be built with
// RHINO_DEBUG_PLUGIN defined.
#if defined(RHINO_DEBUG_PLUGIN) && defined(_DEBUG)
//  Rhino 6 Debug plug-ins should define RHINO_DEBUG_PLUGIN, 
//  but not define _DEBUG in the .vcxproj file.
#error Do not define _DEBUG - use RHINO_DEBUG_PLUGIN instead
#endif

// Rhino SDK Preamble
#include "RhinoSdkStdafxPreamble.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS       // some CString constructors will be explicit
#include <afxwin.h>                              // MFC core and standard components

#if defined(_M_X64) && defined(WIN32) && defined(WIN64)
//  The afxwin.h includes afx.h, which includes afxver_.h, 
//  which unconditionally defines WIN32  This is a bug.
//  Note, all Windows builds (32 and 64 bit) define _WIN32.
//  Only 64-bit builds define _WIN64. Never define/undefine
// _WIN32 or _WIN64.  Only define EXACTLY one of WIN32 or WIN64.
//  See the MSDN "Predefined Macros" help file for details.
#undef WIN32
#endif

// Rhino SDK classes
#include "RhinoSdk.h" 
// Rhino Render Development Kit (RDK) classes
#include "RhRdkHeaders.h" 

//#if defined(RHINO_DEBUG_PLUGIN)
// Now that all the system headers are read, we can
// safely define _DEBUG so the developers can test
// for _DEBUG in their code.
//#define _DEBUG
//#endif

// Rhino SDK linking pragmas
#include "rhinoSdkPlugInLinkingPragmas.h"

// Disable warnings that we have no control over. The USD library
// has a number of double to float warnings
#pragma warning(push)
#pragma warning(disable:4244)
#pragma warning(disable:4305)
#undef min
#undef max
#include "pxr/pxr.h"
#include "pxr/usd/sdf/layer.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/sdf/path.h"
#include "pxr/usd/usdGeom/mesh.h"
#include "pxr/usd/usdGeom/xform.h"
#include "pxr/usd/usdGeom/sphere.h"
#pragma warning(pop)
