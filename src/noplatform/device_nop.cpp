#include "core/device.h"
#include "opengl/opengl.h"
#include <stdio.h>

struct DeviceImpl : public Shared
{
	int width;
	int height;
};


Device::Device( Options const &options ) : m_impl( new DeviceImpl )
{
    m_impl->width = options.width;
    m_impl->height = options.height;
	
	if (gl3wInit()) {
		printf("failed to initialize OpenGL\n");
		//return;
	}
	if (!gl3wIsSupported(3, 2)) {
		printf("OpenGL 3.2 not supported\n");
		//return;
	}
	
	//glViewport( 0, 0, m_impl->width, m_impl->height);
   glClearColor(0.15f, 0.25f, 0.35f, 0.0f);
   glClear( GL_COLOR_BUFFER_BIT );
   glClear( GL_DEPTH_BUFFER_BIT );

   // Enable back face culling.
   glEnable(GL_CULL_FACE);
}

Device::~Device()
{
}

void Device::swap()
{
	glClear( GL_COLOR_BUFFER_BIT );
	glClear( GL_DEPTH_BUFFER_BIT );
}

int Device::width() const
{
	return m_impl->width;
}

int Device::height() const
{
	return m_impl->height;
}

void Device::resize( int width, int height )
{
	m_impl->width = width;
	m_impl->height = height;
}

void Device::do_bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
