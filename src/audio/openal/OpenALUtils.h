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

#ifndef ARX_AUDIO_OPENAL_OPENALUTILS_H
#define ARX_AUDIO_OPENAL_OPENALUTILS_H

#include <boost/math/special_functions/fpclassify.hpp>

#include <al.h>

#include "math/Vector3.h"

const char * getAlcErrorString(ALenum error);

const char * getAlErrorString(ALenum error);

#define AL_CHECK_ERROR(desc) { ALenum error = alGetError(); \
	if(error != AL_NO_ERROR) { \
		ALError << "error " desc ": " << error << " = " << getAlErrorString(error); \
		return AAL_ERROR_SYSTEM; \
	}}

#define AL_CHECK_ERROR_N(desc, todo) { ALenum error = alGetError(); \
	if(error != AL_NO_ERROR) { \
		ALError << "error " desc ": " << error << " = " << getAlErrorString(error); \
		todo \
	}}

template <class T>
inline bool isallfinite(Vector3<T> vec) {
	return (boost::math::isfinite)(vec.x) && (boost::math::isfinite)(vec.y)  && (boost::math::isfinite)(vec.z);
}

#ifdef __ARM_NEON__
inline bool isallfinite(Vec3f vec) {	//*SEB* *TODO* there must be some NEON magic to do that?!
	return (boost::math::isfinite)(vec.x) && (boost::math::isfinite)(vec.y)  && (boost::math::isfinite)(vec.z);
}
#endif

#endif // ARX_AUDIO_OPENAL_OPENALUTILS_H
