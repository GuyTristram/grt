#include "common/xml.h"
#include <algorithm>
#include "math.h"
#include <cctype>

namespace XML
{
	CharRange file_to_buffer( const char *filename )
	{
		using namespace std;

		ifstream is;
		is.open( filename, ios::binary );

		// get length of file:
		is.seekg (0, ios::end);
		int length = is.tellg();
		is.seekg (0, ios::beg);

		// allocate memory:
		char *buffer = new char[ length ];

		// read data as a block:
		is.read( buffer, length );
		is.close();

		return CharRange( buffer, buffer + length );
	}

	bool is_token( int c )
	{
		return strchr( "<>\\=", c ) != 0;
	}
	void eat_white( CharRange &range )
	{
		while( !range.empty() && isspace( range.front() ) )
			++range;
	}

	void require_char( CharRange &range, int ch )
	{
		eat_white( range );
		if( range.empty() || *range.begin != ch )
			throw UnexpectedCharacter();
		++range;
	}

	void read_to( CharRange &range, int ch )
	{
		while( !range.empty() && range.front() != ch )
			++range;
	}

	CharRange read_token( CharRange &range )
	{
		const char *begin = range.begin;
		while( !range.empty() && strchr( "<>\\=/ ", range.front() ) == 0 )
			++range;
		return CharRange( begin, range.begin );
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
				mant += (range.front() - '0') * mul;
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


	Reader::Reader( CharRange range ) : m_range( range )
	{
		require_char( range, '<' );
    if( range.front() == '?' ) // Ignore xml element
    {
		  read_to( range, '>' );
		  ++range;
		  require_char( range, '<' );
    }
		read_node( range );

		std::vector< CopyableNode >::iterator el = m_elements.begin(), end = m_elements.end();
		for( ; el != end; ++el )
		{
			el->property_begin = &m_properties[ 0 ] + el->property_begin_index;
			el->property_end = &m_properties[ 0 ] + el->property_end_index;
			std::sort( el->property_begin, el->property_end );
		}
	}

	void Reader::read_node( CharRange &range )
	{
		eat_white( range );

		CopyableNode element;
		element.next_sibling = 1;

		element.name = read_token( range );

		element.property_begin_index = m_properties.size();
		eat_white( range );
    // Read start tag
		while( !range.empty() && range.front() != '>' )
		{
			if( range.front() == '/' ) // Tag closed itself
			{
				++range;
				require_char( range, '>' );
				element.property_end_index = m_properties.size();
				m_elements.push_back( element );
				return;
			}
      // Parse properties
			Property property( read_token( range ) );
			require_char( range, '=' );
			require_char( range, '"' );
			property.value.begin = range.begin;
			read_to( range, '"' );
			property.value.end = range.begin;
			++range;
			m_properties.push_back( property );
			eat_white( range );
		}
    // Finished start tag - tidy up properties
		element.property_end_index = m_properties.size();
		int element_index = m_elements.size();
		m_elements.push_back( element );
		require_char( range, '>' );

    // Now we might encounter closing tag, child elements or content
		eat_white( range );
		while( !range.empty() )
		{
			if( range.front() == '<' ) // Closing tag or child element
			{
				++range;
				eat_white( range );
				if( range.front() == '/' )
				{
					++range;
					eat_white( range );
					if( read_token( range ) != element.name ) // Assumes token matches element name
          {
      			throw ClosingTagDoesNotMatch();
          }
          
					require_char( range, '>' );
					m_elements[ element_index ].next_sibling = m_elements.size() - element_index;
					return;
				}
				else
				{
					read_node( range );
				}
			}
			else // Content
			{
				m_elements[ element_index ].content.begin = range.begin;
				read_to( range, '<' );
				m_elements[ element_index ].content.end = range.begin;
			}
		}
	}

}

