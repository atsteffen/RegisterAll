#ifndef GEOMETRY_BUFFER_HPP
#define GEOMETRY_BUFFER_HPP

#include "GeometryTypes.hpp"
#include <boost/signal.hpp>

class GeometryEvents
{
public:
   boost::signal<void(int)>   Changed;
   boost::signal<void(int)>   Erased;
   boost::signal<void()>      Updated;
};

// A buffer of objects that:
//* Can be cloned(copied)
//* Can be referenced  (primary reason for using this)
//* Monitored by events
template <typename T>
class GeometryBuffer
{
public:
   typedef T Element;
   typedef std::vector<Element> ElementList;
   typedef boost::shared_ptr<ElementList> ElementListPtr;
   typedef GeometryEvents Events;
   typedef boost::shared_ptr<Events> EventsPtr;
   typedef typename ElementList::iterator iterator;
   typedef typename ElementList::const_iterator const_iterator;
   typedef typename ElementList::reference reference;

   GeometryBuffer() : mEventsPtr(new Events), mElementsPtr(new ElementList) {}
   GeometryBuffer(ElementListPtr aVertexList) : mEventsPtr(new Events), mElementsPtr(aVertexList) {}
   //! Construct as a reference
   GeometryBuffer(const GeometryBuffer<T>& aSrc) : mEventsPtr(aSrc.mEventsPtr), mElementsPtr(aSrc.mElementsPtr) {}

   // Mirror vector methods for ease of use
   iterator begin() { return mElementsPtr->begin(); }
   const_iterator begin() const { return mElementsPtr->begin(); }
   iterator end() { return mElementsPtr->end(); }
   const_iterator end() const { return mElementsPtr->end(); }
   Element& front() { return mElementsPtr->front(); }
   const Element& front() const { return mElementsPtr->front(); }
   Element& back() { return mElementsPtr->back(); }
   const Element& back() const { return mElementsPtr->back(); }
   void push_back(const Element& aVal) { mElementsPtr->push_back(aVal); }
   void resize(size_t aSize) { mElementsPtr->resize(aSize); }
   void clear() { mElementsPtr->clear(); }
   const size_t size() const { return mElementsPtr->size(); }
   Element& operator[](int aIndex) { return (*mElementsPtr)[aIndex]; }
   const Element& operator[](int aIndex) const { return (*mElementsPtr)[aIndex]; }

   // * and -> operators to access element list
   ElementList* operator->() { return &(*mElementsPtr); }
   const ElementList* operator->() const { return &(*mElementsPtr); }
   ElementList& operator*() { return *mElementsPtr; }
   const ElementList& operator*() const { return *mElementsPtr; }
   ElementList* operator&() { return &*mElementsPtr; }
   const ElementList* operator&() const { return &*mElementsPtr; }

   //GeometryBuffer<T>& operator=(const GeometryBuffer<T>& aRHS)
   //{
   //   mEventsPtr = aRHS.mEventsPtr;
   //   mElementsPtr = aRHS.mEventsPtr;
   //   return *this;
   //}
   void clone(const GeometryBuffer<T>& aRHS) const
   {
      //aRHS.mElementsPtr = ElementListPtr(new ElementList(*mElementsPtr));
      //aRHS.mEventsPtr = boost::shared_ptr<Events>(new Events);
      //mElementsPtr = ElementListPtr(new ElementList(*aRHS.mElementsPtr));
      if (mElementsPtr == aRHS.mElementsPtr)
      {
         mElementsPtr = ElementListPtr(new ElementList);
      }
      *mElementsPtr = *aRHS.mElementsPtr;
      mEventsPtr = boost::shared_ptr<Events>(new Events);
   }

   void ref(const GeometryBuffer<T>& aRHS)
   {
      mElementsPtr = aRHS.mElementsPtr;
      mEventsPtr = aRHS.mEventsPtr;
   }


   

   Events& GetEvents() { return *mEventsPtr; }
   const Events& GetEvents() const { return *mEventsPtr; }

private:
   GeometryBuffer<T>& operator=(const GeometryBuffer<T>& aRHS);
   mutable boost::shared_ptr<Events>  mEventsPtr;
   mutable ElementListPtr             mElementsPtr;
};

typedef GeometryBuffer<Vec3>        VertexBuffer;
typedef GeometryBuffer<Face>        FaceBuffer;
typedef GeometryBuffer<Edge>        EdgeBuffer;
typedef GeometryBuffer<Polyhedron>  PolyBuffer;

//
//class TriMesh
//{
//public:
//   VertexBuffer   verts;
//   FaceBuffer     faces;
//};


#endif // VERTEXBUFFER_HPP