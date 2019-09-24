#include "Mesh.hpp"
#include <stdio.h>
#include <time.h>
#include "SparseArray.hpp"

#include "boost/numeric/ublas/matrix.hpp"

#include "slu_ddefs.h"
#include "PolyMesh.hpp"
#include <fstream>
#include <iostream>

using namespace boost::numeric::ublas;

namespace
{
   const int tettris[4][3]={{0,1,2},{0,2,3},{0,3,1},{1,3,2}};
   const int tettip[4] =    {3,      1,      2,      0};
   const int octtris[8][3]={{0,1,2},{3,4,5},{0,4,3},{0,2,4},{2,5,4},{2,1,5},{1,3,5},{1,0,3}};
}

static float getVolume(Vec3& a, Vec3& b, Vec3& c, Vec3& d)
{
   Vec3 na = b - a;
   Vec3 nb = c - b;
   Vec3 nc = c - d;
   Vec3 n = cross(na,nb);
   return nc.dot(n)/6.0f;
}
void PolyMesh::SubdivideMask(string maskfile,
							 int subLevel)
{
	// Subdivide mask
	clock_t start, end;
	printf("Subdividing Mask..."); start = clock();
	
	std::vector<SparseArray> maskb;
	for (size_t i = 0; i < verts.size(); ++i)
	{
		//cout << verts[i] << endl;
		SparseArray temp = SparseArray(1);
		temp.val[0] = 1.0;
		temp.idx[0] = i;
		maskb.push_back(temp);
	}
	
	PolyMesh subMaskMesh;
	subMaskMesh.clone(*this);
	subMaskMesh.SubdivideMask(subLevel, maskb);
	std::vector<TetPair> tetPairs;
	GetTetPairs(tetPairs);
	end = clock(); printf("... (%f) seconds\n",(end-start)/((float)CLOCKS_PER_SEC));
	
	writeMaskToFile(maskfile, maskb);
}

