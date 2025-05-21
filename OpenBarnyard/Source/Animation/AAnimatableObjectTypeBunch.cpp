#include "pch.h"
#include "AAnimatableObjectTypeBunch.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0057f950
AAnimatableObjectTypeBunch::AAnimatableObjectTypeBunch()
    : m_iBunchCount( 1 )
{
}

// $Barnyard: FUNCTION 0057fca0
// $Barnyard: FUNCTION 0057f980
AAnimatableObjectTypeBunch::~AAnimatableObjectTypeBunch()
{
	AAnimatableObjectType::Destroy();
}

// $Barnyard: FUNCTION 0057fa20
TBOOL AAnimatableObjectTypeBunch::CreateFromProperties( const PBProperties* a_pProperties, const Toshi::TPString8& a_rcName )
{
	// Parse base properties
	AAnimatableObjectType::CreateFromProperties( a_pProperties, a_rcName );

	// Parse bunch properties
	a_pProperties->GetOptionalPropertyValue( m_iBunchCount, TPS8D( "BunchCount" ) );

	m_eType = TYPE_BUNCH;

	TPString8 strType;
	a_pProperties->GetOptionalPropertyValue( strType, TPS8D( "Type" ) );

	if ( strType == TPS8D( "Bunch" ) )
		m_eType = TYPE_BUNCH;

	m_strChildBone = TPString8();
	a_pProperties->GetOptionalPropertyValue( m_strChildBone, TPS8D( "ChildBone" ) );

	m_strParentBone = TPString8();
	a_pProperties->GetOptionalPropertyValue( m_strParentBone, TPS8D( "ParentBone" ) );

	a_pProperties->GetOptionalPropertyValue( m_fRange, TPS8D( "range" ) );

	return TTRUE;
}

// $Barnyard: FUNCTION 0057f6f0
void AAnimatableObjectTypeBunch::OnAnimatableObjectCreated( AAnimatableObject* a_pAnimatableObject )
{
	TASSERT( !"Not implemented" );
}
