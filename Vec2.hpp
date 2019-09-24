#ifndef Vec2_HPP
#define Vec2_HPP
#include <math.h>
#include <vector>

class Vec3;

class Vec2
{
public:

   typedef float T;
   typedef T value_type;
   Vec2() : x(0), y(0) {}
   Vec2(const Vec3& aV);
   Vec2(T a, T b) : x(a), y(b) {}
   void set(T a, T b) { x = a; y = b; }
   T& operator[](int aIndex) { return *(&x + aIndex); }
   const T& operator[](int aIndex) const { return *(&x + aIndex); }
   T length_squared() const { return x*x + y*y; }
   T length() const { return sqrt(length_squared()); }
   Vec2 square() const { return Vec2(x*x, y*y); }
   Vec2 normal()
   {
      Vec2 tmp(*this);
      tmp.normalize();
      return tmp;
   }
   T normalize()
   {
      T len = length();
      x /= len;
      y /= len;
      return len;
   }
   bool operator!=(const Vec2& a) const { return x != a.x || y != a.y; }
   Vec2 operator-() const
   {
      return Vec2(-x, -y);
   }
   Vec2& operator/=(float a)
   {
      x /= a; y/=a;
      return *this;
   }
   Vec2& operator*=(float a)
   {
      x *= a; y*=a;
      return *this;
   }
   Vec2 operator+(Vec2 a) const
   {
      return Vec2(x+a.x, y+a.y);
   }
   Vec2 operator-(Vec2 a) const
   {
      return Vec2(x-a.x, y-a.y);
   }
   Vec2 operator*(float a) const
   {
      return Vec2(x*a, y*a);
   }
   Vec2 operator/(float a) const
   {
      return Vec2(x/a, y/a);
   }
   Vec2& operator+=(const Vec2& a)
   {
      *this = *this + a;
      return *this;
   }
   bool operator==(const Vec2& a) const
   {
      return x == a.x && y == a.y;
   }
   Vec2 perp() const
   {
      return Vec2(y, -x);
   }
   float dot(const Vec2& a) const
   {
      return x*a.x + y*a.y;
   }
   float angle() const
   {
      return atan2(x, y);
   }

   template <typename AR>
   void serialize(AR& aAr, unsigned int /*aVersion*/)
   {
      aAr & x & y;
   }

   T x,y;
};
typedef std::vector<Vec2> Vec2List;

inline float cross(const Vec2& a, const Vec2& b)
{
   return a.x * b.y + a.y * b.x;
}
inline float dot(const Vec2& a, const Vec2& b)
{
   return a.dot(b);
}
inline Vec2 operator*(float x, const Vec2& a)
{
   return a * x;
}

inline Vec2 interpolate(float t, const Vec2& a, const Vec2& b)
{
   return (1.0f - t) * a + t * b;
}

#endif
