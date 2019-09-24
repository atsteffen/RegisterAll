#ifndef DMesh_h__
#define DMesh_h__


#include <iostream>

#include <stack>
#include <vector>
#include <string>
#include <set>
#include <queue>

#include "tiny_vector.hpp"
#include <algorithm>
#include <boost/array.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/iterator.hpp>
#include "Vec3.hpp"
#include <list>
#include "Variant.hpp"
#include "GeometryTypes.hpp"
#include "GeometryBuffer.hpp"

//#include "PatchMesh.hpp"

class PatchMesh; //fw declaration

//#include "MeshAlgorithm.hpp" //to use facenormal functioni

// DMesh is a mesh structure that uses pointers instead of indexing for relations.
// This allows DMesh to be more dynamic than a regular indexed mesh.  Elements can
// be added and removed in constant time.
//
// For efficiency, the elements of the mesh allow 'aux' data to be stored.  This is
// used in many of my algorithms to prevent the need for a hash-table.
//
// Usage of boost::intrusive allows the elements of the mesh to be mainly constant size
// while still creating lists.

class Mesh;

class DFace;
class DEdge;
class DVertex;

class DSplitEdge;
class DetachFace;

class PVertex;

typedef std::vector<DVertex*> DVertexArray;
typedef std::vector<DFace*> DFaceArray;
typedef std::vector<DEdge*> DEdgeArray;

namespace DMesh_Detail
{
   template <typename T>
   bool is_even_order(T* v[3])
   {
      bool isEven(true);
      for (int i = 0; i < 2; ++i)
      {
         for (int j = i+1; j < 3; ++j)
         {
            if (v[i]>v[j])
            {
               std::swap(v[i],v[j]);
               isEven = !isEven;
            }
         }
      }
      return isEven;
   }
}

typedef Variant DAuxData;
// A DMesh vertex
class DVertex : public boost::intrusive::list_base_hook<>
{
   public:
      DVertex() : edge(0), sign(0), sEdge(0), aux(0), pv(0) {}
      DVertex(Vec3 aV) : v(aV), edge(0), sign(0), sEdge(0),aux(0), pv(0) {}
      // Return edge containing v2
      inline DEdge* find_edge(DVertex* v2);	 
      inline void   unlink_edge(DEdge* e);

	   // tn: return opposite edge in same face f
	  DEdge * get_opposite_edge(DFace * f);
      void get_adjacent_vertices(DVertexArray& aVertices);
      void get_edges(DEdgeArray& aEdges);
	  void get_crease_edges(DEdgeArray & aEdges);
      void get_faces(DFaceArray& aFaces);
	  bool on_crease_edge();
	  bool on_diff_crease_edge(DEdge *& e);
	  bool is_hidden(std::vector<int> & hiddenRegions);//tn: it not visible

	  bool is_in_vertices_list(DVertexArray& aVertices);
	  bool has_material(int mat);

	  void printInfo();
	  void printInfoIndices() {if(!this) std::cerr << "0"; else std::cerr << this->index;}
	 
	  //determine order of vertices to find crease vertices
	  void get_vertex_link(DVertexArray & lkV, DEdgeArray & lkE); //tn: return link of a vertex

	  friend ostream& operator<<(ostream& s, const DVertex & v)
	  {
		  const DVertex * adv = & v;
		  s << "addr: " << adv << " i:"<<v.index << "(" << v.v << ")";
		  return s;
	  }
	

	  //////////////////////////////////////////////
	  ///DATA MEMBERS
	  //////////////////////////////////////////////

      // position
      Vec3     v;
      // first edge
      DEdge*   edge;
      // user data
      mutable DAuxData aux;	

	  //explicit pointer to PVertex
	  PVertex * pv;

	  //for splitting purpose
	  int sign;
	  DSplitEdge * sEdge;
	  std::vector<DVertex *> loopConnectedVertices;

	  //for detaching purpose
	  int index; //only correct @first loading


};
	

// DMesh face
class DFace : public boost::intrusive::list_base_hook<>
{
public:
	DFace() {dead = false;}

      DFace(DVertex*        v1,
            DVertex*        v2,
            DVertex*        v3,
            MaterialInterface aMaterial)
      {
         aux = 0;
         v[0] = v1;
         v[1] = v2;
         v[2] = v3; 
         materials = aMaterial;

		 this->aux = 0;
		 this->oldPatch = 0;

		 dead = false;
      }

