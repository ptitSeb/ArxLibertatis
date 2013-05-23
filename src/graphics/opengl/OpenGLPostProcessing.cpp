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

#include "OpenGLPostProcessing.h"

static bool checkShader(GLuint object, const char * op, GLuint check) {

	GLint status;
	glGetObjectParameterivARB(object, check, &status);
	if(!status) {
		int logLength;
		glGetObjectParameterivARB(object, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLength);
		char * log = new char[logLength];
		glGetInfoLogARB(object, logLength, NULL, log);
		LogWarning << "Failed to " << op << " vertex shader: " << log;
		delete[] log;
		return false;
	}

	return true;
}

static const char nopVertexShaderSource[] =
	"attribute vec2 v_coord; \n"
	"uniform sampler2D fbo_texture; \n"
	"varying vec2 f_texcoord; \n"
	" \n"
	"void main(void) { \n"
	"	gl_Position = vec4(v_coord, 0.0, 1.0); \n"
	"	f_texcoord = (v_coord + 1.0) / 2.0; \n"
	"} \n";

static const char nopFragmentShaderSource[] =
	"uniform sampler2D fbo_texture; \n"
	"varying vec2 f_texcoord; \n"
	" \n"
	"void main(void) { \n"
	"	gl_FragColor = texture2D(fbo_texture, f_texcoord); \n"
	"} \n";

GLuint OpenGLPostProcesing::createShader()
{
	const char * vertSource = nopVertexShaderSource;
	const char * fragSource = nopFragmentShaderSource;

	GLuint shader = glCreateProgramObjectARB();
	if(!shader) {
		LogWarning << "Failed to create program object";
		return 0;
	}

	GLuint obj = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	if(!obj) {
		LogWarning << "Failed to create shader object";
		glDeleteObjectARB(shader);
		return 0;
	}

	GLuint fragObj = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	if(!fragObj) {
		LogWarning << "Failed to create shader object";
		glDeleteObjectARB(shader);
		return 0;
	}

	glShaderSourceARB(obj, 1, &vertSource, NULL);
	glCompileShaderARB(obj);
	if(!checkShader(obj, "compile", GL_OBJECT_COMPILE_STATUS_ARB)) {
		glDeleteObjectARB(obj);
		glDeleteObjectARB(shader);
		return 0;
	}

	glShaderSourceARB(fragObj, 1, &fragSource, NULL);
	glCompileShaderARB(fragObj);
	if(!checkShader(fragObj, "compile", GL_OBJECT_COMPILE_STATUS_ARB)) {
		glDeleteObjectARB(fragObj);
		glDeleteObjectARB(shader);
		return 0;
	}

	glAttachObjectARB(shader, obj);
	glAttachObjectARB(shader, fragObj);
	//glDeleteObjectARB(fragObj);

	glLinkProgramARB(shader);
	if(!checkShader(shader, "link", GL_OBJECT_LINK_STATUS_ARB)) {
		glDeleteObjectARB(shader);
		return 0;
	}

	const char* attribute_name;
	const char* uniform_name;

	attribute_name = "v_coord";
	attribute_v_coord_postproc = glGetAttribLocation(shader, attribute_name);
	if(attribute_v_coord_postproc == -1) {
		LogWarning << "Could not bind attribute: " << attribute_name;
	}

	uniform_name = "fbo_texture";
	uniform_fbo_texture = glGetUniformLocation(shader, uniform_name);
	if(uniform_fbo_texture == -1) {
		LogWarning << "Could not bind uniform: " << uniform_name;
	}


	uniform_name = "gamma";
	mUniformGamma = glGetUniformLocation(shader, uniform_name);
	if(mUniformGamma == -1) {
		LogWarning << "Could not bind uniform: " << uniform_name;
	}
	return shader;
}

void OpenGLPostProcesing::createFramebuffer(GLuint &fb, GLuint &color, GLuint &depth)
{
	// Texture
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &color);
	glBindTexture(GL_TEXTURE_2D, color);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mSize.right, mSize.bottom, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Depth buffer
	glGenRenderbuffers(1, &depth);
	glBindRenderbuffer(GL_RENDERBUFFER, depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mSize.right, mSize.bottom);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// Framebuffer
	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

	GLenum status;
	status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	switch(status) {
	case GL_FRAMEBUFFER_COMPLETE:
		break;

	case GL_FRAMEBUFFER_UNSUPPORTED:
		/* choose different formats */
		break;

	default:
		/* programming error; will fail on all hardware */
		LogError << "Framebuffer Error\n";
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLPostProcesing::createScreenVerts()
{
	GLfloat fbo_vertices[] = {
		-1, -1,
		1, -1,
		-1,  1,
		1,  1,
	};

	glGenBuffers(1, &vbo_fbo_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_fbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fbo_vertices), fbo_vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

OpenGLPostProcesing::OpenGLPostProcesing(Rect size)
{
	mSize = size;

	program_postproc = createShader();
	createScreenVerts();

	createFramebuffer(fbo, fbo_texture, rbo_depth);
}

OpenGLPostProcesing::~OpenGLPostProcesing()
{
	/* free_resources */
	glDeleteRenderbuffers(1, &rbo_depth);
	glDeleteTextures(1, &fbo_texture);
	glDeleteFramebuffers(1, &fbo);

	/* free_resources */
	glDeleteBuffers(1, &vbo_fbo_vertices);

	/* free_resources */
	glDeleteProgram(program_postproc);
}

void OpenGLPostProcesing::resize(Rect size)
{
	mSize = size;

	/* onReshape */
	// Rescale FBO and RBO as well
	glBindTexture(GL_TEXTURE_2D, fbo_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mSize.right, mSize.bottom, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mSize.right, mSize.bottom);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void OpenGLPostProcesing::attach()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void OpenGLPostProcesing::render()
{
	GLuint oldProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &oldProgram);

	glDisable(GL_CULL_FACE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, mSize.right, mSize.bottom);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glUseProgram(program_postproc);
	glBindTexture(GL_TEXTURE_2D, fbo_texture);

	glUniform1i(uniform_fbo_texture, /*GL_TEXTURE*/0);

	glUniform1f(mUniformGamma, 1.f);


	glEnableVertexAttribArray(attribute_v_coord_postproc);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_fbo_vertices);
	glVertexAttribPointer(
				attribute_v_coord_postproc,  // attribute
				2,                  // number of elements per vertex, here (x,y)
				GL_FLOAT,           // the type of each element
				GL_FALSE,           // take our values as-is
				0,                  // no extra data between each position
				0                   // offset of first element
				);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(attribute_v_coord_postproc);

	glUseProgram(oldProgram);
}
