#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <linux/kd.h>	// keyboard stuff...

extern "C" {
int  read(  int  handle,  void  *buffer,  int  nbyte );
}
namespace
{
	class Keyboard
	{
	public:
		Keyboard()
		{
			for( int i = 0; i < 256; ++i )
				m_keys[i] = false;

			struct termios tty_attr;
			int flags;
			int old_keyboard_mode;

			/* make stdin non-blocking */
			flags = fcntl(0, F_GETFL);
			flags |= O_NONBLOCK;
			fcntl(0, F_SETFL, flags);

			/* save old keyboard mode */
			if (ioctl(0, KDGKBMODE, &old_keyboard_mode) < 0) {
			//return 0;
				printf("couldn't get the keyboard, are you running via ssh?\n");
				printf("keyboard routines do not work as expected over ssh...\n");
			}

			tcgetattr(0, &m_tty_attr_old);

			/* turn off buffering, echo and key processing */
			tty_attr = m_tty_attr_old;
			tty_attr.c_lflag &= ~(ICANON | ECHO | ISIG);
			tty_attr.c_iflag &= ~(ISTRIP | INLCR | ICRNL | IGNCR | IXON | IXOFF);
			tcsetattr(0, TCSANOW, &tty_attr);

			ioctl(0, KDSKBMODE, K_RAW);
		}

		~Keyboard()
		{
			tcsetattr(0, TCSAFLUSH, &m_tty_attr_old);
			ioctl(0, KDSKBMODE, K_XLATE);
		}

		bool key_down( int key )
		{
		    char buf;
			while( ::read(0, (void*)&buf, 1) >= 0 )
				m_keys[buf & ~0x80] = ( ( buf & 0x80 ) == 0 );

			return m_keys[ key ];
		}

	private:
		struct termios m_tty_attr_old;
		bool m_keys[ 256 ];
	};
}

bool key_down( int key )
{
	static Keyboard keyboard;
	return keyboard.key_down( key );
}
