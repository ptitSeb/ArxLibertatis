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

#ifndef ARX_GRAPHICS_OPENGL_GLNOVERTEXBUFFER_H
#define ARX_GRAPHICS_OPENGL_GLNOVERTEXBUFFER_H

#include "graphics/VertexBuffer.h"
#include "graphics/Vertex.h"
#include "graphics/Math.h"
#include "graphics/opengl/OpenGLRenderer.h"
#include "graphics/opengl/OpenGLUtil.h"

#include <stdio.h>

#define MAX_IDX	65536
/*static GLfloat	tex0[MAX_IDX*2];
static GLfloat	tex1[MAX_IDX*2];
static GLfloat	tex2[MAX_IDX*2];*/
static GLfloat	vtx0[MAX_IDX*4];
static GLubyte	col0[MAX_IDX*4];
static bool		btex1;
static bool		btex2;
/*static int 		idx;*/
/*static GLushort Indices[MAX_IDX];*/


template <class Vertex>
static void renderVertex(const Vertex *vertex, size_t index);

template <>
void renderVertex(const TexturedVertex *vertex, size_t index) {
	Color c = Color::fromBGRA(vertex[index].color);
	col0[index*4+0]=c.r; col0[index*4+1]=c.g; col0[index*4+2]=c.b; col0[index*4+3]=c.a;
	
/*	tex0[idx*2+0]=vertex.uv.x; tex0[idx*2+1]=vertex.uv.y;*/
	
	GLfloat w = 1.0f / vertex[index].rhw; 
	vtx0[index*4+0]=vertex[index].p.x * w; vtx0[index*4+1]=vertex[index].p.y * w; vtx0[index*4+2]=vertex[index].p.z * w; vtx0[index*4+3]=w;
/*	idx++;*/
}

template <>
void renderVertex(const SMY_VERTEX *vertex, size_t index) {
	
	Color c = Color::fromBGRA(vertex[index].color);
	col0[index*4+0]=c.r; col0[index*4+1]=c.g; col0[index*4+2]=c.b; col0[index*4+3]=c.a;
	
/*	tex0[idx*2+0]=vertex.uv.x; tex0[idx*2+1]=vertex.uv.y;
	
	vtx0[index*4+0]=vertex[index].p.x; vtx0[index*4+1]=vertex[index].p.y; vtx0[index*4+2]=vertex[index].p.z; vtx0[index*4+3]=1.0f;
	idx++;*/
}

template <>
void renderVertex(const SMY_VERTEX3 *vertex, size_t index) {
	
	Color c = Color::fromBGRA(vertex[index].color);
	col0[index*4+0]=c.r; col0[index*4+1]=c.g; col0[index*4+2]=c.b; col0[index*4+3]=c.a;
	
/*	tex0[idx*2+0]=vertex.uv[0].x; tex0[idx*2+1]=vertex.uv[0].y;
	tex1[idx*2+0]=vertex.uv[1].x; tex0[idx*2+1]=vertex.uv[1].y;
	tex2[idx*2+0]=vertex.uv[2].x; tex0[idx*2+1]=vertex.uv[2].y;
	
	vtx0[index*4+0]=vertex[index].p.x; vtx0[index*4+1]=vertex[index].p.y; vtx0[index*4+2]=vertex[index].p.z; vtx0[index*4+3]=1.0f;
	idx++;*/
}

template <class Vertex>
static void glBeginVertex(const Vertex *vertex);

/*
template <class Vertex>
static void glEndVertex(const Vertex & vertex);
*/
template <>
void glBeginVertex(const TexturedVertex  *vertex) {
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(4, GL_FLOAT, 0, vtx0);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, col0);
	// only one texture
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
/*	glTexCoordPointer(2, GL_FLOAT, 0, tex0);*/
	glTexCoordPointer(2, GL_FLOAT, sizeof(TexturedVertex), &vertex[0].uv.x);
	
	btex1 = btex2 = false;
	
/*	idx = 0;*/
}
template <>
void glBeginVertex(const SMY_VERTEX *vertex) {
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(SMY_VERTEX), &vertex[0].p.x);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, col0);
	// only 1 textures
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
/*	glTexCoordPointer(2, GL_FLOAT, 0, tex0);*/
	glTexCoordPointer(2, GL_FLOAT, sizeof(SMY_VERTEX), &vertex[0].uv.x);
	btex1 = false;
	btex2 = false;
	