void PolyMesh::PrecomputeMatrices(string maskfile,
								  string atafile,
								  string atbfile,
								  MatPointList& aLandmarks,
								  float         fitWt,
								  float         energyWt,
								  int           subLevel)
{
	// Load mask
	clock_t start, end;
	printf("Loading Mask..."); start = clock();
	std::vector<SparseArray> maskb;
	maskb = readMaskFromFile(maskfile);
	end = clock(); printf("... (%f) seconds\n",(end-start)/((float)CLOCKS_PER_SEC));
	
	// Find closest point
	//printf("Iteration %i\n", iterNum);
	printf("Allocating..."); start = clock();
	
	PolyMesh subMesh;
	subMesh.clone(*this);
	subMesh.SubdivideMesh(subLevel);
	
	MatPointList subMeshMatPoints;
	subMesh.GetCreaseMesh().ExtractMatPoints(subMeshMatPoints);
	
	// define empty aTa and aTb
	int nnz = 0; // number of non-zero values
	int numvar = verts.size();
	matrix<double> ata2(numvar, numvar);
	matrix<double> atb2(3,numvar);
	for (int i = 0 ; i < numvar ; i++) {
		for (int j = 0 ; j < numvar ; j++) {
			ata2(i,j) = 0.0;
		}
		for (int j = 0 ; j < 3 ; j++) {
			atb2(j,i) = 0.0;
		}
	}
	end = clock(); printf("... (%f) seconds\n",(end-start)/((float)CLOCKS_PER_SEC));
	
	// Calculate closest mesh point to each landmark
	printf("Adding fitting terms..."); start = clock();
	for (int i = 0 ; i < aLandmarks.size(); ++i)
	{
		MatPoint& landmark = aLandmarks[i];
		Vec3 lm = aLandmarks[i].point;
		int ind = -1 ;
		float mindis2 = 1000000.0f*1000000.0f;
		for (size_t j = 0; j < subMeshMatPoints.size(); ++j)
		{
			MatPoint& pt = subMeshMatPoints[j];
			if (pt.materials.equals(landmark.materials))
			{
				int fi = j;
				float dis = (subMesh.verts[fi] - lm).length_squared();
				if ( dis < mindis2 )
				{
					mindis2 = dis ;
					ind = fi ;
				}
			}
		}
		
		// Store index array as in laplace code and add components to aTa and aTb
		if (ind >= 0) {
			// Add fitting term to aTa and aTb
			for (int j = 0 ; j < maskb[ind].size ; j++ ) {
				for (int k = 0 ; k < maskb[ind].size ; k++ ) {
					bool flagnzz = false;
					if (ata2(maskb[ind].idx[j],maskb[ind].idx[k])==0.0) flagnzz=true;
					
					ata2(maskb[ind].idx[j],maskb[ind].idx[k]) += maskb[ind].val[j]*maskb[ind].val[k]*fitWt*fitWt;
					
					if ((ata2(maskb[ind].idx[j],maskb[ind].idx[k])!=0.0) && flagnzz) nnz++;
					if ((ata2(maskb[ind].idx[j],maskb[ind].idx[k])==0.0) && !flagnzz) nnz--;
				}
				for (int k = 0 ; k < 3 ; ++k){
					atb2(k,maskb[ind].idx[j]) += maskb[ind].val[j]*lm[ k ]*fitWt*fitWt;
				}
			}
		}
	}
	end = clock(); printf("... (%f) seconds\n",(end-start)/((float)CLOCKS_PER_SEC));
	
	// Add energy terms
	printf("Adding energy terms..."); start = clock();
	std::vector<TetPair> tetPairs;
	GetTetPairs(tetPairs);
	for (int i = 0 ; i < tetPairs.size() ; i ++ )
	{
		TetPair& p = tetPairs[i];
		
		// Get relevant vertices
		int ip1 = p.tetTips[0];
		int ip2 = p.tetTips[1];
		int im1 = p.verts[0];
		int im2 = p.verts[1];
		int im3  = p.verts[2];
		Vec3 p1 = verts[ip1];
		Vec3 p2 = verts[ip2];
		Vec3 m1 = verts[im1];
		Vec3 m2 = verts[im2];
		Vec3 m3 = verts[im3];
		
		// Get different volumes
		float vol1 = getVolume( m3,m2,m1,p2 ) ;
		float vol2 = getVolume( m1,m2,m3,p1 ) ;
		float vol3 = getVolume( p1,m3,m2,p2 ) ;
		float vol4 = getVolume( p1,m1,m3,p2 ) ;
		float vol5 = getVolume( p1,m2,m1,p2 ) ;
		
		float normvol = vol1 + vol2 ;
		normvol = abs(normvol);
		matrix<double> aBuffer2(5,2);
		
		aBuffer2(0,0) = vol1 / normvol * energyWt ;
		aBuffer2(0,1) = ip1;
		aBuffer2(1,0) =  vol2 / normvol * energyWt ;
		aBuffer2(1,1) = ip2;
		aBuffer2(2,0) = -vol3 / normvol * energyWt ;
		aBuffer2(2,1) = im1;
		aBuffer2(3,0) = -vol4 / normvol * energyWt ;
		aBuffer2(3,1) = im2;
		aBuffer2(4,0) = -vol5 / normvol * energyWt ;
		aBuffer2(4,1) = im3;
		
		for (int j = 0 ; j < 5 ; j++ ) {
			for (int k = 0 ; k < 5 ; k++ ) {
				bool flagnzz = false;
				if (ata2(aBuffer2(j,1),aBuffer2(k,1)) == 0.0) flagnzz = true;
				
				ata2(aBuffer2(j,1),aBuffer2(k,1)) += aBuffer2(j,0)*aBuffer2(k,0)*energyWt*energyWt;
				
				if ((ata2(aBuffer2(j,1),aBuffer2(k,1)) != 0.0) && flagnzz) nnz++;
				if ((ata2(aBuffer2(j,1),aBuffer2(k,1)) == 0.0) && !flagnzz) nnz--;
			}
		}
	}
	end = clock(); printf("... (%f) seconds\n",(end-start)/((float)CLOCKS_PER_SEC));
	
	// Write out output
	writeMat(atafile, ata2, nnz);
	writeMat(atbfile, atb2, nnz);
}

