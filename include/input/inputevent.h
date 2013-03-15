#ifndef INPUTEVENT_H
#define INPUTEVENT_H

#include "math/vec.h"

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


#endif
