#include "stdafx.h"
#include "UsdImportPacket.h"

void UsdImportPacket::SetGeometry(std::shared_ptr<ON_Geometry> geometry)
{
  m_geometry = geometry;
}

void UsdImportPacket::SetLayer(std::shared_ptr<const ON_Layer> layer)
{
  m_layer = layer;
}

void UsdImportPacket::SetTransform(std::shared_ptr<const ON_Matrix> matrix)
{
  m_transform = matrix;
}


std::shared_ptr<ON_Geometry> UsdImportPacket::GetGeometry() const
{
  return m_geometry;
}

std::shared_ptr<const ON_Layer> UsdImportPacket::GetLayer() const
{
  return m_layer;
}

std::shared_ptr<const ON_Matrix> UsdImportPacket::GetTransform() const
{
  return m_transform;
}

const ON_Xform UsdImportPacket::GetXForm() const
{
  const ON_Matrix* matrix = GetTransform().get();
  return ON_Xform(*matrix);
}