	 // ~DFace() { this = 0;}

      // Face must have vertices in sorted order (by vertex address)
      void sort_face()
      {
         // sort face
         for (int i = 0; i < 2; ++i)
         {
            for (int j = i+1; j < 3; ++j)
            {
               if (v[i]>v[j])
               {
                  std::swap(v[i],v[j]);
                  std::swap(materials[0], materials[1]);
               }
            }
         }
      }

      // Return next face for given edge
      DFace*& next(DVertex* v1,
                   DVertex* v2)
      {
         if (v1 == v[0])
         {
            if (v2 == v[1]) return next_faces[0];
            return next_faces[2];
         }
         assert(v1 == v[1] && v2 == v[2]);
         return next_faces[1];
      }
      // Returns the face's 'next' edge
      inline DFace*& next(DEdge* edge);
      // Bind the face to the edge
      inline void insert_face(DEdge* edge);

      // Get face edges in order of vertices 0->1->2->0
      void get_edges(DEdge* aEdges[3]);

      // Returns the edge from vertex [index] to [index+1 mod 3]
      DEdge* get_edge(int aIndex);
      // return a vertex by index
      Vec3& vertex(int aIndex) { return v[aIndex]->v;}
      const Vec3& vertex(int aIndex) const { return v[aIndex]->v; }
      // return all faces which share an edge with this face
      void get_adjacent_faces(std::vector<DFace*>& aFaces);
	  //tn: return adjacent faces which don't share a non-manifold edge with the face
	  void get_adjacent_faces_sharing_only_manifold_edges(std::vector<DFace*>& aFaces);

	  bool has_same_materials(DFace * face);
	  void set_same_materials_with_face(DFace * face);
	  bool is_same_face_with(DFace * face); //assumed that face is sorted already
	  bool has_material(int mat) {return (materials[0]==mat || materials[1] == mat);}

	  //tn: test if a vertex belongs to this face
	  bool contain(DVertex*  v_in);
	  //tn: test if an edge belongs to this face
	  bool contain(DEdge* e_in);

	  bool is_invalid();
	  bool is_hidden(std::vector<int> & hiddenRegions);

	  bool is_in_list(DFaceArray & faces);

	  friend ostream& operator<<(ostream& s, const DFace & f)
	  {		  
		  s << "face: " << &f <<"-";
		  s << " v0: " << *f.v[0] << " v1: " << *f.v[1] << " v2: " << *f.v[2];
		  s << " m: " << f.materials[0] << "," << f.materials[1];
		  return s;
	  }

	  bool is_counter_clockwise();
	  bool get_clock_direction_along_edge(DEdge * e);
	  bool check_material_consistency_against_face(DFace * f, DEdge * e);

	  void print(){	
		  if(!this) {std::cerr << "NULL face\n";return;}
		  std::cerr << *this;
		  std::cerr << std::endl;
		  std::cerr << "vertex pointer " << v[0] << ", " <<  v[1] << "," <<v[2] << "\n-----------<\n";	
		  //std::cerr << "counter_clockwise? = " << is_counter_clockwise() << "\n";
	  }
	  void printInfo(){print();}
	  void printInfoIndices(){std::cerr << "-vinds(" << v[0]->index << ", " << v[1]->index << ", " << v[2]->index << "): ";
	  }

   private:
   public:
         //Face f;
      typedef boost::array<int, 2> Materials;
      MaterialInterface                materials;
      boost::array<DVertex*, 3>        v;
      boost::array<DFace*, 3>          next_faces;
      mutable DAuxData                 aux;
	  mutable DAuxData                 oldPatch;

	  size_t						   index;
	  bool dead;
};
// A edge in DMesh
class DEdge : public boost::intrusive::list_base_hook<>
{
   public:
      DEdge()
      {
         face = 0;
         v[0] = v[1] = 0;
         next_edges[0] = next_edges[1] = 0;

		 processed = false; //tn
		 splitEdge = 0;
		 aux = 0;
      }
      //DEdge(DVertex* v1, DVertex* v2)
      //{
      //   v[0] = v1;
      //   v[1] = v2;
      //   next[0] = v[0]->edge;
      //   v[0]->edge = this;
      //   next[1] = v[1]->edge;
      //   v[1]->edge = this;
      //}
      void initialize(DVertex* v1,
                      DVertex* v2) //tn: do we use this method anymore?
      {
         assert(next_edges[0]==0);
         v[0] = v1;
         v[1] = v2;
         // sort edge
         if (v[0] > v[1]) std::swap(v[0], v[1]);
         insert_edge(v[0]);
         insert_edge(v[1]);
      }
	  //set vertices to edge explicitly
	  void set_vertices(DVertex* v0, DVertex* v1) { v[0] = v0; v[1] = v1;}

