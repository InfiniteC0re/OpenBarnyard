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
ANamedAnimationSet::ANamedAnimationSet() :
    m_mapAnimations( GetGlobalAllocator() )
{
	m_pAllocator = GetGlobalAllocator();
	m_bFlag      = TFALSE;
}

// $Barnyard: FUNCTION 00580ea0
ANamedAnimationSet::ANamedAnimationSet( Toshi::T2Allocator* a_pAllocator ) :
    m_mapAnimations( a_pAllocator )
{
	m_pAllocator = a_pAllocator;
	m_bFlag      = TFALSE;
}

// $Barnyard: FUNCTION 00581820
// $Barnyard: FUNCTION 005815e0
ANamedAnimationSet::~ANamedAnimationSet()
{
	Delete();
}

TPSTRING8_DECLARE( Name );
TPSTRING8_DECLARE( null );

// $Barnyard: FUNCTION 00581840
TBOOL ANamedAnimationSet::CreateFromProperties( const PBProperties* a_pProperties, Toshi::TSkeleton* a_pSkeleton )
{
	// Get animation set name
	TPString8 strSetName;
	a_pProperties->GetOptionalPropertyValue( strSetName, TPS8( Name ) );

	m_strName = strSetName;

	if ( m_mapAnimations.FindNode( TPS8( null ) ) == m_mapAnimations.End() )
	{
		// Create null (default) animation if it's not created yet
		ANamedAnimation oUnused( m_pAllocator );

		ANamedAnimation* pNullAnim = new ANamedAnimation( m_pAllocator );
		pNullAnim->Reset();

		m_mapAnimations.Insert( TPS8( null ), pNullAnim );
	}

	return TTRUE;
}

void ANamedAnimationSet::Delete()
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

	m_bFlag   = TFALSE;
	m_strName = TPString8();
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
