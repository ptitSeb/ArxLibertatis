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

#ifndef ARX_MATH_VECTOR2FNEON_H
#define ARX_MATH_VECTOR2FNEON_H

#ifdef __ARM_NEON__

#include <limits>
#include <cmath>

#include "math/MathFwd.h"

#include <arm_neon.h>

/*!
 * Representation of a vector in 2d space.
 * Specialized float for NEON class
 * @brief 2x1 Vector class.
 */
class Vec2f {
	
public:
	/*!
	 * Constructor.
	 */
	Vec2f() {}
	
	/*!
	 * Constructor accepting initial values.
	 * @param fX A T representing the x-axis.
	 * @param fY A T representing the y-axis.
	 */
	Vec2f(float pX, float pY) : x(pX), y(pY) { }
	
	/*!
	 * Constructor accepting initial values.
	 * @param fXY A float32x2_t representing the x and y axis.
	 */
	Vec2f(float32x2_t pXY) : xy(pXY) { }

	/*!
	 * Copy constructor.
	 * @param other A vector to be copied.
	 */
	Vec2f(const Vec2f & other) : xy(other.xy) { }
	
	/*!
	 * Conversion constructor.
	 */
	template <typename U>
	explicit Vec2f(const Vector2<U> & other) : x(other.x), y(other.y) { }
	
	/*!
	 * Set this vector to the content of another vector.
	 * @brief Assignement operator.
	 * @param other A vector to be copied.
	 * @return Reference to this vector object.
	 */
	Vec2f & operator=(const Vec2f & other) {
		xy=other.xy;
		return *this;
	}
	template <typename U>
	Vec2f & operator=(const Vector2<U> & other) {
		x=other.x; y=other.y;
		return *this;
	}
	
	/*!
	 * Test if this vector is equal to another vector.
	 * @brief Equal operator.
	 * @param other A vector to be compared to.
	 * @return A boolean, \b true if the two vector are equal(all members are equals), or \b false otherwise.
	 */
	bool operator==(const Vec2f & other) const {
		uint32x2_t res = vceq_f32(xy, other.xy);
		return (vget_lane_u32(res, 0) & vget_lane_u32(res, 1)) ;
	}
	
	/*!
	 * Test if this vector is not equal to another vector.
	 * @brief Not equal operator.
	 * @param other A vector to be compared to.
	 * @return A boolean, \b true if the two vector are not equal(all members are not equal), or \b false otherwise.
	 */
	bool operator!=(const Vec2f & other) const {
		return !((*this) == other);
	}
	
	/*!
	 * Invert the sign of the vector.
	 * @brief Unary minus operator.
	 * @return A new vector, same as this one but with the signs of all the elements inverted.
	 */
	Vec2f operator-() const {
		return Vec2f(vneg_f32(xy));
	}
	
	/*!
	 * Add a vector to this vector.
	 * @brief Addition operator.
	 * @param other a vector, to be added to this vector.
	 * @return A new vector, the result of the addition of the two vector.
	 */
	Vec2f operator+(const Vec2f & other) const {
		return Vec2f(vadd_f32(xy, other.xy));
	}
	
	/*!
	 * Substract a vector to this vector.
	 * @brief Substraction operator.
	 * @param other a vector, to be substracted to this vector.
	 * @return A new vector, the result of the substraction of the two vector.
	 */
	Vec2f operator-(const Vec2f & other) const {
		return Vec2f(vsub_f32(xy, other.xy));
	}
	
	/*!
	 * Multiply a vector to this vector.
	 * @brief Multiplication operator.
	 * @param other a vector, to be Multiplied to this vector.
	 * @return A new vector, the result of the Multiplication of the two vector.
	 */
	Vec2f operator*(const Vec2f & other) const {
		return Vec2f(vmul_f32(xy, other.xy));
	}
	
	/*!
	 * Multiply a vector to this vector.
	 */
	Vec2f & operator*=(const Vec2f & other) {
		xy=vmul_f32(xy, other.xy);
		return *this;
	}

	/*!
	 * Divide this vector by a scale factor.
	 * @brief Division operator for a scalar.
	 * @param scale value to divide this vector by.
	 * @return A new vector, the result of the division.
	 */
	Vec2f operator/(float scale) const {
		return Vec2f(vmul_n_f32(xy, 1.0f/scale));
	}
	
	/*!
	 * Multiply this vector by a scalar.
	 * @brief Multiplication operator for a scalar.
	 * @param scale The vector will be multiplied by this value.
	 * @return A new vector which is the result of the operation.
	 */
	Vec2f operator*(float scale) const {
		return Vec2f(vmul_n_f32(xy, scale));
	}
	
	/*!
	 * Add the content of another vector to this vector.
	 * @brief Addition assignment operator for a vector.
	 * @param other The vector to add to this vector.
	 * @return A const reference to this vector.
	 */
	const Vec2f & operator+=(const Vec2f & other) {
		xy= vadd_f32(xy, other.xy);
		return *this;
	}
	
	/*!
	 * Substract the content of another vector to this vector.
	 * @brief Substraction assigment operator for a vector.
	 * @param other The vector to substract from this vector.
	 * @return A const reference to this vector.
	 */
	const Vec2f & operator-=(const Vec2f & other) {
		xy=vsub_f32(xy, other.xy);
		return *this;
	}
	
	/*!
	 * Divide this vector by a factor.
	 * @brief Division assigment operator for a scalar.
	 * @param scale Value to be used for the division.
	 * @return A const reference to this vector.
	 */
	const Vec2f & operator/=(float scale) {
		xy=vmul_n_f32(xy, 1.0f/scale);
		return *this;
	}
	
