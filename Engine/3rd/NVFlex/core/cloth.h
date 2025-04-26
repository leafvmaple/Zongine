// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2013-2020 NVIDIA Corporation. All rights reserved.

#pragma once

#include <set>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include <numeric>

#include <assert.h>
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

template <typename T>
typename T::value_type Dot3(const T& v1, const T& v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

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
    XVector4(T x_, T y_, T z_, T w_) : x(x_), y(y_), z(z_), w(w_) {}
    XVector4(const Vec3& v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}

    operator T* () { return &x; }
    operator const T* () const { return &x; };

    void Set(T x_, T y_, T z_, T w_) { x = x_; y = y_; z = z_; w = w_; }

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
    XVector4<T> operator -() const { return XVector4<T>(-x, -y, -z, -w); }

    T x, y, z, w;
};

typedef XVector4<float> Vector4;
typedef XVector4<float> Vec4;

class Plane : public Vec4
{
public:

    inline Plane() {}
    inline Plane(float x, float y, float z, float w) : Vec4(x, y, z, w) {}
    inline Plane(const Vec3& p, const Vector3& n)
    {
        x = n.x;
        y = n.y;
        z = n.z;
        w = -Dot3(p, n);
    }

    inline Vec3 GetNormal() const { return Vec3(x, y, z); }
    inline Vec3 GetPoint() const { return Vec3(x * -w, y * -w, z * -w); }

    inline Plane(const Vec3& v) : Vec4(v.x, v.y, v.z, 1.0f) {}
   inline Plane(const Vec4& v) : Vec4(v) {}
};

