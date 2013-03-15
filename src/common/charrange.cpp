#include "common/charrange.h"

#include <fstream>
#include <math.h>

CharRangeFile::CharRangeFile( char const *filename )
{
	using namespace std;

	ifstream is;
	is.open( filename, ios::binary );

	// get length of file:
	is.seekg( 0, ios::end );
	std::streamoff length = is.tellg();
	if( length <= 0 )
		return;
	is.seekg( 0, ios::beg );

	// allocate memory:
	char *buffer = new char[ unsigned int( length ) ];

	// read data as a block:
	is.read( buffer, length );
	is.close();

	m_range.begin = buffer;
	m_range.end = buffer + length;
}

CharRangeFile::~CharRangeFile()
{
	delete [] m_range.begin;
}

void eat_white( CharRange &range )
{
	while( !range.empty() && isspace( range.front() ) )
		++range;
}

/*
void require_char( CharRange &range, int ch )
{
    eat_white( range );
    if( range.empty() || *range.begin != ch )
        throw UnexpectedCharacter();
    ++range;
}
*/

CharRange read_line( CharRange &range )
{
	CharRange ret = range;
	while( !range.empty() && range.front() != '\n' && range.front() != '\r' )
		++range;
	ret.end = range.begin;
	eat_white( range );
	return ret;
}

CharRange read_to( CharRange &range, int ch )
{
	CharRange ret = range;
	while( !range.empty() && range.front() != ch )
		++range;
	ret.end = range.begin;
	return ret;
}

CharRange read_token( CharRange &range )
{
	eat_white( range );
	const char *begin = range.begin;
	while( !range.empty() &&
	        !isspace( range.front() ) &&
	        strchr( "<>\\=/", range.front() ) == 0 )
		++range;
	return CharRange( begin, range.begin );
}

CharRange read_to_token( CharRange &range, CharRange const &token )
{
	CharRange contents = range;

	if( token.length() == 0 )
		return contents;

	while( range.length() >= token.length() )
	{
		if( CharRange( range.begin, range.begin + token.length() ) == token )
		{
			contents.end = range.begin;
			range.begin += token.length();
			return contents;
		}
		++range;
	}
	return contents;
}

float read_float( CharRange &range, float def )
{
	eat_white( range );

	if( range.empty() )
		return def;

	float mant = 0.0f;
	bool neg_mant = range.front() == '-';
	if( neg_mant || range.front() == '+' )
		++range;

	if( range.empty() && !isdigit( range.front() ) )
		return def;

	while( !range.empty() && isdigit( range.front() ) )
	{
		mant = mant * 10 + range.front() - '0';
		++range;
	}
	if( !range.empty() && range.front() == '.' )
	{
		++range;
		float mul = 0.1f;
		while( isdigit( range.front() ) && !range.empty() )
		{
			mant += ( range.front() - '0' ) * mul;
			mul *= 0.1f;
			++range;
		}
	}
	if( !range.empty() && ( range.front() == 'e' || range.front() == 'E' ) )
	{
		++range;
		bool neg_exp = range.front() == '-';
		if( neg_exp || range.front() == '+' )
			++range;
		float exp = 0.0f;
		while( !range.empty() && isdigit( range.front() ) )
		{
			exp = exp * 10 + range.front() - '0';
			++range;
		}
		if( neg_exp )
			exp = -exp;
		mant *= pow( 10, exp );
	}
	if( neg_mant )
		mant = -mant;
	return mant;
}

int read_int( CharRange &range, int def )
{
	eat_white( range );

	if( range.empty() )
		return def;

	bool neg = range.front() == '-';
	if( neg || range.front() == '+' )
		++range;

	if( range.empty() && !isdigit( range.front() ) )
		return def;

	int i = 0;

	while( !range.empty() && isdigit( range.front() ) )
	{
		i = i * 10 + range.front() - '0';
		++range;
	}
	if( neg )
		i = -i;
	return i;
}

void ignore( CharRange &range, int c )
{
	while( !range.empty() && *range.begin != c )
		++range;
	if( !range.empty() )
		++range;
}
