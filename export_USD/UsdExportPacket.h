#pragma once
#include "stdafx.h"

// Small packet of information for Usd Export
// Allows for bundling converted geometry with the original object which has lots of data
class UsdPacket
{
public:

  // Don't call this, it is only for ON_SimpleArray/ON_ClassArray
  UsdPacket()
  {

  }

  // This will store a reference to the Rhino Object
  UsdPacket(const CRhinoObject& rhinoObject, ON::object_type type)
    : m_rhino_object(&rhinoObject), m_type(type)
  {
  }

  ~UsdPacket()
  {
    delete m_mesh;
  }


  UsdPacket(const UsdPacket& src)
    : m_rhino_object(src.m_rhino_object), m_type(src.m_type)
  {
    if (src.m_mesh)
    {
      m_mesh = new ON_Mesh(*src.m_mesh);
    }
  }

  const UsdPacket& operator =(const UsdPacket& src)
  {
    m_rhino_object = src.m_rhino_object;
    m_type = src.m_type;

    if (src.m_mesh)
    {
      m_mesh = new ON_Mesh(*src.m_mesh);
    }
    else
    {
      m_mesh = nullptr;
    }

    return *this;
  }

  // Transfers Ownership
  void SetMesh(ON_Mesh* mesh)
  {
    m_mesh = mesh;
  }

  // Copies Mesh
  void SetMesh(const ON_Mesh& mesh)
  {
    m_mesh = new ON_Mesh(mesh);
  }

  const CRhinoObject& Object() const
  {
    ON_ASSERT(m_rhino_object != nullptr);
    return *m_rhino_object;
  }

  const ON::object_type Type() const
  {
    return m_type;
  }

  ON_Mesh* Mesh() const
  {
    return m_mesh;
  }

private:
  
  const CRhinoObject* m_rhino_object = nullptr;

  ON_Mesh* m_mesh = nullptr;
  
  // Type is type of NewGeometry for quick switching.
  // To start most items will be Meshes
  ON::object_type m_type = ON::object_type::unknown_object_type;

};
