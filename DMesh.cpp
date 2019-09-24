#include "Algorithm.hpp"
#include "DMesh.hpp"
#include "Mesh.hpp"


#include "stdio.h"
#include <set>

const int DPolyhedron::sTetFaces[4*3] = { 0,1,2, 3,1,0, 3,2,1, 3,0,2 };
const int DPolyhedron::sTetTips[4] = { 3, 2, 0, 1 };
const int DPolyhedron::sOctFaces[8*3] = { 0,2,1, 0,1,5, 0,5,4, 0,4,2, 3,1,2, 3,5,1, 3,4,5, 3,2,4 };

//DMESH
DMesh::DMesh()
{
}
DMesh::~DMesh()
{
   clear();
}
DMesh::DMesh(DMesh& aSrc) //TODO: what is this for?
{
   CopyMesh(aSrc);
}
DMesh& DMesh::operator=(DMesh& aSrc)
{
   CopyMesh(aSrc);
   return *this;
}
DMesh& DMesh::operator=(Mesh& aRhs)
{
   int tempsize = vertices.size();
   clear();
   VertexList& verts = *aRhs.verts;
   for (size_t i = 0; i < aRhs.verts.size(); ++i)
   {
	  // cout << 
      insert_vertex(verts[i])->aux = i;
   }
   PolyhedronList& polys = *aRhs.polys;
   DVertexArray dex;
   get_vertex_index(dex);
   for (size_t i = 0; i < polys.size(); ++i)
   {
	   Polyhedron& p = polys[i];
	   int aar = p.arity;
	   assert(aar == 4 || aar == 6);
	   DVertexArray vertList(aar);
	   //cout << "insert poly: " << i << endl;
	   for (int j=0; j < aar; ++j)
	   {
		   vertList[j] = dex[p.vertex[j]];
	   }
	   insert_poly(vertList,p.material);
   }
   return *this;
}

void DMesh::CopyMesh(DMesh& aSrc)
{
   for (vertex_iterator i = aSrc.begin_v(); i != aSrc.end_v(); ++i)
   {
      DVertex* vert = insert_vertex(i->v);
      i->aux = vert;
   }
   for (face_iterator i = aSrc.begin_f(); i != aSrc.end_f(); ++i)
   {
      insert_face(i->v[0]->aux, i->v[1]->aux, i->v[2]->aux, i->materials);
   }
}

//DFACE
void DFace::get_adjacent_faces(std::vector<DFace*>& aFaces)
{
   DEdge* es[3];
   get_edges(es);
   for (int i = 0; i < 3; ++i)
   {
      es[i]->get_adjacent_faces(aFaces);
   }
   Algorithm::make_unique(aFaces);
}

//tn: return adjacent faces which don't share a non-manifold edge with the face
void DFace::get_adjacent_faces_sharing_only_manifold_edges(std::vector<DFace*>& aFaces)
{
	DEdge* es[3];
	get_edges(es);
	for (int i = 0; i < 3; ++i)
	{
		if (es[i]->is_crease_edge()) continue;
		es[i]->get_adjacent_faces(aFaces);
	}
	Algorithm::make_unique(aFaces);
}

bool DFace::has_same_materials(DFace * face)
{
	return (
		(materials[0] == face->materials[0] && materials[1] == face->materials[1]) || 
		(materials[1] == face->materials[0] && materials[0] == face->materials[1]) 
		);
}
void DFace::set_same_materials_with_face(DFace * face)
{
	this->materials[0] = face->materials[0];
	this->materials[1] = face->materials[1];
}

bool DFace::is_same_face_with(DFace * face) //assumed that face is sorted already
{	
	return (this->v[0] == face->v[0] &&
		this->v[1] == face->v[1] &&
		this->v[2] == face->v[2]);
}

bool DFace::contain(DVertex * v_in)
{ 
	return ((v[0] == v_in) || (v[1] == v_in) || (v[2] == v_in));
}

bool DFace::contain(DEdge * e_in)
{
	DEdge * edges[3];
	get_edges(edges);
	for(int i=0;i<3;i++)
	{
		DEdge * ce = edges[i];
		if(e_in->same_edge(ce)) return true;
	}
	return false;
}

bool DFace::is_invalid()
{
	bool result(false);

	if(v[0] == v[1] || v[1] == v[2] || v[2] == v[0])
		result = true;
	if(v[0]->v == v[1]->v || v[1]->v == v[2]->v || v[2]->v == v[0]->v)
		result = true;	

	return result;
		
}

bool DEdge::is_in_edges_list(DEdgeArray& aEdges)
 {
	 return (std::find(aEdges.begin(), aEdges.end(), this) != aEdges.end());

 }

