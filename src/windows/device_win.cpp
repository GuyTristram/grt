#include "core/device.h"
#include "opengl/opengl.h"
#include "input/inputevent.h"
#include "input/keys.h"
#include <Windowsx.h>
#include "wglext.h"

struct DeviceImpl : public Shared
{
	HWND hWnd;
	HDC hDC;
	HGLRC hRC;
	int width;
	int height;
};

bool WGLExtensionSupported(const char *extension_name)
{
    // this is pointer to function which returns pointer to string with list of all wgl extensions
    PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = NULL;

    // determine pointer to wglGetExtensionsStringEXT function
    _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC) wglGetProcAddress("wglGetExtensionsStringEXT");

    if (strstr(_wglGetExtensionsStringEXT(), extension_name) == NULL)
    {
        // string was not found
        return false;
    }

    // extension is supported
    return true;
}


Device::Device( Options const &options ) : m_impl( new DeviceImpl )
{
	int s_width = GetSystemMetrics( SM_CXSCREEN );
	int s_height = GetSystemMetrics( SM_CYSCREEN );

	m_impl->width = options.width;
	m_impl->height = options.height;
	if( options.width < 1 )
	{
		m_impl->width = options.fullscreen ? s_width : 800;
	}
	if( options.height < 1 )
	{
		m_impl->height = options.fullscreen ? s_height : 600;
	}
	WNDCLASS wc;
	// register window class
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = DefWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(0);
	wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "GLSample";
	RegisterClass( &wc );
	
	int style = options.fullscreen ? WS_POPUP | WS_VISIBLE : WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE;
    int x = options.fullscreen ? 0 : 100;
    int y = x;
    RECT rect = { x, y, x + m_impl->width, y + m_impl->height };
    AdjustWindowRect( &rect, style, false );

	m_impl->hWnd = CreateWindow( "GLSample", "OpenGL Sample", style,
		rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
		NULL, NULL, GetModuleHandle( 0 ), NULL );

	SetWindowLongPtr( m_impl->hWnd, GWLP_USERDATA, (LONG)this );
	
	PIXELFORMATDESCRIPTOR pfd;
	int format;
	
	// get the device context (DC)
	m_impl->hDC = GetDC( m_impl->hWnd );
	
	// set the pixel format for the DC
	ZeroMemory( &pfd, sizeof( pfd ) );
	pfd.nSize = sizeof( pfd );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;
	format = ChoosePixelFormat( m_impl->hDC, &pfd );
	SetPixelFormat( m_impl->hDC, format, &pfd );
	
	// create and enable the render context (RC)
	m_impl->hRC = wglCreateContext( m_impl->hDC );
	wglMakeCurrent( m_impl->hDC, m_impl->hRC );
	PFNWGLSWAPINTERVALEXTPROC       wglSwapIntervalEXT = NULL;
	PFNWGLGETSWAPINTERVALEXTPROC    wglGetSwapIntervalEXT = NULL;

	if (WGLExtensionSupported("WGL_EXT_swap_control"))
	{
		// Extension is supported, init pointers.
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT");

		// this is another function from WGL_EXT_swap_control extension
		wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC) wglGetProcAddress("wglGetSwapIntervalEXT");
		//wglSwapIntervalEXT(0);
	}
	
	if (gl3wInit()) {
		//fprintf(stderr, "failed to initialize OpenGL\n");
		return;
	}
	if (!gl3wIsSupported(3, 2)) {
		//fprintf(stderr, "OpenGL 3.2 not supported\n");
		return;
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
	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( m_impl->hRC );
	ReleaseDC( m_impl->hWnd, m_impl->hDC );
}

void Device::swap()
{
	SwapBuffers( m_impl->hDC );
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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
/*
	Device *dev = (Device * )GetWindowLongPtr( hWnd, GWLP_USERDATA );
	
	switch (message)
	{
		
	case WM_CREATE:
		return 0;
		
	case WM_CLOSE:
		if( dev )
			dev->quit();
		PostQuitMessage( 0 );
		return true;
		
	case WM_DESTROY:
		return 0;
		
	case WM_KEYDOWN:
		switch ( wParam )
		{
			
		case VK_ESCAPE:
			if( dev )
				dev->quit();
			PostQuitMessage(0);
			return 0;
			
		}
		return 0;
	case WM_SYSKEYDOWN:
		switch ( wParam )
		{
			
		case VK_ESCAPE:
			if( dev )
				dev->quit();
			PostQuitMessage(0);
			return 0;
			
		}
		return 0;
	case WM_MOUSEMOVE:
		if( dev )
			dev->do_event( InputEvent( InputEvent::MouseMove,
				float2( GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) ) ) );
		return 0;

	case WM_LBUTTONDOWN:
		if( dev )
			dev->do_event( InputEvent( InputEvent::KeyDown, Keys::mouse0 ) );
		return 0;

	case WM_LBUTTONUP:
		if( dev )
			dev->do_event( InputEvent( InputEvent::KeyUp, Keys::mouse0 ) );
		return 0;

	case WM_CHAR:
		if( dev )
			dev->do_event( InputEvent( InputEvent::Character, wParam ) );
		return 0;

	case WM_QUIT:
		if( dev )
			dev->quit();
		return 0;
	
	default:
		return DefWindowProc( hWnd, message, wParam, lParam );
			
	}
*/	
		return DefWindowProc( hWnd, message, wParam, lParam );
}


void Device::do_bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