void PolyMesh::ReComputeMatrices(string maskfile,
								  string atafile,
								  string atbfile,
								  MatPointList& aLandmarks,
								  float         fitWt,
								  float         energyWt,
								  int           subLevel)
{
	// Load mask
	clock_t start, end;
	printf("Loading Mask and Matrices..."); start = clock();
	std::vector<SparseArray> maskb;
	maskb = readMaskFromFile(maskfile);
	
	int numvar = verts.size();
	matrix<double> ata2(numvar, numvar);
	matrix<double> atb2(3,numvar);
	int nnz = 0;
	atb2 = readMat(atbfile, nnz);
	ata2 = readMat(atafile, nnz);
	end = clock(); printf("... (%f) seconds\n",(end-start)/((float)CLOCKS_PER_SEC));
	
	//printf("Iteration %i\n", iterNum);
	printf("Allocating..."); start = clock();
		
	PolyMesh subMesh;
	subMesh.clone(*this);
	subMesh.SubdivideMesh(subLevel);
		
	MatPointList subMeshMatPoints;
	subMesh.GetCreaseMesh().ExtractMatPoints(subMeshMatPoints);
	end = clock(); printf("... (%f) seconds\n",(end-start)/((float)CLOCKS_PER_SEC));
	
	// Calculate closest mesh point to each landmark
	printf("Adding fitting terms..."); start = clock();
	for (int i = 0 ; i < aLandmarks.size(); ++i)
	{
		MatPoint& landmark = aLandmarks[i];
		Vec3 lm = aLandmarks[i].point;
		int ind = -1 ;
		float mindis2 = 1000000.0f*1000000.0f;
		for (size_t j = 0; j < subMeshMatPoints.size(); ++j)
		{
			MatPoint& pt = subMeshMatPoints[j];
			if (pt.materials.equals(landmark.materials))
			{
				int fi = j;
				float dis = (subMesh.verts[fi] - lm).length_squared();
				if ( dis < mindis2 )
				{
					mindis2 = dis ;
					ind = fi ;
				}
			}
		}
		
		// Store index array as in laplace code and add components to aTa and aTb
		if (ind >= 0) {
			// Add fitting term to aTa and aTb
			for (int j = 0 ; j < maskb[ind].size ; j++ ) {
				for (int k = 0 ; k < maskb[ind].size ; k++ ) {
					bool flagnzz = false;
					if (ata2(maskb[ind].idx[j],maskb[ind].idx[k])==0.0) flagnzz=true;
					
					ata2(maskb[ind].idx[j],maskb[ind].idx[k]) += maskb[ind].val[j]*maskb[ind].val[k]*fitWt*fitWt;
					
					if ((ata2(maskb[ind].idx[j],maskb[ind].idx[k])!=0.0) && flagnzz) nnz++;
					if ((ata2(maskb[ind].idx[j],maskb[ind].idx[k])==0.0) && !flagnzz) nnz--;
				}
				for (int k = 0 ; k < 3 ; ++k){
					atb2(k,maskb[ind].idx[j]) += maskb[ind].val[j]*lm[ k ]*fitWt*fitWt;
				}
			}
		}
	}
	end = clock(); printf("... (%f) seconds\n",(end-start)/((float)CLOCKS_PER_SEC));
	
	// Write out output
	writeMat(atafile, ata2, nnz);
	writeMat(atbfile, atb2, nnz);
}
	
