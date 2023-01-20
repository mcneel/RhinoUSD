#include "StdAfx.h"
#include "ImportUSDPlugIn.h"
#include "Resource.h"

#pragma warning(push)
#pragma warning(disable : 4073)
#pragma init_seg(lib)
#pragma warning(pop)

// rhinoSdkPlugInDeclare.h defines the RHINO_PLUG_IN_DECLARE macro
#include "../../../SDK/inc/rhinoSdkPlugInDeclare.h"

// Rhino plug-in declarations
RHINO_PLUG_IN_DECLARE
RHINO_PLUG_IN_NAME(L"Import Pixar USD");
RHINO_PLUG_IN_ID(L"7C5D9A51-4AF7-4CE1-8661-30FE69473C6E");
RHINO_PLUG_IN_VERSION(RHINO_VERSION_NUMBER_STRING)
RHINO_PLUG_IN_UPDATE_URL(L"http://www.rhino3d.com/download");

// The one and only CImportUSDPlugIn object
static class CImportUSDPlugIn thePlugIn;

// CImportUSDPlugIn definition

const wchar_t* CImportUSDPlugIn::PlugInName() const
{
	return RhinoPlugInName();
}

const wchar_t* CImportUSDPlugIn::LocalPlugInName() const
{
  return RhLocalizeString( L"Import Pixar USD", 55139);
}

const wchar_t* CImportUSDPlugIn::PlugInVersion() const
{
  static wchar_t plugin_version[128] = { 0 };
  if (0 == plugin_version[0])
    ON_ConvertUTF8ToWideChar(false, RhinoPlugInVersion(), 0, plugin_version, sizeof(plugin_version) / sizeof(plugin_version[0]) - 1, 0, 0, 0, 0);
  return plugin_version;
}

GUID CImportUSDPlugIn::PlugInID() const
{
  static GUID plugin_id = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
  if (0 == plugin_id.Data1 && ON_UuidIsNil(plugin_id))
    plugin_id = ON_UuidFromString(RhinoPlugInId());
  return plugin_id;
}


void CImportUSDPlugIn::AddFileType(ON_ClassArray<CRhinoFileType>& extensions, const CRhinoFileReadOptions& options)
{
  CRhinoFileType ft;
  ft.SetFileTypePlugInID(PlugInID());
  ft.FileTypeDescription(RhLocalizeString( L"Pixar USD (*.usda, *.usdc, *.usdz)", 55291));
  ft.AddFileTypeExtension(RHSTR_LIT(L"usda"));
  ft.AddFileTypeExtension(RHSTR_LIT(L"usdc"));
  ft.AddFileTypeExtension(RHSTR_LIT(L"usdz"));
  extensions.Append(ft);
}

BOOL32 CImportUSDPlugIn::ReadFile(const wchar_t* filename, int index, CRhinoDoc& doc, const CRhinoFileReadOptions& options)
{
  if (nullptr == filename || 0 == filename[0] || !CRhinoFileUtilities::FileExists(filename))
    return 0;

  tinyusdz::Stage stage;
  std::string warn;
  std::string err;
  ON_String fname(filename);
  bool success = tinyusdz::LoadUSDFromFile(fname.Array(), &stage, &warn, &err);
  if (!success)
    return 0;

  std::vector<tinyusdz::Prim>& primatives = stage.root_prims();
  for (int i = 0; i < primatives.size(); i++)
  {
    tinyusdz::Prim& primative = primatives[i];
    const tinyusdz::GeomMesh* usdzmesh = primative.as<tinyusdz::GeomMesh>();
    if (usdzmesh)
    {
      const std::vector<tinyusdz::value::point3f> points = usdzmesh->get_points();
      const std::vector<int32_t> counts = usdzmesh->get_faceVertexCounts();
      const std::vector<int32_t> indices = usdzmesh->get_faceVertexIndices();

      if (points.size() < 1 || counts.size() < 1 || indices.size() < 1)
        continue;

      ON_Mesh mesh;
      mesh.m_V.Reserve(points.size());
      for (int j = 0; j < points.size(); j++)
      {
        const tinyusdz::value::point3f& pt = points[j];
        mesh.m_V.Append(ON_3fPoint(pt.x, pt.y, pt.z));
      }

      int currentIndex = 0;
      for (int j = 0; j < counts.size(); j++)
      {
        // only deal with 3 and 4 index faces for now
        int count = counts[j];
        if (3 == count || 4 == count)
        {
          ON_MeshFace face;
          face.vi[0] = indices[currentIndex];
          face.vi[1] = indices[currentIndex+1];
          face.vi[2] = indices[currentIndex+2];
          if (3 == count)
            face.vi[3] = face.vi[2];
          else
            face.vi[3] = indices[currentIndex + 3];
          mesh.m_F.Append(face);
        }
        currentIndex += count;
      }

      doc.AddMeshObject(mesh);
    }
  }


	return 1;
}
