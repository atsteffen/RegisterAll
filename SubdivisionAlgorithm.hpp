//#ifndef SUBDIVISIONALGORITHM_HPP
//#define SUBDIVISIONALGORITHM_HPP
//
//#include <vector>
//#include "SparseArray.hpp"
//
//class PolyMesh;
//// interface to Tao's subdivision algorithm
//// copies data to his format, runs subdivide, and copies back
//namespace SubdivisionAlgorithm
//{
//   void SubdivideMesh(PolyMesh& aMesh, int aLevel);
//   void SubdivideMask(PolyMesh& aMesh, int aLevel, std::vector<SparseArray>& maskb);
//
//   void subdivide(
//      int& nVerts1, float*& vertl1, 
//      int& nCrVerts1, int*& crvertl1, 
//      int& nCrEdges1, int*& credgel1, 
//      int& nTris1, int*& tril1, 
//      int& nTets1, int*& tetl1, 
//      int& nOcts1, int*& octl1, 
//      int*& tetmaskl1, int*& octmaskl1, 
//      int*& faceMaskl1,
//	  int comp=3);
//
//     void subdivide2(
//      int& nVerts1, std::vector<SparseArray>& maskb, 
//      int& nCrVerts1, int*& crvertl1, 
//      int& nCrEdges1, int*& credgel1, 
//      int& nTris1, int*& tril1, 
//      int& nTets1, int*& tetl1, 
//      int& nOcts1, int*& octl1, 
//      int*& tetmaskl1, int*& octmaskl1, 
//      int*& faceMaskl1);
//
//}
//
//#endif
