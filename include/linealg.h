#ifndef LINEALG_H_GUARD
#define LINEALG_H_GUARD
#include <cstdlib>
#include <cstring>
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"
#include "matrix3.h"
#include "matrix4.h"

const float PI = 3.1415926535897932384626433832f;

template<class T>
struct VectorPOD2 {
	T x,y;
};

template<class T>
struct VectorPOD3 {
	T x,y, z;
};

template<class T>
struct VectorPOD4 {
	T x,y,z,w;
};

typedef Vector2<int> Vector2i;
typedef Vector2<float> Vector2f;
typedef Vector2<double> Vector2d;

typedef Vector3<int> Vector3i;
typedef Vector3<float> Vector3f;
typedef Vector3<double> Vector3d;

typedef Vector4<int> Vector4i;
typedef Vector4<float> Vector4f;
typedef Vector4<double> Vector4d;

typedef Matrix3<int> Matrix3i;
typedef Matrix3<float> Matrix3f;
typedef Matrix3<double> Matrix3d;

typedef Matrix4<int> Matrix4i;
typedef Matrix4<float> Matrix4f;
typedef Matrix4<double> Matrix4d;


typedef VectorPOD2<int> VectorPOD2i;
typedef VectorPOD2<float> VectorPOD2f;
typedef VectorPOD2<double> VectorPOD2d;

typedef VectorPOD3<int> VectorPOD3i;
typedef VectorPOD3<float> VectorPOD3f;
typedef VectorPOD3<double> VectorPOD3d;

typedef VectorPOD4<int> VectorPOD4i;
typedef VectorPOD4<float> VectorPOD4f;
typedef VectorPOD4<double> VectorPOD4d;

typedef int MatrixPOD2i[4];
typedef float MatrixPOD2f[4];
typedef double MatrixPOD2d[4];

typedef int MatrixPOD3i[9];
typedef float MatrixPOD3f[9];
typedef double MatrixPOD3d[9];

typedef int MatrixPOD4i[16];
typedef float MatrixPOD4f[16];
typedef double MatrixPOD4d[16];

/* Misc functions */

inline float degtorad(float deg)
{
	return deg/180.0f * PI;
}

inline float radtodeg(float theta)
{
	return theta/PI * 180.0f;
}

template<class T> inline T clamp(T x, T mn, T mx)
{
	return std::min(std::max(x, mn), mx);
}


/***********************************************************************
 ******************** Normal vector- and matrix functions **************
 ********************************************************************* */

template<class T>
Vector3<T> operator*(const Matrix4<T>& mat, const Vector3<T>& v)
{
	return Vector3<T>(
	            v.x*mat[ 0] + v.y*mat[ 1] + v.z*mat[ 2] + mat[ 3],
	            v.x*mat[ 4] + v.y*mat[ 5] + v.z*mat[ 6] + mat[ 7],
	            v.x*mat[ 8] + v.y*mat[ 9] + v.z*mat[10] + mat[11]
	       );
}

template<class T>
Vector4<T> operator*(const Matrix4<T>& mat, const Vector4<T>& v)
{
	return Vector4<T>(
	            v.x*mat[ 0] + v.y*mat[ 1] + v.z*mat[ 2] + v.w*mat[ 3],
	            v.x*mat[ 4] + v.y*mat[ 5] + v.z*mat[ 6] + v.w*mat[ 7],
	            v.x*mat[ 8] + v.y*mat[ 9] + v.z*mat[10] + v.w*mat[11],
	            v.x*mat[12] + v.y*mat[13] + v.z*mat[14] + v.w*mat[15]
	       );
}

template<class T>
Vector3<T> operator*(const Matrix3<T>& mat, const Vector3<T>& v)
{
	return Vector3<T>(
	            v.x*mat[ 0] + v.y*mat[ 1] + v.z*mat[ 2],
	            v.x*mat[ 3] + v.y*mat[ 4] + v.z*mat[ 5],
	            v.x*mat[ 6] + v.y*mat[ 7] + v.z*mat[ 8]
	       );
}

