#ifndef ARX_GRAPHICS_OPENGL_OPENGLPOSTPROCESSING_H
#define ARX_GRAPHICS_OPENGL_OPENGLPOSTPROCESSING_H

#include <boost/noncopyable.hpp>

#include "math/Rectangle.h"
#include "OpenGLUtil.h"

class OpenGLPostProcesing : public boost::noncopyable {

public:
	OpenGLPostProcesing(Rect size);
	~OpenGLPostProcesing();

	void resize(Rect size);

	void attach();

	void render();

private:
	Rect mSize;


	GLuint fbo, fbo_texture, rbo_depth;

	/* Global */
	GLuint vbo_fbo_vertices;


	void createFramebuffer(GLuint &fb, GLuint &color, GLuint &depth);
	void createScreenVerts();


	// Shader
	GLuint program_postproc;
	GLuint attribute_v_coord_postproc;
	GLuint uniform_fbo_texture;

	GLuint mUniformGamma;

	GLuint createShader();
};

#endif
