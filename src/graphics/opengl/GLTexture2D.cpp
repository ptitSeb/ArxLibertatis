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

#include "graphics/opengl/GLTexture2D.h"

#include "graphics/Math.h"
#include "graphics/opengl/GLTextureStage.h"
#include "graphics/opengl/OpenGLRenderer.h"

#ifdef HAVE_GLES
#include <stdio.h>
#define GL_NONE		0
#endif

GLTexture2D::GLTexture2D(OpenGLRenderer * _renderer) : renderer(_renderer), tex(GL_NONE) { }
GLTexture2D::~GLTexture2D() {
	Destroy();
}

bool GLTexture2D::Create() {
	
	arx_assert_msg(!tex, "leaking OpenGL texture");
	
	glGenTextures(1, &tex);
	
	// Set our state to the default OpenGL state
	wrapMode = TextureStage::WrapRepeat;
	mipFilter = TextureStage::FilterLinear;
	minFilter = TextureStage::FilterNearest;
	magFilter = TextureStage::FilterLinear;
#ifndef HAVE_GLES
	if(GLEW_ARB_texture_non_power_of_two) {
		storedSize = size;
	} else 
#endif
	{
		storedSize = Vec2i(GetNextPowerOf2(size.x), GetNextPowerOf2(size.y));
	}
	
	CHECK_GL;
	
	return (tex != GL_NONE);
}

void GLTexture2D::Upload() {
	arx_assert(tex != GL_NONE);
	
	glBindTexture(GL_TEXTURE_2D, tex);
	renderer->GetTextureStage(0)->current = this;
#ifdef HAVE_GLES
#define GL_LUMINANCE8			GL_LUMINANCE
#define GL_ALPHA8				GL_ALPHA
#define GL_LUMINANCE8_ALPHA8	GL_LUMINANCE_ALPHA
#define GL_RGB8					GL_RGB
#define	GL_BGR					GL_RGB
#define GL_RGBA8				GL_RGBA
#endif
	
	GLint internal;
	GLenum format;
	if(mFormat == Image::Format_L8) {
		internal = GL_LUMINANCE8, format = GL_LUMINANCE;
	} else if(mFormat == Image::Format_A8) {
		internal = GL_ALPHA8, format = GL_ALPHA;
	} else if(mFormat == Image::Format_L8A8) {
		internal = GL_LUMINANCE8_ALPHA8, format = GL_LUMINANCE_ALPHA;
	} else if(mFormat == Image::Format_R8G8B8) {
		internal = GL_RGB8, format = GL_RGB;
	} else if(mFormat == Image::Format_B8G8R8) {
#ifdef HAVE_GLES
		mImage.ConvertTo(Image::Format_R8G8B8);
		internal = GL_RGB8, format = GL_RGB;
#else
		internal = GL_RGB8, format = GL_BGR;
#endif
	} else if(mFormat == Image::Format_R8G8B8A8) {
		internal = GL_RGBA8, format = GL_RGBA;
	} else if(mFormat == Image::Format_B8G8R8A8) {
#ifdef HAVE_GLES
		mImage.ConvertTo(Image::Format_R8G8B8A8);
		internal = GL_RGBA8, format = GL_RGBA;
#else
		internal = GL_RGBA8, format = GL_BGRA;
#endif
	} else {
		arx_assert_msg(false, "Unsupported image format");
		return;
	}
	
#if 0
printf("GLTexture2D::Upload (%ix%i), Mipmaps=%s, Format=%s\n", size.x, size.y, 
   (hasMipmaps())?"yes":"no",
   (format==GL_LUMINANCE)?"GL_L":(format==GL_ALPHA)?"GL_A":(format==GL_LUMINANCE_ALPHA)?"GL_LA":(format==GL_RGB)?"GL_RGB":"GL_RGBA"
   );	
#endif

#ifdef PANDORA0
	if(hasMipmaps() && ((format==GL_RGB) || (format==GL_RGBA))) {
		// more Gamma on the Pandora
		mImage.QuakeGamma(1.6);
	};
#endif

#ifdef HAVE_GLES
	if(hasMipmaps() && ((size.x>32) || (size.y>32))) {
		// downscale this texture !
		if (mImage.DownScale()) {
			size.x = mImage.GetWidth();
			size.y = mImage.GetHeight();
			storedSize = Vec2i(GetNextPowerOf2(size.x), GetNextPowerOf2(size.y));
		}
	}
#endif
	
	if(hasMipmaps()) 
	{
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	}
	// TODO handle GL_MAX_TEXTURE_SIZE
#ifdef HAVE_GLES
	// convert to 16 bits some textures
	if (internal==GL_RGB8) {
		// convert 24bits RGB to 16bits 565 RGB
		unsigned char *temp = new unsigned char[mImage.GetWidth()*mImage.GetHeight()*2];
		unsigned short *p = (unsigned short*)temp;
		unsigned char *s = mImage.GetData();
		for (unsigned int y=0; y<mImage.GetHeight(); y++)
			for (unsigned int x=0; x<mImage.GetWidth(); x++) {
				unsigned short r = s[0]>>3, g = s[1]>>2, b = s[2]>>3;
				*(p++) = r<<11 | g<<5 | b;
				s+=3;
		}
		if(storedSize != size) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, storedSize.x, storedSize.y, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, temp);
		} else {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, temp);
		}
		delete[] temp;
	} else
	if (internal==GL_RGBA8) {
		// convert 32bits RGBA to 16bits 4444 RGBA
		unsigned char *temp = new unsigned char[mImage.GetWidth()*mImage.GetHeight()*2];
		unsigned short *p = (unsigned short*)temp;
		unsigned char *s = mImage.GetData();
		for (unsigned int y=0; y<mImage.GetHeight(); y++)
			for (unsigned int x=0; x<mImage.GetWidth(); x++) {
				unsigned short r = s[0]>>4, g = s[1]>>4, b = s[2]>>4, a = s[3]>>4;
				*(p++) = r<<12 | g<<8 | b<<4 | a;
				s+=4;
		}
		if(storedSize != size) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, storedSize.x, storedSize.y, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, NULL);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, temp);
		} else {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, temp);
		}
		delete[] temp;
	} else