template<class T>
Vector3<T> operator*(const Matrix3<T>& mat, const Vector4<T>& v)
{
	return Vector4<T>(
	            v.x*mat[ 0] + v.y*mat[ 1] + v.z*mat[ 2],
	            v.x*mat[ 3] + v.y*mat[ 4] + v.z*mat[ 5],
	            v.x*mat[ 6] + v.y*mat[ 7] + v.z*mat[ 8],
	            1.0f
	       );
}

template<class T> T distance(const Vector3<T>& v1, const Vector3<T>& v2)
{
	Vector3<T> tmp = v2-v1;
	return tmp.length();
}

template<class T> T distance(const Vector4<T>& v1, const Vector4<T>& v2)
{
	Vector4<T> tmp = v2-v1;
	return tmp.length();
}

/* Linear interpolation */

inline Vector2f mix(float t, const Vector2f& v1, const Vector2f& v2)
{
	return v1*(t-1.0f) + v2*t;
}

inline Vector3f mix(float t, const Vector3f& v1, const Vector3f& v2)
{
	return v1*(t-1.0f) + v2*t;
}

inline Vector4f mix(float t, const Vector4f& v1, const Vector4f& v2)
{
	return v1*(t-1.0f) + v2*t;
}

inline Matrix4f rotateX(float deg)
{
	float s = std::sin(degtorad(deg));
	float c = std::cos(degtorad(deg));

	return Matrix4f(Vector4f(1.0f, 0.0f, 0.0f, 0.0f),
	                Vector4f(0.0f,    c,   -s, 0.0f),
	                Vector4f(0.0f,    s,    c, 0.0f),
	                Vector4f(0.0f, 0.0f, 0.0f, 1.0f)
	               );
}

inline Matrix4f rotateY(float deg)
{
	float s = std::sin(degtorad(deg));
	float c = std::cos(degtorad(deg));

	return Matrix4f(Vector4f(   c, 0.0f,    s, 0.0f),
	                Vector4f(0.0f, 1.0f, 0.0f, 0.0f),
	                Vector4f(  -s, 0.0f,    c, 0.0f),
	                Vector4f(0.0f, 0.0f, 0.0f, 1.0f)
	               );
}

inline Matrix4f rotateZ(float deg)
{
	float s = std::sin(degtorad(deg));
	float c = std::cos(degtorad(deg));

	return Matrix4f(Vector4f(   c,   -s, 0.0f, 0.0f),
	                Vector4f(   s,    c, 0.0f, 0.0f),
	                Vector4f(0.0f, 0.0f, 1.0f, 0.0f),
	                Vector4f(0.0f, 0.0f, 0.0f, 1.0f)
	               );
}

inline Matrix4f translate(const Vector4f& offset)
{
	return Matrix4f(
	            Vector4f(1.0f, 0.0f, 0.0f, offset.x),
	            Vector4f(0.0f, 1.0f, 0.0f, offset.y),
	            Vector4f(0.0f, 0.0f, 1.0f, offset.z),
	            Vector4f(0.0f, 0.0f, 0.0f,     1.0f)
	       );
}