	  //check if edge contains vert
      bool contain(DVertex* vert) const {return v[0]==vert || v[1]==vert;}
	  // Check if the edges have a commond vertex
	  bool has_common_vertex(DEdge *e) const {return contain(e->v[0]) || contain(e->v[1]); }
	  //check if same edge -tn
	  bool same_edge(DEdge* e) const {return contain(e->v[0]) && contain(e->v[1]);}

	  DVertex * get_common_vertex(DEdge *e)
	  {
		 // if (! this->same_edge(e)) return 0;
		  if(!e) return 0;
		  return (this->contain(e->v[0]))?e->v[0]:e->v[1];
	  }

      // return next edge for vert
      DEdge*& next(DVertex* vert)
      {
         if (v[0]==vert) return next_edges[0];
         return next_edges[1];
      }
      // Return vertex opposite of vert
      DVertex* opposite(DVertex* vert) const
      {
         return v[0]==vert?v[1]:v[0];
      }
      // Return vertex opposite of edge
      DVertex* opposite(DEdge* edge) const
      {
         int oi = (v[0] == edge->v[0] || v[0] == edge->v[1]) ? 1 : 0;
         return v[oi];
      }
	  // Return vertex opposite to the edge in a face
	  DVertex* opposite(DFace * face) const
	  {
		  if (!this->contain(face->v[0]))
			  return face->v[0];
		  else
			  if (!this->contain(face->v[1]))
				  return face->v[1];
			  else
				  return face->v[2];
	  }

      void unlink_face(DFace* aFace)
      {
         DFace* f = aFace;
         DFace* fPrev;
         // remove face from linked list and re-assign first face
         do 
         {
            fPrev = f;
            f = f->next(this);
         } while (f != aFace);
         
         if (fPrev != aFace)
         {
            fPrev->next(this) = aFace->next(this);
            face = fPrev;
         }
         else
         {
            face = 0;
         }
      }
      // return all edges sharing a vertex with this one
      void get_adjacent_edges(DEdgeArray& aEdges);
      // return all adjacent faces
      void get_adjacent_faces(DFaceArray& aFaces);
	  void get_adjacent_faces(DFaceArray& aFaces, int material);
      // return all faces sharing a vertex with this one
      void get_touching_faces(DFaceArray& aFaces);
      
      bool is_edge_adjacent(const DEdge* aEdge) const 
      {
         return aEdge->v[0] == v[0] || aEdge->v[0] == v[1]
             || aEdge->v[1] == v[0] || aEdge->v[1] == v[1];
      }
      
	  int  get_adjacent_face_count(bool debug = false);
	  bool is_crease_edge();

	  bool is_in_edges_list(DEdgeArray& aEdges);

	  bool draw_as_manifold(std::vector<int> & hiddenRegions);

	  bool is_hidden(std::vector<int> & hiddenRegions);

	  /////////////output functions
	  ///just used for debugging purpose

	  friend ostream& operator<<(ostream& s, const DEdge & e)
	  {
		  s << "edge - v0: " << *e.v[0] << " v1: " << *e.v[1];
		  return s;
	  }

	  void printTwoEndPoints(){ 
		  std::cerr << "edge(" << v[0]->index << ", " << v[1]->index << "):\n\tv0 - " << v[0] <<":(" << v[0]->v[0] << "," << v[0]->v[1] << "," << v[0]->v[2] << ")" <<
			  "\n\tv1 - " << v[1] <<":(" << v[1]->v[0] << "," << v[1]->v[1] << "," << v[1]->v[2] << ")\n" ;
		  std::cerr << "\t# adj faces = " << this->get_adjacent_face_count() << "\n";
	  }
	  void printInfo(){printTwoEndPoints();}
	  void printInfoIndices(){std::cerr << "(" << v[0]->index << ", " << v[1]->index << ")";}

