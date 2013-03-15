#include "device.h"
#include "keys.h"
#include "inputevent.h"
#include <assert.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/input.h>

#include "GLES/gl.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"

extern "C" {
int open(const char *path, int oflag, ... );
int read(  int  handle,  void  *buffer,  int  nbyte );
int close(int fildes);
}

struct DeviceImpl : public Shared
{
// OpenGL|ES objects
	EGLDisplay m_display;
	EGLSurface m_surface;
	EGLContext m_context;
	unsigned int m_width;
	unsigned int m_height;
	Device::EventHandler *handler;
	bool m_input_started;
	int m_mouse_fd;
	int m_key_fd;
	int m_mouse_buttons;
	int m_mouse_x;
	int m_mouse_y;
};


Device::Device() : m_impl( new DeviceImpl )
{
   m_impl->m_input_started = false;
   m_impl->m_mouse_buttons = 0;

   int32_t success = 0;
   EGLBoolean result;
   EGLint num_config;

   bcm_host_init();


   static EGL_DISPMANX_WINDOW_T nativewindow;

   DISPMANX_ELEMENT_HANDLE_T dispman_element;
   DISPMANX_DISPLAY_HANDLE_T dispman_display;
   DISPMANX_UPDATE_HANDLE_T dispman_update;
   VC_RECT_T dst_rect;
   VC_RECT_T src_rect;

   static const EGLint attribute_list[] =
   {
      EGL_RED_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE, 8,
      EGL_ALPHA_SIZE, 8,
      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
      EGL_NONE
   };
   
   EGLConfig config;

   // get an EGL display connection
   m_impl->m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   assert(m_impl->m_display!=EGL_NO_DISPLAY);

   // initialize the EGL display connection
   result = eglInitialize(m_impl->m_display, 0, 0);
   assert(EGL_FALSE != result);

   // get an appropriate EGL frame buffer configuration
   result = eglChooseConfig(m_impl->m_display, attribute_list, &config, 1, &num_config);
   assert(EGL_FALSE != result);

   // create an EGL rendering context
   EGLint context_attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE}; 

   m_impl->m_context = eglCreateContext(m_impl->m_display, config, EGL_NO_CONTEXT, context_attribs);
   assert(m_impl->m_context!=EGL_NO_CONTEXT);

   // create an EGL window surface
   success = graphics_get_display_size(0 /* LCD */, &m_impl->m_width, &m_impl->m_height);
   assert( success >= 0 );

   dst_rect.x = 0;
   dst_rect.y = 0;
   dst_rect.width = m_impl->m_width;
   dst_rect.height = m_impl->m_height;
      
   src_rect.x = 0;
   src_rect.y = 0;
   src_rect.width = m_impl->m_width << 16;
   src_rect.height = m_impl->m_height << 16;        

   dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
   dispman_update = vc_dispmanx_update_start( 0 );
         
   dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
      0/*layer*/, &dst_rect, 0/*src*/,
      &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, (DISPMANX_TRANSFORM_T)0/*transform*/);
      
   nativewindow.element = dispman_element;
   nativewindow.width = m_impl->m_width;
   nativewindow.height = m_impl->m_height;
   vc_dispmanx_update_submit_sync( dispman_update );
      
   m_impl->m_surface = eglCreateWindowSurface( m_impl->m_display, config, &nativewindow, 0 );
   assert(m_impl->m_surface != EGL_NO_SURFACE);

   // connect the context to the surface
   result = eglMakeCurrent(m_impl->m_display, m_impl->m_surface, m_impl->m_surface, m_impl->m_context);
   assert(EGL_FALSE != result);

   // Set background color and clear buffers
   glClearColor(0.15f, 0.25f, 0.35f, 0.0f);
   glClear( GL_COLOR_BUFFER_BIT );
   glClear( GL_DEPTH_BUFFER_BIT );
   glShadeModel(GL_FLAT);

   // Enable back face culling.
   glEnable(GL_CULL_FACE);

   m_impl->m_mouse_x = m_impl->m_width / 2;
   m_impl->m_mouse_y = m_impl->m_height / 2;
}

