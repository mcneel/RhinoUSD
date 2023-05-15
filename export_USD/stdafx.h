// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN                             // Exclude rarely-used stuff from Windows headers
#endif

// This plug-in is Rhino 6 ready
#define RHINO_V6_READY

// If you want to use Rhino's MFC UI classes, then
// uncomment the #define RHINO_SDK_MFC statement below. 
// Note, doing so will requrie that your plug-in is
// built with the same version of Visual Studio as was
// used to build Rhino.
#define RHINO_SDK_MFC

// Plug-ins must use the release version of MFC used by Rhino.
// Plug-ins that require debugging information need to be built with
// RHINO_DEBUG_PLUGIN defined.
#if defined(RHINO_DEBUG_PLUGIN) && defined(_DEBUG)
//  Rhino 6 Debug plug-ins should define RHINO_DEBUG_PLUGIN, 
//  but not define _DEBUG in the .vcxproj file.
#error Do not define _DEBUG - use RHINO_DEBUG_PLUGIN instead
#endif

#if defined(MACOS) || defined(TARGET_OS_MAC)
#define CORE_BUILD
//#define ARRAYSIZE(array) sizeof(array)/sizeof(array[0])
#endif

// Rhino SDK Preamble
#if defined( CORE_BUILD )
#include "../../../RhinoCorePlugInStdAfx.h"
#include "../../../rhino3SystemPlugIn.h"

#include "../../RDK/RDK/RhRdkHeaders.h"
#pragma comment(lib, "\"" RHINO_PLUGIN_DIR "/" "rdk.lib" "\"")

#else

#include "RhinoSdkStdafxPreamble.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS       // some CString constructors will be explicit

#include <afxwin.h>                              // MFC core and standard components
#include <afxext.h>                              // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>                              // MFC OLE classes
#include <afxodlgs.h>                            // MFC OLE dialog classes
#include <afxdisp.h>                             // MFC Automation classes
#endif // _AFX_NO_OLE_SUPPORT

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>                               // MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>                              // MFC DAO database classes
#endif // _AFX_NO_DAO_SUPPORT

#include <afxdtctl.h>                            // MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                              // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


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

// Rhino SDK linking pragmas
#include "rhinoSdkPlugInLinkingPragmas.h"

#endif

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
#include "pxr/usd/usdGeom/metrics.h"
#include "pxr/usd/usdShade/material.h"
#include "pxr/usd/usdShade/materialBindingAPI.h"
#include "pxr/base/plug/registry.h"
#include "pxr/usd/usdGeom/nurbsCurves.h"
#include "pxr/usd/usdGeom/nurbsPatch.h"
#include "pxr/usd/usdGeom/primvarsAPI.h"
#pragma warning(pop)
