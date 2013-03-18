#ifndef RENDERSTATE_H
#define RENDERSTATE_H

#include "common/shared.h"


class RenderState : public Shared
{
public:
	typedef SharedPtr< RenderState > Ptr;

	enum BlendMode
	{
	    Opaque,
	    Transparent,
	    Blend,
	    Add
	};

	enum Compare
	{
		Never,
		Less,
		Equal,
		LEqual,
		Greater,
		NotEqual,
		GEqual,
		Always
	};

	explicit RenderState( BlendMode blend_mode = Opaque ); //,
	// bool double_sided = false );

	void blend_mode( BlendMode mode );

	void depth_write( bool f );
	void depth_test( bool f );
	void depth_compare( Compare comp );

	void colour_write( bool f );

	void draw_front( bool f );
	void draw_back( bool f );

	void bind();

	static Ptr const &stock_opaque();

private:
	void unbind();

	BlendMode m_blend_mode;
	Compare m_depth_compare;
	bool m_depth_write;
	bool m_depth_test;
	bool m_draw_front;
	bool m_draw_back;
	bool m_colour_write;
};

#endif // RENDERSTATE_H

