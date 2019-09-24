#include <iostream>
#include "DMesh.hpp"
#include "PolFile.hpp"
#include "Registration.hpp"
#include "Mesh.hpp"
#include "PolyMesh.hpp"
#include <string>
#include <fstream>

char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

int main (int argc, char * argv[]) {
	
	char * c1 = argv[1];
	std::string command = c1;
	
	float fitWeight = 4.0;
	float energy = 1.5;
	int iterations = 4;
	int sublevel = 3;
	
	char * outfile = "";
	char * infile = "";
	char * landmarkfile = "";
	char * afile = "";
	char * bfile = "";
	char * maskfile = "";
	
    char * fit = getCmdOption(argv, argv + argc, "-f");
	if (fit)
    {
        fitWeight = (float) atof(fit);
    }
	char * deform = getCmdOption(argv, argv + argc, "-d");
	if (deform)
    {
        energy= (float) atof(deform);
    }
	char * subs = getCmdOption(argv, argv + argc, "-s");
	if (subs)
    {
        sublevel = (int) atol(subs);
    }
	char * iters = getCmdOption(argv, argv + argc, "-i");
	if (iters)
    {
        iterations = (int) atol(iters);
    }
	char * outf = getCmdOption(argv, argv + argc, "-o");
	if (outf)
    {
        outfile = outf;
    }
	char * inf = getCmdOption(argv, argv + argc, "-p");
	if (inf)
    {
        infile = inf;
    }
	char * landf = getCmdOption(argv, argv + argc, "-l");
	if (landf)
    {
        landmarkfile = landf;
    }
	char * af = getCmdOption(argv, argv + argc, "-a");
	if (af)
    {
        afile = af;
    }
	char * bf = getCmdOption(argv, argv + argc, "-b");
	if (bf)
    {
        bfile = bf;
    }
	char * maskf = getCmdOption(argv, argv + argc, "-m");
	if (maskf)
    {
        maskfile = maskf;
    }
	
	DMesh aMesh;
	if (command == "submask"){
		// load mesh
		PolFile::Read(infile, aMesh);
		// convert to polymesh
		Mesh m2;
		DMesh& q2 = aMesh;
		q2.create_mesh(m2);
		PolyMesh pm2;
		pm2.crease_faces.ref(m2.faces);
		pm2.verts.ref(m2.verts);
		pm2.polys.ref(m2.polys);
		pm2.crease_edges.ref(m2.edges);	
		// subdivide mask
		pm2.SubdivideMask(maskfile,sublevel);
	}
	else if (command == "full") { 
		// load mesh
		PolFile::Read(infile, aMesh);
		// load landmarks
		std::vector<MatPoint> mLandmarks;
		PolFile::ReadLandmarks(landmarkfile,mLandmarks);
		// convert to polymesh
		Mesh m2;
		DMesh& q2 = aMesh;
		q2.create_mesh(m2);
		PolyMesh pm2;
		pm2.crease_faces.ref(m2.faces);
		pm2.verts.ref(m2.verts);
		pm2.polys.ref(m2.polys);
		pm2.crease_edges.ref(m2.edges);	
		// do register
		pm2.AlignLS2(mLandmarks,fitWeight,energy,sublevel,iterations);
		q2 = m2;
		// write mesh
		PolFile::Write(outfile, q2);
	}
	else if (command == "precompute"){
		// load mesh
		PolFile::Read(infile, aMesh);
		// load landmarks
		std::vector<MatPoint> mLandmarks;
		PolFile::ReadLandmarks(landmarkfile, mLandmarks);
		// convert to polymesh
		Mesh m2;
		DMesh& q2 = aMesh;
		q2.create_mesh(m2);
		PolyMesh pm2;
		pm2.crease_faces.ref(m2.faces);
		pm2.verts.ref(m2.verts);
		pm2.polys.ref(m2.polys);
		pm2.crease_edges.ref(m2.edges);	
		// precompute matrices
		pm2.PrecomputeMatrices(maskfile,afile,bfile,mLandmarks,fitWeight,energy,sublevel);
	}
	else if (command == "addfit"){
		// load mesh
		PolFile::Read(infile, aMesh);
		// load landmarks
		std::vector<MatPoint> mLandmarks;
		PolFile::ReadLandmarks(landmarkfile,mLandmarks);
		// convert to polymesh
		Mesh m2;
		DMesh& q2 = aMesh;
		q2.create_mesh(m2);
		PolyMesh pm2;
		pm2.crease_faces.ref(m2.faces);
		pm2.verts.ref(m2.verts);
		pm2.polys.ref(m2.polys);
		pm2.crease_edges.ref(m2.edges);	
		// add fit
		pm2.ReComputeMatrices(maskfile,afile,bfile,mLandmarks,fitWeight,energy,sublevel);
	}
	else if (command == "solve"){
		// load mesh
		PolFile::Read(infile, aMesh);
		// convert to polymesh
		Mesh m2;
		DMesh& q2 = aMesh;
		q2.create_mesh(m2);
		PolyMesh pm2;
		pm2.crease_faces.ref(m2.faces);
		pm2.verts.ref(m2.verts);
		pm2.polys.ref(m2.polys);
		pm2.crease_edges.ref(m2.edges);	
		// solve matrices
		pm2.SolveForMatrices(afile,bfile);
		q2 = m2;
		// write mesh
		PolFile::Write(outfile, q2);
	}
	else if (command == "getlandmarks"){
		// load mesh
		PolFile::Read(infile, aMesh);
		// convert to polymesh
		Mesh m2;
		DMesh& q2 = aMesh;
		q2.create_mesh(m2);
		PolyMesh pm2;
		pm2.crease_faces.ref(m2.faces);
		pm2.verts.ref(m2.verts);
		pm2.polys.ref(m2.polys);
		pm2.crease_edges.ref(m2.edges);
		// pull out landmarks
		MatPointList subMeshMatPoints;
		pm2.GetCreaseMesh().ExtractMatPoints(subMeshMatPoints);
		// write to landmark file
		ofstream myfile;
		myfile.open(outfile);
		myfile << "landmarks\n";
		for (int i = 0 ; i < subMeshMatPoints.size(); ++i){
			Vec3 vecpoint = subMeshMatPoints[i].point;
			myfile << vecpoint[0] << " " << vecpoint[1] << " " << vecpoint[2] << " ";
			for (int j = 0; j < subMeshMatPoints[i].materials.size(); ++j) {
				myfile << subMeshMatPoints[i].materials[j] << " ";
			}
			myfile << "\n";
		}
		myfile.close();
	}
	else if (command == "help"){
		// TODO: add man page
		cout << "Man page still needs to be created" << endl;
	}
	else {
		cout << "error: no command \"" << argv[1] << "\". Type \"registerall help\" to see accepted commands." << endl;
	}
	
    return 0;
}
