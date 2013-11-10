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
static bool		btex1;
static bool		btex2;

template <class Vertex>
static void calcVertex(Vertex *vertex, size_t offset, size_t count);
template <>
void calcVertex(TexturedVertex *vertex,  size_t offset, size_t count) {
	for (size_t index=offset; index<offset+count; index++) {
#ifdef __ARM_NEON__	
		uint32_t col=vertex[index].color;
		col=(col&0xFF00FF00)|((col&0x00FF0000)>>16)|((col&0x000000FF)<<16);
		float32x4_t a = vmulq_n_f32(vcvtq_f32_u32(vmovl_u16(vget_low_u16(vmovl_u8(vreinterpret_u8_u32(vdup_n_u32(col)))))), 1.0f/255.0f);
		vst1q_f32(&vertex[index].colorf[0], a);
		
		float32x4_t p = vsetq_lane_f32(1.0f, vertex[index].p.xyz0, 3);
//		vst1q_f32(&vertex[index].coord[0], vmulq_f32(p, vdupq_n_f32(1.0f/vertex[index].rhw)));
		float32x2_t wx2 = vdup_n_f32(vertex[index].rhw);
		// w = 1.0f / w;
		float32x2_t aa=vrecpe_f32(wx2);
//        aa=vmul_f32(aa,vrecps_f32(wx2, aa));
        wx2=vmul_f32(aa,vrecps_f32(aa, wx2));
		// x2 => x4
		vst1q_f32(&vertex[index].coord[0], vmulq_f32(p, vcombine_f32(wx2, wx2)));
#else
		Color c = Color::fromBGRA(vertex[index].color);
		vertex[index].colorf[0] = (float)c.r*(1.0f/255.0f);
		vertex[index].colorf[1] = (float)c.g*(1.0f/255.0f);
		vertex[index].colorf[2] = (float)c.b*(1.0f/255.0f);	
		vertex[index].colorf[3] = (float)c.a*(1.0f/255.0f);
		
		GLfloat w = 1.0f / vertex[index].rhw; 
		vertex[index].coord[0]=vertex[index].p.x * w; 
		vertex[index].coord[1]=vertex[index].p.y * w; 
		vertex[index].coord[2]=vertex[index].p.z * w; 
		vertex[index].coord[3]=w;
#endif	
	}
}

template <>
void calcVertex(SMY_VERTEX *vertex,  size_t offset, size_t count) {
	
	for (size_t index=offset; index<offset+count; index++) {
#ifdef __ARM_NEON__
		uint32_t col=vertex[index].color;
		col=(col&0xFF00FF00)|((col&0x00FF0000)>>16)|((col&0x000000FF)<<16);
		float32x4_t a = vmulq_n_f32(vcvtq_f32_u32(vmovl_u16(vget_low_u16(vmovl_u8(vreinterpret_u8_u32(vdup_n_u32(col)))))), 1.0f/255.0f);
		vst1q_f32(&vertex[index].colorf[0], a);
#else
		Color c = Color::fromBGRA(vertex[index].color);
		vertex[index].colorf[0] = (float)c.r*(1.0f/255.0f);
		vertex[index].colorf[1] = (float)c.g*(1.0f/255.0f);
		vertex[index].colorf[2] = (float)c.b*(1.0f/255.0f);	
		vertex[index].colorf[3] = (float)c.a*(1.0f/255.0f);
#endif
	}
}

template <>
void calcVertex(SMY_VERTEX3 *vertex,  size_t offset, size_t count) {
	
	for (size_t index=offset; index<offset+count; index++) {
#ifdef __ARM_NEON__
		uint32_t col=vertex[index].color;
		col=(col&0xFF00FF00)|((col&0x00FF0000)>>16)|((col&0x000000FF)<<16);
		float32x4_t a = vmulq_n_f32(vcvtq_f32_u32(vmovl_u16(vget_low_u16(vmovl_u8(vreinterpret_u8_u32(vdup_n_u32(col)))))), 1.0f/255.0f);
		vst1q_f32(&vertex[index].colorf[0], a);
#else
		Color c = Color::fromBGRA(vertex[index].color);
		vertex[index].colorf[0] = (float)c.r*(1.0f/255.0f);
		vertex[index].colorf[1] = (float)c.g*(1.0f/255.0f);
		vertex[index].colorf[2] = (float)c.b*(1.0f/255.0f);	
		vertex[index].colorf[3] = (float)c.a*(1.0f/255.0f);
#endif
	}
	
}




