#include "pch.h"
#include "AHashedObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AHashedObject::AHashedObject()
    : m_pHashNode( TNULL )
{
}

// $Barnyard: FUNCTION 0061ef10
AHashedObject::~AHashedObject()
{
}

TBOOL AHashedObject::IsNoQuery()
{
	return TFALSE;
}

void AHashedObject::SetQueryStamp( void* a_pUnk )
{
	m_pQueryStamp = a_pUnk;
}

void* AHashedObject::GetQueryStamp()
{
	return m_pQueryStamp;
}
