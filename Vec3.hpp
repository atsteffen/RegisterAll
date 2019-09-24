#ifndef VEC3_HPP
#define VEC3_HPP

#include <iostream>

#include <math.h>
#include <vector>
#include "Math.hpp"

using namespace std;

//template <typename T>
class Vec3
{
	friend ostream& operator<<(ostream& s, const Vec3 & f)
	{
		s << "(" << f.x << ", " << f.y << ", " << f.z << ")";
		return s;
	}

public:
   typedef float value_type;
   Vec3() : x(0), y(0), z(0) {}
   Vec3(value_type a, value_type b, value_type c) : x(a), y(b), z(c) {}
   ~Vec3() {}
   value_type& operator[](int aIndex) { return *(&x + aIndex); }
   const value_type& operator[](int aIndex) const { return *(&x + aIndex); }
   value_type length_squared() const { return x * x + y*y + z*z; }
   value_type length() const { return sqrt(length_squared()); }
   Vec3 normal() const { Vec3 tmp(*this); tmp.normalize(); return tmp; }

   void set(value_type a, value_type b, value_type c) { x = a; y = b; z = c; }

   value_type normalize()
   {
      value_type len = length();
      x /= len;
      y /= len;
      z /= len;
      return len;
   }
   bool operator!=(const Vec3& a) const { return x != a.x || y != a.y || z != a.z; }
   Vec3 operator-() const
   {
      return Vec3(-x, -y, -z);
   }
   Vec3& operator/=(value_type a)
   {
      x /= a; y /= a; z /= a;
      return *this;
   }
   Vec3& operator*=(value_type a)
   {
      x *= a; y *= a; z *= a;
      return *this;
   }
   Vec3 operator+(Vec3 a) const
   {
      return Vec3(x + a.x, y + a.y, z + a.z);
   }
   Vec3 operator-(Vec3 a) const
   {
      return Vec3(x - a.x, y - a.y, z - a.z);
   }
   Vec3 operator*(value_type a) const
   {
      return Vec3(x * a, y * a, z * a);
   }
   Vec3 operator/(value_type a) const
   {
      return Vec3(x / a, y / a, z / a);
   }
   Vec3& operator+=(const Vec3& a)
   {
      *this = *this + a;
      return *this;
   }
   Vec3& operator-=(const Vec3& a)
   {
      *this = *this - a;
      return *this;
   }
   bool operator==(const Vec3& a) const
   {
      return x == a.x && y == a.y && z == a.z;
   }
   Vec3 perp_xy() const
   {
      return Vec3(-y, x, z);
   }
   value_type dot(const Vec3& a) const
   {
      return x*a.x + y*a.y + z*a.z;
   }
   Vec3 cross(const Vec3& b) //tn
   {
	   return  Vec3(y * b.z - z * b.y,
           z * b.x - x * b.z,
           x * b.y - y * b.x);
   }
   
   template <typename AR>
   void serialize(AR& aAr, unsigned int aVersion)
   {
      aAr & x & y & z;
   }
   value_type x,y,z;
};

typedef std::vector<Vec3> Vec3List;

inline Vec3 cross(const Vec3& a, const Vec3& b)
{
   return Vec3(a.y * b.z - a.z * b.y,
           a.z * b.x - a.x * b.z,
           a.x * b.y - a.y * b.x);
}
namespace math
{
inline bool is_finite(const Vec3& v)
{
   return is_finite(v.x) && is_finite(v.y) && is_finite(v.z);
}
}
inline float dot(const Vec3& a, const Vec3& b)
{
   return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline Vec3 operator*(float x, const Vec3& a)
{
   return a * x;
}
inline Vec3 interpolate(float t, const Vec3& a, const Vec3& b)
{
   return (1.0f - t) * a + t * b;
}
inline void ortho(Vec3 aInput, Vec3& aX, Vec3& aY)
{
   Vec3 tmp(aInput[1], aInput[2] * 2, aInput[0] * 4);
   aX = cross(tmp, aInput).normal();
   aY = cross(aX, aInput).normal();
}

inline void ortho(Vec3 aInput, Vec3 aX_Hint, Vec3& aX, Vec3& aY)
{
   Vec3 tmp(aInput[1], aInput[2] * 2, aInput[0] * 4);
   aY = cross(aX_Hint, aInput).normal();
   aX = cross(aInput, aY).normal();
}

/** 
 * The distance between two Vec3's
 * @author Dmitriy Karpman
 * @param a the first vertex
 * @param b the second vertex
 * 
 * @return the l2 distance between the two vectors
 */
inline float dist(const Vec3& a, const Vec3& b) {
	return (a - b).length();
}

/**
 * The angle for of a triangle, given 3 Vec3's
 * @author Dmitriy Karpman
 * @param a the first vertex
 * @param b the second vertex
 * @param c the third vertex
 *
 * @return the angle for b 
 **/
inline float angle(const Vec3& a, const Vec3& b,const  Vec3& c) {
	float length_A, length_B, length_C;
	length_A = dist(b, c);
	length_B = dist(a, c);
	length_C = dist(a, b);
	return acos((length_B*length_B + length_C * length_C - length_A*length_A)/(2.0 * length_B * length_C));
}

/**
 * The normal for a triangle, given 3 Vec3's.  Order matters!
 * @author Dmitriy Karpman
 * @param a the first vertex
 * @param b the second vertex
 * @param c the third vertex
 *
 * @return the normal of the triangle 
 */
inline Vec3 normal(const Vec3& a, const Vec3& b, const Vec3& c) {
	return cross(b - a, c - b);
}

/**
 * The angle between two vectors
 * @author Dmitriy Karpman
 * @param a the first vector
 * @param b the second vector
 *
 * @return The angle between these two vectors
 */
inline float angle_between(const Vec3& a, const Vec3& b) {
	return acos( dot(a,b) / (a.length() * b.length()));
}

#endif
