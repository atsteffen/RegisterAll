#ifndef ALGORITHM_HPP
#define ALGORITHM_HPP
#include <algorithm>
#include <vector>

namespace Algorithm
{
   template <typename T>
   void make_unique(T& aVec)
   {
      std::sort(aVec.begin(), aVec.end());
      aVec.erase(std::unique(aVec.begin(), aVec.end()), aVec.end());
   }

   // provided to sort tiny datasets
   inline void ShellSort(int A[],
                         int size) 
   {
      int i, j, increment, temp;
      increment = size / 2;

      while (increment > 0) 
      {
         for (i = increment; i < size; i++) {
            j = i;
            temp = A[i];
            while ((j >= increment) && (A[j-increment] > temp)) {
               A[j] = A[j - increment];
               j = j - increment;
            }
            A[j] = temp;
         }

         if (increment == 2)
            increment = 1;
         else 
            increment = (int) (increment / 2.2);
      }
   }
   //template<class FWDIT, class PR> inline
      //_FwdIt remove_if(_FwdIt _First, _FwdIt _Last, _Pr _Pred)
}
#endif