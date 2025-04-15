#pragma once

#include <string>
#include <vector>
#include <memory>
#include <DirectXMath.h>

#include "../Include/const.h"

#include "NVFlex/include/NvFlexExt.h"

template <typename T = float>
class XVector3
{
public:

    typedef T value_type;

    inline XVector3() : x(0.0f), y(0.0f), z(0.0f) {}
    inline XVector3(T a) : x(a), y(a), z(a) {}
    inline XVector3(const T* p) : x(p[0]), y(p[1]), z(p[2]) {}
    inline XVector3(T x_, T y_, T z_) : x(x_), y(y_), z(z_)
    {
    }

    inline operator T* () { return &x; }
    inline operator const T* () const { return &x; };

    inline void Set(T x_, T y_, T z_) { x = x_; y = y_; z = z_; }

    inline XVector3<T> operator * (T scale) const { XVector3<T> r(*this); r *= scale; return r; }
    inline XVector3<T> operator / (T scale) const { XVector3<T> r(*this); r /= scale; return r; }
    inline XVector3<T> operator + (const XVector3<T>& v) const { XVector3<T> r(*this); r += v; return r; }
    inline XVector3<T> operator - (const XVector3<T>& v) const { XVector3<T> r(*this); r -= v; return r; }
    inline XVector3<T> operator /(const XVector3<T>& v) const { XVector3<T> r(*this); r /= v; return r; }
    inline XVector3<T> operator *(const XVector3<T>& v) const { XVector3<T> r(*this); r *= v; return r; }

    inline XVector3<T>& operator *=(T scale) { x *= scale; y *= scale; z *= scale;  return *this; }
    inline XVector3<T>& operator /=(T scale) { T s(1.0f / scale); x *= s; y *= s; z *= s;  return *this; }
    inline XVector3<T>& operator +=(const XVector3<T>& v) { x += v.x; y += v.y; z += v.z;  return *this; }
    inline XVector3<T>& operator -=(const XVector3<T>& v) { x -= v.x; y -= v.y; z -= v.z;  return *this; }
    inline XVector3<T>& operator /=(const XVector3<T>& v) { x /= v.x; y /= v.y; z /= v.z;  return *this; }
    inline XVector3<T>& operator *=(const XVector3<T>& v) { x *= v.x; y *= v.y; z *= v.z;  return *this; }

    inline bool operator != (const XVector3<T>& v) const { return (x != v.x || y != v.y || z != v.z); }

    // negate
    inline XVector3<T> operator -() const { return XVector3<T>(-x, -y, -z); }
    inline XVector3<T> minimum(const XVector3<T>& v) { return XVector3<T>(min(x, v.x), min(y, v.y), min(z, v.z)); }
    inline XVector3<T> maximum(const XVector3<T>& v) { return XVector3<T>(max(x, v.x), max(y, v.y), max(z, v.z)); }

    T x, y, z;
};

typedef XVector3<float> Vec3;
typedef XVector3<float> Vector3;

template <typename T>
class XVector4
{
public:

    typedef T value_type;

     XVector4() : x(0), y(0), z(0), w(0) {}
     XVector4(T a) : x(a), y(a), z(a), w(a) {}
     XVector4(const T* p) : x(p[0]), y(p[1]), z(p[2]), w(p[3]) {}
     XVector4(T x_, T y_, T z_, T w_) : x(x_), y(y_), z(z_), w(w_) {  }
     XVector4(const Vec3& v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}

     operator T* () { return &x; }
     operator const T* () const { return &x; };

     void Set(T x_, T y_, T z_, T w_) {  x = x_; y = y_; z = z_; w = w_; }

     XVector4<T> operator * (T scale) const { XVector4<T> r(*this); r *= scale;  return r; }
     XVector4<T> operator / (T scale) const { XVector4<T> r(*this); r /= scale;  return r; }
     XVector4<T> operator + (const XVector4<T>& v) const { XVector4<T> r(*this); r += v;  return r; }
     XVector4<T> operator - (const XVector4<T>& v) const { XVector4<T> r(*this); r -= v;  return r; }
     XVector4<T> operator * (XVector4<T> scale) const { XVector4<T> r(*this); r *= scale;  return r; }

     XVector4<T>& operator *=(T scale) { x *= scale; y *= scale; z *= scale; w *= scale;  return *this; }
     XVector4<T>& operator /=(T scale) { T s(1.0f / scale); x *= s; y *= s; z *= s; w *= s;  return *this; }
     XVector4<T>& operator +=(const XVector4<T>& v) { x += v.x; y += v.y; z += v.z; w += v.w;  return *this; }
     XVector4<T>& operator -=(const XVector4<T>& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w;  return *this; }
     XVector4<T>& operator *=(const XVector4<T>& v) { x *= v.x; y *= v.y; z *= v.z; w *= v.w;  return *this; }

     bool operator != (const XVector4<T>& v) const { return (x != v.x || y != v.y || z != v.z || w != v.w); }

    // negate
     XVector4<T> operator -() const {  return XVector4<T>(-x, -y, -z, -w); }

    T x, y, z, w;
};

typedef XVector4<float> Vector4;
typedef XVector4<float> Vec4;

namespace Zongine {
    constexpr int FLEX_NORMALIZE_SCLAE = 100;
    class Entity;

    struct NvFlexContent {
        NvFlexVector<DirectX::XMFLOAT4> Particles;
        NvFlexVector<int> Phases;
        NvFlexVector<DirectX::XMFLOAT3> Velocities;

        NvFlexContent(NvFlexLibrary* l, int size) : Particles(l, size) , Phases(l, size), Velocities(l, size){ }
    };

    struct NvFlexComponent {
        bool bInitialized{ false };
        std::string Path{};
        std::string MeshPath{};

        std::vector<int> ParticleVertices{};

        std::shared_ptr<NvFlexContent> Content{};

        std::vector<FLEX_VERTEX_EXT> FlexVertices{};

        void Initialize(const Entity& entity, NvFlexLibrary* library);
    };
}