Device::~Device()
{
   // clear screen
   glClear( GL_COLOR_BUFFER_BIT );
   eglSwapBuffers(m_impl->m_display, m_impl->m_surface);

   // Release OpenGL resources
   eglMakeCurrent( m_impl->m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
   eglDestroySurface( m_impl->m_display, m_impl->m_surface );
   eglDestroyContext( m_impl->m_display, m_impl->m_context );
   eglTerminate( m_impl->m_display );

}

void Device::swap()
{
   eglSwapBuffers(m_impl->m_display, m_impl->m_surface);
   glClear( GL_COLOR_BUFFER_BIT );
   glClear( GL_DEPTH_BUFFER_BIT );
}

int Device::width() const
{
	return m_impl->m_width;
}

int Device::height() const
{
	return m_impl->m_height;
}

void Device::do_messages( EventHandler &handler )
{
	m_impl->handler = &handler;
	if( !m_impl->m_input_started )
	{
		m_impl->m_mouse_fd = open("/dev/input/mouse0", O_RDONLY);
		if( m_impl->m_mouse_fd >= 0 )
		{
			// make non-blocking
			int flags = fcntl( m_impl->m_mouse_fd, F_GETFL );
			flags |= O_NONBLOCK;
			fcntl( m_impl->m_mouse_fd, F_SETFL, flags );
		}

		m_impl->m_key_fd = open("/dev/input/event0", O_RDONLY);
		if( m_impl->m_key_fd >= 0 )
		{
			// make non-blocking
			int flags = fcntl( m_impl->m_key_fd, F_GETFL );
			flags |= O_NONBLOCK;
			fcntl( m_impl->m_key_fd, F_SETFL, flags );
		}

		m_impl->m_input_started = true;
	}

	if( m_impl->m_mouse_fd >= 0 )
	{
		signed char mbuf[3];
		while( read( m_impl->m_mouse_fd, &mbuf[0], 3 ) >=0 )
		{
			int old_buttons = m_impl->m_mouse_buttons;
			int buttons = mbuf[0] & 7;
			m_impl->m_mouse_buttons = buttons; // In case state is queried in event handler

			if( ( buttons & 1 ) && !( old_buttons & 1 ) )
				do_event( InputEvent( InputEvent::KeyDown, Keys::mouse0 ) );
			if( ( buttons & 2 ) && !( old_buttons & 2 ) )
				do_event( InputEvent( InputEvent::KeyDown, Keys::mouse1 ) );
			if( ( buttons & 4 ) && !( old_buttons & 4 ) )
				do_event( InputEvent( InputEvent::KeyDown, Keys::mouse2 ) );

			if( ( old_buttons & 1 ) && !( buttons & 1 ) )
				do_event( InputEvent( InputEvent::KeyUp, Keys::mouse0 ) );
			if( ( old_buttons & 2 ) && !( buttons & 2 ) )
				do_event( InputEvent( InputEvent::KeyUp, Keys::mouse1 ) );
			if( ( old_buttons & 4 ) && !( buttons & 4 ) )
				do_event( InputEvent( InputEvent::KeyUp, Keys::mouse2 ) );

			if( ( mbuf[1] != 0 ) || ( mbuf[2] != 0 ) )
			{
				m_impl->m_mouse_x += mbuf[1];
				m_impl->m_mouse_y += mbuf[2];

				if( m_impl->m_mouse_x < 0 ) m_impl->m_mouse_x = 0;
				if( m_impl->m_mouse_y < 0 ) m_impl->m_mouse_y = 0;

				if( m_impl->m_mouse_x > m_impl->m_width ) m_impl->m_mouse_x = m_impl->m_width;
				if( m_impl->m_mouse_y > m_impl->m_height ) m_impl->m_mouse_y = m_impl->m_height;

				do_event( InputEvent( InputEvent::MouseMove,
					float2( m_impl->m_mouse_x, m_impl->m_height - m_impl->m_mouse_y ) ) );
			}
		}
	}
	if( m_impl->m_key_fd >= 0 )
	{
		input_event keys;
		while( read( m_impl->m_key_fd, &keys, sizeof( keys ) ) >=0 )
		{
			if( keys.code == 1 )
				quit();
   			//printf("Key %i %i %i\n", keys.type, keys.code, keys.value );
		}
	}
	m_impl->handler = 0;
}

void Device::do_event( InputEvent const &event )
{
	if( m_impl->handler )
		m_impl->handler->do_event( event );
}
