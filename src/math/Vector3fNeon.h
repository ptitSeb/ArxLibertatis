/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARX_MATH_VECTOR3FNEON_H
#define ARX_MATH_VECTOR3FNEON_H

#ifdef __ARM_NEON__

#include <limits>
#include <cmath>
#include <algorithm>

#include "math/MathFwd.h"

#include <arm_neon.h>

/*!
 * Compute the cross product of two vectors.
 * @brief Cross product.
 * @return A new vector that is the result of the cross product of the two vectors.
 */
Vec3f cross(const Vec3f & a, const Vec3f & b);

// This code is from user JesseT from the gp2x board. almost 2* than C or Intrinsics
static uint32x4_t const cross3_mask = { 0x00000000, 0x80000000, 0x00000000, 0x00000000 };

inline __attribute__((always_inline)) float32x4_t cross3_neon_rev_dup_asm(float32x4_t a, float32x4_t b) {
    register float32x4_t v1 asm("q0") = a;
    register float32x4_t v2 asm("q2") = b;
    register float32x4_t result asm("q9");
    
    asm volatile(
        "vrev64.32 d3, d0             \n\t" // d0,d3 = xy1, yx1
        "vrev64.32 d7, d4             \n\t" // d4,d7 = xy2, yx2
        "vdup.32 d2, d1[0]            \n\t" // q1 = v1.zzy
        "vdup.32 d6, d5[0]            \n\t" // q3 = v2.zzy
        "vmov    d1, d0               \n\t"
        "vmov    d0, d3               \n\t" // q0 = v1.yxx
        "vmov    d5, d4               \n\t"
        "vmov    d4, d7               \n\t" // q2 = v2.yxx
        "vmul.f32 q0, q0, q3          \n\t" // q0 = v1.yxx * v2.zzy
        "vldmia  %[mask], {d14-d15}   \n\t"
        "vmls.f32 q0, q1, q2          \n\t" // q0 = q0 - v1.zzy * v2.xxy
		"vmov      s31, s3			  \n\t"
        "veor    q9, q0, q7"                // result = q0 * { 1, -1, 1 }
        : "=w" (v1), "=w" (v2), "=w" (result)
        : "0" (v1), "1" (v2), [mask] "r" (&cross3_mask)
        : "q1", "q3", "q7"
    );

    return result;
}

/*!
 * Compute the dot product of two vectors.
 * @brief Dot product of two vectors.
 * @return Result of the dot product of the two vectors.
 */
float dot(const Vec3f & a, const Vec3f & b);

/*!
 * Representation of a vector in 3d space.
 * @brief 3x1 Vector class.
 */
class Vec3f {
	
public:
	
	/*!
	 * Constructor.
	 */
	Vec3f() :zero(0) {}
	
	/*!
	 * Constructor accepting initial values.
	 * @param fX A T representing the x-axis.
	 * @param fY A T representing the y-axis.
	 * @param fZ A T representing the z-axis.
	 */
	Vec3f(float pX, float pY, float pZ) : x(pX), y(pY), z(pZ), zero(0) { }
	
	static Vec3f repeat(float value) {
		return Vec3f(value, value, value);
	}
	
	/*!
	 * Copy constructor.
	 * @param other A vector to be copied.
	 */
	Vec3f(const Vec3f & other) : xyz0(other.xyz0) { }
	/*!
	 * Constructor accepting initial values.
	 * @param fXY A float32x2_t representing the x and y axis.
	 */
	Vec3f(float32x2_t pXY, float32x2_t pZ0) : xy(pXY), z0(pZ0) { }
	Vec3f(float32x4_t pXYZ0) : xyz0(pXYZ0) { }

	/*!
	 * Conversion constructor.
	 */
	template <typename U>
	explicit Vec3f(const Vector3<U> & other) : x(other.x), y(other.y), z(other.z), zero(0) { }
	
