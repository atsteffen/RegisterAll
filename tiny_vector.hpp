#ifndef tiny_vector_h__
#define tiny_vector_h__
//#include <algorithm>
#include <boost/iterator/iterator_facade.hpp>
#include <stdio.h>
// container useful when a fixed number of elements is expected, but not guaranteed
template <typename T, int CLUSTER_SIZE>
class tiny_vector
{
public:
   typedef tiny_vector<T, CLUSTER_SIZE> ThisType;
   tiny_vector()
   {
      _size = 0;
      extra_clusters = 0;
      extra_data = 0;
   }
	~tiny_vector() { extra_data = NULL; delete extra_data;}
   T& operator[](int i) { if (i < CLUSTER_SIZE){return data[i];} return extra_data[i-CLUSTER_SIZE]; }
   const T& operator[](int i) const 
   { 
      if (i < CLUSTER_SIZE)
      {
         return data[i];
      } 
      return extra_data[i-CLUSTER_SIZE]; 
   }
   size_t size() const { return (size_t)_size; }
   void push_back(const T& aValue)
   {
      resize(_size + 1);
      operator[](_size-1) = aValue;
   }
   bool equals(const tiny_vector& tv) {
	   int wild = 0;
	   int count = 0;
	   int level1 = tv.size();
	   for (int j = 0; j<tv.size(); j++){
		   if (-3 == tv[j]){
			   level1--;
		   }
		   if (-2 == tv[j]){
			   wild++;
		   }
	   }
	   int level2 = (int)_size;
//	   for (int j = 0; j<(int)_size; j++){
//		   if (-3 == data[j]){
//			   level2--;
//		   }
//	   }
	   //std::cout << level1 << " " << level2 << std::endl;
	   for (int i = 0; i<level1; i++){
		   if (-2 != tv[i]) {
			   for (int j = 0; j<level2; j++){
				   if (tv[i] == data[j]){
					   count++;
				   }
			   }
		   }
	   }
	  // std::cout << level1 << " " << level2 << std::endl;
	   if (wild > 0) {
		   return count >= level1-wild;
	   }
		else {
			if (level1 == level2) {
				return count == level1;
			}
			return false;
		}
//	   if (level1 > level2) {
//		   return false;
//	   }
//	   else {
//		   if (wild > 0) {
//			   return count >= level1-wild;
//		   }
//		   else {
//			   if (level1 == level2) {
//				   return count == level1;
//			   }
//			   return false;
//		   }
//
//	   }
   }
   void insert(const T& aValue, int aPos)
   {
      resize(_size+1);
      for (int i = _size-1; i > aPos; ++i)
      {
         operator[](i) = operator[](i-1);
      }
      operator[](aPos) = aValue;
   }
   void resize(size_t aSize)
   {
      capacity(aSize);
      _size = aSize;
   }
   void clear()
   {
      _size = 0;
   }
   void capacity(int aSize)
   {
      int extraClusters2 = (aSize - 1) / CLUSTER_SIZE;
      if (extraClusters2 != extra_clusters)
      {
         T* dataNew = new T[extraClusters2 * CLUSTER_SIZE];
         int recordCount = _size - CLUSTER_SIZE;
         if (recordCount > 0)
         {
            std::copy(extra_data, extra_data + recordCount, dataNew);
         }
         delete extra_data;
         extra_data = dataNew;
         extra_clusters = extraClusters2;
      }
   }
   template <typename CONT_TYPE, typename TYPE>
   class tiny_vector_iterator_base : public boost::iterator_facade<tiny_vector_iterator_base<CONT_TYPE,TYPE>, TYPE, boost::random_access_traversal_tag>
   {
   public:
      CONT_TYPE* containerPtr;
      tiny_vector_iterator_base(CONT_TYPE* cont, int aOffset)
         : containerPtr(cont), offset(aOffset) {}
      typedef tiny_vector_iterator_base<CONT_TYPE, TYPE> ThisType;
      TYPE& dereference() const 
      {
         return (*containerPtr)[offset];
      }
      void increment()
      {
         ++offset;
      }
      void decrement()
      {
         --offset;
      }
      void advance(int o) { offset+=o; }
      int distance_to(const ThisType& iter) const { return iter.offset - offset; }
      bool equal(const ThisType& aRhs) const
      {
         return offset == aRhs.offset;
      }
   private:
      int offset;
   };
   typedef tiny_vector_iterator_base<ThisType, T> iterator;
   typedef tiny_vector_iterator_base<const ThisType, const T> const_iterator;

   iterator begin() { return iterator(this, 0); }
   iterator end() { return iterator(this, _size); }
   const_iterator begin() const { return const_iterator(this, 0); }
   const_iterator end() const { return const_iterator(this, _size); }

   bool operator==(const tiny_vector<T, CLUSTER_SIZE>& aRhs) const
   {
      if (size() != aRhs.size()) return false;
      return std::equal(begin(), end(), aRhs.begin());
   }
   void operator=(const tiny_vector<T, CLUSTER_SIZE>& aRhs)
   {
      tiny_vector<T, CLUSTER_SIZE> r = const_cast<tiny_vector<T, CLUSTER_SIZE>&>(aRhs);
      resize(aRhs.size());
      std::copy(r.begin(), r.end(), begin());
   }

   short _size;
   short extra_clusters;
   T   data[CLUSTER_SIZE];
   T*  extra_data;
};
#endif // tiny_vector_h__