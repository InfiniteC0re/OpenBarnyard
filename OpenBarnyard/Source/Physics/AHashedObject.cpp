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

TBOOL AHashedObject::Unknown3()
{
	return TFALSE;
}

void AHashedObject::SetUnk2( void* a_pUnk )
{
	m_Unk2 = a_pUnk;
}

void* AHashedObject::GetUnk2()
{
	return m_Unk2;
}
