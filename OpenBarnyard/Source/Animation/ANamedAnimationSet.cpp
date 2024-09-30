#include "pch.h"
#include "ANamedAnimationSet.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TDEFINE_CLASS( ANamedAnimationSet );

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00580e30
ANamedAnimationSet::ANamedAnimationSet()
    : m_mapAnimations( GetGlobalAllocator() )
{
	m_pAllocator   = GetGlobalAllocator();
	m_eOverlayType = OVERLAYTYPE_NONE;
}

// $Barnyard: FUNCTION 00580ea0
ANamedAnimationSet::ANamedAnimationSet( Toshi::T2Allocator* a_pAllocator )
    : m_mapAnimations( a_pAllocator )
{
	m_pAllocator   = a_pAllocator;
	m_eOverlayType = OVERLAYTYPE_NONE;
}

// $Barnyard: FUNCTION 00581820
// $Barnyard: FUNCTION 005815e0
ANamedAnimationSet::~ANamedAnimationSet()
{
	Destroy();
}

TPSTRING8_DECLARE( Name );
TPSTRING8_DECLARE( null );
TPSTRING8_DECLARE( blink );
TPSTRING8_DECLARE( lookl );
TPSTRING8_DECLARE( lookr );
TPSTRING8_DECLARE( looku );
TPSTRING8_DECLARE( lookd );

// $Barnyard: FUNCTION 00581840
TBOOL ANamedAnimationSet::CreateFromProperties( const PBProperties* a_pProperties, Toshi::TSkeleton* a_pSkeleton )
{
	// Get animation set name
	TPString8 strSetName;
	a_pProperties->GetOptionalPropertyValue( strSetName, TPS8( Name ) );

	m_strName      = strSetName;
	auto pNullAnim = m_mapAnimations.Find( TPS8( null ) );

	if ( !m_mapAnimations.IsValid( pNullAnim ) || pNullAnim.GetValue()->GetSecond() == TNULL )
	{
		// Create null (default) animation if it's not created yet
		ANamedAnimation oUnused( m_pAllocator );

		ANamedAnimation* pNullAnim = new ANamedAnimation( m_pAllocator );
		pNullAnim->Reset();

		m_mapAnimations.Insert( TPS8( null ), pNullAnim );
	}

	// Load all named animations
	T2_FOREACH( ( *a_pProperties ), it )
	{
		if ( it->GetValue()->GetType() == PBPropertyValue::Type::Properties &&
		     it->GetName().GetString()[ 0 ] == 'N' )
		{
			// NamedAnimation
			ANamedAnimation* pAnim = LoadNamedAnimation( it->GetValue()->GetProperties(), a_pSkeleton );

			if ( pAnim && pAnim->IsOverlay() )
			{
				if ( pAnim->GetName() == TPS8( blink ) )
					m_eOverlayType = OVERLAYTYPE_BLINK;
				else if ( pAnim->GetName() == TPS8( lookl ) )
					m_eOverlayType = OVERLAYTYPE_LOOK_LEFT;
				else if ( pAnim->GetName() == TPS8( lookr ) )
					m_eOverlayType = OVERLAYTYPE_LOOK_RIGHT;
				else if ( pAnim->GetName() == TPS8( looku ) )
					m_eOverlayType = OVERLAYTYPE_LOOK_UP;
				else if ( pAnim->GetName() == TPS8( lookd ) )
					m_eOverlayType = OVERLAYTYPE_LOOK_DOWN;
			}
		}
	}

	// Load transitions
	T2_FOREACH( ( *a_pProperties ), it )
	{
		if ( it->GetValue()->GetType() == PBPropertyValue::Type::Properties &&
		     it->GetName().GetString()[ 0 ] == 'A' )
		{
			TBOOL bRes = LoadAnimationTransition( it->GetValue()->GetProperties() );

			TASSERT( bRes == TTRUE );
		}
	}

	return TTRUE;
}