template <typename T>
inline T Dot(const XVector3<T>& v1, const XVector3<T>& v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

template <typename T>
inline T Dot(const XVector4<T>& v1, const XVector4<T>& v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

// helper function that assumes a w of 0
inline float Dot(const Plane& p, const Vector3& v)
{
    return p.x * v.x + p.y * v.y + p.z * v.z;
}

inline Vec3 Cross(const Vec3& b, const Vec3& c)
{
    return Vec3(b.y * c.z - b.z * c.y,
        b.z * c.x - b.x * c.z,
        b.x * c.y - b.y * c.x);
}

template <typename T>
inline T Min(T a, T b)
{
    return a < b ? a : b;
}

template <typename T>
inline T Max(T a, T b)
{
    return a > b ? a : b;
}

template <typename T>
inline typename T::value_type LengthSq(const T v)
{
    return Dot(v, v);
}

inline float Sqrt(float x)
{
    return sqrtf(x);
}

template <typename T>
inline typename T::value_type Length(const T& v)
{
    typename T::value_type lSq = LengthSq(v);
    if (lSq)
        return Sqrt(LengthSq(v));
    else
        return 0.0f;
}

class ClothMesh
{
public:

    struct Edge
    {
        int vertices[2];
        int tris[2];

        int stretchConstraint;
        int bendingConstraint;


        Edge(int a, int b)
        {
            assert(a != b);

            vertices[0] = Min(a, b);
            vertices[1] = Max(a, b);

            tris[0] = -1;
            tris[1] = -1;

            stretchConstraint = -1;
            bendingConstraint = -1;
        }

        bool IsBoundary() const { return tris[0] == -1 || tris[1] == -1; }

        bool Contains(int index) const
        {
            return (vertices[0] == index) || (vertices[1] == index);
        }

        void Replace(int oldIndex, int newIndex)
        {
            if (vertices[0] == oldIndex)
                vertices[0] = newIndex;
            else if (vertices[1] == oldIndex)
                vertices[1] = newIndex;
            else
                assert(0);
        }

        void RemoveTri(int index)
        {
            if (tris[0] == index)
                tris[0] = -1;
            else if (tris[1] == index)
                tris[1] = -1;
            else
                assert(0);
        }

        bool AddTri(int index)
        {
            if (tris[0] == -1)
            {
                tris[0] = index;
                return true;
            }
            else if (tris[1] == -1)
            {
                // check tri not referencing same edge
                if (index == tris[0])
                    return false;
                else
                {
                    tris[1] = index;
                    return true;
                }
            }
            else
                return true;
        }

        bool operator < (const Edge& rhs) const
        {
            if (vertices[0] != rhs.vertices[0])
                return vertices[0] < rhs.vertices[0];
            else
                return vertices[1] < rhs.vertices[1];
        }
    };

    struct Triangle
    {
        Triangle(int a, int b, int c)
        {
            assert(a != b && a != c);
            assert(b != c);

            vertices[0] = a;
            vertices[1] = b;
            vertices[2] = c;

            edges[0] = -1;
            edges[1] = -1;
            edges[2] = -1;

            side = -1;

            component = -1;
        }

        bool Contains(int v) const
        {
            if (vertices[0] == v ||
                vertices[1] == v ||
                vertices[2] == v)
                return true;
            else
                return false;
        }

        void ReplaceEdge(int oldIndex, int newIndex)
        {
            for (int i = 0; i < 3; ++i)
            {
                if (edges[i] == oldIndex)
                {
                    edges[i] = newIndex;
                    return;
                }

            }
            assert(0);
        }

        int ReplaceVertex(int oldIndex, int newIndex)
        {
            for (int i = 0; i < 3; ++i)
            {
                if (vertices[i] == oldIndex)
                {
                    vertices[i] = newIndex;
                    return i;
                }
            }

            assert(0);
            return -1;
        }

        int GetOppositeVertex(int v0, int v1) const
        {
            for (int i = 0; i < 3; ++i)
            {
                if (vertices[i] != v0 && vertices[i] != v1)
                    return vertices[i];
            }

            assert(0);
            return -1;
        }

        int vertices[3];
        int edges[3];

        // used during splitting
        int side;

        // used during singular vertex removal
        mutable int component;
    };

    ClothMesh(const Vec4* vertices, int numVertices,
        const int* indices, int numIndices,
        float stretchStiffness,
        float bendStiffness, bool tearable = true)
    {
        mValid = false;

        mNumVertices = numVertices;

        if (tearable)
        {
            // tearable cloth uses a simple bending constraint model that allows easy splitting of vertices and remapping of constraints
            typedef std::set<Edge> EdgeSet;
            EdgeSet edges;

            // build unique edge list
            for (int i = 0; i < numIndices; i += 3)
            {
                mTris.push_back(Triangle(indices[i + 0], indices[i + 1], indices[i + 2]));

                const int triIndex = i / 3;

                // breaking the rules
                Edge& e1 = const_cast<Edge&>(*edges.insert(Edge(indices[i + 0], indices[i + 1])).first);
                Edge& e2 = const_cast<Edge&>(*edges.insert(Edge(indices[i + 1], indices[i + 2])).first);
                Edge& e3 = const_cast<Edge&>(*edges.insert(Edge(indices[i + 2], indices[i + 0])).first);

                if (!e1.AddTri(triIndex))
                    return;
                if (!e2.AddTri(triIndex))
                    return;
                if (!e3.AddTri(triIndex))
                    return;
            }

            // flatten set to array
            mEdges.assign(edges.begin(), edges.end());

            // second pass, set edge indices to faces
            for (int i = 0; i < numIndices; i += 3)
            {
                int e1 = int(std::lower_bound(mEdges.begin(), mEdges.end(), Edge(indices[i + 0], indices[i + 1])) - mEdges.begin());
                int e2 = int(std::lower_bound(mEdges.begin(), mEdges.end(), Edge(indices[i + 1], indices[i + 2])) - mEdges.begin());
                int e3 = int(std::lower_bound(mEdges.begin(), mEdges.end(), Edge(indices[i + 2], indices[i + 0])) - mEdges.begin());


                if (e1 != e2 && e1 != e3 && e2 != e3)
                {
                    const int triIndex = i / 3;

                    mTris[triIndex].edges[0] = e1;
                    mTris[triIndex].edges[1] = e2;
                    mTris[triIndex].edges[2] = e3;
                }
                else
                {
                    // degenerate tri
                    return;
                }
            }

            // generate distance constraints
            for (size_t i = 0; i < mEdges.size(); ++i)
            {
                Edge& edge = mEdges[i];

                // stretch constraint along mesh edges
                edge.stretchConstraint = AddConstraint(vertices, edge.vertices[0], edge.vertices[1], stretchStiffness);

                const int t1 = edge.tris[0];
                const int t2 = edge.tris[1];

                // add bending constraint between connected tris
                if (t1 != -1 && t2 != -1 && bendStiffness > 0.0f)
                {
                    int v1 = mTris[t1].GetOppositeVertex(edge.vertices[0], edge.vertices[1]);
                    int v2 = mTris[t2].GetOppositeVertex(edge.vertices[0], edge.vertices[1]);
                    edge.bendingConstraint = AddConstraint(vertices, v1, v2, bendStiffness);
                }
            }
        }

        // calculate rest volume
        mRestVolume = 0.0f;
        mConstraintScale = 0.0f;

        std::vector<Vec3> gradients(numVertices);

        for (int i = 0; i < numIndices; i += 3)
        {
            Vec3 v1 = Vec3(vertices[indices[i + 0]]);
            Vec3 v2 = Vec3(vertices[indices[i + 1]]);
            Vec3 v3 = Vec3(vertices[indices[i + 2]]);

            const Vec3 n = Cross(v2 - v1, v3 - v1);
            const float signedVolume = Dot(v1, n);

            mRestVolume += signedVolume;

            gradients[indices[i + 0]] += n;
            gradients[indices[i + 1]] += n;
            gradients[indices[i + 2]] += n;
        }

        for (int i = 0; i < numVertices; ++i)
            mConstraintScale += Dot(gradients[i], gradients[i]);

        mConstraintScale = 1.0f / mConstraintScale;

        mValid = true;

    }

    int AddConstraint(const Vec4* vertices, int a, int b, float stiffness, float give = 0.0f)
    {
        int index = int(mConstraintRestLengths.size());

        mConstraintIndices.push_back(a);
        mConstraintIndices.push_back(b);

        const float restLength = Length(Vec3(vertices[a]) - Vec3(vertices[b]));

        mConstraintRestLengths.push_back(restLength * (1.0f + give));
        mConstraintCoefficients.push_back(stiffness);

        return index;
    }

    int IsSingularVertex(int vertex) const
    {
        std::vector<int> adjacentTriangles;

        // gather adjacent triangles
        for (int i = 0; i < int(mTris.size()); ++i)
        {
            if (mTris[i].Contains(vertex))
                adjacentTriangles.push_back(i);
        }

        // number of identified components
        int componentCount = 0;

        // while connected tris not colored
        for (int i = 0; i < int(adjacentTriangles.size()); ++i)
        {
            // pop off a triangle
            int seed = adjacentTriangles[i];

            // triangle already belongs to a component
            if (mTris[seed].component != -1)
                continue;

            std::vector<int> stack;
            stack.push_back(seed);

            while (!stack.empty())
            {
                int t = stack.back();
                stack.pop_back();

                const Triangle& tri = mTris[t];

                if (tri.component == componentCount)
                {
                    // we're back to the beginning
                    // component is fully connected
                    break;
                }

                tri.component = componentCount;

                // update mesh
                for (int e = 0; e < 3; ++e)
                {
                    const Edge& edge = mEdges[tri.edges[e]];

                    if (edge.Contains(vertex))
                    {
                        if (!edge.IsBoundary())
                        {
                            // push unprocessed neighbors on stack
                            for (int s = 0; s < 2; ++s)
                            {
                                assert(mTris[edge.tris[s]].component == -1 || mTris[edge.tris[s]].component == componentCount);

                                if (edge.tris[s] != t && mTris[edge.tris[s]].component == -1)
                                    stack.push_back(edge.tris[s]);
                            }
                        }
                    }
                }
            }

            componentCount++;
        }

        // reset component indices
        for (int i = 0; i < int(adjacentTriangles.size()); ++i)
        {
            assert(mTris[adjacentTriangles[i]].component != -1);

            mTris[adjacentTriangles[i]].component = -1;
        }

        return componentCount;
    }

    struct TriangleUpdate
    {
        int triangle;
        int vertex;
    };

    struct VertexCopy
    {
        int srcIndex;
        int destIndex;
    };

    int SeparateVertex(int singularVertex, std::vector<TriangleUpdate>& replacements, std::vector<VertexCopy>& copies, int maxCopies)
    {
        std::vector<int> adjacentTriangles;

        // gather adjacent triangles
        for (int i = 0; i < int(mTris.size()); ++i)
        {
            if (mTris[i].Contains(singularVertex))
                adjacentTriangles.push_back(i);
        }

        // number of identified components
        int componentCount = 0;

        // first component keeps the existing vertex
        int newIndex = singularVertex;

        // while connected tris not colored
        for (int i = 0; i < int(adjacentTriangles.size()); ++i)
        {
            if (maxCopies == 0)
                break;

            // pop off a triangle
            int seed = adjacentTriangles[i];

            // triangle already belongs to a component
            if (mTris[seed].component != -1)
                continue;

            std::vector<int> stack;
            stack.push_back(seed);

            while (!stack.empty())
            {
                int t = stack.back();
                stack.pop_back();

                Triangle& tri = mTris[t];

                // test if we're back to the beginning, in which case the component is fully connected
                if (tri.component == componentCount)
                    break;

                assert(tri.component == -1);

                tri.component = componentCount;

                // update triangle
                int v = tri.ReplaceVertex(singularVertex, newIndex);

                if (singularVertex != newIndex)
                {
                    // output replacement
                    TriangleUpdate r;
                    r.triangle = t * 3 + v;
                    r.vertex = newIndex;
                    replacements.push_back(r);
                }

                // update mesh
                for (int e = 0; e < 3; ++e)
                {
                    Edge& edge = mEdges[tri.edges[e]];

                    if (edge.Contains(singularVertex))
                    {
                        // update edge to point to new vertex
                        edge.Replace(singularVertex, newIndex);

                        const int stretching = edge.stretchConstraint;
                        if (mConstraintIndices[stretching * 2 + 0] == singularVertex)
                            mConstraintIndices[stretching * 2 + 0] = newIndex;
                        else if (mConstraintIndices[stretching * 2 + 1] == singularVertex)
                            mConstraintIndices[stretching * 2 + 1] = newIndex;
                        else
                            assert(0);

                        if (!edge.IsBoundary())
                        {
                            // push unprocessed neighbors on stack
                            for (int s = 0; s < 2; ++s)
                            {
                                assert(mTris[edge.tris[s]].component == -1 || mTris[edge.tris[s]].component == componentCount);

                                if (edge.tris[s] != t && mTris[edge.tris[s]].component == -1)
                                    stack.push_back(edge.tris[s]);
                            }
                        }
                    }
                    else
                    {
                        const int bending = edge.bendingConstraint;

                        if (bending != -1)
                        {
                            if (mConstraintIndices[bending * 2 + 0] == singularVertex)
                                mConstraintIndices[bending * 2 + 0] = newIndex;
                            else if (mConstraintIndices[bending * 2 + 1] == singularVertex)
                                mConstraintIndices[bending * 2 + 1] = newIndex;
                        }
                    }
                }
            }

            // copy vertex
            if (singularVertex != newIndex)
            {
                VertexCopy copy;
                copy.srcIndex = singularVertex;
                copy.destIndex = newIndex;

                copies.push_back(copy);

                mNumVertices++;
                maxCopies--;
            }

            // component traversal finished
            newIndex = mNumVertices;

            componentCount++;
        }

        // reset component indices
        for (int i = 0; i < int(adjacentTriangles.size()); ++i)
        {
            //assert(mTris[adjacentTriangles[i]].component != -1);

            mTris[adjacentTriangles[i]].component = -1;
        }

        return componentCount;
    }

    int SplitVertex(const Vec4* vertices, int index, Vec3 splitPlane, std::vector<int>& adjacentTris, std::vector<int>& adjacentVertices, std::vector<TriangleUpdate>& replacements, std::vector<VertexCopy>& copies, int maxCopies)
    {
        if (maxCopies == 0)
            return -1;

        float w = Dot(vertices[index], splitPlane);

        int leftCount = 0;
        int rightCount = 0;

        const int newIndex = mNumVertices;

        // classify all tris attached to the split vertex according 
        // to which side of the split plane their centroid lies on O(N)
        for (size_t i = 0; i < mTris.size(); ++i)
        {
            Triangle& tri = mTris[i];

            if (tri.Contains(index))
            {
                const Vec4 centroid = (vertices[tri.vertices[0]] + vertices[tri.vertices[1]] + vertices[tri.vertices[2]]) / 3.0f;

                if (Dot(Vec3(centroid), splitPlane) < w)
                {
                    tri.side = 1;

                    ++leftCount;
                }
                else
                {
                    tri.side = 0;

                    ++rightCount;
                }

                adjacentTris.push_back(int(i));
                for (int v = 0; v < 3; ++v)
                {
                    if (std::find(adjacentVertices.begin(), adjacentVertices.end(), tri.vertices[v]) == adjacentVertices.end())
                    {
                        adjacentVertices.push_back(tri.vertices[v]);
                    }
                }
            }
        }

        // if all tris on one side of split plane then do nothing
        if (leftCount == 0 || rightCount == 0)
            return -1;

        // remap triangle indices
        for (size_t i = 0; i < adjacentTris.size(); ++i)
        {
            const int triIndex = adjacentTris[i];

            Triangle& tri = mTris[triIndex];

            // tris on the negative side of the split plane are assigned the new index
            if (tri.side == 0)
            {
                int v = tri.ReplaceVertex(index, newIndex);

                TriangleUpdate update;
                update.triangle = triIndex * 3 + v;
                update.vertex = newIndex;
                replacements.push_back(update);

                // update edges and constraints
                for (int e = 0; e < 3; ++e)
                {
                    Edge& edge = mEdges[tri.edges[e]];

                    if (edge.Contains(index))
                    {
                        bool exposed = false;

                        if (edge.tris[0] != -1 && edge.tris[1] != -1)
                        {
                            Triangle& t1 = mTris[edge.tris[0]];
                            Triangle& t2 = mTris[edge.tris[1]];

                            // Case 1: connected tris lie on opposite sides of the split plane
                            // creating a new exposed edge, need to break bending constraint
                            // and create new stretch constraint for exposed edge
                            if (t1.side != t2.side)
                            {
                                // create new edge
                                Edge newEdge(edge.vertices[0], edge.vertices[1]);
                                newEdge.Replace(index, newIndex);
                                newEdge.AddTri(triIndex);

                                // remove neighbor from old edge
                                edge.RemoveTri(triIndex);

                                // replace bending constraint with stretch constraint
                                assert(edge.bendingConstraint != -1);

                                newEdge.stretchConstraint = edge.bendingConstraint;

                                mConstraintIndices[newEdge.stretchConstraint * 2 + 0] = newEdge.vertices[0];
                                mConstraintIndices[newEdge.stretchConstraint * 2 + 1] = newEdge.vertices[1];
                                mConstraintCoefficients[newEdge.stretchConstraint] = mConstraintCoefficients[edge.stretchConstraint];
                                mConstraintRestLengths[newEdge.stretchConstraint] = mConstraintRestLengths[edge.stretchConstraint];

                                edge.bendingConstraint = -1;

                                // don't access Edge& after this 
                                tri.ReplaceEdge(tri.edges[e], int(mEdges.size()));
                                mEdges.push_back(newEdge);

                                exposed = true;
                            }
                        }

                        if (!exposed)
                        {
                            // Case 2: both tris on same side of split plane or boundary edge, simply remap edge and constraint
                            // may have processed this edge already so check that it still contains old vertex
                            edge.Replace(index, newIndex);

                            const int stretching = edge.stretchConstraint;
                            if (mConstraintIndices[stretching * 2 + 0] == index)
                                mConstraintIndices[stretching * 2 + 0] = newIndex;
                            else if (mConstraintIndices[stretching * 2 + 1] == index)
                                mConstraintIndices[stretching * 2 + 1] = newIndex;
                            else
                                assert(0);
                        }
                    }
                    else
                    {
                        // Case 3: tri is adjacent to split vertex but this edge is not connected to it
                        // therefore there can be a bending constraint crossing this edge connected 
                        // to vertex that needs to be remapped
                        const int bending = edge.bendingConstraint;

                        if (bending != -1)
                        {
                            if (mConstraintIndices[bending * 2 + 0] == index)
                                mConstraintIndices[bending * 2 + 0] = newIndex;
                            else if (mConstraintIndices[bending * 2 + 1] == index)
                                mConstraintIndices[bending * 2 + 1] = newIndex;
                        }
                    }
                }

            }
        }

        // output vertex copy
        VertexCopy copy;
        copy.srcIndex = index;
        copy.destIndex = newIndex;

        copies.push_back(copy);

        mNumVertices++;

        return newIndex;
    }

    std::vector<int> mConstraintIndices;
    std::vector<float> mConstraintCoefficients;
    std::vector<float> mConstraintRestLengths;

    std::vector<Edge> mEdges;
    std::vector<Triangle> mTris;

    int mNumVertices;

    float mRestVolume;
    float mConstraintScale;

    bool mValid;
};
