#include "StdAfx.h"
#include "ExportUSDPlugIn.h"
#include "Resource.h"

#pragma warning(push)
#pragma warning(disable : 4073)
#pragma init_seg(lib)
#pragma warning(pop)

// rhinoSdkPlugInDeclare.h defines the RHINO_PLUG_IN_DECLARE macro
#include "../../../SDK/inc/rhinoSdkPlugInDeclare.h"

// Rhino plug-in declarations
RHINO_PLUG_IN_DECLARE
RHINO_PLUG_IN_NAME(L"Export Pixar USD");
RHINO_PLUG_IN_ID(L"36803D82-097C-4B55-8F42-E4D44764C9DD");
RHINO_PLUG_IN_VERSION(RHINO_VERSION_NUMBER_STRING)
RHINO_PLUG_IN_UPDATE_URL(L"http://www.rhino3d.com/download");

// The one and only CExportUSDPlugIn object
static class CExportUSDPlugIn thePlugIn;

const wchar_t* CExportUSDPlugIn::PlugInName() const
{
	return RhinoPlugInName();
}

const wchar_t* CExportUSDPlugIn::LocalPlugInName() const
{
	return RhLocalizeString( L"Export Pixar USD", 55137);
}

const wchar_t* CExportUSDPlugIn::PlugInVersion() const
{
	static wchar_t plugin_version[128] = { 0 };
	if (0 == plugin_version[0])
		ON_ConvertUTF8ToWideChar(false, RhinoPlugInVersion(), 0, plugin_version, sizeof(plugin_version) / sizeof(plugin_version[0]) - 1, 0, 0, 0, 0);
	return plugin_version;
}

GUID CExportUSDPlugIn::PlugInID() const
{
	static GUID plugin_id = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
	if (0 == plugin_id.Data1 && ON_UuidIsNil(plugin_id))
		plugin_id = ON_UuidFromString(RhinoPlugInId());
	return plugin_id;
}

void CExportUSDPlugIn::AddFileType(ON_ClassArray<CRhinoFileType>& extensions, const CRhinoFileWriteOptions& options)
{
	CRhinoFileType ft;
	ft.SetFileTypePlugInID(PlugInID());
	ft.FileTypeDescription(L"Pixar USD (*.usda)");
	ft.AddFileTypeExtension(RHSTR_LIT(L"usda"));
  //ft.AddFileTypeExtension(RHSTR_LIT(L"usdc"));
  //ft.AddFileTypeExtension(RHSTR_LIT(L"usdz"));
  extensions.Append(ft);
}


int CExportUSDPlugIn::WriteFile(const wchar_t* filename, int index, CRhinoDoc& doc, const CRhinoFileWriteOptions& options)
{
	if (nullptr == filename || 0 == filename[0])
		return 0;


  ON_SimpleArray<const CRhinoObject*> object(256);

  CRhinoObjectIterator it(doc.RuntimeSerialNumber(), options);
  for (CRhinoObject* obj = it.First(); obj; obj = it.Next())
  {
    object.Append(obj);
  }

  // Get meshes to export (meshes breps, copys mesh object meshes,
  // deals with instance references that contain meshes and breps,
  ON_MeshParameters mp = ON_MeshParameters::DefaultMesh;
  int mesh_ui_style = 4;
  ON_ClassArray<CRhinoObjectMesh> mesh_list;
  CRhinoCommand::result rcMesh = RhinoMeshObjects(object, mp, options.Transformation(), mesh_ui_style, mesh_list);
  if (CRhinoCommand::cancel == rcMesh || mesh_list.Count()<1)
    return 0;
  
  tinyusdz::Stage stage; // empty scene
  for (int meshIndex = 0; meshIndex < mesh_list.Count(); meshIndex++)
  {
    const CRhinoObjectMesh& objectMesh = mesh_list[meshIndex];
    if (nullptr == objectMesh.m_mesh)
      continue;

    tinyusdz::GeomMesh mesh;
    ON_String s;
    s.Format("mesh%d", meshIndex + 1);
    mesh.name = s;

    std::vector<tinyusdz::value::point3f> pts;
    for (int i = 0; i < objectMesh.m_mesh->m_V.Count(); i++)
    {
      const ON_3fPoint pt = objectMesh.m_mesh->m_V[i];
      tinyusdz::value::point3f pt3f;
      pt3f.x = pt.x;
      pt3f.y = pt.y;
      pt3f.z = pt.z;
      pts.push_back(pt3f);
    }
    mesh.points.set_value(pts);

    std::vector<int> indices;
    std::vector<int> counts;
    for (int i = 0; i < objectMesh.m_mesh->m_F.Count(); i++)
    {
      const ON_MeshFace& face = objectMesh.m_mesh->m_F[i];
      indices.push_back(face.vi[0]);
      indices.push_back(face.vi[1]);
      indices.push_back(face.vi[2]);
      if (face.IsTriangle())
      {
        counts.push_back(3);
      }
      else
      {
        counts.push_back(4);
        indices.push_back(face.vi[3]);
      }
    }
    mesh.faceVertexCounts.set_value(counts);
    mesh.faceVertexIndices.set_value(indices);

    tinyusdz::Prim meshPrim(mesh);
    stage.root_prims().push_back(meshPrim);
  }

  std::string warn;
  std::string err;

  bool success = false;
  ON_wString extension = ON_FileSystemPath::FileNameExtensionFromPath(filename);
  if (extension.EqualOrdinal(L".usda", true))
  {
    success = tinyusdz::usda::SaveAsUSDA(filename, stage, &warn, &err);
  }
  // USDC writing is not implemented yet in tinyusdz
  //else
  //{
  //  ON_String fname(filename);
  //  success = tinyusdz::usdc::SaveAsUSDCToFile(fname.Array(), stage, &warn, &err);
  //}
  return success ? 1 : 0;
}