template <class Vertex>
static void renderVertex(Vertex *vertex, size_t index);

template <>
void renderVertex(TexturedVertex *vertex, size_t index) {
	Color c = Color::fromBGRA(vertex[index].color);
	vertex[index].colorf[0] = (float)c.r*(1.0f/255.0f);
	vertex[index].colorf[1] = (float)c.g*(1.0f/255.0f);
	vertex[index].colorf[2] = (float)c.b*(1.0f/255.0f);	
	vertex[index].colorf[3] = (float)c.a*(1.0f/255.0f);
	
	
	GLfloat w = 1.0f / vertex[index].rhw; 
	vertex[index].coord[0]=vertex[index].p.x * w; 
	vertex[index].coord[1]=vertex[index].p.y * w; 
	vertex[index].coord[2]=vertex[index].p.z * w; 
	vertex[index].coord[3]=w;
}

template <>
void renderVertex(SMY_VERTEX *vertex, size_t index) {
	
	Color c = Color::fromBGRA(vertex[index].color);
	vertex[index].colorf[0] = (float)c.r*(1.0f/255.0f);
	vertex[index].colorf[1] = (float)c.g*(1.0f/255.0f);
	vertex[index].colorf[2] = (float)c.b*(1.0f/255.0f);	
	vertex[index].colorf[3] = (float)c.a*(1.0f/255.0f);
	
}

template <>
void renderVertex(SMY_VERTEX3 *vertex, size_t index) {
	
	Color c = Color::fromBGRA(vertex[index].color);
	vertex[index].colorf[0] = (float)c.r*(1.0f/255.0f);
	vertex[index].colorf[1] = (float)c.g*(1.0f/255.0f);
	vertex[index].colorf[2] = (float)c.b*(1.0f/255.0f);	
	vertex[index].colorf[3] = (float)c.a*(1.0f/255.0f);
	
}

template <class Vertex>
static void glBeginVertex(Vertex *vertex);

template <>
void glBeginVertex(TexturedVertex  *vertex) {
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(4, GL_FLOAT, sizeof(TexturedVertex), &vertex[0].coord[0]);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_FLOAT, sizeof(TexturedVertex), &vertex[0].colorf[0]);
	// only one texture
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(TexturedVertex), &vertex[0].uv.x);
	
	btex1 = btex2 = false;
	
}
template <>
void glBeginVertex(SMY_VERTEX *vertex) {
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(SMY_VERTEX), &vertex[0].p.x);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_FLOAT, sizeof(SMY_VERTEX), &vertex[0].colorf[0]);
	// only 1 textures
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(SMY_VERTEX), &vertex[0].uv.x);
	btex1 = false;
	btex2 = false;
	
}
template <>
void glBeginVertex(SMY_VERTEX3 *vertex) {
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(SMY_VERTEX3), &vertex[0].p.x);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_FLOAT, sizeof(SMY_VERTEX3), &vertex[0].colorf[0]);
	// 3 textures
	glClientActiveTexture(GL_TEXTURE2);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(SMY_VERTEX3), &vertex[0].uv[2].x);
	glClientActiveTexture(GL_TEXTURE1);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(SMY_VERTEX3), &vertex[0].uv[1].x);
	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(SMY_VERTEX3), &vertex[0].uv[0].x);
	btex1 = true;
	btex2 = true;
	
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
			
			calcVertex(buffer, offset, count);
/*			
			for(size_t i = 0; i < count; i++) {
				renderVertex(pBuf, i);
			}
*/			
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

			calcVertex(buffer, offset, count);
/*			
			for(size_t i = 0; i < nbindices; i++) {
				renderVertex(pBuf, indices[i]);
			}
*/			
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
