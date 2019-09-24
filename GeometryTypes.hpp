#ifndef GEOMETRYTYPES_HPP
#define GEOMETRYTYPES_HPP
#include "Vec2.hpp"
#include "Vec3.hpp"
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <list>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include "tiny_vector.hpp"
//
// This file contains many of the basic geometry structures
// and some simple methods
// 

class PolyMesh;
typedef std::pair<int, int> Edge;
typedef std::pair<int, int> RegionInterface;
typedef boost::array<int, 2>  MaterialInterface;
inline Edge swap_edge(const Edge& e) { return Edge(e.second, e.first); }
typedef std::vector<Edge> EdgeList;

template <typename T>
void cycle_left(T& a, T& b, T& c)
{
   T tmp(a);
   a = b;
   b = c;
   c = tmp;
}
template <typename T>
void cycle_right(T& a, T& b, T& c)
{
   T tmp(c);
   c = b;
   b = a;
   a = tmp;
}

template <typename T>
void cycle_left(boost::array<T, 3>& a)
{
   cycle_left(a[0], a[1], a[2]);
}
template <typename T>
void cycle_right(boost::array<T, 3>& a)
{
   cycle_right(a[0], a[1], a[2]);
}

inline void OrderFace(boost::array<int, 3>& a)
{
   if (a[0]>a[1]) std::swap(a[0], a[1]);
   if (a[1]>a[2]) std::swap(a[1], a[2]);
   if (a[0]>a[1]) std::swap(a[0], a[1]);
}
inline void OrderEdge(Edge& e)
{
   if (e.first > e.second) std::swap(e.first, e.second);
}
inline Edge OrderEdge(int a, int b)
{
   if (a>b) return Edge(b,a);
   return Edge(a,b);
}
inline bool region_equal(const RegionInterface& r1, const RegionInterface& r2)
{
   return r1 == r2 || (r1.first == r2.second && r1.second == r2.first);
}
inline bool region_equal(const MaterialInterface& r1, const RegionInterface& r2)
{
   return region_equal(RegionInterface(r1[0], r1[1]), r2);
}
inline bool region_equal(const MaterialInterface& r1, const MaterialInterface& r2)
{
   return region_equal(r1,RegionInterface(r2[0], r2[1]));
}

class Face
{
public:
   bool operator==(const Face& aF) const { return aF.vertex == vertex && aF.materials == materials; }
   bool operator<(const Face& aF) const {
	   return (aF.vertex[0] != vertex[0] ? aF.vertex[0] < vertex[0] :
		        (aF.vertex[1] != vertex[1] ? aF.vertex[1] < vertex[1] :
				  (aF.vertex[2] != vertex[2] ? aF.vertex[2] < vertex[2] :
				    (aF.materials[0] != materials[0] ? aF.materials[0] < materials[0] :
					  aF.materials[1] < materials[1]
				    )
				   )
		         )
		       );
   }
   template <typename AR>
   void serialize(AR& aAr, unsigned int aVersion)
   {
      aAr & vertex & materials;
   }
   Edge GetEdge(int aIndex) const
   {
      return OrderEdge(vertex[0+aIndex], vertex[(1+aIndex)%3]);
   }
   typedef boost::array<int, 2> Materials;

   boost::array<int, 3> vertex;
   Materials            materials;
};

class MatPoint
{
public:
   Vec3 point;
   tiny_vector<int, 4> materials;
};
typedef std::vector<MatPoint> MatPointList;

inline RegionInterface face_region(const Face& f)
{
   return RegionInterface(f.materials[0], f.materials[1]);
}
inline bool region_equal(RegionInterface& r1, const Face& r2)
{
   return region_equal(r1, face_region(r2));
}
// Re-order face so that v1 is first, v2 is second
// Flips material if required
inline void OrderFace(Face& f, int v1, int v2)
{
   if (f.vertex[0] == v1 && f.vertex[1] == v2) return;
   if (f.vertex[1] == v1 && f.vertex[2] == v2)
   {
      cycle_left(f.vertex);
   }
   else if (f.vertex[2] == v1 && f.vertex[0] == v2)
   {
      cycle_right(f.vertex);
   }
   else if (f.vertex[0] == v2 && f.vertex[1] == v1)
   {
      std::swap(f.vertex[0], f.vertex[1]);
      std::swap(f.materials[0], f.materials[1]);
   }
   else if (f.vertex[1] == v2 && f.vertex[2] == v1)
   {
      std::swap(f.vertex[0], f.vertex[2]);
      std::swap(f.materials[0], f.materials[1]);
   }
   else if (f.vertex[2] == v2 && f.vertex[0] == v1)
   {
      std::swap(f.vertex[2], f.vertex[1]);
      std::swap(f.materials[0], f.materials[1]);
   }
}

// Tet or Oct
class Polyhedron
{
public:

   static const int sTetFaces[4*3];// = { 0,1,2, 3,1,0, 3,2,1, 3,0,2 };
   static const int sTetTips[4];// = { 3, 2, 0, 1 };
   static const int sOctFaces[8*3];// = { 0,2,1, 0,1,5, 0,5,4, 0,4,2, 3,1,2, 3,5,1, 3,4,5, 3,2,4 };

   Polyhedron()
      : material(-1),
      arity(0)
   {

   }

   typedef boost::array<int, 3> FaceVerts;

   void GetFace(int        aFaceIndex,
                FaceVerts& aFace) const
   {
      const int* faces = (arity == 4) ? sTetFaces : sOctFaces;
      faces = faces + aFaceIndex * 3;
      aFace[0] = vertex[faces[0]];
      aFace[1] = vertex[faces[1]];
      aFace[2] = vertex[faces[2]];
   }

   int GetTip(int aFaceIndex) const
   {
      assert(arity == 4);
      return vertex[sTetTips[aFaceIndex]];
   }

   size_t GetFaceCount() const { return arity == 4 ? 4 : 8; }

   template <typename AR>
   void serialize(AR& aAr, unsigned int aVersion)
   {
      aAr & material & vertex & arity;
   }

   bool operator==(const Polyhedron& aRhs) const { return arity == aRhs.arity && std::equal(vertex.begin(), vertex.begin() + arity, aRhs.vertex.begin()) && material == aRhs.material; }
   int                  material;
   boost::array<int, 6> vertex;
   int                  arity;
};

typedef std::vector<int>  IndexList;
//
//struct VertexLoop
//{
//   void RecomputeHash()
//   {
//      hash_value = 0;
//      for (size_t i = 0; i < vertices.size(); ++i)
//      {
//         hash_value += vertices[i];
//      }
//   }
//   typedef boost::tuple<int,int,int> ID;
//   ID GetId() const { return ID(hash_value, vertices.size(), vertices.front()); }
//   bool operator==(const VertexLoop& aRhs) const
//   {
//      return GetId() == aRhs.GetId();
//   }
//   bool operator<(const VertexLoop& aRhs) const
//   {
//      return GetId() < aRhs.GetId();
//   }
//
//   int         hash_value;
//   IndexList   vertices;  
//};
//
//struct RegionCrease
//{
//   RegionInterface         mRegions;
//   int                     mInteriorFaceCount;
//
//   std::list<VertexLoop>   mLoops;
//};


typedef std::vector<Polyhedron> PolyhedronList;
typedef std::vector<Vec3> VertexList;
typedef std::vector<Face> FaceList;

typedef boost::shared_ptr<VertexList> VertexListPtr;
typedef boost::shared_ptr<FaceList> FaceListPtr;
typedef boost::shared_ptr<PolyhedronList> PolyListPtr;



#endif