#include "input.h"
#include "keys.h"
#include <assert.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/input.h>

extern "C" {
//int open( const char *path, int oflag, ... );
//int read( int  handle,  void  *buffer,  int  nbyte );
//int close( int fildes );
}

struct InputImpl : public Shared
{
	int m_mouse_fd;
	int m_key_fd;

	int m_mouse_buttons;
	int m_mouse_x;
	int m_mouse_y;

	int m_width;
	int m_height;

	struct termios m_oldt;
	int m_oldf;

	bool m_key_down[ Keys::count ];
	bool m_caps_on;
};

namespace
{
	Keys::Code from_scan( int scan )
	{
	    static const Keys::Code codes[] = {
			/*---0-RESERVED     */ Keys::unknown,
			/*---1-ESC          */ Keys::escape,
			/*---2-1            */ Keys::n1,
			/*   3 2            */ Keys::n2,
			/*   4 3            */ Keys::n3,
			/*   5 4            */ Keys::n4,
			/*   6 5            */ Keys::n5,
			/*   7 6            */ Keys::n6,
			/*   8 7            */ Keys::n7,
			/*   9 8            */ Keys::n8,
			/*  10 9            */ Keys::n9,
			/*  11 0            */ Keys::n0,
			/*  12 MINUS        */ Keys::minus,
			/*  13 EQUAL        */ Keys::equals,
			/*  14 BACKSPACE    */ Keys::backspace,
			/*  15 TAB          */ Keys::tab,
			/*  16 Q            */ Keys::q,
			/*  17 W            */ Keys::w,
			/*  18 E            */ Keys::e,
			/*  19 R            */ Keys::r,
			/*  20 T            */ Keys::t,
			/*  21 Y            */ Keys::y,
			/*  22 U            */ Keys::u,
			/*  23 I            */ Keys::i,
			/*  24 O            */ Keys::o,
			/*  25 P            */ Keys::p,
			/*  26 LEFTBRACE    */ Keys::open,
			/*  27 RIGHTBRACE   */ Keys::close,
			/*  28 ENTER        */ Keys::enter,
			/*  29 LEFTCTRL     */ Keys::left_control,
			/*  30 A            */ Keys::a,
			/*  31 S            */ Keys::s,
			/*  32 D            */ Keys::d,
			/*  33 F            */ Keys::f,
			/*  34 G            */ Keys::g,
			/*  35 H            */ Keys::h,
			/*  36 J            */ Keys::j,
			/*  37 K            */ Keys::k,
			/*  38 L            */ Keys::l,
			/*  39 SEMICOLON    */ Keys::semicolon,
			/*  40 APOSTROPHE   */ Keys::apostrophe,
			/*  41 GRAVE        */ Keys::grave,
			/*  42 LEFTSHIFT    */ Keys::left_shift,
			/*  43 BACKSLASH    */ Keys::hash,
			/*  44 Z            */ Keys::z,
			/*  45 X            */ Keys::x,
			/*  46 C            */ Keys::c,
			/*  47 V            */ Keys::v,
			/*  48 B            */ Keys::b,
			/*  49 N            */ Keys::n,
			/*  50 M            */ Keys::m,
			/*  51 COMMA        */ Keys::comma,
			/*  52 DOT          */ Keys::dot,
			/*  53 SLASH        */ Keys::slash,
			/*  54 RIGHTSHIFT   */ Keys::right_shift,
			/*  55 KPASTERISK   */ Keys::num_asterisk,
			/*  56 LEFTALT      */ Keys::left_alt,
			/*  57 SPACE        */ Keys::space,
			/*  58 CAPSLOCK     */ Keys::caps_lock,
			/*  59 F1           */ Keys::f1,
			/*  60 F2           */ Keys::f2,
			/*  61 F3           */ Keys::f3,
			/*  62 F4           */ Keys::f4,
			/*  63 F5           */ Keys::f5,
			/*  64 F6           */ Keys::f6,
			/*  65 F7           */ Keys::f7,
			/*  66 F8           */ Keys::f8,
			/*  67 F9           */ Keys::f9,
			/*  68 F10          */ Keys::f10,
			/*  69 NUMLOCK      */ Keys::num_lock,
			/*  70 SCROLLLOCK   */ Keys::scroll_lock,
			/*  71 KP7          */ Keys::num_7,
			/*  72 KP8          */ Keys::num_8,
			/*  73 KP9          */ Keys::num_9,
			/*  74 KPMINUS      */ Keys::num_minus,
			/*  75 KP4          */ Keys::num_4,
			/*  76 KP5          */ Keys::num_5,
			/*  77 KP6          */ Keys::num_6,
			/*  78 KPPLUS       */ Keys::num_plus,
			/*  79 KP1          */ Keys::num_1,
			/*  80 KP2          */ Keys::num_2,
			/*  81 KP3          */ Keys::num_3,
			/*  82 KP0          */ Keys::num_0,
			/*  83 KPDOT        */ Keys::num_dot,
			/*  84 ?            */ Keys::unknown,
			/*  85 ZENKAKUHANKAKU */ Keys::unknown,
			/*  86 102ND        */ Keys::backslash,
			/*  87 F11          */ Keys::f11,
			/*  88 F12          */ Keys::f12,
			/*  89 RO           */ Keys::unknown,
			/*  90 KATAKANA     */ Keys::unknown,
			/*  91 HIRAGANA     */ Keys::unknown,
			/*  92 HENKAN       */ Keys::unknown,
			/*  93 KATAKANAHIRAGANA */ Keys::unknown,
			/*  94 MUHENKAN     */ Keys::unknown,
			/*  95 KPJPCOMMA    */ Keys::unknown,
			/*  96 KPENTER      */ Keys::num_enter,
			/*  97 RIGHTCTRL    */ Keys::right_control,
			/*  98 KPSLASH      */ Keys::num_slash,
			/*  99 SYSRQ        */ Keys::unknown,
			/* 100 RIGHTALT     */ Keys::right_alt,
			/* 101 LINEFEED     */ Keys::unknown,
			/* 102 HOME         */ Keys::home,
			/* 103 UP           */ Keys::up,
			/* 104 PAGEUP       */ Keys::page_up,
			/* 105 LEFT         */ Keys::left,
			/* 106 RIGHT        */ Keys::right,
			/* 107 END          */ Keys::end,
			/* 108 DOWN         */ Keys::down,
			/* 109 PAGEDOWN     */ Keys::page_down,
			/* 110 INSERT       */ Keys::insert,
			/* 111 DELETE       */ Keys::del
		};
		return ( scan >= 0 ) && (scan <= 111 ) ? codes[ scan ] : Keys::unknown;
    }

