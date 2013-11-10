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
#ifdef __ARM_NEON__

#include "math/Vector2.h"
#include "math/Vector3.h"


// Constants
const Vec2f Vec2f::X_AXIS(float(1), float(0));
const Vec2f Vec2f::Y_AXIS(float(0), float(1));
const Vec2f Vec2f::ZERO(float(0), float(0));
const Vec2f Vec2f::ONE(float(1), float(1));

// Constants
const Vec3f Vec3f::X_AXIS(float(1), float(0), float(0));
const Vec3f Vec3f::Y_AXIS(float(0), float(1), float(0));
const Vec3f Vec3f::Z_AXIS(float(0), float(0), float(1));
const Vec3f Vec3f::ZERO(float(0), float(0), float(0));
const Vec3f Vec3f::ONE(float(1), float(1), float(1));

#endif