void ANamedAnimationSet::Destroy()
{
	// Delete all named animations
	while ( !m_mapAnimations.IsEmpty() )
	{
		auto             it    = m_mapAnimations.Begin();
		ANamedAnimation* pAnim = it->GetSecond();

		if ( pAnim )
			delete pAnim;

		m_mapAnimations.Remove( it );
	}

	m_eOverlayType = OVERLAYTYPE_NONE;
	m_strName      = TPString8();
}

// $Barnyard: FUNCTION 00580d50
TBOOL ANamedAnimationSet::AddSoundBreakpoint( ASoundBreakpoint* a_pSoundBreakpoint, Toshi::TSkeleton* a_pSkeleton )
{
	return TTRUE;
}

// $Barnyard: FUNCTION 00581700
ANamedAnimation* ANamedAnimationSet::LoadNamedAnimation( const PBProperties* a_pProperties, Toshi::TSkeleton* a_pSkeleton )
{
	// Get animation name
	TPString8 strName;
	a_pProperties->GetOptionalPropertyValue( strName, TPS8( Name ) );

	ANamedAnimation* pNamedAnimation = new ANamedAnimation( m_pAllocator );

	if ( pNamedAnimation->Create( a_pProperties, a_pSkeleton ) )
	{
		m_mapAnimations.Insert( strName, pNamedAnimation );
		return pNamedAnimation;
	}

	// Couldn't create animation, free memory
	delete pNamedAnimation;
	return TNULL;
}

TPSTRING8_DECLARE( BlendInTime );
TPSTRING8_DECLARE( BlendOutTime );
TPSTRING8_DECLARE( TransitionName );
TPSTRING8_DECLARE( AutoTransition );
TPSTRING8_DECLARE( FinishType );
TPSTRING8_DECLARE( LinkTo );

// $Barnyard: FUNCTION 00580f10
TBOOL ANamedAnimationSet::LoadAnimationTransition( const PBProperties* a_pProperties )
{
	TFLOAT fBlendInTime;
	TBOOL  bHasBlendIn = a_pProperties->GetOptionalPropertyValue( fBlendInTime, TPS8( BlendInTime ) );

	TFLOAT fBlendOutTime;
	TBOOL  bHasBlendOut = a_pProperties->GetOptionalPropertyValue( fBlendOutTime, TPS8( BlendOutTime ) );

	TPString8 strTransitionName;
	a_pProperties->GetOptionalPropertyValue( strTransitionName, TPS8( TransitionName ) );

	TBOOL bIsAutoTransition;
	a_pProperties->GetOptionalPropertyValue( bIsAutoTransition, TPS8( AutoTransition ) );

	TPString8                   strFinishType;
	ANamedAnimation::FINISHTYPE eFinishType    = ANamedAnimation::FINISHTYPE_MANUAL;
	TBOOL                       bHasFinishType = a_pProperties->GetOptionalPropertyValue( strFinishType, TPS8( FinishType ) );

	if ( bHasFinishType )
		eFinishType = ANamedAnimation::GetFinishType( strFinishType );

	// Get animation to link to
	TPString8 strLinkTo;
	a_pProperties->GetOptionalPropertyValue( strLinkTo, TPS8( LinkTo ) );

	auto pLinkTo = m_mapAnimations.Find( strLinkTo );
	if ( m_mapAnimations.IsValid( pLinkTo ) && pLinkTo->GetSecond() != TNULL )
	{
		const PBPropertyValue* pLinkFromProperty = a_pProperties->GetProperty( "LinkFrom" );
		TVALIDPTR( pLinkFromProperty );

		if ( pLinkFromProperty->GetType() == PBPropertyValue::Type::String )
		{
			auto pLinkFrom = m_mapAnimations.Find( pLinkFromProperty->GetTPString8() );

			if ( !m_mapAnimations.IsValid( pLinkFrom ) || pLinkFrom->GetSecond() == TNULL )
			{
				TERROR( "Invalid LinkFrom property specified." );
				return TFALSE;
			}

			TASSERT( !"Not implemented" );
			return TTRUE;
		}
	}

	return TFALSE;
}
