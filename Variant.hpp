#ifndef Variant_h__
#define Variant_h__
// can store integers and pointers
// saves the user from casting to access the value
class Variant
{
   public:

      union
      {
         void* mPointer;
         int   mInteger;
      };

      Variant() {}
      Variant(int aVal) : mInteger(aVal) {}
      template <typename T>
      Variant(T* aPtr) : mPointer(aPtr) {}
      template <typename T>
      Variant& operator=(T* aPtr)
      {
         mPointer = (T*)aPtr;
         return *this;
      }

      Variant& operator=(int aInt)
      {
         mInteger = aInt;
         return *this;
      }

      operator int()
      {
         return mInteger;
      }
      template <typename T>
      operator T*()
      {
         return (T*)mPointer;
      }
      
};
#endif // Variant_h__