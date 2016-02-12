#ifndef ANIMATION_H
#define ANIMATION_H

#include "common/shared.h"
#include "math/vec3.h"
#include "math/quat.h"

#include <vector>
#include <algorithm>

class TextureCube;

class SceneNode;
class SceneMesh;
class SceneLight;
class SceneCamera;

class Animation : public Shared
{
public:
	typedef SharedPtr< Animation > Ptr;

	virtual void update( double time ) = 0;

	virtual ~Animation() {}
};

class AnimationGroup : public Animation
{
public:
	typedef SharedPtr< AnimationGroup > Ptr;

    virtual void update( double time ) override { for( auto &a : m_animations ) a->update( time ); }

	void add( Animation::Ptr const &anim ) { m_animations.push_back( anim ); }
private:
	std::vector< Animation::Ptr > m_animations;
};

inline float3 interp( float3 const &a, float3 const &b, double t ) {return a + t * ( b - a );}
inline floatq interp( floatq const &a, floatq const &b, double t ) {return slerp( a, b, (float)t );}


template< typename T >
class KeyData : public Shared
{
public:
	typedef SharedPtr< KeyData< T > > Ptr;

	T get( double time )
	{
		if( m_times.empty() )
			return T();

		auto after = std::upper_bound( m_times.begin(), m_times.end(), time );

		if( after == m_times.end() )
			return m_keys.back();
		if( after == m_times.begin() )
			return m_keys.front();

		int i_after = after - m_times.begin();
		int i_before = i_after - 1;

		return interp( m_keys[i_before], m_keys[i_after],
			( time - m_times[i_before] ) / ( m_times[i_after] - m_times[i_before] ) );
	}

	void reserve( int size )
	{
		m_times.reserve( size );
		m_keys.reserve( size );
	}

	void push_back( double time, T const &key )
	{
		m_times.push_back( time );
		m_keys.push_back( key );
	}

private:
	std::vector< double > m_times;
	std::vector< T > m_keys;
};

#endif // ANIMATION_H
