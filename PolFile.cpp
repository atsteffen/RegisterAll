#include "PolFile.hpp"
#include "DMesh.hpp"
//#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
using namespace boost;


bool PolFile::ReadSurfaceLandmarks(const std::string& aFile,
							std::vector<MatPoint>& mLandmarks)
{
	ifstream file(aFile.c_str());
	std::string identifier;
	if (!(file >> identifier && identifier == "landmarks"))
	{
		return false;
	}
	
	string line;
	getline(file, line);
	
	while (getline(file, line)) {
		MatPoint mp;
		
		istringstream tokenizer(line);
		string token;
		
		float a, b, c;
		getline(tokenizer, token, ' ');
		istringstream float_iss1(token);
		float_iss1 >> a;
		getline(tokenizer, token, ' ');
		istringstream float_iss2(token);
		float_iss2 >> b;
		getline(tokenizer, token, ' ');
		istringstream float_iss3(token);
		float_iss3 >> c;
		mp.point[0] = a;
		mp.point[1] = b;
		mp.point[2] = c;
		
		mp.materials[0] = -1;
		mp.materials[1] = -2;
		
		mLandmarks.push_back(mp);
    }
	
	return true;
}

bool PolFile::ReadLandmarks(const std::string& aFile,
                   std::vector<MatPoint>& mLandmarks)
{
	ifstream file(aFile.c_str());
	std::string identifier;
	if (!(file >> identifier && identifier == "landmarks"))
	{
		return false;
	}
	
	string line;
	getline(file, line);
	while (getline(file, line)) {
		MatPoint mp;
		
		istringstream tokenizer(line);
		string token;
		
		float a, b, c;
		getline(tokenizer, token, ' ');
		istringstream float_iss1(token);
		float_iss1 >> a;
		getline(tokenizer, token, ' ');
		istringstream float_iss2(token);
		float_iss2 >> b;
		getline(tokenizer, token, ' ');
		istringstream float_iss3(token);
		float_iss3 >> c;
		mp.point[0] = a;
		mp.point[1] = b;
		mp.point[2] = c;
		
		getline(tokenizer, token, ' ');
		istringstream int_iss(token);
		int i;
		int_iss >> i;
		if (i == '\r'){
			mp.materials.push_back(-2);
		}
		while (token.size() != 0) {
			mp.materials.push_back(i);

			getline(tokenizer, token, ' ');
			istringstream int_iss(token);
			int_iss >> i;
		}
		
		mLandmarks.push_back(mp);
    }
	
	return true;
}

bool PolFile::Read(const std::string& aFile,
                   DMesh&             aMesh)
{
   aMesh.clear();
   ifstream file(aFile.c_str());
   std::string identifier;
   if (!(file >> identifier && identifier == "poly"))
   {
      return false;
   }

   int vertCount, polycount;
   file >> vertCount >> polycount;
   for (int i = 0; i < vertCount; ++i)
   {
      float a,b,c;
      file >> a >> b >> c;
      aMesh.insert_vertex(Vec3(a,b,c));
   }
   DVertexArray index;
   aMesh.get_vertex_index(index);
   for (int i = 0; i < polycount; ++i)
   {
      int arity, material;
      file >> arity;
      assert(arity == 4 || arity == 6);
      DVertexArray verts(arity);
	  //Polyhedron polyidxs[arity];
      for (int j = 0; j < arity; ++j)
      {
         int vi;
         file >> vi;
         verts[j] = index[vi];
      }
      file >> material;
      aMesh.insert_poly(verts, material);
	  //aMesh.insert_polyidx(polyidxs);
   }
   return true;
}

bool PolFile::Write(const std::string& aFile,
                    DMesh&             aMesh)
{
   ofstream io(aFile.c_str());
   std::string identifier;
   io << "poly" << '\n';
   io << aMesh.vertex_count() << ' ' << aMesh.poly_count() << '\n';
   for (DMesh::vertex_iterator i = aMesh.begin_v(); i != aMesh.end_v(); ++i)
   {
      const Vec3& v = i->v;
      io << v[0] << ' ' << v[1] << ' ' << v[2] << '\n';
   }
   aMesh.index_vertices();
   for (DMesh::poly_iterator i = aMesh.begin_p(); i != aMesh.end_p(); ++i)
   {
      io << i->arity << "  ";
      for (int j = 0; j < i->arity; ++j)
      {
         io << (int)i->v[j]->aux << ' ';
      }
      io << ' ' << i->material;
      io << '\n';
   }
   return true;
}
