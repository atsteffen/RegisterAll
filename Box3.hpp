#ifndef BOX3_HPP
#define BOX3_HPP
#include "Vec3.hpp"

class Box3
{
   public:
      Box3(const Vec3& aMin, const Vec3& aMax)
         : mMin(aMin), mMax(aMax) 
      {}

      Box3() {}
      Vec3& Min() { return mMin; }
      
      Vec3& Max() { return mMax; }
      
      Vec3 size() { return mMax - mMin; }

   private:
      Vec3 mMin;
      Vec3 mMax;
};

#endif
