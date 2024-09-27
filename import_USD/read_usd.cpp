#include "stdafx.h"
#include "pxr/usd/usd/primRange.h"

using namespace pxr;

// using namespace pxrInternal_v0_23__pxrReserved__;

bool ReadUSDFile(const wchar_t* filename, CRhinoDoc& doc, const CRhinoFileReadOptions& options)
{
  // use an ON_String to convert from unicode to mbcs which is what usd wants
  ON_String usdPath(filename);
  UsdStageRefPtr usdModel = UsdStage::Open(usdPath.Array());

  // Traverse all prims in the stage
  UsdPrimRange primRange = usdModel->TraverseAll();
  for (auto& prim : primRange)
  {
    std::string desc = prim.GetDescription();
    bool hasPayload = prim.HasPayload();
    
    if (UsdGeomMesh mesh = UsdGeomMesh(prim))
    {
      
      VtArray<int> faceVertexCounts;
      mesh.GetFaceVertexCountsAttr().Get(&faceVertexCounts);
      
      VtArray<int> faceVertexIndices;
      mesh.GetFaceVertexIndicesAttr().Get(&faceVertexIndices);

      VtArray<GfVec3f> normals;
      mesh.GetNormalsAttr().Get(&normals);

      // Extract the geometry data
      VtArray<GfVec3f> points;
      mesh.GetPointsAttr().Get(&points);
      
      // TODO : Mesh CAN have Texture coordinates!!!
      ON_Mesh rhinoMesh = ON_Mesh(faceVertexCounts.size(), faceVertexIndices.size(), true, false);
      
      int faceCount = 0;
      int vertexCount = 0;
      while(vertexCount < faceVertexIndices.size())
      {
        int faceVertexCount = faceVertexCounts[faceCount];
        
        int faceIndicies[4];
        for (int i = 0; i < faceVertexCount; i++)
        {
          int index = vertexCount + i;
          int vIndex = faceVertexIndices[index];
          
          faceIndicies[i] = index;
          
          auto normal1 = normals[index];
          auto vertex = points[vIndex];

          rhinoMesh.SetVertex(index, ON_3dPoint(vertex[0], vertex[1], vertex[2]));
          rhinoMesh.SetVertexNormal(index, ON_3dVector(normal1[0], normal1[1], normal1[2]));
        }

        if (faceVertexCount == 4)
        {
          rhinoMesh.SetQuad(faceCount, faceIndicies[0], faceIndicies[1], faceIndicies[2], faceIndicies[3]);
        }
        else
        {
          rhinoMesh.SetTriangle(faceCount, faceIndicies[0], faceIndicies[1], faceIndicies[2]);
        }

        faceCount++;
        vertexCount += faceVertexCount;
      }

      

      doc.AddMeshObject(rhinoMesh);
    }

  }

  return true;
}