void DEdge::get_adjacent_faces(DFaceArray& aFaces)
{
   DFace* f = face;
   if (!f) return;
   do
   {
      aFaces.push_back(f);
      f = f->next(this);
   } while(f != face);
}
//get only adjacent faces within a region
void DEdge::get_adjacent_faces(DFaceArray& aFaces, int material) 
{
	DFace* f = face;
	if (!f) return;
	do
	{
		if(f->materials[0] == material || f->materials[1] == material)
			aFaces.push_back(f);
		f = f->next(this);
	} while(f != face);
	
}
int DEdge::get_adjacent_face_count(bool debug)
{
   int count(0);
   DFace* f = face;
   if (!f) return 0;
   do
   {
      ++count;
	  if(!f) break;	  
	  
	 /* if(debug) {
		  std::cerr << "f->dead = " << f->dead << std::endl;
		  if(debug && f->dead) {system("pause");}
		  f->print();
	  }*/

      f = f->next(this);
   } while(f != face);
   return count;
}

bool DEdge::is_crease_edge()
{
	return (get_adjacent_face_count() != 2);
}

bool DEdge::draw_as_manifold(std::vector<int> & hiddenRegions)
{
	//is it manifold edge?
	if (this->get_adjacent_face_count() == 2) return false;

	//checking regions?
	std::set<int> associatedRegions;	
	//go thru face adjacent with the edge
	DFace* f = face;
	if (!f) return 0;
	do
	{
		for(size_t i=0;i<f->materials.size();i++)
		{
			associatedRegions.insert(f->materials[i]);
		}

      f = f->next(this);
	} while(f != face);
	
	//count how many regions should be hidden
	for(size_t i = 0;i < hiddenRegions.size();i++)
		associatedRegions.erase(hiddenRegions[i]);
	if (associatedRegions.empty())
		return false;
	else
		return true;
}

bool DEdge::is_hidden(std::vector<int> & hiddenRegions)
{
	//checking regions?
	std::set<int> associatedRegions;
	
	//go thru face adjacent with the edge
	DFace* f = face;
	if (!f) return 0;
	do
	{
		if(!f) break;

		for(size_t i=0;i<f->materials.size();i++)
		{
			associatedRegions.insert(f->materials[i]);
		}

		f = f->next(this);
	} while(f != face);
	
	//count how many regions should be hidden
	for(size_t i = 0;i < hiddenRegions.size();i++)
		associatedRegions.erase(hiddenRegions[i]);
	if (associatedRegions.empty())
		return true;
	else
		return false;

}

bool DFace::is_hidden(std::vector<int> & hiddenRegions)
{
	//checking regions?
	std::set<int> associatedRegions;

	for(size_t i=0;i<materials.size();i++)
	{
		associatedRegions.insert(materials[i]);
	}	
	
	//count how many regions should be hidden
	for(size_t i = 0;i < hiddenRegions.size();i++)
		associatedRegions.erase(hiddenRegions[i]);
	if (associatedRegions.empty())
		return true;
	else
		return false;
}

bool DFace::is_in_list(DFaceArray & faces)
{
	if(faces.empty()) return false;
	return (std::find(faces.begin(), faces.end(), this) != faces.end());
}


//void DMesh::create_mesh(Mesh& m, PolyBuffer& pb)
void DMesh::create_mesh(Mesh& m)
{
   m.verts.clear();
   m.faces.clear();
   m.polys.clear();
   m.edges.clear();
   index_vertices();
   VertexList& mverts = *m.verts;
   FaceList& mfaces = *m.faces;
   PolyhedronList& mpolys = *m.polys;
   EdgeList& medges = *m.edges;
   mverts.resize(vertex_count());
   mfaces.resize(face_count());
   mpolys.resize(poly_count());
   int counter = 0;
   for (edge_iterator i = begin_e(); i != end_e(); ++i){
	   if(i->is_crease_edge()){
		   counter++;
	   }
   }
   medges.resize(counter);

   int ind = 0;
   for (vertex_iterator i = begin_v(); i != end_v(); ++i)
   {
      mverts[ind++] = i->v;
   }
   ind = 0;
   for (poly_iterator i = begin_p(); i != end_p(); ++i)
   {
	  Polyhedron& p = mpolys[ind++];
	  int aar = i->arity;
	  DVertex* vd;
	  for (int j=0;j<aar;j++){
        vd = i->v[j];
		//cout << vd->v << " ";
		//cout << (int) vd->aux << endl;
		p.vertex[j] = vd->aux;
	  }
	  p.material = i->material;
	  p.arity = i->arity;
	  //cout << "--------------------" << endl;
   }
   ind = 0;
   for (edge_iterator i = begin_e(); i != end_e(); ++i)
   {
	   if (i->is_crease_edge()){
		   Edge& e = medges[ind++];
		   e.first = i->v[0]->aux;
		   e.second = i->v[1]->aux;
	   }
   }
   ind = 0;
   for (face_iterator i = begin_f(); i != end_f(); ++i)
   {
      Face& f = mfaces[ind++];
      f.materials = i->materials;
      f.vertex[0] = i->v[0]->aux;
      f.vertex[1] = i->v[1]->aux;
      f.vertex[2] = i->v[2]->aux;
   }
}