/*
inline Matrix4f perspective(float fov, float aspect, float near, float far)
{
	// Restrict fov to 179 degrees, for numerical stability
	if(fov > 179.0f)
		fov = 179.0f;
	float f = 1.0f / std::tan(degtorad(fov) * 0.5f);
	float y = f*aspect;
	float x = f;
	float z1 = (far+near)/(near-far);
	float z2 = (2.0f*far*near)/(near-far);
	Matrix4f m(Vector4f(x, 0,  0,  0),
	           Vector4f(0, y,  0,  0),
	           Vector4f(0, 0, z1, z2),
	           Vector4f(0, 0,  -1,  0)
	          );
	return m;
}
*/
inline Matrix4f perspective(float fov, float aspect, float near, float far)
{
    /* Restrict fov to 179 degrees, for numerical stability */
    if(fov >= 180.0f)
        fov = 179.0f;
    
    float f = 1.0f / std::tan(degtorad(fov) * 0.5f);
	float x = f; //* aspect;
    float y = f * aspect;
    float z1 = (far+near)/(near-far);
    float z2 = (2.0f*far*near)/(near-far);
    Matrix4f m(Vector4f(x, 0,  0,  0),
               Vector4f(0, y,  0,  0),
               Vector4f(0, 0, z1, z2),
               Vector4f(0, 0,  -1,  0)
               );
    return m;
}

inline Vector4f project(const Vector4f& v, float width, float height)
{
	Vector4f proj;
	float centerX = width*0.5f;
	float centerY = height*0.5f;
	float wInv = 1.0f / v.w;
	/* perspective divide */
	proj.x = v.x * wInv;
	proj.y = v.y * wInv;
	proj.z = v.z * wInv;
	proj.x = proj.x*centerX + centerX;
	proj.y = proj.y*centerY + centerY;
	proj.z = proj.z*0.5f + 0.5f;
	proj.w = v.w; //wInv;
	return proj;
}


/***********************************************************************
 ******************** vector- and matrix functions for POD types *******
 ********************************************************************* */

inline void Mat4Mat4Mul(MatrixPOD4f& dst, const MatrixPOD4f& m0, const MatrixPOD4f& m1)
{
	MatrixPOD4f tmp;
	memset(tmp, 0, sizeof(float) * 16);
	for(int i=0; i<4; ++i) {
		for(int j=0; j<4; ++j) {
			//tmp[j+i*4] = 0;
			for(int k=0; k<4; ++k) {
				tmp[j+i*4] += m0[k + i*4]*m1[j + k*4];
			}
		}
	}
	memcpy(dst, tmp, sizeof(float) * 16);
}

template<class T>
VectorPOD3<T> Mat4Vec3Mul(const MatrixPOD4f& mat, const VectorPOD3<T>& v)
{
	VectorPOD3<T> r;
	r.x = v.x*mat[ 0] + v.y*mat[ 1] + v.z*mat[ 2] + mat[ 3];
	r.y = v.x*mat[ 4] + v.y*mat[ 5] + v.z*mat[ 6] + mat[ 7];
	r.z = v.x*mat[ 8] + v.y*mat[ 9] + v.z*mat[10] + mat[11];
	return r;
}

template<class T>
VectorPOD4<T> Mat4Vec4Mul(const MatrixPOD4f& mat, const VectorPOD4<T>& v)
{
	VectorPOD4<T> r;
	r.x = v.x*mat[ 0] + v.y*mat[ 1] + v.z*mat[ 2] + v.w*mat[ 3];
	r.y = v.x*mat[ 4] + v.y*mat[ 5] + v.z*mat[ 6] + v.w*mat[ 7];
	r.z = v.x*mat[ 8] + v.y*mat[ 9] + v.z*mat[10] + v.w*mat[11];
	r.w = v.x*mat[12] + v.y*mat[13] + v.z*mat[14] + v.w*mat[15];
	return r;
}

template<class T>
VectorPOD3<T> Mat3Vec3Mul(const MatrixPOD3f& mat, const VectorPOD3<T>& v)
{
	VectorPOD3<T> r;
	r.x = v.x*mat[ 0] + v.y*mat[ 1] + v.z*mat[ 2];
	r.y = v.x*mat[ 3] + v.y*mat[ 4] + v.z*mat[ 5];
	r.z = v.x*mat[ 6] + v.y*mat[ 7] + v.z*mat[ 8];
	return r;
}

