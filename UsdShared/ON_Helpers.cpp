#include "stdafx.h"
#include "ON_Helpers.h"

std::string ON_Helpers::ON_wString_to_StdString(const ON_wString& onwstr)
{
  // todo: problems with potentially losing data
  std::wstring wstr(static_cast<const wchar_t*>(onwstr));
  std::string str(wstr.begin(), wstr.end());
  return str;
}

ON_wString ON_Helpers::ON_TextureTYPE_to_ON_wString(const ON_Texture::TYPE type)
{
  // there's probably an better way to convert an enum to string in C++

  switch (type)
  {
    //case ON_Texture::TYPE::bitmap_texture: { return "bitmap_texture"; break; } // "standard" image texture.  // Deprecated.  Use Diffuse.
    //case ON_Texture::TYPE::diffuse_texture: { return "diffuse_texture"; break; } // ideally albedo.
    case ON_Texture::TYPE::bump_texture: { return "bump_texture"; break; } // bump map - see m_bump_scale comment
    case ON_Texture::TYPE::transparency_texture: { return "transparency_texture"; break; } // value = alpha (see m_tranparancy_id)  Deprecated.  Use Opacity.  No change needed to functionality - transparency in Rhino has always meant opacity.
    //case ON_Texture::TYPE::opacity_texture: { return "opacity_texture"; break; } // value = alpha.

    // The following textures are only for PBR materials
    // They are not supported by the basic ON_Material definition, and should only be used when
    // rendering physically based (PBR) materials.
    case ON_Texture::TYPE::pbr_base_color_texture: { return "pbr_base_color_texture"; break; }   //Reuse diffuse texture.
    case ON_Texture::TYPE::pbr_subsurface_texture: { return "pbr_subsurface_texture"; break; }
    case ON_Texture::TYPE::pbr_subsurface_scattering_texture: { return "pbr_subsurface_scattering_texture"; break; }
    case ON_Texture::TYPE::pbr_subsurface_scattering_radius_texture: { return "pbr_subsurface_scattering_radius_texture"; break; }
    case ON_Texture::TYPE::pbr_metallic_texture: { return "pbr_metallic_texture"; break; }
    case ON_Texture::TYPE::pbr_specular_texture: { return "pbr_specular_texture"; break; }
    case ON_Texture::TYPE::pbr_specular_tint_texture: { return "pbr_specular_tint_texture"; break; }
    case ON_Texture::TYPE::pbr_roughness_texture: { return "pbr_roughness_texture"; break; }
    case ON_Texture::TYPE::pbr_anisotropic_texture: { return "pbr_anisotropic_texture"; break; }
    case ON_Texture::TYPE::pbr_anisotropic_rotation_texture: { return "pbr_anisotropic_rotation_texture"; break; }
    case ON_Texture::TYPE::pbr_sheen_texture: { return "pbr_sheen_texture"; break; }
    case ON_Texture::TYPE::pbr_sheen_tint_texture: { return "pbr_sheen_tint_texture"; break; }
    case ON_Texture::TYPE::pbr_clearcoat_texture: { return "pbr_clearcoat_texture"; break; }
    case ON_Texture::TYPE::pbr_clearcoat_roughness_texture: { return "pbr_clearcoat_roughness_texture"; break; }
    case ON_Texture::TYPE::pbr_opacity_ior_texture: { return "pbr_opacity_ior_texture"; break; }
    case ON_Texture::TYPE::pbr_opacity_roughness_texture: { return "pbr_opacity_roughness_texture"; break; }
    case ON_Texture::TYPE::pbr_emission_texture: { return "pbr_emission_texture"; break; }
    case ON_Texture::TYPE::pbr_ambient_occlusion_texture: { return "pbr_ambient_occlusion_texture"; break; }
    //pbr_smudge_texture                 = 27U,
    case ON_Texture::TYPE::pbr_displacement_texture: { return "pbr_displacement_texture"; break; }
    case ON_Texture::TYPE::pbr_clearcoat_bump_texture: { return "pbr_clearcoat_bump_texture"; break; }
    case ON_Texture::TYPE::pbr_alpha_texture: { return "pbr_alpha_texture"; break; }

    // emap_texture is OBSOLETE - set m_mapping_channel_id = ON_MappingChannel::emap_mapping
    case ON_Texture::TYPE::emap_texture: { return "emap_texture"; break; } // spherical environment mapping.
    default: {return ""; break; }
  }
}

ON_wString ON_Helpers::ON_wString_vector_to_ON_wString_path(const std::vector<ON_wString>& names)
{
  ON_wString path;
  for (ON_wString name : names) {
    path = path + L"/" + name;
  }
  return path;
}

ON_wString ON_Helpers::ON_UUID_to_ON_wString(const ON_UUID& uuid)
{
  // todo: there must be a function that already does this
  ON_wString uuidStr;
  uuidStr.Format(L"%X-%X-%X-%X", uuid.Data1, uuid.Data2, uuid.Data3, uuid.Data4);
  return uuidStr;
}

std::string ON_Helpers::ON_UUID_to_StdString(const ON_UUID& uuid)
{
  return ON_wString_to_StdString(ON_Helpers::ON_UUID_to_ON_wString(uuid));
}

void ON_Helpers::RotateYUp(ON_Mesh* mesh)
{
  ON_Xform rotate_y_up;
  double ninetyDegrees = ON_PI / -2.0;
  rotate_y_up.Rotation(ninetyDegrees, ON_3dVector::XAxis, ON_3dPoint::Origin);
  mesh->Transform(rotate_y_up);
}

void ON_Helpers::RotateGeometryYUp(ON_Geometry* geom)
{
  ON_Xform rotate_y_up;
  double ninetyDegrees = ON_PI / -2.0;
  rotate_y_up.Rotation(ninetyDegrees, ON_3dVector::XAxis, ON_3dPoint::Origin);
  geom->Transform(rotate_y_up);
}
