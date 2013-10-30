#ifndef MATRIX3_H_GUARD
#define MATRIX3_H_GUARD
#include <algorithm>
#include "vector3.h"

template<class T> struct Matrix3 {
	T m[9];
	Matrix3() {
		std::fill(m, m+9, T(0));
	}
	Matrix3(const Vector3<T>& c1,
	        const Vector3<T>& c2,
	        const Vector3<T>& c3) {
		m[ 0] = c1.x;
		m[ 1] = c1.y;
		m[ 2] = c1.z;
		m[ 3] = c2.x;
		m[ 4] = c2.y;
		m[ 5] = c2.z;
		m[ 6] = c3.x;
		m[ 7] = c3.y;
		m[ 8] = c3.z;
	}

	T operator[](size_t index) const {
		return m[index];
	}

	T& operator[](size_t index) {
		return m[index];
	}

	void identity() {
		std::fill(m, m+9, T(0));
		m[0] = m[4] = m[8] = 1.0f;
	}

	Matrix3<T> operator*(const Matrix3<T>& mat) const {
		Matrix3<T> result;
		for(int i=0; i<3; ++i) {
			for(int j=0; j<3; ++j) {
				for(int k=0; k<3; ++k) {
					result[j+i*3] += m[k + i*3]*mat[j + k*3];
				}
			}
		}
		return result;
	}
};

#endif