   private:
      void insert_edge(DVertex* vert)
      {
         if (vert->edge == 0)
         {
            vert->edge = this;
            next(vert) = this;
         }
         else
         {
            DEdge* nextNext = vert->edge->next(vert);
            vert->edge->next(vert) = this;
            next(vert) = nextNext;
         }
      }
   public:
      // endpoints
      DVertex* v[2];
      // Pointers to 1-ring s-list
      DEdge*   next_edges[2];
      DFace*   face;

      mutable DAuxData aux;

	  //tn added
	  bool processed;
	  DSplitEdge * splitEdge;
};

// A polyhedron in DMesh.  A DMesh doesn't require polyhedrons and can be strictly triangular.
// When Polyhedrons are added, it is assumed they construct a mesh from their outer shells.
class DPolyhedron : public boost::intrusive::list_base_hook<>
{
   public:
      // indices that make up a tetrahedron's faces
      static const int sTetFaces[4*3];// = { 0,1,2, 3,1,0, 3,2,1, 3,0,2 };
      static const int sTetTips[4];// = { 3, 2, 0, 1 };
      // indices that make up a octahedron's faces
      static const int sOctFaces[8*3];// = { 0,2,1, 0,1,5, 0,5,4, 0,4,2, 3,1,2, 3,5,1, 3,4,5, 3,2,4 };
      // return the vertices which make up a polyhedron's nth face
      void GetFace(int      aFaceIndex,
                   DVertex* aVertices[3])
      {
         const int* faces = (arity == 4) ? sTetFaces : sOctFaces;
         faces = faces + aFaceIndex * 3;
         aVertices[0] = v[faces[0]];
         aVertices[1] = v[faces[1]];
         aVertices[2] = v[faces[2]];
      }

      int      arity;
      DVertex* v[6];
      int      material;
};

typedef boost::intrusive::list<DVertex, boost::intrusive::constant_time_size<true> > DVertexList;
typedef boost::intrusive::list<DEdge, boost::intrusive::constant_time_size<true> > DEdgeList;
typedef boost::intrusive::list<DFace, boost::intrusive::constant_time_size<true> > DFaceList;
typedef boost::intrusive::list<DPolyhedron, boost::intrusive::constant_time_size<true> > DPolyList;
typedef GeometryBuffer<Polyhedron> PolyList;

class DMesh
{
   public:
      //typedef std::list<DVertex> DVertexList;
      //typedef std::list<DEdge> DEdgeList;
      //typedef std::list<DFace> DFaceList;
      //typedef DMesh_Detail::vec3_iterator vec3_iterator;
      typedef DVertexList::iterator vertex_iterator;
      typedef DFaceList::iterator face_iterator;
      typedef DEdgeList::iterator edge_iterator;
      typedef DPolyList::iterator poly_iterator;

      DMesh();
      ~DMesh();
      DMesh(DMesh& aSrc);
      DMesh& operator=(DMesh& aRhs);
      DMesh& operator=(Mesh& aRhs);
      
      // Iterate the vertices of the mesh
      vertex_iterator begin_v() {return vertices.begin();}
      vertex_iterator end_v() {return vertices.end();}
      // Iterate the edges of the mesh
      edge_iterator begin_e() {return edges.begin();}
      edge_iterator end_e(){return edges.end();}
      // Iterate the faces of the mesh
      face_iterator begin_f(){return faces.begin();}
      face_iterator end_f(){return faces.end();}

      // Iterate the polyhedrons of the mesh
      poly_iterator begin_p() { return polys.begin();}
      poly_iterator end_p() { return polys.end();}

      // Iterate the position of the mesh's vertices
      //vec3_iterator begin_vec3() { return vec3_iterator(vertices.begin()); }
      //vec3_iterator end_vec3() { return vec3_iterator(vertices.end()); }

      size_t vertex_count() const { return vertices.size(); }
      size_t face_count() const { return faces.size(); }
      size_t edge_count() const { return edges.size(); }
      size_t poly_count() const { return polys.size(); }

	  // Get patch mesh
	  //PatchMesh * get_patch_mesh(){return pMesh;}

      // assign vertex aux value an index number
      void index_vertices()
      {
         int ind = 0;
         for (vertex_iterator i = begin_v(); i != end_v(); ++i)
         {
            i->aux = ind;
			i->index = ind++;
         }
		 //std::cerr << "finished index vert w/ ind = " << ind <<"\n";
		 //std::cerr << "\n";
      }

