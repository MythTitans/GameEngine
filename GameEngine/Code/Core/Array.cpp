#include "Array.h"

ArrayBase::ArrayBase()
	: m_uCount( 0 )
	, m_uCapacity( 0 )
{
}

ArrayBase::ArrayBase( const uint uCount, const uint uCapacity )
	: m_uCount( uCount )
	, m_uCapacity( uCapacity )
{
}

ArrayBase::~ArrayBase()
{
	m_uCount = 0;
	m_uCapacity = 0;
}

uint ArrayBase::Count() const
{
	return m_uCount;
}

uint ArrayBase::Capacity() const
{
	return m_uCapacity;
}

bool ArrayBase::Empty() const
{
	return m_uCount == 0;
}
