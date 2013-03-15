#ifndef INPUT_H
#define INPUT_H

#include "math/vec.h"
#include "common/shared.h"
#include "common/uncopyable.h"
#include "keys.h"

struct InputEvent
{
	enum Type
	{
		Character,
		SystemKey,
		KeyDown,
		KeyUp,
		MouseMove
	};
	InputEvent( Type type, int character ) : type( type ), character( character ) {}
	InputEvent( Type type, float2 position ) : type( type ), position( position ) {}
	Type type;
	int character;
	float2 position;
};

struct InputImpl;

class Input : public Uncopyable
{
public:
	Input( int width, int height );
	~Input();

	bool key_down( Keys::Code key );

	void set_mouse_position( float2 position );
	float2 get_mouse_position();

	struct EventHandler
	{
		virtual void do_event( InputEvent const &event ) {}
	};
	void do_messages( EventHandler &handler );

private:
    SharedPtr< InputImpl > m_impl;
};

#endif
