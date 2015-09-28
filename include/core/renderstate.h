#ifndef RENDERSTATE_H
#define RENDERSTATE_H

#include "common/shared.h"
#include "common/valuepack.h"
#include <cstdint>


enum class BlendMode
{
    Opaque,
    Transparent,
    Blend,
    Add,
    Count
};

enum class Compare
{
    Never,
    Less,
    Equal,
    LEqual,
    Greater,
    NotEqual,
    GEqual,
    Always,
    Count
};

class RenderState
{
public:
    explicit RenderState( BlendMode blend_mode = BlendMode::Opaque );

	void blend_mode( BlendMode mode );

	void depth_write( bool f );
	void depth_test( bool f );
	void depth_compare( Compare comp );

	void colour_write( bool f );

	void draw_front( bool f );
	void draw_back( bool f );

	void bind();

    static RenderState stock_opaque();

private:
    struct Property { enum Properties
    {
        BlendMode,
        DepthCompare,
        DepthWrite,
        DepthTest,
        DrawFront,
        DrawBack,
        ColourWrite
    }; };

    using Pack = grt::value_pack<std::uint32_t, BlendMode, Compare, bool, bool, bool, bool, bool>;
    //using Pack = grt::enum_indexed_value_pack<Property, std::uint32_t, BlendMode, Compare, bool, bool, bool, bool, bool>;
    Pack m_pack;

/*
    template<Property p, class T> void set(T v ) { m_pack.set<static_cast< size_t >( p )>( v ); }
    //template<Property p>
    //auto get() -> Pack::type_at<static_cast< size_t >( p )> { return m_pack.get<static_cast< size_t >( p )>(); }
    template<Property p>
    auto get()->decltype( m_pack.get<static_cast< size_t >( p )>() ) { return m_pack.get<static_cast< size_t >( p )>(); }
    template<Property p>
*/
    template<size_t p, class T> void set( T v ) { m_pack.set<p>( v ); }
    template<size_t p> Pack::type_at<p> get() { return m_pack.get<p>(); }


    template<size_t p>
    bool differ( RenderState other ) { return get<p>() != other.get<p>(); }
};

#endif // RENDERSTATE_H

