/*
* Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef HAVE_GLES
#include "graphics/opengl/GLVertexBuffer.h"

static GLArrayClientState glArrayClientState = GL_NoArray;
static const void * glArrayClientStateRef = NULL;
static int glArrayClientStateTexCount = 0;

std::vector<GLushort> glShortIndexBuffer;
std::vector<GLuint> glIntIndexBuffer;

void setVertexArrayTexCoord(int index, const void * coord, size_t stride) {

	glClientActiveTexture(GL_TEXTURE0 + index);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, stride, coord);

}

bool switchVertexArray(GLArrayClientState type, const void * ref, int texcount) {

	if(glArrayClientState == type && glArrayClientStateRef == ref) {
		return false;
	}

	if(glArrayClientState != type) {
		for(int i = texcount; i < glArrayClientStateTexCount; i++) {
			glClientActiveTexture(GL_TEXTURE0 + i);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		glArrayClientStateTexCount = texcount;
	}

	glArrayClientState = type;
	glArrayClientStateRef = ref;

	return true;
}
#endif