template<class T>
VectorPOD3<T> Mat3Vec4Mul(const MatrixPOD3f& mat, const VectorPOD4<T>& v)
{
	VectorPOD4<T> r;
	r.x = v.x*mat[ 0] + v.y*mat[ 1] + v.z*mat[ 2];
	r.y = v.x*mat[ 3] + v.y*mat[ 4] + v.z*mat[ 5];
	r.z = v.x*mat[ 6] + v.y*mat[ 7] + v.z*mat[ 8];
	return r;
}

template<class T> T distance(const VectorPOD3<T>& v1, const VectorPOD3<T>& v2)
{
	VectorPOD3<T> tmp;
	tmp.x = v2.x-v1.x;
	tmp.y = v2.y-v1.y;
	tmp.z = v2.z-v1.z;
	return std::sqrt(tmp.x*tmp.x + tmp.y*tmp.y + tmp.z*tmp.z);
}

template<class T> T distance(const VectorPOD4<T>& v1, const VectorPOD4<T>& v2)
{
	VectorPOD4<T> tmp;
	tmp.x = v2.x-v1.x;
	tmp.y = v2.y-v1.y;
	tmp.z = v2.z-v1.z;
	return std::sqrt(tmp.x*tmp.x + tmp.y*tmp.y + tmp.z*tmp.z);
}

/* Linear interpolation */

inline VectorPOD2f mix(float t, const VectorPOD2f& v1, const VectorPOD2f& v2)
{
	VectorPOD2f r;
	r.x = v1.x + (v2.x - v1.x)*t;
	r.y = v1.y + (v2.y - v1.y)*t;
	return r;
}

inline VectorPOD3f mix(float t, const VectorPOD3f& v1, const VectorPOD3f& v2)
{
	VectorPOD3f r;
	r.x = v1.x + (v2.x - v1.x)*t;
	r.y = v1.y + (v2.y - v1.y)*t;
	r.z = v1.z + (v2.z - v1.z)*t;
	return r;
}

inline VectorPOD4f mix(float t, const VectorPOD4f& v1, const VectorPOD4f& v2)
{
	VectorPOD4f r;
	r.x = v1.x + (v2.x - v1.x)*t;
	r.y = v1.y + (v2.y - v1.y)*t;
	r.z = v1.z + (v2.z - v1.z)*t;
	r.w = 1.0f;
	return r;
}

inline void rotateX(MatrixPOD4f& dst, float deg)
{
	float s = std::sin(degtorad(deg));
	float c = std::cos(degtorad(deg));

	dst[ 0] = 1.0f;
	dst[ 1] = 0.0f;
	dst[ 2] = 0.0f;
	dst[ 3] = 0.0f;
	dst[ 4] = 0.0f;
	dst[ 5] =    c;
	dst[ 6] =   -s;
	dst[ 7] = 0.0f;
	dst[ 8] = 0.0f;
	dst[ 9] =    s;
	dst[10] =    c;
	dst[11] = 0.0f;
	dst[12] = 0.0f;
	dst[13] = 0.0f;
	dst[14] = 0.0f;
	dst[15] = 1.0f;
}

inline void rotateY(MatrixPOD4f& dst, float deg)
{
	float s = std::sin(degtorad(deg));
	float c = std::cos(degtorad(deg));

	dst[ 0] =    c;
	dst[ 1] = 0.0f;
	dst[ 2] =    s;
	dst[ 3] = 0.0f;
	dst[ 4] = 0.0f;
	dst[ 5] = 1.0f;
	dst[ 6] = 0.0f;
	dst[ 7] = 0.0f;
	dst[ 8] =   -s;
	dst[ 9] = 0.0f;
	dst[10] =    c;
	dst[11] = 0.0f;
	dst[12] = 0.0f;
	dst[13] = 0.0f;
	dst[14] = 0.0f;
	dst[15] = 1.0f;
}

