#pragma once

#include <algorithm>

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

template < typename Element, typename Predicate >
bool NoneOf( const Array< Element >& aArray, Predicate oPredicate )
{
	return std::none_of( aArray.begin(), aArray.end(), oPredicate );
}

template < typename Element, typename Predicate >
bool AnyOf( const Array< Element >& aArray, Predicate oPredicate )
{
	return std::any_of( aArray.begin(), aArray.end(), oPredicate );
}

template < typename Element, typename Predicate >
bool AllOf( const Array< Element >& aArray, Predicate oPredicate )
{
	return std::all_of( aArray.begin(), aArray.end(), oPredicate );
}