void PolyMesh::SolveForMatrices(string atafile, string atbfile) {
	
	clock_t start, end;
	int numvar = verts.size();
	matrix<double> ata2(numvar, numvar);
	matrix<double> atb2(3,numvar);
	int nnz = 0;
	atb2 = readMat(atbfile, nnz);
	ata2 = readMat(atafile, nnz);
	
	// Solving
	printf("Solving...") ;
	start = clock();
	
	SuperMatrix A, L, U, B;
	double *a, *rhs;
	int *asub, *xa;
	int *perm_r;
	int *perm_c;
	int nrhs, info;
	superlu_options_t options;
	SuperLUStat_t stat;
	
	a = (double *) malloc ( nnz * sizeof( double ));
	asub = (int *) malloc ( nnz * sizeof( int ));
	xa = (int *) malloc ( (ata2.size2()+1) * sizeof( int ));
	
	int last_column_head = 0;
	int counter = 0;
	for (size_t i =0; i<ata2.size1();i++){
		xa[i] = last_column_head;
		for (size_t j =0; j<ata2.size2();j++){
			if (ata2(i,j) != 0.0) {
				a[counter] = ata2(i,j);
				asub[counter] = j;
				counter++;
				last_column_head = counter;
			}
		}
	}
	xa[ata2.size1()] = nnz;
	
	dCreate_CompCol_Matrix(&A, ata2.size1(), ata2.size2(), nnz, a, asub, xa, SLU_NC, SLU_D, SLU_GE);
	
	for (size_t j = 0 ; j < 3 ; j ++ ) {
		nrhs = 1;
		rhs = (double *) malloc ( ata2.size1() * sizeof( double ));
		for (size_t i = 0; i < ata2.size1(); ++i) {
			rhs[i] = atb2(j,i);
		}
		
		dCreate_Dense_Matrix(&B, ata2.size1(), nrhs, rhs, ata2.size1(), SLU_DN, SLU_D, SLU_GE);
		
		perm_r = (int *) malloc ( ata2.size1() * sizeof( int ));
		perm_c = (int *) malloc ( ata2.size2() * sizeof( int ));
		
		/* Set the default input options. */
		set_default_options(&options);
		options.ColPerm = NATURAL;
		/* Initialize the statistics variables. */
		StatInit(&stat);
		/* Solve the linear system. */
		dgssv(&options, &A, perm_c, perm_r, &L, &U, &B, &stat, &info);
		
		DNformat     *Astore = (DNformat *) B.Store;
		double       *solution2;
		solution2 = (double *) Astore->nzval;
		
		for (int i = 0 ; i < numvar ; i ++ ) {
			verts[i][j] = solution2[i];
		}
	}
	
	/* De-allocate storage */
	SUPERLU_FREE (rhs);
	SUPERLU_FREE (perm_r);
	SUPERLU_FREE (perm_c);
	Destroy_CompCol_Matrix(&A);
	Destroy_SuperMatrix_Store(&B);
	Destroy_SuperNode_Matrix(&L);
	Destroy_CompCol_Matrix(&U);
	StatFree(&stat);
	
	end = clock();
	printf("... (%f) seconds\n",(end-start)/((float)CLOCKS_PER_SEC));
}	

