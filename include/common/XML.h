#ifndef XML_H
#define XML_H

//#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <string>

namespace XML
{

	struct CharRange
	{
		CharRange() : begin( 0 ), end( 0 ) {}
		CharRange( const char *begin, const char *end ) : begin( begin ), end( end ) {}
		CharRange( const char *begin ) : begin( begin ), end( begin + strlen( begin ) ) {}

		bool empty() const {return begin == end;}
		char front() const {return *begin;}
		CharRange &operator++() {++begin; return *this;}


		const char *begin;
		const char *end;
	};

	inline bool operator==( CharRange const &a, CharRange const &b )
	{
#pragma warning( push )
#pragma warning( disable : 4996 )
		return a.end - a.begin == b.end - b.begin && std::equal( a.begin, a.end, b.begin );
#pragma warning( pop )
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
	CharRange read_token( CharRange &range );
	float read_float( CharRange &range, float def = 0.0f );
	int read_int( CharRange &range, int def = 0 );

	inline std::string to_std_string( CharRange const &c ) {return std::string( c.begin, c.end );}

	CharRange file_to_buffer( const char *filename );

	class Property
	{
	public:
		Property( CharRange const &name, CharRange const &value = CharRange() ) : name( name ), value( value ) {}
		CharRange name;
		CharRange value;
	};

	inline bool operator<( Property const &a, Property const &b )
	{
		return a.name < b.name;
	}


	struct comp_property_name
	{
		bool operator()( CharRange const &name, Property const &a )
		{
			return name < a.name;
		}
		bool operator()( Property const &a, CharRange const &name )
		{
			return a.name < name;
		}
		bool operator()( Property const &a, Property const &b )
		{
			return a.name < b.name;
		}

	};

	class NodeData
	{
	public:
		int next_sibling;
		CharRange name;
		CharRange content;
		union
		{
			int property_begin_index;
			Property *property_begin;
		};
		union
		{
			int property_end_index;
			Property *property_end;
		};
		int parent;
	};

	class CopyableNode;

	class Node : public NodeData
	{
	public:
		CharRange const &property( CharRange const &name ) const
		{
			static CharRange empty;
			//Property *pos = std::lower_bound( property_begin_it, property_end_it, name );
			Property *pos = std::lower_bound( property_begin, property_end, name, comp_property_name() );
			if( pos != property_end && pos->name == name )
				return pos->value;
			else
				return empty;
		}

		class iterator : public std::iterator< std::forward_iterator_tag, Node >
		{
		public:
			inline iterator( Node const *p, int offset );
			inline Node const &operator*() const;
			inline Node const *operator->() const;
			inline iterator &operator++();
			inline iterator operator++( int );
			inline bool operator==( iterator const &other ) const;
			inline bool operator!=( iterator const &other ) const;
		protected:
			CopyableNode const *p;
		};

		class child_iterator : public iterator
		{
		public:
			inline child_iterator( Node const *p, int offset ) : iterator( p, offset ) {}
			inline child_iterator &operator++();
			inline child_iterator operator++( int );
		};

		iterator begin() const {return iterator( this, 1 );}
		iterator end() const {return iterator( this, next_sibling );}

		child_iterator child_begin() const {return child_iterator( this, 1 );}
		child_iterator child_end() const {return child_iterator( this, next_sibling );}

	protected:
		Node() {}
		Node( Node const &other ) : NodeData( other ) {}
		Node &operator=( Node const &other ) { NodeData::operator=( other ); return *this; }
	};


	class CopyableNode : public Node
	{
	public:
		CopyableNode() {}
		CopyableNode( CopyableNode const &other ) : Node( other ) {}
		CopyableNode &operator=( Node const &other ) {Node::operator=( other ); return *this;}
	};

	inline Node::iterator::iterator( Node const *p, int offset ) : p( static_cast< CopyableNode const * >( p ) + offset ) {}
	inline Node const &Node::iterator::operator*() const {return *p;}
	inline Node const *Node::iterator::operator->() const {return p;}
	inline Node::iterator &Node::iterator::operator++() {++p; return *this;}
	inline Node::iterator Node::iterator::operator++( int ) { iterator temp( *this ); ++p; return temp;}
	inline bool Node::iterator::operator==( iterator const &other ) const {return p == other.p;}
	inline bool Node::iterator::operator!=( iterator const &other ) const {return p != other.p;}

	inline Node::child_iterator &Node::child_iterator::operator++() {p += p->next_sibling; return *this;}
	inline Node::child_iterator Node::child_iterator::operator++( int ) { child_iterator temp( *this ); p += p->next_sibling; return temp;}

	class Exception : public std::exception {};
	class ClosingTagDoesNotMatch : public Exception {};
	class PropertiesAddedAfterContent : public Exception {};
	class UnexpectedCharacter : public Exception {};

	class Reader
	{
	public:
		Reader( CharRange range );

		void read_node( CharRange &range );

		Node::iterator begin() const {return Node::iterator( &m_elements[0], 0 ); }
		Node::iterator end() const {return Node::iterator( &m_elements[0], m_elements[0].next_sibling ); }

		Node const &root() const {return m_elements[0];}

	private:
		CharRange m_range;
		std::vector< CopyableNode > m_elements;
		std::vector< Property > m_properties;
	};
	struct property_match
	{
		property_match( CharRange const &name, CharRange const &value ) : name( name ), value( value ) {}
		bool operator()( Node const &el ) {return el.property( name ) == value;}
		CharRange name, value;
	};

	struct name_match
	{
		name_match( CharRange const &name ) : name( name ) {}
		bool operator()( Node const &el ) {return el.name == name;}
		CharRange name;
	};

  class OutNode;

  class Writer
  {
  public:
    Writer( std::ostream &stream ) : m_stream( stream ) {}

    std::ostream &stream() {return m_stream;}

    void push_node( OutNode *node ) {m_node_stack.push_back( node );}
    OutNode *top_node() {return m_node_stack.empty() ? 0 : m_node_stack.back();}
    void pop_node() {m_node_stack.pop_back();}
    int node_depth() {return m_node_stack.size();}
  private:
    std::ostream &m_stream;
    std::vector< OutNode * > m_node_stack;
  };

  class OutNode
  {
  public:
    OutNode( Writer &writer, char const *name ) : m_writer( writer ), m_has_children( false ), m_name( name )
    {
      if( writer.top_node() )
      {
        writer.top_node()->end_tag();
      }
      for( int i = 0; i < writer.node_depth(); ++i )
        writer.stream() << "  ";

      writer.push_node( this );
      writer.stream() << "<" << name;
    }

    ~OutNode()
    {
      if( !m_has_children )
      {
        m_writer.stream() << "/>\n";
      }
      else
      {
        m_writer.stream() << "</" << m_name << ">\n";
      }
      m_writer.pop_node();
    }

    void add_property( char const *name, char const *value )
    {
      if( m_has_children )
        throw PropertiesAddedAfterContent();
      m_writer.stream() << " " << name << "=\"" << value << "\"";
    }

    void set_content( char const *content, bool indent = false )
    {
      end_tag();
      m_writer.stream() << content;
    }

  private:
    void end_tag()
    {
      if( !m_has_children )
      {
        m_writer.stream() << ">\n";
        m_has_children = true;
      }
    }
    Writer &m_writer;
    bool m_has_children;
    std::string m_name;
  };
}

#endif