	/*!
	 * Set this vector to the content of another vector.
	 * @brief Assignement operator.
	 * @param other A vector to be copied.
	 * @return Reference to this vector object.
	 */
	Vec3f & operator=(const Vec3f & other) {
		xyz0 = other.xyz0;
		return *this;
	}
	
	/*!
	 * Test if this vector is equal to another vector.
	 * @brief Equal operator.
	 * @param other A vector to be compared to.
	 * @return A boolean, \b true if the two vector are equal(all members are equals), or \b false otherwise.
	 */
	bool operator==(const Vec3f & other) const {
		uint32x4_t res = vceqq_f32(xyz0, other.xyz0);
		return (vgetq_lane_u32(res, 0) & vgetq_lane_u32(res, 1) & vgetq_lane_u32(res, 2)) ;
	}
	
	/*!
	 * Test if this vector is not equal to another vector.
	 * @brief Not equal operator.
	 * @param other A vector to be compared to.
	 * @return A boolean, \b true if the two vector are not equal(all members are not equal), or \b false otherwise.
	 */
	bool operator!=(const Vec3f & other) const {
		return !((*this) == other);
	}
	
	bool operator<(const Vec3f & other) const {
		uint32x4_t res = vcltq_f32(xyz0, other.xyz0);
		return (vgetq_lane_u32(res, 0) & vgetq_lane_u32(res, 1) & vgetq_lane_u32(res, 2)) ;
	}
	
	bool operator>(const Vec3f & other) const {
		uint32x4_t res = vcgtq_f32(xyz0, other.xyz0);
		return (vgetq_lane_u32(res, 0) & vgetq_lane_u32(res, 1) & vgetq_lane_u32(res, 2)) ;
	}
	
	/*!
	 * Invert the sign of the vector.
	 * @brief Unary minus operator.
	 * @return A new vector, same as this one but with the signs of all the elements inverted.
	 */
	Vec3f operator-() const {
		return Vec3f(vnegq_f32(xyz0));
	}
	
	/*!
	 * Add a vector to this vector.
	 * @brief Addition operator.
	 * @param other a vector, to be added to this vector.
	 * @return A new vector, the result of the addition of the two vector.
	 */
	Vec3f operator+(const Vec3f & other) const {
		return Vec3f(vaddq_f32(xyz0, other.xyz0));
	}
	

	Vec3f operator*(const Vec3f & other) const {
		return Vec3f(vmulq_f32(xyz0, other.xyz0));
	}
	
	/*!
	 * Substract a vector to this vector.
	 * @brief Substraction operator.
	 * @param other a vector, to be substracted to this vector.
	 * @return A new vector, the result of the substraction of the two vector.
	 */
	Vec3f operator-(const Vec3f & other) const {
		return Vec3f(vsubq_f32(xyz0, other.xyz0));
	}
	
	/*!
	 * Divide this vector by a scale factor.
	 * @brief Division operator for a scalar.
	 * @param scale value to divide this vector by.
	 * @return A new vector, the result of the division.
	 */
	Vec3f operator/(float scale) const {
		return Vec3f(vmulq_n_f32(xyz0, 1.0f/scale));
	}
	
	/*!
	 * Multiply this vector by a scalar.
	 * @brief Multiplication operator for a scalar.
	 * @param scale The vector will be multiplied by this value.
	 * @return A new vector which is the result of the operation.
	 */
	Vec3f operator*(float scale) const {
		return Vec3f(vmulq_n_f32(xyz0, scale));
	}
	
	/*!
	 * Add the content of another vector to this vector.
	 * @brief Addition assignment operator for a vector.
	 * @param other The vector to add to this vector.
	 * @return A const reference to this vector.
	 */
	const Vec3f & operator+=(const Vec3f & other) {
		xyz0=vaddq_f32(xyz0, other.xyz0);
		return *this;
	}
	
	const Vec3f & operator*=(const Vec3f & other) {
		xyz0=vmulq_f32(xyz0, other.xyz0);
		return *this;
	}
	
