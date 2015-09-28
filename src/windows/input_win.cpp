#include "input/input.h"
#include "input/keys.h"
#include <assert.h>
#include <Windows.h>
#include <Windowsx.h>

struct InputImpl : public Shared
{
	int m_mouse_buttons;
	int m_mouse_x;
	int m_mouse_y;

	int m_width;
	int m_height;

	bool m_key_down[ Keys::count ];
	bool m_caps_on;

	bool m_mouse_relative = false;
	float2 m_abs_mouse;
};

namespace
{
	Keys::Code from_vk( int scan )
	{
	    static const Keys::Code codes[] = {
			/* RESERVED               */ Keys::unknown,
			/* VK_LBUTTON        0x01 */ Keys::mouse0,
			/* VK_RBUTTON        0x02 */ Keys::mouse1,
			/* VK_CANCEL         0x03 */ Keys::unknown,
			/* VK_MBUTTON        0x04 */ Keys::mouse2,
			/* RESERVED          0x05 */ Keys::unknown,
			/* RESERVED          0x06 */ Keys::unknown,
			/* RESERVED          0x07 */ Keys::unknown,
			/* VK_BACK           0x08 */ Keys::backspace,
			/* VK_TAB            0x09 */ Keys::tab,
			/* RESERVED          0x0A */ Keys::unknown,
			/* RESERVED          0x0B */ Keys::unknown,
			/* VK_CLEAR          0x0C */ Keys::unknown,
			/* VK_RETURN         0x0D */ Keys::enter,
			/* RESERVED          0x0E */ Keys::unknown,
			/* RESERVED          0x0F */ Keys::unknown,
			/* VK_SHIFT          0x10 */ Keys::left_shift,
			/* VK_CONTROL        0x11 */ Keys::left_control,
			/* VK_MENU           0x12 */ Keys::unknown,
			/* VK_PAUSE          0x13 */ Keys::unknown,
			/* VK_CAPITAL        0x14 */ Keys::unknown,
			/* VK_KANA           0x15 */ Keys::unknown,
			/* RESERVED          0x16 */ Keys::unknown,
			/* VK_JUNJA          0x17 */ Keys::unknown,
			/* VK_FINAL          0x18 */ Keys::unknown,
			/* VK_HANJA          0x19 */ Keys::unknown,
			/* RESERVED          0x1A */ Keys::unknown,
			/* VK_ESCAPE         0x1B */ Keys::escape,
			/* VK_CONVERT        0x1C */ Keys::unknown,
			/* VK_NONCONVERT     0x1D */ Keys::unknown,
			/* VK_ACCEPT         0x1E */ Keys::unknown,
			/* VK_MODECHANGE     0x1F */ Keys::unknown,
			/* VK_SPACE          0x20 */ Keys::space,
			/* VK_PRIOR          0x21 */ Keys::escape,
			/* VK_NEXT           0x22 */ Keys::escape,
			/* VK_END            0x23 */ Keys::end,
			/* VK_HOME           0x24 */ Keys::home,
			/* VK_LEFT           0x25 */ Keys::left,
			/* VK_UP             0x26 */ Keys::up,
			/* VK_RIGHT          0x27 */ Keys::right,
			/* VK_DOWN           0x28 */ Keys::down,
			/* VK_SELECT         0x29 */ Keys::unknown,
			/* VK_PRINT          0x2A */ Keys::unknown,
			/* VK_EXECUTE        0x2B */ Keys::unknown,
			/* VK_SNAPSHOT       0x2C */ Keys::unknown,
			/* VK_INSERT         0x2D */ Keys::insert,
			/* VK_DELETE         0x2E */ Keys::del,
			/* VK_HELP           0x2F */ Keys::unknown,
			/* 0                 0x30 */ Keys::n0,
			/* 1                 0x31 */ Keys::n1,
			/* 2                 0x32 */ Keys::n2,
			/* 3                 0x33 */ Keys::n3,
			/* 4                 0x34 */ Keys::n4,
			/* 5                 0x35 */ Keys::n5,
			/* 6                 0x36 */ Keys::n6,
			/* 7                 0x37 */ Keys::n7,
			/* 8                 0x38 */ Keys::n8,
			/* 9                 0x39 */ Keys::n9,
			/* RESERVED          0x3A */ Keys::unknown,
			/* RESERVED          0x3B */ Keys::unknown,
			/* RESERVED          0x3C */ Keys::unknown,
			/* RESERVED          0x3D */ Keys::unknown,
			/* RESERVED          0x3E */ Keys::unknown,
			/* RESERVED          0x3F */ Keys::unknown,
			/* RESERVED          0x40 */ Keys::unknown,
			/* A                 0x41 */ Keys::a,
			/* B                 0x42 */ Keys::b,
			/* C                 0x43 */ Keys::c,
			/* D                 0x44 */ Keys::d,
			/* E                 0x45 */ Keys::e,
			/* F                 0x46 */ Keys::f,
			/* G                 0x47 */ Keys::g,
			/* H                 0x48 */ Keys::h,
			/* I                 0x49 */ Keys::i,
			/* J                 0x4A */ Keys::j,
			/* K                 0x4B */ Keys::k,
			/* L                 0x4C */ Keys::l,
			/* M                 0x4D */ Keys::m,
			/* N                 0x4E */ Keys::n,
			/* O                 0x4F */ Keys::o,
			/* P                 0x50 */ Keys::p,
			/* Q                 0x51 */ Keys::q,
			/* R                 0x52 */ Keys::r,
			/* S                 0x53 */ Keys::s,
			/* T                 0x54 */ Keys::t,
			/* U                 0x55 */ Keys::u,
			/* V                 0x56 */ Keys::v,
			/* W                 0x57 */ Keys::w,
			/* X                 0x58 */ Keys::x,
			/* Y                 0x59 */ Keys::y,
			/* Z                 0x5A */ Keys::z,
			/* VK_LWIN           0x5B */ Keys::unknown,
			/* VK_RWIN           0x5C */ Keys::unknown,
			/* VK_APPS           0x5D */ Keys::unknown,
			/* RESERVED          0x5E */ Keys::unknown,
			/* VK_SLEEP          0x5F */ Keys::unknown,
			/* VK_NUMPAD0        0x60 */ Keys::num_0,
			/* VK_NUMPAD1        0x61 */ Keys::num_1,
			/* VK_NUMPAD2        0x62 */ Keys::num_2,
			/* VK_NUMPAD3        0x63 */ Keys::num_3,
			/* VK_NUMPAD4        0x64 */ Keys::num_4,
			/* VK_NUMPAD5        0x65 */ Keys::num_5,
			/* VK_NUMPAD6        0x66 */ Keys::num_6,
			/* VK_NUMPAD7        0x67 */ Keys::num_7,
			/* VK_NUMPAD8        0x68 */ Keys::num_8,
			/* VK_NUMPAD9        0x69 */ Keys::num_9,
			/* VK_MULTIPLY       0x6A */ Keys::num_asterisk,
			/* VK_ADD            0x6B */ Keys::num_plus,
			/* VK_SEPARATOR      0x6C */ Keys::num_enter,
			/* VK_SUBTRACT       0x6D */ Keys::num_minus,
			/* VK_DECIMAL        0x6E */ Keys::num_dot,
			/* VK_DIVIDE         0x6F */ Keys::num_slash,
			/* VK_F1             0x70 */ Keys::f1,
			/* VK_F2             0x71 */ Keys::f2,
			/* VK_F3             0x72 */ Keys::f3,
			/* VK_F4             0x73 */ Keys::f4,
			/* VK_F5             0x74 */ Keys::f5,
			/* VK_F6             0x75 */ Keys::f6,
			/* VK_F7             0x76 */ Keys::f7,
			/* VK_F8             0x77 */ Keys::f8,
			/* VK_F9             0x78 */ Keys::f9,
			/* VK_F10            0x79 */ Keys::f10,
			/* VK_F11            0x7A */ Keys::f11,
			/* VK_F12            0x7B */ Keys::f12,
			/* VK_F13            0x7C */ Keys::unknown,
			/* VK_F14            0x7D */ Keys::unknown,
			/* VK_F15            0x7E */ Keys::unknown,
			/* VK_F16            0x7F */ Keys::unknown,
			/* VK_F17            0x80 */ Keys::unknown,
			/* VK_F18            0x81 */ Keys::unknown,
			/* VK_F19            0x82 */ Keys::unknown,
			/* VK_F20            0x83 */ Keys::unknown,
			/* VK_F21            0x84 */ Keys::unknown,
			/* VK_F22            0x85 */ Keys::unknown,
			/* VK_F23            0x86 */ Keys::unknown,
			/* VK_F24            0x87 */ Keys::unknown,
			/* RESERVED          0x88 */ Keys::unknown,
			/* RESERVED          0x89 */ Keys::unknown,
			/* RESERVED          0x8A */ Keys::unknown,
			/* RESERVED          0x8B */ Keys::unknown,
			/* RESERVED          0x8C */ Keys::unknown,
			/* RESERVED          0x8D */ Keys::unknown,
			/* RESERVED          0x8E */ Keys::unknown,
			/* RESERVED          0x8F */ Keys::unknown,
			/* VK_NUMLOCK        0x90 */ Keys::num_lock,
			/* VK_SCROLL         0x91 */ Keys::scroll_lock,
			/* VK_OEM_FJ_JISHO   0x92 */ Keys::unknown,
			/* VK_OEM_FJ_MASSHOU 0x93 */ Keys::unknown,
			/* VK_OEM_FJ_TOUROKU 0x94 */ Keys::unknown,
			/* VK_OEM_FJ_LOYA    0x95 */ Keys::unknown,
			/* VK_OEM_FJ_ROYA    0x96 */ Keys::unknown,
			/* RESERVED          0x97 */ Keys::unknown,
			/* RESERVED          0x98 */ Keys::unknown,
			/* RESERVED          0x99 */ Keys::unknown,
			/* RESERVED          0x9A */ Keys::unknown,
			/* RESERVED          0x9B */ Keys::unknown,
			/* RESERVED          0x9C */ Keys::unknown,
			/* RESERVED          0x9D */ Keys::unknown,
			/* RESERVED          0x9E */ Keys::unknown,
			/* RESERVED          0x9F */ Keys::unknown,
			/* VK_LSHIFT         0xA0 */ Keys::left_shift,
			/* VK_RSHIFT         0xA1 */ Keys::right_shift,
			/* VK_LCONTROL       0xA2 */ Keys::left_control,
			/* VK_RCONTROL       0xA3 */ Keys::right_control,
			/* VK_LMENU          0xA4 */ Keys::unknown,
			/* VK_RMENU          0xA5 */ Keys::unknown
		};
		static const int size = sizeof( codes ) / sizeof( Keys::Code );
		return ( scan >= 0 ) && (scan < size ) ? codes[ scan ] : Keys::unknown;
    }
}

