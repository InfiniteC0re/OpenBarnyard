#include "pch.h"
#include "AAnimatableObjectTypeBunch.h"
#include "AAnimatableObjectManager.h"
#include "ARandom.h"

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
	if ( m_eType != TYPE_BUNCH )
		return;

	// The extra objects of the bunch are created from this type too, keep them from making bunches of their own
	TINT iBunchCount = m_iBunchCount;
	m_iBunchCount    = 1;

	for ( TINT i = 1; i < iBunchCount; i++ )
	{
		TRandom&    rRandom      = ARandom::GetSingleton()->m_oRandom;
		TQuaternion quatRotation = TQuaternion::IDENTITY;

		quatRotation.RotateAroundAxis( TVector4::VEC_NEGX.AsVector3(), rRandom.GetFloatMinMax( -m_fRange, m_fRange ) );
		quatRotation.RotateAroundAxis( TVector4::VEC_NEGY.AsVector3(), rRandom.GetFloatMinMax( -m_fRange, m_fRange ) );
		quatRotation.RotateAroundAxis( TVector4::VEC_NEGZ.AsVector3(), rRandom.GetFloatMinMax( -TMath::PI, TMath::PI ) );

		AAnimatableObject* pObject = AAnimatableObjectManager::GetSingleton()->CreateAnimatableObject( this, a_pAnimatableObject->GetOwner(), 0 );

		if ( !m_strChildBone.IsEmpty() || !m_strParentBone.IsEmpty() )
			AAnimatableObjectManager::GetSingleton()->AttachObject( pObject, a_pAnimatableObject, m_strChildBone, m_strParentBone, TVector4::VEC_ZERO, quatRotation, 1.0f );
		else
			AAnimatableObjectManager::GetSingleton()->AttachObject( pObject, a_pAnimatableObject, 0, 0, TVector4::VEC_ZERO, quatRotation, 1.0f );
	}

	m_iBunchCount = iBunchCount;
}