#endif
	if(storedSize != size) {
		glTexImage2D(GL_TEXTURE_2D, 0, internal, storedSize.x, storedSize.y, 0, format, GL_UNSIGNED_BYTE, NULL);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y, format, GL_UNSIGNED_BYTE, mImage.GetData());
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, internal, size.x, size.y, 0, format, GL_UNSIGNED_BYTE, mImage.GetData());
	}
	
	if(renderer->GetMaxAnisotropy() != 1.f) {
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, renderer->GetMaxAnisotropy());
	}
	
	CHECK_GL;
}

void GLTexture2D::Destroy() {
	
	if(tex) {
		glDeleteTextures(1, &tex), tex = GL_NONE;
		CHECK_GL;
	}
	
	for(size_t i = 0; i < renderer->GetTextureStageCount(); i++) {
		GLTextureStage * stage = renderer->GetTextureStage(i);
		if(stage->tex == this) {
			stage->tex = NULL;
		}
		if(stage->current == this) {
			stage->current = NULL;
		}
	}
}
#ifdef HAVE_GLES
#define GL_MIRRORED_REPEAT			GL_MIRRORED_REPEAT_OES
#endif

static const GLint arxToGlWrapMode[] = {
	GL_REPEAT, // WrapRepeat,
	GL_MIRRORED_REPEAT, // WrapMirror
	GL_CLAMP_TO_EDGE // WrapClamp
};

static const GLint arxToGlFilter[][3] = {
	// Mipmap: FilterNone
	{
		-1, // FilterNone
		GL_NEAREST, // FilterNearest
		GL_LINEAR   // FilterLinear
	},
	// Mipmap: FilterNearest
	{
		-1, // FilterNone
		GL_NEAREST_MIPMAP_NEAREST, // FilterNearest
		GL_LINEAR_MIPMAP_NEAREST   // FilterLinear
	},
	// Mipmap: FilterLinear
	{
		-1, // FilterNone
		GL_NEAREST_MIPMAP_LINEAR, // FilterNearest
		GL_LINEAR_MIPMAP_LINEAR   // FilterLinear
	}
};

void GLTexture2D::apply(GLTextureStage * stage) {
	
	arx_assert(stage != NULL);
	arx_assert(stage->tex == this);
	
	if(stage->wrapMode != wrapMode) {
		wrapMode = stage->wrapMode;
		GLint glwrap = arxToGlWrapMode[wrapMode];
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glwrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glwrap);
	}
	
	TextureStage::FilterMode newMipFilter = hasMipmaps() ? stage->mipFilter : TextureStage::FilterNone;
	
	if(newMipFilter != mipFilter || stage->minFilter != minFilter) {
		minFilter = stage->minFilter, mipFilter = newMipFilter;
		arx_assert(minFilter != TextureStage::FilterNone);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, arxToGlFilter[mipFilter][minFilter]);
	}
	
	if(stage->magFilter != magFilter) {
		magFilter = stage->magFilter;
		arx_assert(magFilter != TextureStage::FilterNone);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, arxToGlFilter[0][magFilter]);
	}
	
}