/*	idx = 0;*/
}
template <>
void glBeginVertex(const SMY_VERTEX3 *vertex) {
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(SMY_VERTEX3), &vertex[0].p.x);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, col0);
	// 3 textures
	glClientActiveTexture(GL_TEXTURE2);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
/*	glTexCoordPointer(2, GL_FLOAT, 0, tex2);*/
	glTexCoordPointer(2, GL_FLOAT, sizeof(SMY_VERTEX3), &vertex[0].uv[2].x);
	glClientActiveTexture(GL_TEXTURE1);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
/*	glTexCoordPointer(2, GL_FLOAT, 0, tex1);*/
	glTexCoordPointer(2, GL_FLOAT, sizeof(SMY_VERTEX3), &vertex[0].uv[1].x);
	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
/*	glTexCoordPointer(2, GL_FLOAT, 0, tex0);*/
	glTexCoordPointer(2, GL_FLOAT, sizeof(SMY_VERTEX3), &vertex[0].uv[0].x);
	btex1 = true;
	btex2 = true;
	
/*	idx = 0;*/
}

void glEndVertex(const GLenum what, size_t count)
{
	if (count)
		glDrawArrays(what, 0, count);
	
	if (btex2) {
		glClientActiveTexture(GL_TEXTURE2);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	if (btex1) {
		glClientActiveTexture(GL_TEXTURE1);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	if (btex1 || btex2) {
		glClientActiveTexture(GL_TEXTURE0);
		btex1 = btex2 = false;
	}
}


extern const GLenum arxToGlPrimitiveType[];

template <class Vertex>
class GLNoVertexBuffer : public VertexBuffer<Vertex> {
	
public:
	
	using VertexBuffer<Vertex>::capacity;
	
	GLNoVertexBuffer(OpenGLRenderer * _renderer, size_t capacity) : VertexBuffer<Vertex>(capacity), renderer(_renderer), buffer(new Vertex[capacity]) { }
	
	void setData(const Vertex * vertices, size_t count, size_t offset, BufferFlags flags) {
		ARX_UNUSED(flags);
		
		arx_assert(offset + count <= capacity());
		
		std::copy(vertices, vertices + count, buffer + offset);
	}
	
	Vertex * lock(BufferFlags flags, size_t offset, size_t count) {
		ARX_UNUSED(flags), ARX_UNUSED(count);
		return buffer + offset;
	}
	
	void unlock() {
		// nothing to do
	}
	
	void draw(Renderer::Primitive primitive, size_t count, size_t offset) const {
		
		arx_assert(offset + count <= capacity());
		
		renderer->beforeDraw<Vertex>();
		
		Vertex * pBuf = buffer + offset;
		
		if (count>0) {
			glBeginVertex(pBuf);
			
			for(size_t i = 0; i < count; i++) {
				renderVertex(pBuf, i);
			}
			
			glEndVertex(arxToGlPrimitiveType[primitive], count);
		}
		CHECK_GL;
	}
	
	void drawIndexed(Renderer::Primitive primitive, size_t count, size_t offset, unsigned short * indices, size_t nbindices) const {
		
		arx_assert(offset + count <= capacity());
		arx_assert(indices != NULL);
		
		renderer->beforeDraw<Vertex>();

		Vertex * pBuf = buffer + offset;
		
		if (nbindices>0) {
			glBeginVertex(pBuf);
			
			for(size_t i = 0; i < nbindices; i++) {
				renderVertex(pBuf, indices[i]);
			}
			
			glDrawElements(arxToGlPrimitiveType[primitive], nbindices, GL_UNSIGNED_SHORT, indices);
			
			glEndVertex(arxToGlPrimitiveType[primitive], 0);
		}

		CHECK_GL;
	}
	
	~GLNoVertexBuffer() {
		delete[] buffer;
	};
	
private:
	
	OpenGLRenderer * renderer;
	Vertex * buffer;
	
};

#endif // ARX_GRAPHICS_OPENGL_GLNOVERTEXBUFFER_H
