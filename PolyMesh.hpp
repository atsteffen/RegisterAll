#ifndef TETRA_HPP
#define TETRA_HPP
#include "Vec3.hpp"
#include "Mesh.hpp"
#include <vector>
#include <boost/array.hpp>
#include <boost/tuple/tuple.hpp>
#include <map>
#include <algorithm>
#include "GeometryTypes.hpp"
#include "GeometryBuffer.hpp"
#include "SparseArray.hpp"
#include "boost/numeric/ublas/matrix.hpp"

using namespace boost::numeric::ublas;

//class PolyMesh;

struct TetPair
{
	boost::array<int, 2> tetTips;
	boost::array<int, 3> verts;
};

//class PolyMeshView;
// Represents the actual mesh object.  Can be a polyhedral (tet/oct) mesh, or a polygonal mesh
class PolyMesh
{
   public:
      friend class PolyMeshView;
      friend class Polyhedron;
      typedef Polyhedron Poly;

      PolyMesh();
      void Clear()
      {
         crease_faces.clear();
         crease_edges.clear();
         verts.clear(); 
         polys.clear();
      }
      void UpdatePolys();
      void UpdateFaces();
      // makes this mesh a clone of another one
      void clone(const PolyMesh& aRHS)
      {
         verts.clone(aRHS.verts);
         polys.clone(aRHS.polys);
         crease_faces.clone(aRHS.crease_faces);
         crease_edges.clone(aRHS.crease_edges);
      }
      // makes this mesh a reference to another mesh (no copy)
      void ref(PolyMesh& aRHS)
      {
         verts.ref(aRHS.verts);
         polys.ref(aRHS.polys);
         crease_faces.ref(aRHS.crease_faces);
         crease_edges.ref(aRHS.crease_edges);
      }
	
	  void GetTetPairs(std::vector<TetPair>& aTetPairs);
	void SubdivideMask(string maskfile,
								 int subLevel);
	void PrecomputeMatrices(string maskfile,
									  string atafile,
									  string atbfile,
									  MatPointList& aLandmarks,
									  float         fitWt,
									  float         energyWt,
									  int           subLevel);
	void ReComputeMatrices(string maskfile,
									 string atafile,
									 string atbfile,
									 MatPointList& aLandmarks,
									 float         fitWt,
									 float         energyWt,
									 int           subLevel);
	void SolveForMatrices(string atafile, string atbfile);
	  void AlignLS2(MatPointList& aLandmarks,
				  float         fitWt=40.0f,
				  float         energyWt=1.0f,
				  int           subLevel=3,
				  int           iterations=1);
	  void SubdivideMesh(int aLevel);
	  void SubdivideMask(int aLevel, std::vector<SparseArray>& maskb);
	  void writeMaskToFile(string filename,std::vector<SparseArray> maskb);
	std::vector<SparseArray> readMaskFromFile(string filename);
	
	void writeMat(string filename,matrix<double> ata, int nnz);
	matrix<double> readMat(string filename, int& nnz);

      void Updated();
      // every vertex in the mesh
      VertexBuffer   verts;
      // tetrahedra / octahedra if available
      PolyBuffer     polys;
      // triangle surfaces
      FaceBuffer     crease_faces;
      // crease edges
      EdgeBuffer     crease_edges;
      
      void Recompute();

      //void AddView(PolyMeshView* aView);
      
      FaceBuffer& GetCreaseFaces() { return crease_faces; }
      EdgeBuffer& GetCreaseEdges() { return crease_edges; }

      // returns the polygonal mesh portion of this mesh.  (this is a reference)
      Mesh GetCreaseMesh() { return Mesh(verts, crease_faces); }

      template <typename AR>
      void serialize(AR& aAr, unsigned int aVersion)
      {
         aAr & *verts;
         aAr & *polys;
         aAr & *crease_faces & *crease_edges;
      }
   private:

      void ComputeCreases();
};


#endif
