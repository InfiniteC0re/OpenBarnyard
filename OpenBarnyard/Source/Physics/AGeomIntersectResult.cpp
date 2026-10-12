#include "pch.h"
#include "AGeomIntersectResult.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005795b0, CreateObject 00579880, CreateObjectInPlace 00579840
TDEFINE_CLASS( AGeomIntersectResult, 1, 0 );

// NOTE: GetClass 005795a0, CreateObject 00579610, CreateObjectInPlace 005796a0
TDEFINE_CLASS( AGeomRayIntersectResult, 1, 0 );

// NOTE: inlined into the constructor of AGeomIntersectResult
AGeomContactList::AGeomContactList()
    : m_aContacts( 10, 0 )
{
}

// $Barnyard: FUNCTION 005795e0
// $Barnyard: FUNCTION 005795c0
AGeomContactList::~AGeomContactList()
{
}

// $Barnyard: FUNCTION 0057a370
void AGeomContactList::AddContact( const TVector3& a_rcPosition, const TVector3& a_rcNormal, TFLOAT a_fDepth )
{
	Contact oContact;
	oContact.vPosition = a_rcPosition;
	oContact.vNormal   = a_rcNormal;
	oContact.fDepth    = a_fDepth;

	if ( m_bFlipNormals )
	{
		oContact.vNormal.x = oContact.vNormal.x * -1.0f;
		oContact.vNormal.y = oContact.vNormal.y * -1.0f;
		oContact.vNormal.z = oContact.vNormal.z * -1.0f;
	}

	m_aContacts.Push( oContact );
}

// $Barnyard: FUNCTION 00579730
AGeomIntersectResult::AGeomIntersectResult()
{
}

// $Barnyard: FUNCTION 005797b0
// $Barnyard: FUNCTION 005797d0
AGeomIntersectResult::~AGeomIntersectResult()
{
}

// NOTE: inlined into CreateObject / CreateObjectInPlace, the destructor is a shared one (0057fd50)
AGeomRayIntersectResult::AGeomRayIntersectResult()
    : m_iUnk10( 0 )
    , m_fDistance( TMath::MAXFLOAT )
{
}