void PolyMesh::AlignLS2(MatPointList& aLandmarks,
						float         fitWt,
						float         energyWt,
						int           subLevel,
						int           iterations)
{
   // Subdivide mask
   printf("Subdividing Mask...") ;
   clock_t start, end;
   start = clock();
	
   std::vector<SparseArray> maskb;
   for (size_t i = 0; i < verts.size(); ++i)
   {
	   //cout << verts[i] << endl;
	  SparseArray temp = SparseArray(1);
	  temp.val[0] = 1.0;
	  temp.idx[0] = i;
	  maskb.push_back(temp);
   }

   PolyMesh subMaskMesh;
   subMaskMesh.clone(*this);
   subMaskMesh.SubdivideMask(subLevel, maskb);
   std::vector<TetPair> tetPairs;
   GetTetPairs(tetPairs);

   end = clock();
   printf("... (%f) seconds\n",(end-start)/((float)CLOCKS_PER_SEC));

	
	// Find closest point
   for (int iterNum = 0; iterNum < iterations; ++iterNum)
   {
      printf("Iteration %i\n", iterNum);
	  printf("Allocating...");
	  start = clock();

      PolyMesh subMesh;
      subMesh.clone(*this);
      subMesh.SubdivideMesh(subLevel);

      MatPointList subMeshMatPoints;
      subMesh.GetCreaseMesh().ExtractMatPoints(subMeshMatPoints);
      
	  // define empty aTa and aTb
	  int nnz = 0; // number of non-zero values
	  int numvar = verts.size();
	  matrix<double> ata2(numvar, numvar);
	  matrix<double> atb2(3,numvar);
	  for (int i = 0 ; i < numvar ; i++) {
		  for (int j = 0 ; j < numvar ; j++) {
			  ata2(i,j) = 0.0;
		  }
		  for (int j = 0 ; j < 3 ; j++) {
			  atb2(j,i) = 0.0;
		  }
	  }

	  end = clock();
	  printf("... (%f) seconds\n",(end-start)/((float)CLOCKS_PER_SEC));

	  printf("Adding fitting terms...");
	  start = clock();
	   
	  //cout << "what?" << endl;

	  for (int i = 0 ; i < aLandmarks.size(); ++i)
	  {
         MatPoint& landmark = aLandmarks[i];
         Vec3 lm = aLandmarks[i].point;
		  //cout << endl;
//		  cout << "landmark " << i << " (" << lm[0] << "," << lm[1] << "," << lm[2] << ")";
//		  for (int z=0; z<landmark.materials.size(); z++) {
//			  cout << landmark.materials[z] << " ";
//		  }
//		  cout << endl;
         int ind = -1 ;
         float mindis2 = 1000000.0f*1000000.0f;
		  //cout << subMeshMatPoints.size() << endl;
         for (size_t j = 0; j < subMeshMatPoints.size(); ++j)
         {
            MatPoint& pt = subMeshMatPoints[j];
			 //cout << "  " << j <<  ": (" << pt.point[0] << "," << pt.point[1] << "," << pt.point[2] << ")" << pt.materials[0] << " " << pt.materials[1]<< " " << pt.materials[2]<< " " << pt.materials[3] << endl;
            if (pt.materials.equals(landmark.materials))
            {
				//cout << "  " << j <<  ": (" << pt.point[0] << "," << pt.point[1] << "," << pt.point[2] << ")" << pt.materials[0] << " " << pt.materials[1]<< " " << pt.materials[2]<< " " << pt.materials[3] << endl;

               int fi = j;
               float dis = (subMesh.verts[fi] - lm).length_squared();
               if ( dis < mindis2 )
               {
                  mindis2 = dis ;
                  ind = fi ;
               }
            }
		 }
//		  MatPoint& pt = subMeshMatPoints[ind];
//		  cout << "  pick: " << ind <<  ": (" << pt.point[0] << "," << pt.point[1] << "," << pt.point[2] << ")";
//		  for (int z=0; z<pt.materials.size(); z++) {
//			  cout << pt.materials[z] << " ";
//		  }
//		  pt = subMeshMatPoints[i];
//		  cout << "  shoulda?: " << i <<  ": (" << pt.point[0] << "," << pt.point[1] << "," << pt.point[2] << ")";
//		  for (int z=0; z<pt.materials.size(); z++) {
//			  cout << pt.materials[z] << " ";
//		  }
//		  cout << endl;
		  //<< pt.materials[0] << " " << pt.materials[1]<< " " << pt.materials[2]<< " " << pt.materials[3] << endl;
		//cout << "pick: " << ind << endl;

		 // Store index array as in laplace code and add components to aTa and aTb
		 if (ind >= 0 && mindis2 <= 10.0) {

			// Add fitting term to aTa and aTb
			for (int j = 0 ; j < maskb[ind].size ; j++ ) {
				for (int k = 0 ; k < maskb[ind].size ; k++ ) {
					bool flagnzz = false;
					if (ata2(maskb[ind].idx[j],maskb[ind].idx[k])==0.0) flagnzz=true;

					ata2(maskb[ind].idx[j],maskb[ind].idx[k]) += maskb[ind].val[j]*maskb[ind].val[k]*fitWt*fitWt;

					if ((ata2(maskb[ind].idx[j],maskb[ind].idx[k])!=0.0) && flagnzz) nnz++;
					if ((ata2(maskb[ind].idx[j],maskb[ind].idx[k])==0.0) && !flagnzz) nnz--;
				}
				for (int k = 0 ; k < 3 ; ++k){
					atb2(k,maskb[ind].idx[j]) += maskb[ind].val[j]*lm[ k ]*fitWt*fitWt;
				}
			}

         }
	  }
	   
//	   cout << "------------------ AtA post fit term ----------------" << endl;
//	   //int cnnz = 0;
//	   for (int i = 0 ; i < numvar ; i++) {
//		   for (int j = 0 ; j < numvar ; j++) {
//			   //if (ata2(i,j) != 0.0) cnnz++;
//			   cout << ata2(i,j) << ",";
//		   }
//		   cout << endl;
//	   }
//	   cout << " ----- AtB ------ " << endl;
//	   for (int i = 0 ; i < numvar ; i++) {
//		   for (int j = 0 ; j < 3 ; j++) {
//			   cout << atb2(j,i) << ",";
//		   }
//		   cout << endl;
//	   }

	  //for (int i = 0 ; i < subMeshMatPoints.size(); i++)
	  //{
		 //int ind = -1 ;
   //      float mindis2 = 1000000.0f*1000000.0f;

		 //MatPoint& pt = subMeshMatPoints[i];
		 //for (int j = 0 ; j < aLandmarks.size(); ++j)
   //      {
			//MatPoint& landmark = aLandmarks[j];
   //         if (pt.materials.equals(landmark.materials))
   //         {
   //            float dis = (pt.point - landmark.point).length_squared();
   //            if ( dis < mindis2 )
   //            {
   //               mindis2 = dis ;
   //               ind = j;
   //            }
   //         }
		 //}

		 //// Store index array as in laplace code and add components to aTa and aTb
		 //if (ind >= 0) {

			//// Add fitting term to aTa and aTb
			//for (int j = 0 ; j < maskb[i].size ; j++ ) {
			//	for (int k = 0 ; k < maskb[i].size ; k++ ) {
			//		bool flagnzz = false;
			//		if (ata2(maskb[i].idx[j],maskb[i].idx[k])==0.0) flagnzz=true;

			//		ata2(maskb[i].idx[j],maskb[i].idx[k]) += maskb[i].val[j]*maskb[i].val[k]*fitWt*fitWt;

			//		if ((ata2(maskb[i].idx[j],maskb[i].idx[k])!=0.0) && flagnzz) nnz++;
			//		if ((ata2(maskb[i].idx[j],maskb[i].idx[k])==0.0) && !flagnzz) nnz--;
			//	}
			//	for (int k = 0 ; k < 3 ; ++k){
			//		atb2(k,maskb[i].idx[j]) += maskb[i].val[j]*aLandmarks[ind].point[k]*fitWt*fitWt;
			//	}
			//}

   //      }
	  //}

	  //// Add Laplacian surface deformation term to aTa and aTb
//	  for (DVertexList::iterator i = dMesh.begin_v(); i != dMesh.end_v(); ++i){
//		  int ind = i->aux;
//
//		  SparseArray aTemp = maskb[ind];
//
//		  DFaceArray aFaces;
//		  i->get_faces(aFaces);
//		  std::set<int> aMaterials;
//		  for(size_t j=0; j<aFaces.size(); ++j){
//			  aMaterials.insert(aFaces[j]->materials[0]);
//			  aMaterials.insert(aFaces[j]->materials[1]);
//		  }
//		  //int size1 = aMaterials.size();
//		  //for(set<int>::iterator n = aMaterials.begin();  n!= aMaterials.end(); n++){
//		  //int mat = *n;
//		  if(!aMaterials.empty()){
//			  int mat = *aMaterials.begin();
//			  int sz2 = 0;
//			  float meanVert[3] = {0.0,0.0,0.0};
//
//			  DVertexArray aVertices;
//			  i->get_adjacent_vertices(aVertices);
//			  for(size_t k=0;k<aVertices.size();k++){
//				  if (aVertices.at(k)->has_material(mat)){
//					  sz2++;
//				  }
//			  }
//			  for(size_t k=0;k<aVertices.size();k++){
//				  if (aVertices.at(k)->has_material(mat)){
//					  int ind2 = aVertices.at(k)->aux;
//					  meanVert[0] += aVertices.at(k)->v[0];
//					  meanVert[1] += aVertices.at(k)->v[1];
//					  meanVert[2] += aVertices.at(k)->v[2];
//
//					  aTemp = aTemp - maskb[ind2]/sz2;
//				  }
//			  }
//			  for (size_t u=0; u<aTemp.size; ++u) {
//				  for (size_t v=0; v<aTemp.size; ++v){
//					  bool flagnzz = false;
//					  if (ata2(aTemp.idx[u],aTemp.idx[v])== 0.0) flagnzz = true;
//
//					  ata2(aTemp.idx[u],aTemp.idx[v]) += aTemp.val[u]*aTemp.val[v];
//
//					  if ((ata2(aTemp.idx[u],aTemp.idx[v])!=0.0) && flagnzz) nnz++;
//					  if ((ata2(aTemp.idx[u],aTemp.idx[v])==0.0) && !flagnzz) nnz--;
//				  }
//				  for (size_t k=0; k<3; ++k){
//					  atb2(k,aTemp.idx[u]) += (i->v[k]-meanVert[k]/sz2)*aTemp.val[u];
//				  }
//			  }
//		  }
//	  }

	  end = clock();
	  printf("... (%f) seconds\n",(end-start)/((float)CLOCKS_PER_SEC));

      // Add energy terms
      printf("Adding energy terms...") ;
	  start = clock();

      // Find all interior faces
      for (int i = 0 ; i < tetPairs.size() ; i ++ )
	  {
         TetPair& p = tetPairs[i];

         // Get relevant vertices
         int ip1 = p.tetTips[0];
         int ip2 = p.tetTips[1];
         int im1 = p.verts[0];
         int im2 = p.verts[1];
         int im3  = p.verts[2];
         Vec3 p1 = verts[ip1];
         Vec3 p2 = verts[ip2];
         Vec3 m1 = verts[im1];
         Vec3 m2 = verts[im2];
         Vec3 m3 = verts[im3];

         // Get different volumes
         float vol1 = getVolume( m3,m2,m1,p2 ) ;
         float vol2 = getVolume( m1,m2,m3,p1 ) ;
         float vol3 = getVolume( p1,m3,m2,p2 ) ;
         float vol4 = getVolume( p1,m1,m3,p2 ) ;
         float vol5 = getVolume( p1,m2,m1,p2 ) ;

         float normvol = vol1 + vol2 ;
		 //float power1 = 4.0/3.0;
		 //float power2 = 3.0/2.0;
		 //float normvol5 = 1;
		 //float normvol4 = pow(abs(normvol),power2);
		 //float normvol3 = pow(abs(normvol),power1);
		 float normvol2 = abs(normvol);
		 //float normvol1 = normvol*normvol;

		 normvol = normvol2;
		 matrix<double> aBuffer2(5,2);

		aBuffer2(0,0) = vol1 / normvol * energyWt ;
		aBuffer2(0,1) = ip1;
		aBuffer2(1,0) =  vol2 / normvol * energyWt ;
		aBuffer2(1,1) = ip2;
		aBuffer2(2,0) = -vol3 / normvol * energyWt ;
		aBuffer2(2,1) = im1;
		aBuffer2(3,0) = -vol4 / normvol * energyWt ;
		aBuffer2(3,1) = im2;
		aBuffer2(4,0) = -vol5 / normvol * energyWt ;
		aBuffer2(4,1) = im3;

		for (int j = 0 ; j < 5 ; j++ ) {
			for (int k = 0 ; k < 5 ; k++ ) {
				bool flagnzz = false;
				if (ata2(aBuffer2(j,1),aBuffer2(k,1)) == 0.0) flagnzz = true;

				ata2(aBuffer2(j,1),aBuffer2(k,1)) += aBuffer2(j,0)*aBuffer2(k,0)*energyWt*energyWt;

				if ((ata2(aBuffer2(j,1),aBuffer2(k,1)) != 0.0) && flagnzz) nnz++;
				if ((ata2(aBuffer2(j,1),aBuffer2(k,1)) == 0.0) && !flagnzz) nnz--;
			}
		}
      }
//	  cout << "energy weight " << energyWt << endl;
//	  cout << " Nnz number of non-zeros: " << nnz << endl;
//	  cout << "------------------ AtA post energy term ----------------" << endl;
//	  int cnnz = 0;
//	  for (int i = 0 ; i < numvar ; i++) {
//		  for (int j = 0 ; j < numvar ; j++) {
//			  if (ata2(i,j) != 0.0) cnnz++;
//			  cout << ata2(i,j) << ",";
//		  }
//		  cout << endl;
//	  }
//	  cout << "actual number of non-zeros: " << cnnz << endl;
//	  nnz = cnnz;
//	   cout << "------------------ AtA post energy term ----------------" << endl;
//	   //int cnnz = 0;
//	   for (int i = 0 ; i < numvar ; i++) {
//		   for (int j = 0 ; j < numvar ; j++) {
//			   //if (ata2(i,j) != 0.0) cnnz++;
//			   cout << ata2(i,j) << ",";
//		   }
//		   cout << endl;
//	   }
//	   cout << " ----- AtB ------ " << endl;
//	   for (int i = 0 ; i < numvar ; i++) {
//		   for (int j = 0 ; j < 3 ; ++j){
//			   cout << atb2(j,i) << ",";
//		   }
//		   cout << endl;
//	   }
	  end = clock();
	  printf("... (%f) seconds\n",(end-start)/((float)CLOCKS_PER_SEC));


	   // test: printing Ata and Atb
	   int new_nnz;
	   writeMat("/Users/atsteffen/mat_write_test/test_ata.txt",ata2,nnz);
	   matrix<double> test_ata1 = readMat("/Users/atsteffen/mat_write_test/test_ata.txt",new_nnz);
	   writeMat("/Users/atsteffen/mat_write_test/test_ata2.txt",test_ata1,nnz);
	   
	   writeMat("/Users/atsteffen/mat_write_test/test_atb.txt",atb2,nnz);
	   matrix<double> test_atb1 = readMat("/Users/atsteffen/mat_write_test/test_atb.txt",new_nnz);
	   writeMat("/Users/atsteffen/mat_write_test/test_atb2.txt",test_atb1,nnz);
	   
      // Solving
      printf("Solving...") ;
	  start = clock();

	  SuperMatrix A, L, U, B;
	  double *a, *rhs;
	  int *asub, *xa;
	  int *perm_r;
	  int *perm_c;
	  int nrhs, info;
	  superlu_options_t options;
	  SuperLUStat_t stat;
	
	  a = (double *) malloc ( nnz * sizeof( double ));
	  asub = (int *) malloc ( nnz * sizeof( int ));
	  xa = (int *) malloc ( (ata2.size2()+1) * sizeof( int ));

	  int last_column_head = 0;
	  int counter = 0;
	  for (size_t i =0; i<ata2.size1();i++){
		  xa[i] = last_column_head;
		  for (size_t j =0; j<ata2.size2();j++){
			  if (ata2(i,j) != 0.0) {
				  a[counter] = ata2(i,j);
				  asub[counter] = j;
				  counter++;
				  last_column_head = counter;
			  }
		  }
	  }
	  xa[ata2.size1()] = nnz;

	  dCreate_CompCol_Matrix(&A, ata2.size1(), ata2.size2(), nnz, a, asub, xa, SLU_NC, SLU_D, SLU_GE);

	  for (size_t j = 0 ; j < 3 ; j ++ ) {
		  nrhs = 1;
		  rhs = (double *) malloc ( ata2.size1() * sizeof( double ));
		  for (size_t i = 0; i < ata2.size1(); ++i) {
			  rhs[i] = atb2(j,i);
		  }
		  
		  dCreate_Dense_Matrix(&B, ata2.size1(), nrhs, rhs, ata2.size1(), SLU_DN, SLU_D, SLU_GE);
		  
		  perm_r = (int *) malloc ( ata2.size1() * sizeof( int ));
		  perm_c = (int *) malloc ( ata2.size2() * sizeof( int ));
		  
		  /* Set the default input options. */
		  set_default_options(&options);
		  options.ColPerm = NATURAL;
		  /* Initialize the statistics variables. */
		  StatInit(&stat);
		  /* Solve the linear system. */
		  dgssv(&options, &A, perm_c, perm_r, &L, &U, &B, &stat, &info);
		  
		  DNformat     *Astore = (DNformat *) B.Store;
		  double       *solution2;
		  solution2 = (double *) Astore->nzval;

		  for (int i = 0 ; i < numvar ; i ++ ) {
			  verts[i][j] = solution2[i];
		  }
	  }

	  /* De-allocate storage */
	  SUPERLU_FREE (rhs);
	  SUPERLU_FREE (perm_r);
	  SUPERLU_FREE (perm_c);
	  Destroy_CompCol_Matrix(&A);
	  Destroy_SuperMatrix_Store(&B);
	  Destroy_SuperNode_Matrix(&L);
	  Destroy_CompCol_Matrix(&U);
	  StatFree(&stat);

	  end = clock();
	  printf("... (%f) seconds\n",(end-start)/((float)CLOCKS_PER_SEC));
   }

}
