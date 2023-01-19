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

BOOL CExportUSDPlugIn::OnLoadPlugIn()
{
	return TRUE;
}

void CExportUSDPlugIn::OnUnloadPlugIn()
{
}

void CExportUSDPlugIn::AddFileType(ON_ClassArray<CRhinoFileType>& extensions, const CRhinoFileWriteOptions& options)
{
	CRhinoFileType ft;
	ft.SetFileTypePlugInID(PlugInID());
	ft.FileTypeDescription(RhLocalizeString( L"Pixar USD (*.usda)", 55138));
	ft.AddFileTypeExtension(RHSTR_LIT(L"usda"));
	extensions.Append(ft);
}


int CExportUSDPlugIn::WriteFile(const wchar_t* filename, int index, CRhinoDoc& doc, const CRhinoFileWriteOptions& options)
{
	if (nullptr == filename || 0 == filename[0])
		return FALSE;

  tinyusdz::Stage stage; // empty scene
  tinyusdz::Xform xform;
  xform.name = "root";

  tinyusdz::XformOp op;
  op.op_type = tinyusdz::XformOp::OpType::Translate;
  tinyusdz::value::double3 translate;
  translate[0] = 1.0;
  translate[1] = 2.0;
  translate[2] = 3.0;
  op.set_value(translate);

  xform.xformOps.push_back(op);

  tinyusdz::GeomMesh mesh;
  mesh.name = "quad";

  {
    std::vector<tinyusdz::value::point3f> pts;
    pts.push_back({ 0.0f, 0.0f, 0.0f });

    pts.push_back({ 1.0f, 0.0f, 0.0f });

    pts.push_back({ 1.0f, 1.0f, 0.0f });

    pts.push_back({ 0.0f, 1.0f, 0.0f });

    mesh.points.set_value(pts);
  }

  {
    // quad plane composed of 2 triangles.
    std::vector<int> indices;
    std::vector<int> counts;
    counts.push_back(3);
    counts.push_back(3);
    mesh.faceVertexCounts.set_value(counts);

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);

    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(3);

    mesh.faceVertexIndices.set_value(indices);
  }

  // primvar and custom attribute can be added to generic Property container `props`
  {
    // primvar is simply an attribute with prefix `primvars:`
    //
    // texCoord2f[] primvars:uv = [ ... ] ( interpolation = "vertex" )
    // int[] primvars:uv:indices = [ ... ]
    //
    {
      tinyusdz::Attribute uvAttr;
      std::vector<tinyusdz::value::texcoord2f> uvs;

      uvs.push_back({ 0.0f, 0.0f });
      uvs.push_back({ 1.0f, 0.0f });
      uvs.push_back({ 1.0f, 1.0f });
      uvs.push_back({ 0.0f, 1.0f });

      // Fast path. Set the value directly to Attribute.
      uvAttr.set_value(uvs);

      // or we can first build primvar::PrimVar
      //tinyusdz::primvar::PrimVar uvVar;
      //uvVar.set_value(uvs);
      //uvAttr.set_var(std::move(uvVar));

      // Currently `interpolation` is described in Attribute metadataum.
      tinyusdz::AttrMeta meta;
      meta.interpolation = tinyusdz::Interpolation::Vertex;
      uvAttr.metas() = meta;

      tinyusdz::Property uvProp(uvAttr, /* custom*/false);

      mesh.props.emplace("primvars:uv", uvProp);

      // ----------------------

      tinyusdz::Attribute uvIndexAttr;
      std::vector<int> uvIndices;

      // FIXME: Validate
      uvIndices.push_back(0);
      uvIndices.push_back(1);
      uvIndices.push_back(2);
      uvIndices.push_back(3);


      tinyusdz::primvar::PrimVar uvIndexVar;
      uvIndexVar.set_value(uvIndices);
      uvIndexAttr.set_var(std::move(uvIndexVar));

      tinyusdz::Property uvIndexProp(uvIndexAttr, /* custom*/false);
      mesh.props.emplace("primvars:uv:indices", uvIndexProp);

    }

    // `custom uniform double myvalue = 3.0 ( hidden = 0 )`
    {
      tinyusdz::Attribute attrib;
      double myvalue = 3.0;
      tinyusdz::primvar::PrimVar var;
      var.set_value(myvalue);
      attrib.set_var(std::move(var));

      attrib.variability() = tinyusdz::Variability::Uniform;

      tinyusdz::AttrMeta meta;
      meta.hidden = false;
      attrib.metas() = meta;

      tinyusdz::Property prop(attrib, /* custom*/true);

      mesh.props.emplace("myvalue", prop);
    }

  }

  tinyusdz::Prim meshPrim(mesh);
  tinyusdz::Prim xformPrim(xform);

  // [Xform]
  //  |
  //  +- [Mesh]
  //
  xformPrim.children().emplace_back(std::move(meshPrim));

  stage.root_prims().emplace_back(std::move(xformPrim));
  std::string warn;
  std::string err;
  bool ret = tinyusdz::usda::SaveAsUSDA(filename, stage, &warn, &err);
	return ret ? TRUE : FALSE;
}
