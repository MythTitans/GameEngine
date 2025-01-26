#pragma once

#include "Array.h"

template < typename Element >
bool Contains( const Array< Element >& aArray, const Element& oElement )
{
	return Find( aArray, oElement ) != -1;
}

template < typename Element >
int Find( const Array< Element >& aArray, const Element& oElement )
{
	for( uint u = 0; u < aArray.Count(); ++u )
	{
		if( aArray[ u ] == oElement )
			return ( int )u;
	}

	return -1;
}

template < typename Element, typename Predicate >
void Sort( Array< Element >& aArray, Predicate oPredicate )
{
	std::sort( std::begin( aArray ), std::end( aArray ), oPredicate );
}
