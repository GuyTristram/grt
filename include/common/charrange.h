#ifndef CHARRANGE_H
#define CHARRANGE_H

#include <string>
#include <string.h>
#include "uncopyable.h"

struct CharRange
{
	CharRange() : begin( 0 ), end( 0 ) {}
	CharRange( const char *begin, const char *end ) : begin( begin ), end( end ) {}
	CharRange( const char *begin ) : begin( begin ), end( begin + strlen( begin ) ) {}

	bool empty() const {return begin == end;}
	char front() const {return *begin;}
	CharRange &operator++() {++begin; return *this;}

	int length() const {return end - begin;}


	const char *begin;
	const char *end;
};

class CharRangeFile : public Uncopyable
{
public:
	explicit CharRangeFile( char const *filename );
	~CharRangeFile();

	CharRange const &range() { return m_range; }
private:
	CharRange m_range;
};

inline bool operator==( CharRange const &a, CharRange const &b )
{
	return a.end - a.begin == b.end - b.begin && std::equal( a.begin, a.end, b.begin );
}

inline bool operator!=( CharRange const &a, CharRange const &b )
{
	return !operator==( a, b );
}

inline bool operator<( CharRange const &a, CharRange const &b )
{
	return std::lexicographical_compare( a.begin, a.end, b.begin, b.end );
}
void eat_white( CharRange &range );
CharRange read_line( CharRange &range );
CharRange read_to( CharRange &range, int ch );
CharRange read_token( CharRange &range );
CharRange read_to_token( CharRange &range, CharRange const &token );
float read_float( CharRange &range, float def = 0.0f );
int read_int( CharRange &range, int def = 0 );
void ignore( CharRange &range, int c );

inline std::string to_std_string( CharRange const &c ) {return std::string( c.begin, c.end );}


#endif // CHARRANGE_H
