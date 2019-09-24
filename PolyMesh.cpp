#include "PolyMesh.hpp"
#include <set>
#include <math.h>
#include "tiny_vector.hpp"
#include <boost/iterator/counting_iterator.hpp>
#include <iostream>
#include <fstream>
//using namespace std;

const int Polyhedron::sTetFaces[4*3] = { 0,1,2, 3,1,0, 3,2,1, 3,0,2 };
const int Polyhedron::sTetTips[4] = { 3, 2, 0, 1 };
const int Polyhedron::sOctFaces[8*3] = { 0,2,1, 0,1,5, 0,5,4, 0,4,2, 3,1,2, 3,5,1, 3,4,5, 3,2,4 };

namespace
{
   Vec3 Midpoint(Vec3 a, Vec3 b) { return (a+b)/2; }
}

PolyMesh::PolyMesh()
{
}

// returns IsReversed()
bool GetFace(boost::array<int, 3>& aFace)
{
   boost::array<int, 3> orig = aFace;
   std::sort(aFace.begin(), aFace.end());
   int eq = ((orig[0] == aFace[0]) ? 1 : 0)
          + ((orig[1] == aFace[1]) ? 1 : 0)
          + ((orig[2] == aFace[2]) ? 1 : 0);
   bool sameOrder = (eq == 0 || eq == 3);
   return !sameOrder;
}

// Polys have been modified, rebuild creases
void PolyMesh::UpdatePolys()
{
   // don't update face-creases meshes without polys
   if ((*polys).empty())
   {
      UpdateFaces();
      return;
   }
   
   crease_faces.clear();

   typedef std::pair<int, int> Mats;
   typedef std::map<Poly::FaceVerts, Mats> FaceMap;
   
   FaceMap faces;
   for (size_t i = 0; i < polys.size(); ++i)
   {
      Poly& p = polys[i];
      for (size_t j = 0; j < p.GetFaceCount(); ++j)
      {
         Poly::FaceVerts verts;
         p.GetFace(j, verts);
         bool reversed = GetFace(verts);
         Mats* m;
         FaceMap::iterator iter = faces.find(verts);
         if (iter == faces.end())
         {
            m = &faces[verts];
            *m = Mats(-1,-1);
         }
         else
         {
            m = &iter->second;
         }
         if (reversed)
         {
            m->second = p.material;
         }
         else
         {
            m->first = p.material;
         }
      }
   }
   for (FaceMap::iterator i = faces.begin(); i != faces.end(); ++i)
   {
      Mats& m = i->second;
      if (m.first != m.second)
      {
         Face f;
         f.vertex = i->first;
         f.materials[1] = m.first;
         f.materials[0] = m.second;
         crease_faces.push_back(f);
      }
   }
   
}
void PolyMesh::UpdateFaces()
{
   //DEBUG_LOG("PolyMesh::UpdateFaces")
   crease_edges.clear();
   for (size_t i = 0; i < crease_faces.size(); ++i)
   {
      Face& f = crease_faces[i];
      if (f.materials[0] != -1 && f.materials[1] != -1)
      {
         for (int j = 0; j < 3; ++j)
         {
            Edge e = Edge(f.vertex[j], f.vertex[(j+1)%3]);
            OrderEdge(e);
            crease_edges.push_back(e);
         }
      }
   }
   std::sort(crease_edges.begin(), crease_edges.end());
   std::vector<Edge> tmpCreases;
   {
      size_t i(1), j(0);
      for (; i < crease_edges.size();)
      {
         while (i < crease_edges.size() && crease_edges[j] == crease_edges[i])
         {
            ++i;
         }
         int count = i-j;
         if (count != 2)
         {
            tmpCreases.push_back(crease_edges[j]);
         }
         j=i;
      }
   }

   std::swap(*crease_edges, tmpCreases);
   crease_faces.GetEvents().Updated(); 
   crease_edges.GetEvents().Updated();
   //DEBUG_LOG(".\n")
}

struct RegionInfo
{
   RegionInterface mRegion;
   EdgeList        mEdges;
};

void PolyMesh::ComputeCreases()
{
   UpdatePolys();
}

void PolyMesh::Updated()
{ 
   verts.GetEvents().Updated(); 
   polys.GetEvents().Updated();
}

void PolyMesh::Recompute()
{
   Updated();
   ComputeCreases();
}