      // return vector of vertices
      void get_vertex_index(DVertexArray& aVerts)
      {
         for (vertex_iterator i = begin_v(); i != end_v(); ++i)
         {
            aVerts.push_back(&*i);
         }
      }

      // Add a vertex.  It is assumed vertices are inserted prior to associated faces
      DVertex* insert_vertex(Vec3 v)
      {
         vertices.push_back(*new DVertex(v));
         return &vertices.back();
      }
      // Add a face to the mesh
      DFace* insert_face(DFace* aFacePtr)
      {
         DFace& f = *aFacePtr;
         faces.push_back(f);
         f.sort_face();		
         // insert/find 3 edges
         DEdge* e1 = insert_edge(f.v[0], f.v[1]);
         DEdge* e2 = insert_edge(f.v[1], f.v[2]);
         DEdge* e3 = insert_edge(f.v[2], f.v[0]);

         // bind to face
         f.insert_face(e1);
         f.insert_face(e2);
         f.insert_face(e3);
         return aFacePtr;
      }
	  
      // Create and insert a face
      DFace* insert_face(DVertex*          v1,
                         DVertex*          v2,
                         DVertex*          v3,
                         MaterialInterface aInterface)
      {
         return insert_face(new DFace(v1,v2,v3, aInterface));
      }
      // Find the face with these vertices
      DFace* find_face(DVertex* v1, DVertex* v2, DVertex* v3)
      {
         DEdge* e1 = v1->find_edge(v2);
         if (e1)
         {
            DFace* f = e1->face;
            do
            {
               if (f->v[0] == v3 || f->v[1] == v3 || f->v[2] == v3)
                  return f;
               f = f->next(e1);
            } while (f != e1->face);
         }
         return 0;
      }
      // Remove a face from the mesh
      void remove_face(DFace* f)
      {
         DEdge* fedges[3];
         f->get_edges(fedges);
         for (int i = 0; i < 3; ++i)
         {
            DEdge* e = fedges[i];
            e->unlink_face(f);
            if (e->face == 0)
            {
               remove_edge(e);
            }
         }
         faces.erase(faces.iterator_to(*f));
         delete f;
      }
      
      // Insert a polyhedron given its vertices.  Associated faces are added/removed
      void insert_poly(DVertexArray& aVerts,
                       int           aMaterial)
      {
         polys.push_back(*new DPolyhedron());
         DPolyhedron& p = polys.back();
         p.material = aMaterial;
         p.arity = aVerts.size();
         for (int i = 0; i < p.arity; ++i)
         {
            p.v[i] = aVerts[i];
         }
		 int nface=4;
		 if (p.arity == 6) {
			 nface = 8;
		 }
		
		 //cout << "arity: " << p.arity << "& faces: " << nface << endl;
         for (int i = 0; i < nface; ++i)
		 //for (int i = 0; i < p.arity; ++i)
         {
            DVertex* verts[3];
            p.GetFace(i, verts);
            DFace* f = find_face(verts[0], verts[1], verts[2]);
			//cout << "face " << i << " {" << verts[0]->v << "," << verts[1]->v << "," << verts[2]->v << "}" << endl;
            if (! f)
            {
				//cout << "can't find a face" << endl;
               //MaterialInterface mat = { aMaterial, aMaterial };
			   int matIndex = (DMesh_Detail::is_even_order(verts) ? 1 : 0);
				MaterialInterface mat;
			    mat[!matIndex] = aMaterial;
				//cout << !matIndex << endl;
				mat[matIndex] = -1;
               insert_face(verts[0], verts[1], verts[2], mat);
            }
            else
            {
               int matIndex = (DMesh_Detail::is_even_order(verts) ? 1 : 0);
				if (f->materials[!matIndex] == -1) {
					f->materials[!matIndex] = aMaterial;
				};
//				if (f->materials[matIndex] == -1) {
//					f->materials[matIndex] = aMaterial;
//				};
			   //f->materials[0] = aMaterial;
               if (f->materials[0] == f->materials[1])
               {
                  remove_face(f);
               }
            }
         }
      }
      // Set the user data on all vertices to val
      void set_vertex_aux(Variant val)
      {
         for (vertex_iterator i = begin_v(); i != end_v(); ++i)
         {
            i->aux = val;
         }
      }
      // Set the user data on all faces to val
      void set_face_aux(Variant val)
      {
         for (face_iterator i = begin_f(); i != end_f(); ++i)
         {
            i->aux = val;
         }
      }
      // Set the user data on all edges to val
      void set_edge_aux(Variant val)
      {
         for (edge_iterator i = begin_e(); i != end_e(); ++i)
         {
            i->aux = val;
         }
      }
      // Set the user data on all faces and vertices to 0
      void clear_aux_data()
      {		 
         for (face_iterator i = begin_f(); i != end_f(); ++i)
         {
            i->aux = 0;
         }
         for (vertex_iterator i = begin_v(); i != end_v(); ++i)
         {
            i->aux = 0;
         }
      }
	  void clear_vertex_aux_data()
	  {
		  for (vertex_iterator i = begin_v(); i != end_v(); ++i)
         {
            i->aux = 0; i->pv = 0;
         }
	  }
      