	/*!
	 * Substract the content of another vector to this vector.
	 * @brief Substraction assigment operator for a vector.
	 * @param other The vector to substract from this vector.
	 * @return A const reference to this vector.
	 */
	const Vec3f & operator-=(const Vec3f & other) {
		xyz0=vsubq_f32(xyz0, other.xyz0);
		return *this;
	}
	
	/*!
	 * Divide this vector by a factor.
	 * @brief Division assigment operator for a scalar.
	 * @param scale Value to be used for the division.
	 * @return A const reference to this vector.
	 */
	const Vec3f & operator/=(float scale) {
		xyz0=vmulq_n_f32(xyz0, 1.0f/scale);
		return *this;
	}
	
	/*!
	 * Multiply this vector by a factor.
	 * @brief Multiplication assigment operator for a scalar.
	 * @param scale Value to be used for the multiplication
	 * @return A const reference to this vector.
	 */
	const Vec3f & operator *=(float scale) {
		xyz0=vmulq_n_f32(xyz0, scale);
		return *this;
	}
	
	/*!
	 * Access vector elements by their indexes.
	 * @brief Subscript operator used to access vector elements(const).
	 * @param pIndex Index of the element to obtain.
	 * @return A reference to the element at index pIndex.
	 */
	float operator()(const int& pIndex) const {
		return elem[pIndex];
	}
	
	/*!
	 * Access vector elements by their indexes.
	 * @brief Subscript operator used to access vector elements.
	 * @param pIndex Index of the element to obtain.
	 * @return A reference to the element at index pIndex.
	 */
	float & operator()(const int& pIndex) {
		return elem[pIndex];
	}
	
	/*!
	 * Access to the internal array of the vector.
	 * @brief Indirection operator(const).
	 * @return Internal array used to store the vector values.
	 */
	operator const float*() const {
		return elem;
	}
	
	/*!
	 * Access to the internal array of the vector.
	 * @brief Indirection operator.
	 * @return Internal array used to store the vector values.
	 */
	operator float*() {
		return elem;
	}
	
	/*!
	 * Normalize the vector(divide by its length).
	 * @brief Normalize the vector.
	 * @return The old magnitude.
	 */
	float normalize() {
		
		float magnitude = length();
		float scalar;
		if(magnitude > 0.0f) {
			scalar = 1.0f / magnitude;
		} else {
			scalar = 0;
		}
		
		xyz0 = vmulq_n_f32(xyz0, scalar);
		
		return magnitude;
	}
	
	/*!
	 * Create a normalized copy of this vector(Divide by its length).
	 * @brief Create a normalized copy of this vector.
	 * @return A normalized copy of the vector.
	 */
	Vec3f getNormalized() const {
		
		float scalar = length();
		if(scalar > 0) {
			scalar = 1.0f / scalar;
		} else {
			scalar = 0;
		}
		
		return Vec3f(vmulq_n_f32(xyz0, scalar));
	}
	
	/*!
	 * Returns true if the vector is normalized, false otherwise.
	 */
	bool normalized() const {
		return lengthSqr() == 1;
	}
	
	/*!
	 * Build a vector using 2 angles in the x and y planes.
	 * @param pAngleX Rotation around the X axis, in randians.
	 * @param pAngleY Rotation around the Y axis, in radians.
	 * @return This vector pointing in the right direction.
	 */
	static Vec3f fromAngle(float pAngleX, float pAngleY) {
		
		Vec3f res(cos(pAngleY), sin(pAngleX), sin(pAngleY));
		res.normalize();
		
		return res;
	}
	
	/*!
	 * Get the length of this vector.
	 * @return The length of this vector.
	 */
	float length() const {
		return sqrt(lengthSqr());
	}
	
	/*!
	 * Get the squared length of this vector.
	 * @return The squared length of this vector.
	 */
	float lengthSqr() const {
		float32x4_t a=vmulq_f32(xyz0, xyz0);
		float32x2_t res=vadd_f32(vget_low_f32(a), vget_high_f32(a));
		return vget_lane_f32(vpadd_f32(res, res), 0);
	}
	