Input::Input( int width, int height ) : m_impl( new InputImpl )
{
	m_impl->m_mouse_buttons = 0;
	m_impl->m_mouse_x = width / 2;
	m_impl->m_mouse_y = height / 2;
	m_impl->m_width = width;
	m_impl->m_height = height;
	m_impl->m_caps_on = false;
	for( int i = 0; i < Keys::count; ++i )
		m_impl->m_key_down[i] = false;


}

Input::~Input()
{
}

bool Input::key_down( Keys::Code key )
{
	return m_impl->m_key_down[ key ];
}

void Input::do_messages( EventHandler &handler )
{
	MSG msg;
	while( PeekMessage( &msg, 0,  0, 0, PM_REMOVE ) ) 
	{ 
		TranslateMessage(&msg);
		switch (msg.message)
		{
		case WM_CLOSE:
			//if( dev )
			//	dev->quit();
			PostQuitMessage( 0 );
			break;

		case WM_DESTROY:
			break;
		
		case WM_KEYDOWN:
			{
				Keys::Code key = from_vk( msg.wParam );
				if( key != Keys::unknown )
				{
					m_impl->m_key_down[ key ] = true;
					handler.do_event( InputEvent( InputEvent::KeyDown, key ) );
				}
			}
			break;

		case WM_KEYUP:
			{
				Keys::Code key = from_vk( msg.wParam );
				if( key != Keys::unknown )
				{
					m_impl->m_key_down[ key ] = false;
					handler.do_event( InputEvent( InputEvent::KeyUp, key ) );
				}
			}
			break;

		case WM_SYSKEYDOWN:
			{
				Keys::Code key = from_vk( msg.wParam );
				if( key != Keys::unknown )
					handler.do_event( InputEvent( InputEvent::KeyDown, key ) );
			}
			break;
		case WM_MOUSEMOVE:
			if( m_impl->m_mouse_relative )
			{
				handler.do_event( InputEvent( InputEvent::MouseMove,
					float2( float( GET_X_LPARAM( msg.lParam ) ),
					        float( GET_Y_LPARAM( msg.lParam ) ) ) - float2( 300.f, 300.f ) ) );
				if( GET_X_LPARAM( msg.lParam ) != 300 || GET_Y_LPARAM( msg.lParam ) != 300 )
					set_mouse_position( float2( 300.f, 300.f ) );
			}
			else
			{
				handler.do_event( InputEvent( InputEvent::MouseMove,
					float2( float( GET_X_LPARAM( msg.lParam ) ),
					float( GET_Y_LPARAM( msg.lParam ) ) ) ) );
			}
			break; 

		case WM_MOUSEWHEEL:
			handler.do_event( InputEvent( InputEvent::MouseWheel,
				GET_WHEEL_DELTA_WPARAM( msg.wParam ) ) );
			break; 

		case WM_LBUTTONDOWN:
			handler.do_event( InputEvent( InputEvent::KeyDown, Keys::mouse0 ) );
			break;

		case WM_LBUTTONUP:
			handler.do_event( InputEvent( InputEvent::KeyUp, Keys::mouse0 ) );
			break;

		case WM_RBUTTONDOWN:
			handler.do_event( InputEvent( InputEvent::KeyDown, Keys::mouse1 ) );
			break;

		case WM_RBUTTONUP:
			handler.do_event( InputEvent( InputEvent::KeyUp, Keys::mouse1 ) );
			break;

		case WM_CHAR:
			handler.do_event( InputEvent( InputEvent::Character, msg.wParam ) );
			break;

		case WM_QUIT:
			break;
	
		default:
			DispatchMessage(&msg);
			
		}
	}

}

void Input::set_mouse_position( float2 position )
{
	SetCursorPos( (int)position.x, (int)position.y );
}

float2 Input::get_mouse_position()
{
	POINT point;
	GetCursorPos( &point );
	return float2( (float)point.x, (float)point.y );
}

void Input::set_mouse_relative( bool relative )
{
	m_impl->m_mouse_relative = relative;
	if( relative )
	{
		m_impl->m_abs_mouse = get_mouse_position();
		set_mouse_position( float2( 300.f, 300.f ) );
	}
	else
	{
		set_mouse_position( m_impl->m_abs_mouse );
	}
}