      template<typename T> struct Disposer
      {
         void operator()(T* aVal) const
         {
            delete aVal;
         }
      };
      // empty the mesh
      void clear()
      {
         // boost::intrusive is picky about deleting objects
         polys.clear_and_dispose(Disposer<DPolyhedron>());
         faces.clear_and_dispose(Disposer<DFace>());
         edges.clear_and_dispose(Disposer<DEdge>());
         vertices.clear_and_dispose(Disposer<DVertex>());
      }
      // Create a DMesh from Mesh
      //void create_mesh(Mesh& m, PolyBuffer& pb);
	  void create_mesh(Mesh& m);

   public:
	  void CopyMesh(DMesh& aRhs);
      void remove_edge(DEdge* e)
      {
         assert(e->face == 0);
         for (int i = 0; i < 2; ++i)
         {
            DVertex* vert = e->v[i];
            vert->unlink_edge(e);
            //if (vert->edge == 0) remove_vertex(vert);
         }
         edges.erase(edges.iterator_to(*e));
         delete e;
      }
      void remove_vertex(DVertex* v)
      {
         assert(v->edge == 0);
         vertices.erase(vertices.iterator_to(*v));
         delete v;
      }
      DEdge* insert_edge(DVertex* v1,
                         DVertex* v2)
      {
         DEdge* e = v1->find_edge(v2);
         if (! e)
         {
            // Create the edge
            edges.push_back(*new DEdge());
            e = &edges.back();
            e->initialize(v1,v2);
         }
         return e;
      }

	  //data members

//public:
	//PatchMesh * pMesh;
	//DFaceList            faces;//tmp put it here to debug


private:

      DVertexList          vertices;
      DEdgeList            edges;
      DFaceList            faces; //Dan's
      DPolyList            polys;
	  PolyList			   polyIDs;
};


inline DEdge* DVertex::find_edge(DVertex* v2)
{
   if (!edge) return 0;
   DEdge* e = edge;
   do
   {
      if (e->contain(v2)) return e;
      e = e->next(this);
   } while (e != edge);
   return 0;
}


inline void DVertex::unlink_edge(DEdge* aEdge)
{
   DEdge* e = aEdge;
   DEdge* ePrev;
   // remove face from linked list and re-assign first face
   do 
   {
      ePrev = e;
      e = e->next(this);
   } while (e != aEdge);

   if (ePrev != aEdge)
   {
      ePrev->next(this) = aEdge->next(this);
      edge = ePrev;
   }
   else
   {
      edge = 0;
   }
}

inline void DFace::insert_face(DEdge* edge)
{
   if (edge->face == 0)
   {
      edge->face = this;
      next(edge) = this;
   }
   else
   {
      DFace*& nextNextRef = edge->face->next(edge);
      DFace* nextNext = nextNextRef;
      nextNextRef = this;
      next(edge) = nextNext;
   }
}
inline DFace*& DFace::next(DEdge* edge)
{
   return next(edge->v[0], edge->v[1]);
}

inline void DFace::get_edges(DEdge* aEdges[3])
{
   for (int i = 0; i < 3; ++i)
   {
      DVertex* v1 = v[i];
      DVertex* v2 = v[(i+1)%3];
      if (i == 2) std::swap(v1,v2);
      DEdge* e = v1->edge;
      do 
      {
         if (e->v[0] == v1 && e->v[1] == v2)
         {
            aEdges[i] = e;
            break;
         }
         e = e->next(v1);
      } while (e != v1->edge);
   }
}

#endif // DMesh_h__