#include <iostream>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "PolyMesh.hpp"
#include "SparseArray.hpp"

namespace
{
using namespace std;

/* A hash table hashed by two int integers
 *
 */

const int MAX_HASH = 1<<16;
const int HASH_LENGTH = 16;
const int HASH_BIT_1 = 8;
const int HASH_BIT_2 = 8;

struct HashElement
{
/// Key of hash element
   int key[2];
/// Actually content of hash element
   int index ;
/// Link when collision
   HashElement  *nextHash;
};

class HashMap
{
   /// Hash table
   HashElement *table[MAX_HASH];

   /// Create hash key
   int createKey( int k1, int k2 )
   {
      int ind = (
					(
						(k1 & ( (1<<HASH_BIT_1) - 1 ))<< ( HASH_BIT_2  )
					) | ( 
						k2 & ( (1<<HASH_BIT_2) - 1)
					) 
				) & (
					(1<<HASH_LENGTH) - 1
				);

      return ind;
   }

public:

   /// Constructor
   HashMap ( )
   {
      for (int i = 0; i < MAX_HASH; i ++)
      {
         table[i] = NULL;
      }
   };

   /// Lookup Method
   int findInsert( int k1, int k2, int index )
   {
      /// Create hash key
      int ind = createKey ( k1, k2 );

      /// Find it in the table
      HashElement *p = table[ind];
	  //int count = 0;
      while (p)
      {
         if ((p->key[0] == k1) && (p->key[1] == k2))
         {
            return p->index ;
         }
         p = p->nextHash;
      }

      // Not found
      p = new HashElement ;
      p->key[0] = k1;
      p->key[1] = k2;
      p->index = index ;
      p->nextHash = table[ind];
      table[ind] = p;

      return index ;
   };


   int findInsertSort( int k1, int k2, int index )
   {
      /// sort by index
      if ( k1 > k2 )
      {
         int temp = k1 ;
         k1 = k2 ; 
         k2 = temp ;
      }

	  return findInsert(k1,k2,index);
   };

   // Destruction method
   ~HashMap()
   {
      HashElement *p, *pp;

      for (int i = 0; i < MAX_HASH; i ++)
      {
         p = table[i];

         while (p)
         {
            pp = p->nextHash;
            delete p;
            p = pp;
         }

      }

   };

};

} 


//#define SUBDIVIDE_NO_DEL_INPUT