	int code_to_character( Keys::Code code, bool shift_down, bool caps_on )
	{
		char plain[] = " abcdefghijklmnopqrstuvwxyz0123456789`-=[];'#\\,./\t\n";
		char shift[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ)!\"£$%^&*(¬_+{}:@~|<>?\t\n";
		char caps[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789`-=[];'#\\,./\t\n";
		char shift_caps[] = " abcdefghijklmnopqrstuvwxyz)!\"£$%^&*(¬_+{}:@~|<>?\t\n";

		char *table = caps_on ? ( shift_down ? shift_caps : caps ) : ( shift_down ? shift : plain );

		if( code >= Keys::space && code <= Keys::enter )
			return table[code - Keys::space];
		else
			return 0;
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

	tcgetattr( STDIN_FILENO, &m_impl->m_oldt );
	struct termios newt = m_impl->m_oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	m_impl->m_oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, m_impl->m_oldf | O_NONBLOCK);

}

Input::~Input()
{
	if( m_impl->m_mouse_fd >= 0 )
		close( m_impl->m_mouse_fd );
	if( m_impl->m_key_fd >= 0 )
		close( m_impl->m_key_fd );

  tcsetattr(STDIN_FILENO, TCSANOW, &m_impl->m_oldt);
  fcntl(STDIN_FILENO, F_SETFL, m_impl->m_oldf);
}

bool Input::key_down( Keys::Code key )
{
	return m_impl->m_key_down[ key ];
}

void Input::do_messages( EventHandler &handler )
{
	if( m_impl->m_mouse_fd >= 0 )
	{
		signed char mbuf[3];
		while( read( m_impl->m_mouse_fd, &mbuf[0], 3 ) >=0 )
		{
			int old_buttons = m_impl->m_mouse_buttons;
			int buttons = mbuf[0] & 7;
			m_impl->m_mouse_buttons = buttons; // In case state is queried in event handler

			if( ( buttons & 1 ) && !( old_buttons & 1 ) )
				handler.do_event( InputEvent( InputEvent::KeyDown, Keys::mouse0 ) );
			if( ( buttons & 2 ) && !( old_buttons & 2 ) )
				handler.do_event( InputEvent( InputEvent::KeyDown, Keys::mouse1 ) );
			if( ( buttons & 4 ) && !( old_buttons & 4 ) )
				handler.do_event( InputEvent( InputEvent::KeyDown, Keys::mouse2 ) );

			if( ( old_buttons & 1 ) && !( buttons & 1 ) )
				handler.do_event( InputEvent( InputEvent::KeyUp, Keys::mouse0 ) );
			if( ( old_buttons & 2 ) && !( buttons & 2 ) )
				handler.do_event( InputEvent( InputEvent::KeyUp, Keys::mouse1 ) );
			if( ( old_buttons & 4 ) && !( buttons & 4 ) )
				handler.do_event( InputEvent( InputEvent::KeyUp, Keys::mouse2 ) );

			if( ( mbuf[1] != 0 ) || ( mbuf[2] != 0 ) )
			{
				m_impl->m_mouse_x += mbuf[1];
				m_impl->m_mouse_y += mbuf[2];

				if( m_impl->m_mouse_x < 0 ) m_impl->m_mouse_x = 0;
				if( m_impl->m_mouse_y < 0 ) m_impl->m_mouse_y = 0;

				if( m_impl->m_mouse_x > m_impl->m_width ) m_impl->m_mouse_x = m_impl->m_width;
				if( m_impl->m_mouse_y > m_impl->m_height ) m_impl->m_mouse_y = m_impl->m_height;

				handler.do_event( InputEvent( InputEvent::MouseMove,
					float2( m_impl->m_mouse_x, m_impl->m_height - m_impl->m_mouse_y ) ) );
			}
		}
	}
	if( m_impl->m_key_fd >= 0 )
	{
		input_event keys;
		while( read( m_impl->m_key_fd, &keys, sizeof( keys ) ) >=0 )
		{
			if( keys.type == EV_KEY )
			{
				Keys::Code key = from_scan( keys.code );

				if( keys.value == 1 )
				{
					handler.do_event( InputEvent( InputEvent::KeyDown, key ) );
					m_impl->m_key_down[ key ] = true;
					if( key == Keys::caps_lock )
						m_impl->m_caps_on = !m_impl->m_caps_on;
				}

				if( keys.value == 0 )
				{
					handler.do_event( InputEvent( InputEvent::KeyUp, key ) );
					m_impl->m_key_down[ key ] = false;
				}

				if( keys.value == 1 || keys.value == 2 )
				{
					bool shift = key_down( Keys::left_shift ) || key_down( Keys::right_shift );
					int ch = code_to_character( key, shift, m_impl->m_caps_on );
					if( ch )
						handler.do_event( InputEvent( InputEvent::Character, ch ) );
					else
						handler.do_event( InputEvent( InputEvent::SystemKey, key ) );
				}
			}
			//if( keys.code == 1 )
				//quit();
   			//printf("Key %i %i %i\n", keys.type, keys.code, keys.value );
		}
	}
	int ch;
	while( ( ch = getchar() ) != EOF )
	{
		// handler.do_event( InputEvent( InputEvent::Character, ch ) );
	}


}