void PolyMesh::GetTetPairs(std::vector<TetPair>& aTetPairs)
{
	typedef boost::array<int, 3> FaceInd;
	typedef std::map<FaceInd, int> TetFaceMap;
	TetFaceMap tetFaces;
	for (size_t i = 0; i < polys.size(); ++i)
	{
		Polyhedron& poly = polys[i];
		FaceInd verts;
		for (size_t j = 0; j < poly.GetFaceCount(); ++j)
		{
			poly.GetFace(j, verts);
			int tipIndex = poly.GetTip(j);
			OrderFace(verts);
			TetFaceMap::iterator iter = tetFaces.find(verts);
			if (iter != tetFaces.end())
			{
				TetPair p;
				p.tetTips[0] = iter->second;
				p.tetTips[1] = tipIndex;
				p.verts = verts;
				aTetPairs.push_back(p);
			}
			else
			{
				tetFaces[verts] = tipIndex;
			}
		}
	}
}

void PolyMesh::writeMaskToFile(string filename,std::vector<SparseArray> maskb){
	ofstream myfile;
	myfile.open(filename.c_str());
	if (!myfile.is_open()){
		cout << "Could not open file " << filename << ". Check that path exists and has the correct permissions." << endl;
		return;
	}
	myfile << "maskb\n";
	myfile << maskb.size();
	myfile << "\n";
	for (size_t i = 0; i < maskb.size(); ++i)
	{
		SparseArray temp = maskb[i];
		myfile << temp.size << " ";
		for (size_t j = 0; j < temp.size; ++j) {
			myfile << temp.idx[j] << " " << temp.val[j] << " ";
		}
		myfile << "\n";
	}
	myfile.close();
}

std::vector<SparseArray> PolyMesh::readMaskFromFile(string filename){
	ifstream file(filename.c_str());
	std::string identifier;
	std::vector<SparseArray> maskb;
	if (!file.is_open()){
		cout << "Could not write to file: " << filename << ". Check that the file exists and has the correct permissions." << endl;
		return maskb;
	}
	if (!(file >> identifier && identifier == "maskb"))
	{
		cout << "Mask file: " << filename << " has the wrong format." << endl;
		return maskb;
	}
	int rowcount;
	int sparseCount;
	file >> rowcount;
	for (size_t i = 0; i < rowcount; ++i)
	{
		file >> sparseCount;
		SparseArray temp = SparseArray(sparseCount);
		for (size_t j = 0; j < sparseCount;  ++j) {
			file >> temp.idx[j];
			file >> temp.val[j];
		}
		maskb.push_back(temp);
	}
	file.close();
	return maskb;
}
void PolyMesh::writeMat(string filename,matrix<double> ata, int nnz){
	ofstream myfile;
	myfile.open(filename.c_str());
	if (!myfile.is_open()){
		cout << "Could not open file " << filename << ". Check that path exists and has the correct permissions." << endl;
		return;
	}
	myfile << "matrix\n";
	myfile << ata.size1() << " " << ata.size2() << " " << nnz;
	myfile << "\n";
	for (size_t i = 0; i < ata.size1(); ++i) {
		for (size_t j = 0; j < ata.size2(); ++j) {
			if (ata(i,j) != 0.0) {
				myfile << ata.size2()*i + j << " " << ata(i,j) << " ";
			}
		}
	}
	myfile.close();
}
matrix<double> PolyMesh::readMat(string filename, int& nnz){
	ifstream myfile(filename.c_str());
	std::string identifier;
	int size1, size2;
	matrix<double> ata_null(0,0);
	if (!myfile.is_open()){
		cout << "Could not write to file: " << filename << ". Check that the file exists and has the correct permissions." << endl;
		return ata_null;
	}
	if (!(myfile >> identifier && identifier == "matrix"))
	{
		cout << "Matrix file: " << filename << " has the wrong format." << endl;
		return ata_null;
	}
	myfile >> size1;
	myfile >> size2;
	myfile >> nnz;
	matrix<double> ata(size1,size2);
	int index;
	double value;
	myfile >> index;
	myfile >> value;
	for (size_t i = 0; i < size1; ++i) {
		for (size_t j = 0; j < size2; ++j) {
			if (index == size2*i + j) {
				ata(i,j) = value;
				myfile >> index;
				myfile >> value;
			}
			else {
				ata(i,j) = 0.0;
			}
		}
	}
	myfile.close();
	return ata;
}
