#include "core/uniform.h"

#include <string>
#include <set>

namespace
{
   std::set< UniformInfo > g_uniform_info;
}

bool UniformInfo::operator<( UniformInfo const &id2 ) const
{
	if( name == id2.name )
		return type < id2.type;
	else
		return name < id2.name;
}

UniformInfo const *UniformInfo::get( char const *name, int type )
{
    UniformInfo id;
    id.name = name;
    id.type = type;
    id.count = 1;
    id.location = -1;
    id.texture_unit = -1;

	auto loc = g_uniform_info.find( id );
	if( loc != g_uniform_info.end() )
		return &(*loc);
    
	auto new_info = g_uniform_info.insert( id );
	return &(*new_info.first);
}

UniformBase *UniformGroup::get( char const *name, int type, bool is_array )
{
	for( auto uni = m_uniforms.begin(); uni != m_uniforms.end(); ++uni )
		if( (*uni)->info->name == name &&
			(*uni)->info->type == type &&
			(*uni)->is_array   == is_array ) return uni->get();

	return 0;
}


void UniformGroup::bind() const
{
	for( auto uni = m_uniforms.begin(); uni != m_uniforms.end(); ++uni )
		(*uni)->bind();
}
