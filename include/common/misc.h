#ifndef MISC_H
#define MISC_H

#include <algorithm>

template< typename T >
T clamp( T v, T min, T max )
{
	return std::min( std::max( v, min ), max );
}

#endif // MISC_H
