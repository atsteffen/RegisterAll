#ifndef MESH_HPP
#define MESH_HPP
#include "Vec3.hpp"
#include <vector>
#include <iostream>
#include <algorithm>
#include <boost/array.hpp>
#include "GeometryBuffer.hpp"
//#include "PolyMesh.hpp"

// A triangle mesh
class Mesh
{
public:
   //typedef std::vector<Vec3> VertexList;
   //typedef std::vector<Face> FaceList;
   void clone(const Mesh& aRHS)
   {
      verts.clone(aRHS.verts);
      faces.clone(aRHS.faces);
	  polys.clone(aRHS.polys);
	  edges.clone(aRHS.edges);
   }

   void ref(const Mesh& aRHS)
   {
      verts.ref(aRHS.verts);
      faces.ref(aRHS.faces);
	  polys.ref(aRHS.polys);
	  edges.ref(aRHS.edges);
   }
	
	void ExtractMatPoints(std::vector<MatPoint>& aPoints);

   Mesh(VertexBuffer& aVerts, FaceBuffer& aFaces) : verts(aVerts), faces(aFaces) {}
   Mesh() {}
   //VertexList      verts;
   //FaceList        faces;

   VertexBuffer   verts;
   FaceBuffer     faces;
   PolyBuffer     polys;
   EdgeBuffer     edges;
};


inline Vec3 FaceNormal(Mesh& aMesh,
                       Face& f)
{
   const Vec3& a = aMesh.verts[f.vertex[0]];
   const Vec3& b = aMesh.verts[f.vertex[1]];
   const Vec3& c = aMesh.verts[f.vertex[2]];
   return cross(b-a, c-b).normal();
}
inline Vec3 FaceNormal(Mesh& aMesh,
                       Face& f,
                       int aMaterial)
{
   const Vec3& a = aMesh.verts[f.vertex[0]];
   const Vec3& b = aMesh.verts[f.vertex[1]];
   const Vec3& c = aMesh.verts[f.vertex[2]];
   Vec3 cp = cross(b-a, c-b).normal();
   if (f.materials[0] != aMaterial)
   {
      cp = -cp;
   }
   return cp;
}

namespace std
{
   template<> 
   inline void swap(Mesh& aLeft, Mesh& aRight)
   {	
      std::swap(*aLeft.verts, *aRight.verts);
      std::swap(*aLeft.faces, *aRight.faces);
   }
}

#endif