	/*!
	 * Multiply this vector by a factor.
	 * @brief Multiplication assigment operator for a scalar.
	 * @param scale Value to be used for the multiplication
	 * @return A const reference to this vector.
	 */
	const Vec2f & operator*=(float scale) {
		xy=vmul_n_f32(xy, scale);
		return *this;
	}
	
	/*!
	 * Access vector elements by their indexes.
	 * @brief Function call operator used to access vector elements.
	 * @param pIndex Index of the element to obtain.
	 * @return A reference to the element at index pIndex.
	 */
	float & operator()(const int & pIndex) {
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
	 * @return Reference to the vector.
	 */
	const Vec2f & normalize() {
		
		float length = lengthSqr();
		arx_assert(length != 0);

		//	length = 1.0f/length;
		
		float32x2_t a,b;
		b=vdup_n_f32(length);
		a=vrsqrte_f32(b);
		a=vmul_f32(a,vrsqrts_f32(vmul_f32(b,a), a));
//		a=vmul_f32(a,vrsqrts_f32(vmul_f32(a,a), b));

		xy = vmul_f32(xy, a);
		return *this;
	}
	
	/*!
	 * Create a normalized copy of this vector(Divide by its length).
	 * @brief Create a normalized copy of this vector.
	 * @return A normalized copy of the vector.
	 */
	Vec2f getNormalized() const {
		float length = lengthSqr();
		arx_assert(length != 0);
		float32x2_t a,b;
		b=vdup_n_f32(length);
		a=vrsqrte_f32(b);
		a=vmul_f32(a,vrsqrts_f32(vmul_f32(b,a), a));
		return Vec2f(vmul_f32(xy, a));
		//a=vmul_f32(a,vrsqrts_f32(b, vmul_f32(a,a)));*/	// some precisions issue here it seems

		//return Vec2f(vmul_n_f32(xy, 1.0f/sqrt(length)));
	}
	
	/*!
	 * Returns true if the vector is normalized, false otherwise.
	 */
	bool normalized() const {
		return lengthSqr() == 1;
	}
	
	/*!
	 * Get the length of this vector.
	 * @return The length of this vector.
	 */
	float length() const {
		float32x2_t a=vmul_f32(xy, xy);
		return sqrt(vget_lane_f32(vpadd_f32(a,a),0));
	}
	
	/*!
	 * Get the squared length of this vector.
	 * @return The squared length of this vector.
	 */
	float lengthSqr() const {
		float32x2_t a=vmul_f32(xy, xy);
		return vget_lane_f32(vpadd_f32(a,a),0);
	}
	
	/*!
	 * Get the distance between two vectors.
	 * @param other The other vector.
	 * @return The distance between the two vectors.
	 */
	float distanceFrom(const Vec2f & other) const {
		return Vec2f(other - *this).length();
	}
	
	float distanceFromSqr(const Vec2f & other) const {
		return Vec2f(other - *this).lengthSqr();
	}
	
	/*!
	 * Check if two vector are equals using an epsilon.
	 * @param other The other vector.
	 * @param pEps The epsilon value.
	 * @return \bTrue if the vectors values fit in the epsilon range.
	 */
	bool equalEps(const Vec2f & other, float pEps = std::numeric_limits<float>::epsilon()) const {
		float32x2_t fEps=vdup_n_f32(pEps);
		uint32x2_t a=vand_u32(vcgt_f32 (xy, vsub_f32(other.xy, fEps)) , vclt_f32 (xy, vadd_f32(other.xy, fEps)));
		return vget_lane_u32(a, 0) & vget_lane_u32(a, 1);
	}
	
	union {
		float elem[2]; //!< This vector as a 2 elements array.
		struct {
			float x; //!< X component of the vector.
			float y; //!< Y component of the vector.
		};
		float32x2_t xy;	//!< XY NEON Helper
	};
	
	template <typename O>
	Vector2<O> to() const {
		return Vector2<O>(O(x), O(y));
	}
	
	static const Vec2f X_AXIS; //!< The X axis.
	static const Vec2f Y_AXIS; //!< The Y axis.
	static const Vec2f ZERO; //!< A null vector.
	static const Vec2f ONE; //!< A (1, 1) vector.
	
};

inline float dist(const Vec2f & a, const Vec2f & b) {
	return a.distanceFrom(b);
}

inline float distSqr(const Vec2f & a, const Vec2f & b) {
	return a.distanceFromSqr(b);
}

inline bool closerThan(const Vec2f & a, const Vec2f & b, float d) {
	float32x2_t ab=vsub_f32(a.xy, b.xy);
	float32x2_t dd=vdup_n_f32(d);
	ab=vmul_f32(ab, ab);
	uint32x2_t res=vclt_f32(vpadd_f32(ab,ab), vmul_f32(dd, dd));
	return vget_lane_u32(res, 0);
}

inline bool fartherThan(const Vec2f & a, const Vec2f & b, float d) {
	float32x2_t ab=vsub_f32(a.xy, b.xy);
	float32x2_t dd=vdup_n_f32(d);
	ab=vmul_f32(ab, ab);
	uint32x2_t res=vcgt_f32(vpadd_f32(ab,ab), vmul_f32(dd, dd));
	return vget_lane_u32(res, 0);
}

template<class T>
Vector2<T>::Vector2(const Vec2f & other) : x(other.x), y(other.y) { }

#endif	// __ARM_NEON__
#endif // ARX_MATH_VECTOR2_H