inline void rotateZ(MatrixPOD4f& dst, float deg)
{
	float s = std::sin(degtorad(deg));
	float c = std::cos(degtorad(deg));

	dst[ 0] =    c;
	dst[ 1] =   -s;
	dst[ 2] = 0.0f;
	dst[ 3] = 0.0f;
	dst[ 4] =    s;
	dst[ 5] =    c;
	dst[ 6] = 0.0f;
	dst[ 7] = 0.0f;
	dst[ 8] = 0.0f;
	dst[ 9] = 0.0f;
	dst[10] = 1.0f;
	dst[11] = 0.0f;
	dst[12] = 0.0f;
	dst[13] = 0.0f;
	dst[14] = 0.0f;
	dst[15] = 1.0f;
}

inline void translate(MatrixPOD4f& dst, const VectorPOD4f& offset)
{
	MatrixPOD4f mat = {
		1.0f, 0.0f, 0.0f, offset.x,
		0.0f, 1.0f, 0.0f, offset.y,
		0.0f, 0.0f, 1.0f, offset.z,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	dst[ 0] = 1.0f;
	dst[ 1] = 0.0f;
	dst[ 2] = 0.0f;
	dst[ 3] = offset.x;
	dst[ 4] = 0.0f;
	dst[ 5] = 1.0f;
	dst[ 6] = 0.0f;
	dst[ 7] = offset.y;
	dst[ 8] = 0.0f;
	dst[ 9] = 0.0f;
	dst[10] = 1.0f;
	dst[11] = offset.z;
	dst[12] = 0.0f;
	dst[13] = 0.0f;
	dst[14] = 0.0f;
	dst[15] = 1.0f;
}

inline void perspective(MatrixPOD4f& dst, float fov, float aspect, float near, float far)
{
	/* Restrict fov to 179 degrees, for numerical stability */
	if(fov > 179.0f)
		fov = 179.0f;
	float f = 1.0f / std::tan(degtorad(fov) * 0.5f);
	float y = f*aspect;
	float x = f;
	float z1 = (far+near)/(near-far);
	float z2 = (2.0f*far*near)/(near-far);

	dst[ 0] = x;
	dst[ 1] = 0.0f;
	dst[ 2] = 0.0f;
	dst[ 3] = 0.0f;
	dst[ 4] = 0.0f;
	dst[ 5] = y;
	dst[ 6] = 0.0f;
	dst[ 7] = 0.0f;
	dst[ 8] = 0.0f;
	dst[ 9] = 0.0f;
	dst[10] = z1;
	dst[11] = z2;
	dst[12] = 0.0f;
	dst[13] = 0.0f;
	dst[14] =-1.0f;
	dst[15] = 0.0f;
}

inline VectorPOD4f project(const VectorPOD4f& v, float width, float height)
{
	VectorPOD4f proj;
	float centerX = width*0.5f;
	float centerY = height*0.5f;
	float wInv = 1.0f / v.w;
	/* perspective divide */
	proj.x = v.x * wInv;
	proj.y = v.y * wInv;
	proj.z = v.z * wInv;
	proj.x = proj.x*centerX + centerX;
	proj.y = proj.y*centerY + centerY;
	proj.z = proj.z*0.5f + 0.5f;
	proj.w = v.w; //wInv;
	return proj;
}



/***********************************************************************
 ************** Assembly optimizations (Currently for x86)**************
 ********************************************************************* */
inline int iround(float f)
{
	int r;

	asm volatile ("fistpl %[output]\n"
	              : [output] "=m" (r)
	              : [input] "t" (f)
	              : "st(0)");

	return r;
}

inline int fpceil15(int fp)
{
	return (fp & 32767) ? ((fp & ~32767) + 32768) : fp;
}

inline int fpceil10(int fp)
{
	return (fp & 1023) ? ((fp & ~1023) + 1024) : fp;
}


/* NEW for X86, fix gcc bug. Reci computes 1/int */
int reci15(int val);
int reci8(int val);
int reci11(int val);
#endif