void subdivide(
int& nVerts1, float*& vertl1, 
int& nCrVerts1, int*& crvertl1, 
int& nCrEdges1, int*& credgel1, 
int& nTris1, int*& tril1, 
int& nTets1, int*& tetl1, 
int& nOcts1, int*& octl1, 
int*& tetmaskl1, int*& octmaskl1, 
int*& faceMaskl1,
int comp) 
{	
   //clock_t start, end;
   int i, j ;
   int trimap[3][2]={{0,1},{0,2},{1,2}};
   int tetmap[6][2]={{0,1},{0,2},{0,3},{1,2},{1,3},{2,3}};
   int octmap[12][2]={{0,1},{1,2},{0,2},{3,4},{4,5},{3,5},{0,3},{1,3},{1,5},{2,5},{2,4},{0,4}};

   int nVerts, nCrVerts, nCrEdges, nTris, nTets, nOcts ;

   /* First, determine array size */
   nVerts = nVerts1 ;
   HashMap* hash = new HashMap() ;
   // divide crease edges in half
   for ( i = 0 ; i < nCrEdges1 ; i ++ )
   {
      int *v = &(credgel1[ 2 * i ]) ;

      if ( hash->findInsertSort( v[0], v[1], nVerts ) == nVerts )
      {
         nVerts ++ ;
      }
   }
   // divide each side of triangle
   for ( i = 0 ; i < nTris1 ; i ++ )
   {
      int *v = &(tril1[ 3 * i ]) ;

      for ( j = 0 ; j < 3 ; j ++ )
      {
         if ( hash->findInsertSort( v[trimap[j][0]], v[trimap[j][1]], nVerts ) == nVerts )
         {
            nVerts ++ ;
         }
      }
   }
   // divide each tet side
   for ( i = 0 ; i < nTets1 ; i ++ )
   {
      int *v = &(tetl1[ 4 * i ]) ;

      for ( j = 0 ; j < 6 ; j ++ )
      {
         if ( hash->findInsertSort( v[tetmap[j][0]], v[tetmap[j][1]], nVerts ) == nVerts )
         {
            nVerts ++ ;
         }
      }
   }
   // divide each oct side
   for ( i = 0 ; i < nOcts1 ; i ++ )
   {
      int *v = &(octl1[ 6 * i ]) ;

      for ( j = 0 ; j < 12 ; j ++ )
      {
         if ( hash->findInsertSort( v[octmap[j][0]], v[octmap[j][1]], nVerts ) == nVerts )
         {
            nVerts ++ ;
         }
      }
   }
   nVerts += nOcts1 ;  // point added to the middle of each oct
   nCrVerts = nCrVerts1 ;
   nCrEdges = nCrEdges1 * 2 ;
   nTris = nTris1 * 4 ;
   nTets = nTets1 * 4 + nOcts1 * 8 ;
   nOcts = nTets1 + nOcts1 * 6 ;

   // allocate new arrays with new sizes
   float* vertl = new float[ nVerts * comp ] ;
   for ( i = 0 ; i < nVerts1 * comp ; i ++ )
   {
      vertl[ i ] = vertl1[ i ] ;
   }
   int* crvertl = new int[ nCrVerts ] ;
   int* credgel = new int[ nCrEdges * 2 ] ;
   int* tril = new int[ nTris * 3 ] ;
   int* tetl = new int[ nTets * 4 ] ;
   int* octl = new int[ nOcts * 6 ] ;
   int* tetmaskl = new int[ nTets ] ;
   int* octmaskl = new int[ nOcts ] ;
   int* facemaskl = new int[ nTris * 2 ] ;

   /* Linear subdivision */
   delete hash ;
   hash = new HashMap( ) ;
   int curvert = nVerts1 ;
   
   // Crease points stay
   for ( i = 0 ; i < nCrVerts1 ; i ++ )
   {
      crvertl[ i ] = crvertl1[ i ] ;
   }
   // Crease edge splits
   int cur = 0 ;
   for ( i = 0 ; i < nCrEdges1 ; i ++ )
   {
      int *v = &(credgel1[ 2 * i ]) ;
      int ind = hash->findInsertSort( v[0], v[1], curvert ) ;

      if ( ind == curvert )
      {
         for ( j = 0 ; j < comp ; j ++ )
         {
            vertl[ comp * curvert + j ] = ( vertl[ comp * v[0] + j ] + vertl[ comp * v[1] + j ] ) / 2 ;
         }
         curvert ++ ;
      }

	  // create new edges from split edge
      credgel[ 2 * cur ] = credgel1[ 2 * i ] ; 
      credgel[ 2 * cur + 1 ] = ind ; 
      cur ++ ;
      credgel[ 2 * cur ] = ind ;
      credgel[ 2 * cur + 1 ] = credgel1[ 2 * i + 1 ] ; 
      cur ++ ;
   }
   // Face splits
   cur = 0 ;
   for ( i = 0 ; i < nTris1 ; i ++ )
   {
      int m1 = faceMaskl1[ i * 2 ] ;
      int m2 = faceMaskl1[ i * 2 + 1] ;
      int *v = &(tril1[ 3 * i ]) ;
      int ind[3] ;

      for ( j = 0 ; j < 3 ; j ++ )
      {
         ind[ j ] = hash->findInsertSort( v[trimap[j][0]], v[trimap[j][1]], curvert ) ;
         if ( ind[ j ] == curvert )
         {
            for ( int k = 0 ; k < comp ; k ++ )
            {
               vertl[ comp * curvert + k ] = ( vertl[ comp * v[trimap[j][0]] + k ] + vertl[ comp * v[trimap[j][1]] + k ] ) / 2 ;
            }
            curvert ++ ;
         }
      }

      tril[ 3 * cur ] = tril1[ 3 * i ] ; 
      tril[ 3 * cur + 1 ] = ind[ 0 ] ; 
      tril[ 3 * cur + 2 ] = ind[ 1 ] ; 
      facemaskl[ cur * 2 ] = m1 ;
      facemaskl[ cur * 2 + 1 ] = m2 ;
      cur ++ ;
      tril[ 3 * cur ] = tril1[ 3 * i + 1 ] ; 
      tril[ 3 * cur + 1 ] = ind[ 2 ] ; 
      tril[ 3 * cur + 2 ] = ind[ 0 ] ; 
      facemaskl[ cur * 2 ] = m1 ;
      facemaskl[ cur * 2 + 1 ] = m2 ;
      cur ++ ;
      tril[ 3 * cur ] = tril1[ 3 * i + 2 ] ; 
      tril[ 3 * cur + 1 ] = ind[ 1 ] ; 
      tril[ 3 * cur + 2 ] = ind[ 2 ] ;
      facemaskl[ cur * 2 ] = m1 ;
      facemaskl[ cur * 2 + 1 ] = m2 ; 
      cur ++ ;
      tril[ 3 * cur ] = ind[ 0 ] ; 
      tril[ 3 * cur + 1 ] = ind[ 2 ] ; 
      tril[ 3 * cur + 2 ] = ind[ 1 ] ; 
      facemaskl[ cur * 2 ] = m1 ;
      facemaskl[ cur * 2 + 1 ] = m2 ;
      cur ++ ;
   }
   // Tet splits
   int curtet = 0, curoct = 0 ;
   for ( i = 0 ; i < nTets1 ; i ++ )
   {
      int mk = tetmaskl1[ i ] ;
      int *v = &(tetl1[ 4 * i ]) ;
      int ind[6] ;

      for ( j = 0 ; j < 6 ; j ++ )
      {
         ind[ j ] = hash->findInsertSort( v[tetmap[j][0]], v[tetmap[j][1]], curvert ) ;
         if ( ind[ j ] == curvert )
         {
            for ( int k = 0 ; k < comp ; k ++ )
            {
               vertl[ comp * curvert + k ] = ( vertl[ comp * v[tetmap[j][0]] + k ] + vertl[ comp * v[tetmap[j][1]] + k ] ) / 2 ;
            }
            curvert ++ ;
         }
      }

      tetl[ 4 * curtet ] = tetl1[ 4 * i ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 0 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 1 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 2 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;
      tetl[ 4 * curtet ] = tetl1[ 4 * i + 1 ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 3 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 0 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 4 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;
      tetl[ 4 * curtet ] = tetl1[ 4 * i + 2 ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 1 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 3 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 5 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;
      tetl[ 4 * curtet ] = tetl1[ 4 * i + 3 ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 2 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 5 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 4 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;

      octl[ 6 * curoct ] = ind[ 0 ] ;
      octl[ 6 * curoct + 1 ] = ind[ 1 ] ;
      octl[ 6 * curoct + 2 ] = ind[ 2 ] ;
      octl[ 6 * curoct + 3 ] = ind[ 3 ] ;
      octl[ 6 * curoct + 4 ] = ind[ 4 ] ;
      octl[ 6 * curoct + 5 ] = ind[ 5 ] ;
      octmaskl[ curoct ] = mk ;
      curoct ++ ;
   }
   // Oct splits
   for ( i = 0 ; i < nOcts1 ; i ++ )
   {
      int mk = octmaskl1[ i ] ;
      int *v = &(octl1[ 6 * i ]) ;
      int ind[13] ;

      for ( j = 0 ; j < 12 ; j ++ )
      {
         ind[ j ] = hash->findInsertSort( v[octmap[j][0]], v[octmap[j][1]], curvert ) ;
         if ( ind[ j ] == curvert )
         {
            for ( int k = 0 ; k < comp ; k ++ )
            {
               vertl[ comp * curvert + k ] = ( vertl[ comp * v[octmap[j][0]] + k ] + vertl[ comp * v[octmap[j][1]] + k ] ) / 2 ;
            }
            curvert ++ ;
         }
      }
      ind[12] = curvert ;
      for ( int k = 0 ; k < comp ; k ++ )
      {
         float sum = 0 ;
         for ( j = 0 ; j < 6 ; j ++ )
         {
            sum += vertl[ comp * octl1[ 6 * i + j ] + k ] ;
         }

         vertl[ comp * curvert + k ] = sum / 6 ;
      }
      curvert ++ ;

      tetl[ 4 * curtet ] = ind[ 0 ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 1 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 2 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 12 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;
      tetl[ 4 * curtet ] = ind[ 0 ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 6 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 7 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 12 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;
      tetl[ 4 * curtet ] = ind[ 1 ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 8 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 9 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 12 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;
      tetl[ 4 * curtet ] = ind[ 2 ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 10 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 11 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 12 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;
      tetl[ 4 * curtet ] = ind[ 3 ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 4 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 5 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 12 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;
      tetl[ 4 * curtet ] = ind[ 3 ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 6 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 11 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 12 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;
      tetl[ 4 * curtet ] = ind[ 4 ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 10 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 9 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 12 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;
      tetl[ 4 * curtet ] = ind[ 5 ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 8 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 7 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 12 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;

      octl[ 6 * curoct ] = octl1[ 6 * i ] ;
      octl[ 6 * curoct + 1 ] = ind[ 0 ] ;
      octl[ 6 * curoct + 2 ] = ind[ 2 ] ;
      octl[ 6 * curoct + 3 ] = ind[ 6 ] ;
      octl[ 6 * curoct + 4 ] = ind[ 11 ] ;
      octl[ 6 * curoct + 5 ] = ind[ 12 ] ;
      octmaskl[ curoct ] = mk ;
      curoct ++ ;
      octl[ 6 * curoct ] = octl1[ 6 * i + 1 ] ;
      octl[ 6 * curoct + 1 ] = ind[ 1 ] ;
      octl[ 6 * curoct + 2 ] = ind[ 0 ] ;
      octl[ 6 * curoct + 3 ] = ind[ 8 ] ;
      octl[ 6 * curoct + 4 ] = ind[ 7 ] ;
      octl[ 6 * curoct + 5 ] = ind[ 12 ] ;
      octmaskl[ curoct ] = mk ;
      curoct ++ ;
      octl[ 6 * curoct ] = octl1[ 6 * i + 2 ] ;
      octl[ 6 * curoct + 1 ] = ind[ 2 ] ;
      octl[ 6 * curoct + 2 ] = ind[ 1 ] ;
      octl[ 6 * curoct + 3 ] = ind[ 10 ] ;
      octl[ 6 * curoct + 4 ] = ind[ 9 ] ;
      octl[ 6 * curoct + 5 ] = ind[ 12 ] ;
      octmaskl[ curoct ] = mk ;
      curoct ++ ;
      octl[ 6 * curoct ] = octl1[ 6 * i + 3 ] ;
      octl[ 6 * curoct + 1 ] = ind[ 3 ] ;
      octl[ 6 * curoct + 2 ] = ind[ 5 ] ;
      octl[ 6 * curoct + 3 ] = ind[ 6 ] ;
      octl[ 6 * curoct + 4 ] = ind[ 7 ] ;
      octl[ 6 * curoct + 5 ] = ind[ 12 ] ;
      octmaskl[ curoct ] = mk ;
      curoct ++ ;
      octl[ 6 * curoct ] = octl1[ 6 * i + 4 ] ;
      octl[ 6 * curoct + 1 ] = ind[ 4 ] ;
      octl[ 6 * curoct + 2 ] = ind[ 3 ] ;
      octl[ 6 * curoct + 3 ] = ind[ 10 ] ;
      octl[ 6 * curoct + 4 ] = ind[ 11 ] ;
      octl[ 6 * curoct + 5 ] = ind[ 12 ] ;
      octmaskl[ curoct ] = mk ;
      curoct ++ ;
      octl[ 6 * curoct ] = octl1[ 6 * i + 5 ] ;
      octl[ 6 * curoct + 1 ] = ind[ 5 ] ;
      octl[ 6 * curoct + 2 ] = ind[ 4 ] ;
      octl[ 6 * curoct + 3 ] = ind[ 8 ] ;
      octl[ 6 * curoct + 4 ] = ind[ 9 ] ;
      octl[ 6 * curoct + 5 ] = ind[ 12 ] ;
      octmaskl[ curoct ] = mk ;
      curoct ++ ;
   }
   
   /* Prepare dimensions and valences */
   int *dim = new int[ nVerts ] ;
   int *val = new int[ nVerts ] ;
   for ( i = 0 ; i < nVerts ; i ++ )
   {
      dim[ i ] = 3 ;
      val[ i ] = 0 ;
   }

   for ( i = 0 ; i < nCrVerts ; i ++ )
   {
      dim[ crvertl[i] ] = 0 ;
      val[ crvertl[i] ] = 1 ;
   }
   for ( i = 0 ; i < nCrEdges * 2 ; i ++ )
   {
      int d = dim[ credgel[i] ] ;
      if ( d > 1 )
      {
         dim[ credgel[i] ] = 1 ;
         val[ credgel[i] ] = 1 ;
      }
      else if ( d == 1 )
      {
         val[ credgel[i] ] ++ ;
      }
   }
   for ( i = 0 ; i < nTris * 3 ; i ++ )
   {
      int d = dim[ tril[i] ] ;
      if ( d > 2 )
      {
         dim[ tril[i] ] = 2 ;
         val[ tril[i] ] = 1 ;
      }
      else if ( d == 2 )
      {
         val[ tril[i] ] ++ ;
      }
   }
   for ( i = 0 ; i < nTets * 4 ; i ++ )
   {
      if ( dim[ tetl[i] ] == 3 )
      {
         val[ tetl[i] ] ++ ;
      }
   }
   for ( i = 0 ; i < nOcts * 6 ; i ++ )
   {
      if ( dim[ octl[i] ] == 3 )
      {
         val[ octl[i] ] ++ ;
      }
   }

   /* Averaging */
   float* nvertl = new float[ nVerts * comp ] ;
   for ( i = 0 ; i < nVerts * comp ; i ++ )
   {
      nvertl[i] = 0 ;
   }
   float* cent = new float[comp] ;
   
   // Points stay
   for ( i = 0 ; i < nCrVerts ; i ++ )
   {
      for ( j = 0 ; j < comp ; j ++ )
      {
         nvertl[ comp * crvertl[i] + j ] = vertl[ comp * crvertl[i] + j ] ;
      }
   }
   // Edge: midpoint
   for ( i = 0 ; i < nCrEdges ; i ++ )
   {
      int *ind = &( credgel[ 2 * i ] ) ;

      for ( int k = 0 ; k < comp ; k ++ )
      {
         cent[k] = ( vertl[ comp * ind[ 0 ] + k ] + vertl[ comp * ind[ 1 ] + k ] ) / 2 ;
      }

      if ( dim[ind[0]] == 1 )
      {
         for (int k = 0 ; k < comp ; k ++ )
         {
            nvertl[ comp * ind[ 0 ] + k ] += cent[k] / val[ ind[ 0 ] ] ;
         }
      }
      if ( dim[ind[1]] == 1 )
      {
         for (int k = 0 ; k < comp ; k ++ )
         {
            nvertl[ comp * ind[ 1 ] + k ] += cent[k] / val[ ind[ 1 ] ] ;
         }
      }
   }
   // Triangle: Loop
   float PI = 3.14159265f ;
   for ( i = 0 ; i < nTris ; i ++ )
   {
      int *ind = &( tril[ 3 * i ] ) ;

      // j: index of vertex to average
      for ( j = 0 ; j < 3 ; j ++ )
      {
         if ( dim[ind[j]] != 2 )
         {
            continue ;
         }
         float w = 3.0f/8.0f + cos( 2 * PI / val[ind[j]] )/4;
         w = 5.0f/8.0f - w * w ;
         float ws[3] ;
         ws[0] = w ; ws[1] = w ; ws[2] = w ;
         ws[j] = 1 - 2 * w ;

         // k: x,y,z component
         for ( int k = 0 ; k < comp ; k ++ )
         {
            cent[k] = 0 ;
            // l: index of each vertex
            for ( int l = 0 ; l < 3 ; l ++ )
            {
               cent[k] += vertl[ comp * ind[l] + k ] * ws[l];
            }

            nvertl[ comp * ind[j] + k ] += cent[k]/val[ind[j]] ;
         }
      }
   }
   // Tets: smooth
   float tetmask[4][4]={
      {-1.0f/16,17.0f/48,17.0f/48,17.0f/48},
      {17.0f/48,-1.0f/16,17.0f/48,17.0f/48},
      {17.0f/48,17.0f/48,-1.0f/16,17.0f/48},
      {17.0f/48,17.0f/48,17.0f/48,-1.0f/16}};
   for ( i = 0 ; i < nTets ; i ++ )
   {
      int *ind = &( tetl[ 4 * i ] ) ;

      // j: index of vertex to average
      for ( j = 0 ; j < 4 ; j ++ )
      {
         if ( dim[ind[j]] != 3 )
         {
            continue ;
         }

         // k: x,y,z component
         for ( int k = 0 ; k < comp ; k ++ )
         {
            cent[k] = 0 ;
            // l: index of each vertex
            for ( int l = 0 ; l < 4 ; l ++ )
            {
               cent[k] += tetmask[j][l] * vertl[ comp * ind[l] + k ] ;
            }

            nvertl[ comp * ind[j] + k ] += cent[k]/val[ind[j]] ;
         }
      }
   }
   // Octs: smooth
   float octmask[6][6]={
      {3.0f/8,1.0f/12,1.0f/12,1.0f/12,1.0f/12,7.0f/24},
      {1.0f/12,3.0f/8,1.0f/12,1.0f/12,7.0f/24,1.0f/12},
      {1.0f/12,1.0f/12,3.0f/8,7.0f/24,1.0f/12,1.0f/12},
      {1.0f/12,1.0f/12,7.0f/24,3.0f/8,1.0f/12,1.0f/12},
      {1.0f/12,7.0f/24,1.0f/12,1.0f/12,3.0f/8,1.0f/12},
      {7.0f/24,1.0f/12,1.0f/12,1.0f/12,1.0f/12,3.0f/8}};
   for ( i = 0 ; i < nOcts ; i ++ )
   {
      int *ind = &( octl[ 6 * i ] ) ;

      // j: index of vertex to average
      for ( j = 0 ; j < 6 ; j ++ )
      {
         if ( dim[ind[j]] != 3 )
         {
            continue ;
         }

         // k: x,y,z component
         for ( int k = 0 ; k < comp ; k ++ )
         {
            cent[k] = 0 ;
            // l: index of each vertex
            for ( int l = 0 ; l < 6 ; l ++ )
            {
               cent[k] += octmask[j][l] * vertl[ comp * ind[l] + k ] ;
            }

            nvertl[ comp * ind[j] + k ] += cent[k]/val[ind[j]] ;
         }
      }
   }

   delete vertl ;
   vertl = nvertl ;
   
   delete cent ;
   
   
   delete hash ;
   delete dim ;
   delete val ;
//#ifndef SUBDIVIDE_NO_DEL_INPUT               
//   delete vertl1 ;
//   delete crvertl1 ;
//   delete credgel1 ;
//   delete tril1 ;
//   delete tetl1 ;
//   delete octl1 ;
//   delete tetmaskl1 ;
//   delete octmaskl1 ;
//   delete faceMaskl1 ;
//#endif
   
   vertl1 = vertl ;
   crvertl1 = crvertl ;
   credgel1 = credgel ;
   tril1 = tril ;
   tetl1 = tetl ;
   octl1 = octl ;
   tetmaskl1 = tetmaskl ;
   octmaskl1 = octmaskl ;
   faceMaskl1 = facemaskl ;
   
   nVerts1 = nVerts ;
   nCrVerts1 = nCrVerts ;
   nCrEdges1 = nCrEdges ;
   nTris1 = nTris ;
   nTets1 = nTets ;
   nOcts1 = nOcts ;
}
   
void subdivide2(
int& nVerts1, std::vector<SparseArray>& maskb, 
int& nCrVerts1, int*& crvertl1, 
int& nCrEdges1, int*& credgel1, 
int& nTris1, int*& tril1, 
int& nTets1, int*& tetl1, 
int& nOcts1, int*& octl1, 
int*& tetmaskl1, int*& octmaskl1, 
int*& faceMaskl1) 
{	
	//clock_t start,end;
	//start = clock();
   int i, j ;
   int trimap[3][2]={{0,1},{0,2},{1,2}};
   int tetmap[6][2]={{0,1},{0,2},{0,3},{1,2},{1,3},{2,3}};
   int octmap[12][2]={{0,1},{1,2},{0,2},{3,4},{4,5},{3,5},{0,3},{1,3},{1,5},{2,5},{2,4},{0,4}};

   int nVerts, nCrVerts, nCrEdges, nTris, nTets, nOcts ;

   /* First, determine array size */
   nVerts = nVerts1 ;
   HashMap* hash = new HashMap() ;
   // divide crease edges in half
   for ( i = 0 ; i < nCrEdges1 ; i ++ )
   {
      int *v = &(credgel1[ 2 * i ]) ;

      if ( hash->findInsertSort( v[0], v[1], nVerts ) == nVerts )
      {
         nVerts ++ ;
      }
   }
   // divide each side of triangle
   for ( i = 0 ; i < nTris1 ; i ++ )
   {
      int *v = &(tril1[ 3 * i ]) ;

      for ( j = 0 ; j < 3 ; j ++ )
      {
         if ( hash->findInsertSort( v[trimap[j][0]], v[trimap[j][1]], nVerts ) == nVerts )
         {
            nVerts ++ ;
         }
      }
   }
   // divide each tet side
   for ( i = 0 ; i < nTets1 ; i ++ )
   {
      int *v = &(tetl1[ 4 * i ]) ;

      for ( j = 0 ; j < 6 ; j ++ )
      {
         if ( hash->findInsertSort( v[tetmap[j][0]], v[tetmap[j][1]], nVerts ) == nVerts )
         {
            nVerts ++ ;
         }
      }
   }
   // divide each oct side
   for ( i = 0 ; i < nOcts1 ; i ++ )
   {
      int *v = &(octl1[ 6 * i ]) ;

      for ( j = 0 ; j < 12 ; j ++ )
      {
         if ( hash->findInsertSort( v[octmap[j][0]], v[octmap[j][1]], nVerts ) == nVerts )
         {
            nVerts ++ ;
         }
      }
   }
   nVerts += nOcts1 ;  // point added to the middle of each oct
   nCrVerts = nCrVerts1 ;
   nCrEdges = nCrEdges1 * 2 ;
   nTris = nTris1 * 4 ;
   nTets = nTets1 * 4 + nOcts1 * 8 ;
   nOcts = nTets1 + nOcts1 * 6 ;

   int* crvertl = new int[ nCrVerts ] ;
   int* credgel = new int[ nCrEdges * 2 ] ;
   int* tril = new int[ nTris * 3 ] ;
   int* tetl = new int[ nTets * 4 ] ;
   int* octl = new int[ nOcts * 6 ] ;
   int* tetmaskl = new int[ nTets ] ;
   int* octmaskl = new int[ nOcts ] ;
   int* facemaskl = new int[ nTris * 2 ] ;

   //end=clock();
   //cout<<"get sizes: " << (end-start)/((float)CLOCKS_PER_SEC) << endl;
   //start = clock();
   /* Linear subdivision */
   delete hash ;
   hash = new HashMap( ) ;
   int curvert = nVerts1 ;
   
   // Crease points stay
   for ( i = 0 ; i < nCrVerts1 ; i ++ )
   {
      crvertl[ i ] = crvertl1[ i ] ;
   }
   // Crease edge splits
   int cur = 0 ;
   for ( i = 0 ; i < nCrEdges1 ; i ++ )
   {
      int *v = &(credgel1[ 2 * i ]) ;
      int ind = hash->findInsertSort( v[0], v[1], curvert ) ;

      if ( ind == curvert )
      {
		SparseArray temp;
		temp = (maskb[v[0]]+maskb[v[1]])/2;
		maskb.push_back(temp);
        curvert ++ ;
      }

	  // create new edges from split edge
      credgel[ 2 * cur ] = credgel1[ 2 * i ] ; 
      credgel[ 2 * cur + 1 ] = ind ; 
      cur ++ ;
      credgel[ 2 * cur ] = ind ;
      credgel[ 2 * cur + 1 ] = credgel1[ 2 * i + 1 ] ; 
      cur ++ ;
   }
   // Face splits
   cur = 0 ;
   for ( i = 0 ; i < nTris1 ; i ++ )
   {
      int m1 = faceMaskl1[ i * 2 ] ;
      int m2 = faceMaskl1[ i * 2 + 1] ;
      int *v = &(tril1[ 3 * i ]) ;
      int ind[3] ;

      for ( j = 0 ; j < 3 ; j ++ )
      {
         ind[ j ] = hash->findInsertSort( v[trimap[j][0]], v[trimap[j][1]], curvert ) ;
         if ( ind[ j ] == curvert )
         {
			SparseArray temp;
			temp = (maskb[v[trimap[j][0]]]+maskb[v[trimap[j][1]]])/2;
			maskb.push_back(temp);
			curvert ++ ;
         }
      }

      tril[ 3 * cur ] = tril1[ 3 * i ] ; 
      tril[ 3 * cur + 1 ] = ind[ 0 ] ; 
      tril[ 3 * cur + 2 ] = ind[ 1 ] ; 
      facemaskl[ cur * 2 ] = m1 ;
      facemaskl[ cur * 2 + 1 ] = m2 ;
      cur ++ ;
      tril[ 3 * cur ] = tril1[ 3 * i + 1 ] ; 
      tril[ 3 * cur + 1 ] = ind[ 2 ] ; 
      tril[ 3 * cur + 2 ] = ind[ 0 ] ; 
      facemaskl[ cur * 2 ] = m1 ;
      facemaskl[ cur * 2 + 1 ] = m2 ;
      cur ++ ;
      tril[ 3 * cur ] = tril1[ 3 * i + 2 ] ; 
      tril[ 3 * cur + 1 ] = ind[ 1 ] ; 
      tril[ 3 * cur + 2 ] = ind[ 2 ] ;
      facemaskl[ cur * 2 ] = m1 ;
      facemaskl[ cur * 2 + 1 ] = m2 ; 
      cur ++ ;
      tril[ 3 * cur ] = ind[ 0 ] ; 
      tril[ 3 * cur + 1 ] = ind[ 2 ] ; 
      tril[ 3 * cur + 2 ] = ind[ 1 ] ; 
      facemaskl[ cur * 2 ] = m1 ;
      facemaskl[ cur * 2 + 1 ] = m2 ;
      cur ++ ;
   }
   // Tet splits
   int curtet = 0, curoct = 0 ;
   for ( i = 0 ; i < nTets1 ; i ++ )
   {
      int mk = tetmaskl1[ i ] ;
      int *v = &(tetl1[ 4 * i ]) ;
      int ind[6] ;

      for ( j = 0 ; j < 6 ; j ++ )
      {
         ind[ j ] = hash->findInsertSort( v[tetmap[j][0]], v[tetmap[j][1]], curvert ) ;
         if ( ind[ j ] == curvert )
         {
			SparseArray temp;
			temp = (maskb[v[tetmap[j][0]]]+maskb[v[tetmap[j][1]]])/2;
			maskb.push_back(temp);
			curvert ++ ;
         }
      }

      tetl[ 4 * curtet ] = tetl1[ 4 * i ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 0 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 1 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 2 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;
      tetl[ 4 * curtet ] = tetl1[ 4 * i + 1 ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 3 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 0 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 4 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;
      tetl[ 4 * curtet ] = tetl1[ 4 * i + 2 ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 1 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 3 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 5 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;
      tetl[ 4 * curtet ] = tetl1[ 4 * i + 3 ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 2 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 5 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 4 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;

      octl[ 6 * curoct ] = ind[ 0 ] ;
      octl[ 6 * curoct + 1 ] = ind[ 1 ] ;
      octl[ 6 * curoct + 2 ] = ind[ 2 ] ;
      octl[ 6 * curoct + 3 ] = ind[ 3 ] ;
      octl[ 6 * curoct + 4 ] = ind[ 4 ] ;
      octl[ 6 * curoct + 5 ] = ind[ 5 ] ;
      octmaskl[ curoct ] = mk ;
      curoct ++ ;
   }
   // Oct splits
   for ( i = 0 ; i < nOcts1 ; i ++ )
   {
      int mk = octmaskl1[ i ] ;
      int *v = &(octl1[ 6 * i ]) ;
      int ind[13] ;

      for ( j = 0 ; j < 12 ; j ++ )
      {
         ind[ j ] = hash->findInsertSort( v[octmap[j][0]], v[octmap[j][1]], curvert ) ;
         if ( ind[ j ] == curvert )
         {
			SparseArray temp;
			temp = (maskb[v[octmap[j][0]]]+maskb[v[octmap[j][1]]])/2;
			maskb.push_back(temp);
			curvert ++ ;
         }
      }
      ind[12] = curvert ;
	  SparseArray temp;
	  temp = maskb[octl1[6*i]];
      for ( j = 1 ; j < 6 ; j ++ )
      {
        temp = temp + maskb[octl1[6*i + j]];
      }
	  temp = temp/6;
	  maskb.push_back(temp);
      curvert ++ ;

      tetl[ 4 * curtet ] = ind[ 0 ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 1 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 2 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 12 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;
      tetl[ 4 * curtet ] = ind[ 0 ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 6 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 7 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 12 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;
      tetl[ 4 * curtet ] = ind[ 1 ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 8 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 9 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 12 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;
      tetl[ 4 * curtet ] = ind[ 2 ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 10 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 11 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 12 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;
      tetl[ 4 * curtet ] = ind[ 3 ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 4 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 5 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 12 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;
      tetl[ 4 * curtet ] = ind[ 3 ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 6 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 11 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 12 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;
      tetl[ 4 * curtet ] = ind[ 4 ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 10 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 9 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 12 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;
      tetl[ 4 * curtet ] = ind[ 5 ] ; 
      tetl[ 4 * curtet + 1 ] = ind[ 8 ]; 
      tetl[ 4 * curtet + 2 ] = ind[ 7 ]; 
      tetl[ 4 * curtet + 3 ] = ind[ 12 ]; 
      tetmaskl[ curtet ] = mk ;
      curtet ++ ;

      octl[ 6 * curoct ] = octl1[ 6 * i ] ;
      octl[ 6 * curoct + 1 ] = ind[ 0 ] ;
      octl[ 6 * curoct + 2 ] = ind[ 2 ] ;
      octl[ 6 * curoct + 3 ] = ind[ 6 ] ;
      octl[ 6 * curoct + 4 ] = ind[ 11 ] ;
      octl[ 6 * curoct + 5 ] = ind[ 12 ] ;
      octmaskl[ curoct ] = mk ;
      curoct ++ ;
      octl[ 6 * curoct ] = octl1[ 6 * i + 1 ] ;
      octl[ 6 * curoct + 1 ] = ind[ 1 ] ;
      octl[ 6 * curoct + 2 ] = ind[ 0 ] ;
      octl[ 6 * curoct + 3 ] = ind[ 8 ] ;
      octl[ 6 * curoct + 4 ] = ind[ 7 ] ;
      octl[ 6 * curoct + 5 ] = ind[ 12 ] ;
      octmaskl[ curoct ] = mk ;
      curoct ++ ;
      octl[ 6 * curoct ] = octl1[ 6 * i + 2 ] ;
      octl[ 6 * curoct + 1 ] = ind[ 2 ] ;
      octl[ 6 * curoct + 2 ] = ind[ 1 ] ;
      octl[ 6 * curoct + 3 ] = ind[ 10 ] ;
      octl[ 6 * curoct + 4 ] = ind[ 9 ] ;
      octl[ 6 * curoct + 5 ] = ind[ 12 ] ;
      octmaskl[ curoct ] = mk ;
      curoct ++ ;
      octl[ 6 * curoct ] = octl1[ 6 * i + 3 ] ;
      octl[ 6 * curoct + 1 ] = ind[ 3 ] ;
      octl[ 6 * curoct + 2 ] = ind[ 5 ] ;
      octl[ 6 * curoct + 3 ] = ind[ 6 ] ;
      octl[ 6 * curoct + 4 ] = ind[ 7 ] ;
      octl[ 6 * curoct + 5 ] = ind[ 12 ] ;
      octmaskl[ curoct ] = mk ;
      curoct ++ ;
      octl[ 6 * curoct ] = octl1[ 6 * i + 4 ] ;
      octl[ 6 * curoct + 1 ] = ind[ 4 ] ;
      octl[ 6 * curoct + 2 ] = ind[ 3 ] ;
      octl[ 6 * curoct + 3 ] = ind[ 10 ] ;
      octl[ 6 * curoct + 4 ] = ind[ 11 ] ;
      octl[ 6 * curoct + 5 ] = ind[ 12 ] ;
      octmaskl[ curoct ] = mk ;
      curoct ++ ;
      octl[ 6 * curoct ] = octl1[ 6 * i + 5 ] ;
      octl[ 6 * curoct + 1 ] = ind[ 5 ] ;
      octl[ 6 * curoct + 2 ] = ind[ 4 ] ;
      octl[ 6 * curoct + 3 ] = ind[ 8 ] ;
      octl[ 6 * curoct + 4 ] = ind[ 9 ] ;
      octl[ 6 * curoct + 5 ] = ind[ 12 ] ;
      octmaskl[ curoct ] = mk ;
      curoct ++ ;
   }

   //end=clock();
   //cout<<"add verts: " << (end-start)/((float)CLOCKS_PER_SEC) << endl;
   //start=clock();
   /* Prepare dimensions and valences */
   int *dim = new int[ nVerts ] ;
   int *val = new int[ nVerts ] ;
   for ( i = 0 ; i < nVerts ; i ++ )
   {
      dim[ i ] = 3 ;
      val[ i ] = 0 ;
   }

   for ( i = 0 ; i < nCrVerts ; i ++ )
   {
      dim[ crvertl[i] ] = 0 ;
      val[ crvertl[i] ] = 1 ;
   }
   for ( i = 0 ; i < nCrEdges * 2 ; i ++ )
   {
      int d = dim[ credgel[i] ] ;
      if ( d > 1 )
      {
         dim[ credgel[i] ] = 1 ;
         val[ credgel[i] ] = 1 ;
      }
      else if ( d == 1 )
      {
         val[ credgel[i] ] ++ ;
      }
   }
   for ( i = 0 ; i < nTris * 3 ; i ++ )
   {
      int d = dim[ tril[i] ] ;
      if ( d > 2 )
      {
         dim[ tril[i] ] = 2 ;
         val[ tril[i] ] = 1 ;
      }
      else if ( d == 2 )
      {
         val[ tril[i] ] ++ ;
      }
   }
   for ( i = 0 ; i < nTets * 4 ; i ++ )
   {
      if ( dim[ tetl[i] ] == 3 )
      {
         val[ tetl[i] ] ++ ;
      }
   }
   for ( i = 0 ; i < nOcts * 6 ; i ++ )
   {
      if ( dim[ octl[i] ] == 3 )
      {
         val[ octl[i] ] ++ ;
      }
   }

   //end=clock();
   //cout<<"valencies: " << (end-start)/((float)CLOCKS_PER_SEC) << endl;
   //start=clock();
   /* Averaging */
   std::vector<SparseArray> nmaskb;
   for (int i = 0; i < nVerts; i++) {
	   SparseArray temp;
	   nmaskb.push_back(temp);
   }
   //nmaskb = maskb;
   SparseArray cent;
   
   // Points stay
   for ( i = 0 ; i < nCrVerts ; i ++ )
   {
	   nmaskb[crvertl[i]] = maskb[crvertl[i]];
   }
   //end=clock();
   //cout<<"1. duplicating and crease points: " << (end-start)/((float)CLOCKS_PER_SEC) << endl;
   //start=clock();
   // Edge: midpoint
   for ( i = 0 ; i < nCrEdges ; i ++ )
   {
      int *ind = &( credgel[ 2 * i ] ) ;
	  cent = (maskb[ind[0]] + maskb[ind[1]])/2;

      if ( dim[ind[0]] == 1 )
      {
		  nmaskb[ind[0]] = nmaskb[ind[0]] + cent/val[ind[0]];
      }
      if ( dim[ind[1]] == 1 )
      {
		 nmaskb[ind[1]] = nmaskb[ind[1]] + cent/val[ind[1]];
      }
   }
   //end=clock();
   //cout<<"2. edges: " << (end-start)/((float)CLOCKS_PER_SEC) << endl;
   //start=clock();
   // Triangle: Loop
   float PI = 3.14159265f ;
   for ( i = 0 ; i < nTris ; i ++ )
   {
      int *ind = &( tril[ 3 * i ] ) ;

      // j: index of vertex to average
      for ( j = 0 ; j < 3 ; j ++ )
      {
         if ( dim[ind[j]] != 2 )
         {
            continue ;
         }
         float w = 3.0f/8.0f + cos( 2 * PI / val[ind[j]] )/4;
         w = 5.0f/8.0f - w * w ;
         float ws[3] ;
         ws[0] = w ; ws[1] = w ; ws[2] = w ;
         ws[j] = 1 - 2 * w ;

         cent = maskb[ind[0]]*ws[0];
         // l: index of each vertex
         for ( int l = 1 ; l < 3 ; l ++ )
         {
            cent = cent + maskb[ind[l]] * ws[l];
         }
		 nmaskb[ind[j]] = nmaskb[ind[j]] + cent/val[ind[j]];
      }
   }
   //end=clock();
   //cout<<"3. faces: " << (end-start)/((float)CLOCKS_PER_SEC) << endl;
   //start=clock();
   // Tets: smooth
   float tetmask[4][4]={
      {-1.0f/16,17.0f/48,17.0f/48,17.0f/48},
      {17.0f/48,-1.0f/16,17.0f/48,17.0f/48},
      {17.0f/48,17.0f/48,-1.0f/16,17.0f/48},
      {17.0f/48,17.0f/48,17.0f/48,-1.0f/16}};
   for ( i = 0 ; i < nTets ; i ++ )
   {
      int *ind = &( tetl[ 4 * i ] ) ;

      // j: index of vertex to average
      for ( j = 0 ; j < 4 ; j ++ )
      {
         if ( dim[ind[j]] != 3 )
         {
            continue ;
         }
         cent = maskb[ind[0]]*tetmask[j][0];
         // l: index of each vertex
         for ( int l = 1 ; l < 4 ; l ++ )
         {
            cent = cent + maskb[ind[l]]*tetmask[j][l];
         }
		 nmaskb[ind[j]] = nmaskb[ind[j]] + cent/val[ind[j]];
      }
   }
   //end=clock();
  // cout<<"3. tets: " << (end-start)/((float)CLOCKS_PER_SEC) << endl;
   //start=clock();
   // Octs: smooth
   float octmask[6][6]={
      {3.0f/8,1.0f/12,1.0f/12,1.0f/12,1.0f/12,7.0f/24},
      {1.0f/12,3.0f/8,1.0f/12,1.0f/12,7.0f/24,1.0f/12},
      {1.0f/12,1.0f/12,3.0f/8,7.0f/24,1.0f/12,1.0f/12},
      {1.0f/12,1.0f/12,7.0f/24,3.0f/8,1.0f/12,1.0f/12},
      {1.0f/12,7.0f/24,1.0f/12,1.0f/12,3.0f/8,1.0f/12},
      {7.0f/24,1.0f/12,1.0f/12,1.0f/12,1.0f/12,3.0f/8}};
   for ( i = 0 ; i < nOcts ; i ++ )
   {
      int *ind = &( octl[ 6 * i ] ) ;

      // j: index of vertex to average
      for ( j = 0 ; j < 6 ; j ++ )
      {
         if ( dim[ind[j]] != 3 )
         {
            continue ;
         }
         cent = maskb[ind[0]]*octmask[j][0];
         // l: index of each vertex
         for ( int l = 1 ; l < 6 ; l ++ )
         {
            cent = cent + maskb[ind[l]]*octmask[j][l];
         }
		 nmaskb[ind[j]] = nmaskb[ind[j]] + cent/val[ind[j]];
      }
   }
   //end=clock();
   //cout<<"4. octs: " << (end-start)/((float)CLOCKS_PER_SEC) << endl;
   
   delete hash ;
   delete dim ;
   delete val ;
//#ifndef SUBDIVIDE_NO_DEL_INPUT               
//   //delete vertl1 ;
//   delete crvertl1 ;
//   delete credgel1 ;
//   delete tril1 ;
//   delete tetl1 ;
//   delete octl1 ;
//   delete tetmaskl1 ;
//   delete octmaskl1 ;
//   delete faceMaskl1 ;
//#endif
   
   maskb = nmaskb;
   crvertl1 = crvertl ;
   credgel1 = credgel ;
   tril1 = tril ;
   tetl1 = tetl ;
   octl1 = octl ;
   tetmaskl1 = tetmaskl ;
   octmaskl1 = octmaskl ;
   faceMaskl1 = facemaskl ;
   
   nVerts1 = nVerts ;
   nCrVerts1 = nCrVerts ;
   nCrEdges1 = nCrEdges ;
   nTris1 = nTris ;
   nTets1 = nTets ;
   nOcts1 = nOcts ;
   //end=clock();
   //cout<<"averaging: " << (end-start)/((float)CLOCKS_PER_SEC) << endl;
}
void PolyMesh::SubdivideMesh(int aLevel)
{
   //DEBUG_LOG("SubdivideMesh")
   //clock_t start, end;

   std::vector<float> verts1;
   for (size_t i = 0; i < verts.size(); ++i)
   {
      Vec3& v = verts[i];
      verts1.insert(verts1.end(), &v[0], &v[0]+3);
   }
   //int nCreaseVerts=0;
   //int* creaseVerts=0;
   int nCreaseEdges = crease_edges.size();
   //cout << "num crease edges: " << nCreaseEdges << endl;
   int* creaseEdges1 = new int[nCreaseEdges*2];
   for (size_t i = 0; i < crease_edges.size(); ++i)
   {
      creaseEdges1[i*2] = crease_edges[i].first;
      creaseEdges1[i*2+1] = crease_edges[i].second;
   }
   int nTris = crease_faces.size();
   //cout << "num crease faces: " << nTris << endl;
   std::vector<int> faceMask(nTris*2);
   int* tris = new int[nTris*3];
   for (size_t i = 0; i < crease_faces.size(); ++i)
   {
      Face& f = crease_faces[i];
      tris[i*3] = f.vertex[0];
      tris[i*3+1] = f.vertex[1];
      tris[i*3+2] = f.vertex[2];
      faceMask[i*2] = f.materials[0];
      faceMask[i*2+1] = f.materials[1];
   }
   
   int* faceMaskPtr = 0;
   if (nTris > 0) faceMaskPtr = &faceMask[0];
   
   //const int toTheirOctVert[] = { 0, 2, 1, 4, 5, 3};
   const int toTheirOctVert[] = { 0, 2, 1, 4, 5, 3 };
   std::vector<int> tets, octs, tetMask, octMask;
   for (size_t i = 0; i < polys.size(); ++i)
   {
      Polyhedron& p = polys[i];
      int arity = p.arity;
      std::vector<int>* polys1=0;
      if (arity==4)
      {
         polys1 = &tets;
         tetMask.push_back(p.material);
         for (int j = 0; j < arity; ++j) 
         { 
            polys1->push_back(p.vertex[j]);
         }
      }
      if (arity==6)
      {
         polys1 = &octs;
         octMask.push_back(p.material);
         for (int j = 0; j < arity; ++j) 
         { 
            polys1->push_back(p.vertex[toTheirOctVert[j]]);
         }
      }
   }
   
   int nVerts(verts1.size() / 3);
   float* vertPtr = &verts1[0];
   int nCrVerts(0);
   int* crVerts=0;

   int nTets(tets.size()/4);
   int* tetPtr = 0;
   int* tetMaskPtr = 0;
   if (nTets > 0) 
   {
      tetPtr = &tets[0];
      tetMaskPtr = &tetMask[0];
   }

   int nOcts(octs.size()/6);
   int* octPtr = 0;
   int* octMaskPtr = 0;
   if (nOcts > 0)
   {
      octPtr = &octs[0];
      octMaskPtr = &octMask[0];
   }

   for (size_t i = 0; i < (size_t)aLevel; ++i) {
      subdivide(nVerts, vertPtr, 
      nCrVerts, crVerts, nCreaseEdges, creaseEdges1,
      nTris, tris, 
      nTets, tetPtr, 
      nOcts, octPtr,
      tetMaskPtr, octMaskPtr, faceMaskPtr, 3);
   }
   
   verts.resize(nVerts);
   polys.resize(nTets + nOcts);
   crease_edges.resize(nCreaseEdges);
   crease_faces.resize(nTris);
   for (int i = 0; i < nVerts; ++i)
   {
      verts[i].set(vertPtr[i*3], vertPtr[i*3+1], vertPtr[i*3+2]);
   }
   for (int i = 0; i < nTris; ++i)
   {
      Face& f = crease_faces[i];
      f.vertex[0] = tris[i*3];
      f.vertex[1] = tris[i*3+1];
      f.vertex[2] = tris[i*3+2];
      f.materials[0] = faceMaskPtr[i*2];
      f.materials[1] = faceMaskPtr[i*2+1];
   }
   for (int i = 0; i < nCreaseEdges; ++i)
   {
      crease_edges[i].first = creaseEdges1[i*2];
      crease_edges[i].second = creaseEdges1[i*2+1];
   }
   //const int tettris[4][3]={{0,1,2},{0,2,3},{0,3,1},{1,3,2}};
   //static const int sTetFaces[4*3];// = { 0,1,2, 3,1,0, 3,2,1, 3,0,2 };
   for (int i = 0; i < nTets; ++i)
   {
	   for (int j = 0; j < 4; ++j) {
         polys[i].vertex[j] = tetPtr[i*4+j];
	   }
      polys[i].material = tetMaskPtr[i];
      polys[i].arity = 4;
   }
   //const int octtris[8][3]={{0,1,2},{3,4,5},{0,4,3},{0,2,4},{2,5,4},{2,1,5},{1,3,5},{1,0,3}}; // Theirs
   //static const int sOctFaces[8*3];// = { 0,2,1, 0,1,5, 0,5,4, 0,4,2, 3,1,2, 3,5,1, 3,4,5, 3,2,4 }; // Mine
   const int toMyOctVert[] = { 0, 2, 1, 4, 5, 3};
   for (int i = 0; i < nOcts; ++i)
   {
      int octInd(i+nTets);
	  for (int j = 0; j < 6; ++j) {
         polys[octInd].vertex[toMyOctVert[j]] = octPtr[i*6+j];
	  }
      polys[octInd].material = octMaskPtr[i];
      polys[octInd].arity = 6;
   }
   
   delete[] vertPtr;
   delete[] crVerts;
   delete[] tetPtr;
   delete[] octPtr;
   delete[] tetMaskPtr;
   delete[] octMaskPtr;
   delete[] faceMaskPtr;

   //if (aLevel > 1) SubdivideMesh(aMesh, aLevel - 1);
   //DEBUG_LOG(".\n")
}

void PolyMesh::SubdivideMask(int aLevel, std::vector<SparseArray>& maskb)
{
   //DEBUG_LOG("SubdivideMask");
   //int nCreaseVerts=0;
   //int* creaseVerts=0;
   int nCreaseEdges = crease_edges.size();
   int* creaseEdges1 = new int[nCreaseEdges*2];
   for (size_t i = 0; i < crease_edges.size(); ++i)
   {
      creaseEdges1[i*2] = crease_edges[i].first;
      creaseEdges1[i*2+1] = crease_edges[i].second;
   }
   int nTris = crease_faces.size();
   std::vector<int> faceMask(nTris*2);
   int* tris = new int[nTris*3];
   for (size_t i = 0; i < crease_faces.size(); ++i)
   {
      Face& f = crease_faces[i];
      tris[i*3] = f.vertex[0];
      tris[i*3+1] = f.vertex[1];
      tris[i*3+2] = f.vertex[2];
      faceMask[i*2] = f.materials[0];
      faceMask[i*2+1] = f.materials[1];
   }

   int* faceMaskPtr = 0;
   if (nTris > 0) faceMaskPtr = &faceMask[0];

   const int toTheirOctVert[] = { 0, 2, 1, 4, 5, 3 };
   std::vector<int> tets, octs, tetMask, octMask;
   for (size_t i = 0; i < polys.size(); ++i)
   {
      Polyhedron& p = polys[i];
      int arity = p.arity;
      std::vector<int>* polys1=0;
      if (arity==4)
      {
         polys1 = &tets;
         tetMask.push_back(p.material);
         for (int j = 0; j < arity; ++j) 
         { 
            polys1->push_back(p.vertex[j]);
         }
      }
      if (arity==6)
      {
         polys1 = &octs;
         octMask.push_back(p.material);
         for (int j = 0; j < arity; ++j) 
         { 
            polys1->push_back(p.vertex[toTheirOctVert[j]]);
         }
      }
   }

   int nVerts(verts.size());
   //cout << "nVerts: " << nVerts << endl;
   //float* vertPtr = aVerts;
   int nCrVerts(0);
   int* crVerts=0;

   int nTets(tets.size()/4);
   int* tetPtr = 0;
   int* tetMaskPtr = 0;
   if (nTets > 0) 
   {
      tetPtr = &tets[0];
      tetMaskPtr = &tetMask[0];
   }

   int nOcts(octs.size()/6);
   int* octPtr = 0;
   int* octMaskPtr = 0;
   if (nOcts > 0)
   {
      octPtr = &octs[0];
      octMaskPtr = &octMask[0];
   }

   for (size_t i = 0; i < (size_t)aLevel; ++i)
      subdivide2(nVerts, maskb, 
      nCrVerts, crVerts, nCreaseEdges, creaseEdges1,
      nTris, tris, 
      nTets, tetPtr, 
      nOcts, octPtr,
      tetMaskPtr, octMaskPtr, faceMaskPtr);

   
   delete[] crVerts;
   delete[] tetPtr;
   delete[] octPtr;
   delete[] tetMaskPtr;
   delete[] octMaskPtr;
   delete[] faceMaskPtr;

   //DEBUG_LOG(".\n")
}
