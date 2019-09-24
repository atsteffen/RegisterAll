#ifndef MATH_HPP
#define MATH_HPP
#include <limits>
#include <math.h>
#include <algorithm>

template <typename T, int I, int IS_ODD>
struct PowerImpl;

template <typename T, int I>
struct PowerImpl<T, I, 1>
{
   static T Compute(T aValue)
   {
      T v = PowerImpl<T, I/2, (I/2)%2>::Compute(aValue);
      return v*v*aValue;
   }
};
template <typename T, int I>
struct PowerImpl<T, I, 0>
{
   static T Compute(T aValue)
   {
      T v = PowerImpl<T, I/2, (I/2)%2>::Compute(aValue);
      return v*v;
   }
};
template <typename T>
struct PowerImpl<T, 1, 1>
{
   static T Compute(T aValue)
   {
      return aValue;
   }
};
template <typename T>
struct PowerImpl<T, 0, 0>
{
   static T Compute(T aValue)
   {
      return T(1);
   }
};

class Math
{
public:
   static const float cZERO_TOLERANCE;
   static const float cPI;
   static const float cTWO_PI;
   static void clamp(float& val, float low, float high)
   {
      if (val < low) val = low;
      else if (val > high) val = high;
   }
   template <int I>
   static float power(float aValue)
   {
      return PowerImpl<float, I, I%2>::Compute(aValue);
   }


   static int Quadratic(float  a,
                        float  b,
                        float  c,
                        float& t1,
                        float& t2)
   {
      if (a == 0) return 0;
      float rooted = b*b - 4 * a * c;
      if (rooted < 0)
      {
         return 0;
      }
      else if (rooted == 0)
      {
         t1 = -b / (2 * a);
         return 1;
      }
      else
      {
         rooted = sqrt(rooted);
         t1 = (-b - rooted) / (2 * a);
         t2 = (-b + rooted) / (2 * a);
         if (t1 > t2)
         {
            std::swap(t1, t2);
         }
         return 2;
      }
   }

   static int PositiveQuadratic(float  a,
                                float  b,
                                float  c,
                                float& t1,
                                float& t2)
   {
      int answers = Quadratic(a,b,c,t1,t2);
      if (answers == 2 && t1 <= 0)
      {
         t1 = t2;
         --answers;
      }
      if (answers == 1 && t1 <= 0)
      {
         --answers;
      }
      return answers;
   }
private:
   //template <int I, bool valid>
   //struct factorial_helper { static const int value = factorial_helper<I-1, true>::value * I; };
   //template <int I>
   //struct factorial_helper<I, false>;
   //template <>
   //struct factorial_helper<0, true> { static const int value = 1; };
public:
   //template <int I>
   //struct factorial
   //{
   //   static const int value = factorial_helper<I, (I>=0)>::value; //factorial<I-1>::value * I;
   //};
private:
   /*template <int n, int k, bool DONE>
   struct combination_helper
   {
      static const int value = n * combination_helper<n-1,k,n-1 <= k>::value;
   };
   template <int n, int k>
   struct combination_helper<n,k,true>
   {
      static const int value = 1;
   };*/

public:
   /*template <int n, int k>
   struct combination
   {
      static const int value = factorial<n>::value / (factorial<k>::value * factorial<n-k>::value);
      //static const int value = combination_helper<n,k,n<=k>::value;
   };*/
};
namespace math
{
   inline bool is_finite(float aValue)
   {
      return (aValue <= std::numeric_limits<float>::max())
          && (aValue >= -std::numeric_limits<float>::max());
   }


}
#endif