	/*!
	 * Get the distance between two vectors.
	 * @param other The other vector.
	 * @return The distance between the two vectors.
	 */
	float distanceFrom(const Vec3f & other) const {
		return Vec3f(other - *this).length();
	}
	
	float distanceFromSqr(const Vec3f & other) const {
		return Vec3f(other - *this).lengthSqr();
	}
	
	/*!
	 * Get the angle, in radian, between two vectors.
	 * @param other The other vector.
	 * @return The angle between the two vectors, in radians.
	 */
	float angleBetween(const Vec3f & other) const {
		return acos(dot(*this, other) /(length()*other.length()));
	}
	
	/*!
	 * Check if two vector are equals using an epsilon.
	 * @param other The other vector.
	 * @param pEps The epsilon value.
	 * @return \bTrue if the vectors values fit in the epsilon range.
	 */
	bool equalEps(const Vec3f & other, float pEps = std::numeric_limits<float>::epsilon()) const {
		float32x4_t fEps=vdupq_n_f32(pEps);
		uint32x4_t res=vandq_u32(vcgtq_f32 (xyz0, vsubq_f32(other.xyz0, fEps)) , vcltq_f32 (xyz0, vaddq_f32(other.xyz0, fEps)));
		return (vgetq_lane_u32(res, 0) & vgetq_lane_u32(res, 1) & vgetq_lane_u32(res, 2)) ;
	}
	
	union {
		float elem[4]; //!< This vector as a 3 elements array.
		struct {
			float x; //!< X component of the vector.
			float y; //!< Y component of the vector.
			float z; //!< Z component of the vector.
			float zero;
		};
		struct {
			float32x2_t	xy;
			float32x2_t	z0;
		};
		float32x4_t	xyz0;
	};
	
	static const Vec3f X_AXIS; //!< The X axis.
	static const Vec3f Y_AXIS; //!< The Y axis.
	static const Vec3f Z_AXIS; //!< The Z axis.
	static const Vec3f ZERO; //!< A null vector.
	static const Vec3f ONE; //!< A (1, 1, 1) vector.
	
};

inline float dist(const Vec3f & a, const Vec3f & b) {
	return a.distanceFrom(b);
}

inline float distSqr(const Vec3f & a, const Vec3f & b) {
	return a.distanceFromSqr(b);
}

inline bool closerThan(const Vec3f & a, const Vec3f & b, float d) {
	return (distSqr(a, b) < (d * d));
}

inline bool fartherThan(const Vec3f & a, const Vec3f & b, float d) {
	return (distSqr(a, b) > (d * d));
}

inline Vec3f cross(const Vec3f & a, const Vec3f & b) {
	//return Vec3f(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
	#if 0
	float32x4_t ayzx0 = {a.y, a.z, a.x, a.zero};
	float32x4_t byzx0 = {b.y, b.z, b.x, b.zero};
	float32x4_t azxy0 = {a.z, a.x, a.y, a.zero};
	float32x4_t bzxy0 = {b.z, b.x, b.y, b.zero};
	return Vec3f(vmlsq_f32(vmulq_f32(ayzx0, bzxy0), azxy0, byzx0));
	#else
	return Vec3f(cross3_neon_rev_dup_asm(a.xyz0, b.xyz0));
	#endif
}

inline float dot(const Vec3f & a, const Vec3f & b) {
	float32x4_t r1 = vmulq_f32(a.xyz0, b.xyz0);
	float32x2_t r2 = vadd_f32(vget_low_f32(r1), vget_high_f32(r1));
	return vget_lane_f32(vpadd_f32(r2, r2), 0);
}

inline Vec3f componentwise_min(Vec3f v0, Vec3f v1) {
	return Vec3f(vminq_f32(v0.xyz0, v1.xyz0));
}
inline Vec3f componentwise_max(Vec3f v0, Vec3f v1) {
	return Vec3f(vmaxq_f32(v0.xyz0, v1.xyz0));
}

#endif // __ARM_NEON__
#endif // ARX_MATH_VECTOR3_H