DEdge* DFace::get_edge(int aIndex)
{
   DVertex* v1 = v[aIndex];
   DVertex* v2 = v[(aIndex+1)%3];
   return v1->find_edge(v2);
}

void DVertex::get_adjacent_vertices(DVertexArray& aVertices)
{
   DEdge* e = edge;
   if (e != 0)
   {
      do 
      {
         aVertices.push_back(e->opposite(this));
         e = e->next(this);
      } while (e != edge);
   }
}

bool DVertex::is_in_vertices_list(DVertexArray& aVertices)//t
{
	return (std::find(aVertices.begin(), aVertices.end(), this) != aVertices.end());
}

void DVertex::get_edges(DEdgeArray& aEdges)
{
   DEdge* e = edge;
   if (e != 0)
   {
      do 
      {
		 assert(e->contain(this));
         aEdges.push_back(e);
         e = e->next(this);
      } while (e != edge);
   }
}

void DVertex::get_crease_edges(DEdgeArray & aEdges)
{
	DEdge* e = edge;
	if (e != 0)
	{
		do 
		{
			assert(e->contain(this));
			if(e->is_crease_edge())	aEdges.push_back(e);
			e = e->next(this);
		} while (e != edge);
	}
}

bool DVertex::on_crease_edge()
{
   DEdge* e = edge;
   if (e != 0)
   {
      do 
      {
		 if (e->is_crease_edge()) 
			  return true;
         e = e->next(this);
      } while (e != edge);
   }
   return false;
}

bool DVertex::on_diff_crease_edge(DEdge *& e)
{
	DEdge* ce = edge;
	if (ce != 0)
	{
		do 
		{
			if (ce!=e && ce->is_crease_edge())
				return true;
			ce = ce->next(this);
		} while (ce != edge);
	}
	return false;
}

bool DVertex::is_hidden(std::vector<int> & hiddenRegions)
{
	//checking regions?
	std::set<int> associatedRegions;

	DFaceArray faces;
	this->get_faces(faces);
	Algorithm::make_unique(faces);

	//go thru face adjacent with the edge
	for(size_t i=0;i<faces.size();i++)
	{
		DFace* f = faces[i];
		if (!f) return 0;
		for(size_t i=0;i<f->materials.size();i++)
		{
			associatedRegions.insert(f->materials[i]);
		}
	}
	//count how many regions should be hidden
	for(size_t i = 0;i < hiddenRegions.size();i++)
		associatedRegions.erase(hiddenRegions[i]);
	if (associatedRegions.empty())
		return true;
	else
		return false;
}
bool DVertex::has_material(int mat)
{
	DFaceArray af;
	get_faces(af);
	for(size_t j=0; j<af.size(); ++j){
		if (af[j]->materials[0]==mat) return true;
		if (af[j]->materials[1]==mat) return true;
	}
	return false;
}
void DVertex::get_faces(DFaceArray& aFaces)
{
   aFaces.clear();
   DEdge* e = edge;
   if (e != 0)
   {
      do 
      {
         e->get_adjacent_faces(aFaces);
         e = e->next(this);
      } while (e != edge);
   }
   Algorithm::make_unique(aFaces);
}

DEdge * DVertex::get_opposite_edge(DFace * f)
{
	if (! f->contain(this)) return 0;

	int evs[2];
	int i = 0;
	for(int j=0;j<3;j++)
	{
		if(f->v[j]!= this) evs[i++] = j;
		if (i>2) break;
	}
	return (f->v[evs[0]]->find_edge(f->v[evs[1]]));
}

void DVertex::printInfo()
{
	std::cerr << "DVertex: " << this << "(" << v[0] << ", " << v[1] << ", " << v[2] << ")\n";
	std::cerr << "aux: " << (int) aux << " | (pv*) aux " << (PVertex*)  aux << "pv* : " << pv << std::endl;
	std::cerr << "loopConnectedVertices : ";
	for(size_t i=0;i<loopConnectedVertices.size();i++)
	{
		DVertex * v = loopConnectedVertices.at(i);
		std::cerr << v << " ";
	}		 
	std::cerr << ".end.\n";
}

void DVertex::get_vertex_link(DVertexArray & lkV, DEdgeArray & lkE)
{
	lkV.clear();
	lkE.clear();

	DFaceArray faces;
	this->get_faces(faces);

	for(size_t i=0;i<faces.size();++i)
	{
		DFace * f = faces[i];
		DEdge * e = this->get_opposite_edge(f);
		lkE.push_back(e);
		lkV.push_back(e->v[0]);
		lkV.push_back(e->v[1]);
	}
}

void DEdge::get_adjacent_edges(DEdgeArray& aEdges)
{
   v[0]->get_edges(aEdges);
   v[1]->get_edges(aEdges);
   Algorithm::make_unique(aEdges);
}