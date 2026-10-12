#include "pch.h"
#include "AUnitFlagSetsManager.h"
#include "Memory/AMemory.h"

#include <File/TTRB.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: CreateObject 005d21f0, CreateObjectInPlace 005d2250
// NOTE: GetClass 005d20a0, CreateObject 005d21f0, CreateObjectInPlace 005d2250
TDEFINE_CLASS( AUnitFlagSetsManager );

// $Barnyard: FUNCTION 005d1aa0
AUnitFlagSet::AUnitFlagSet()
    : m_iNumFlags( 0 )
    , m_iNumGroups( 0 )
    , m_oMasks( AMemory::GetAllocator( AMemory::POOL_Misc ) )
{
	for ( TINT i = 0; i < MAX_NUM_GROUPS; i++ )
		m_aGroups[ i ].uiMask = 0;
}

// $Barnyard: FUNCTION 005d2080
// $Barnyard: FUNCTION 005d1ed0
AUnitFlagSet::~AUnitFlagSet()
{
}

// $Barnyard: FUNCTION 005d1f40
TBOOL AUnitFlagSet::Load( const TCHAR* a_szFileName )
{
	TTRB oTRB;

	if ( oTRB.Load( a_szFileName ) != TTRB::ERROR_OK )
		return TFALSE;

	const PBProperties* pProperties = PBProperties::LoadFromTRB( oTRB );
	oTRB.DeleteSymbolTable();

	// The flags come first, the masks are made of them
	for ( auto it = pProperties->Begin(); it != pProperties->End(); it++ )
	{
		const TCHAR* szName = it->GetName().GetString();

		if ( szName[ 0 ] != 'm' || TStringManager::String8Compare( szName, "maskgroup" ) != 0 )
			AddGroup( it->GetValue()->GetArray() );
	}

	for ( auto it = pProperties->Begin(); it != pProperties->End(); it++ )
	{
		const TCHAR* szName = it->GetName().GetString();

		if ( szName[ 0 ] == 'm' && TStringManager::String8Compare( szName, "maskgroup" ) == 0 )
			AddMaskGroup( it->GetValue()->GetProperties() );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005d1890
void AUnitFlagSet::AddGroup( const PBPropertyValueArray* a_pFlags )
{
	if ( !a_pFlags || a_pFlags->GetSize() == 0 )
		return;

	Group& rGroup     = m_aGroups[ m_iNumGroups++ ];
	rGroup.iFirstFlag = m_iNumFlags;

	for ( TUINT i = 0; i < a_pFlags->GetSize(); i++ )
		m_aFlagNames[ m_iNumFlags++ ] = a_pFlags->GetValue( i )->GetTPString8();

	rGroup.iEndFlag = m_iNumFlags;
	rGroup.uiMask   = 0;

	for ( TINT i = rGroup.iFirstFlag; i < rGroup.iEndFlag; i++ )
		rGroup.uiMask |= 1U << ( i & 31 );
}

// $Barnyard: FUNCTION 005d1d40
void AUnitFlagSet::AddMaskGroup( const PBProperties* a_pProperties )
{
	TPString8 strName;
	a_pProperties->GetOptionalPropertyValue( strName, "name" );

	// NOTE: retail doesn't check that "flags" exists
	const PBPropertyValueArray* pFlags = a_pProperties->GetProperty( "flags" )->GetArray();

	if ( pFlags->GetSize() != 0 )
	{
		TUINT uiMask = 0;

		for ( TUINT i = 0; i < pFlags->GetSize(); i++ )
		{
			// NOTE: retail inlines GetFlagIndex, so an unknown flag (-1) sets bit 31
			TINT iFlag = GetFlagIndex( pFlags->GetValue( i )->GetTPString8() );
			uiMask |= 1U << ( iFlag & 31 );
		}

		m_oMasks.Insert( strName, uiMask );
	}
}

// $Barnyard: FUNCTION 005d1720
TINT AUnitFlagSet::GetFlagIndex( const TPString8& a_rcName ) const
{
	for ( TINT i = 0; i < m_iNumFlags; i++ )
	{
		if ( a_rcName == m_aFlagNames[ i ] )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 005d1750
TBOOL AUnitFlagSet::GetMask( TUINT& a_ruiOutMask, const TPString8& a_rcName )
{
	auto it = m_oMasks.Find( a_rcName );

	if ( it == m_oMasks.End() )
		return TFALSE;

	a_ruiOutMask = it->GetSecond();
	return TTRUE;
}

// $Barnyard: FUNCTION 005d17a0
TBOOL AUnitFlagSet::CanSetFlag( const TUINT& a_ruiFlags, TINT a_iFlag ) const
{
	for ( TINT i = 0; i < m_iNumGroups; i++ )
	{
		const Group& rGroup = m_aGroups[ i ];

		// NOTE: retail counts iEndFlag as part of the group here
		if ( rGroup.iFirstFlag <= a_iFlag && a_iFlag <= rGroup.iEndFlag )
		{
			for ( TINT k = rGroup.iFirstFlag; k < rGroup.iEndFlag; k++ )
			{
				if ( k != a_iFlag && ( a_ruiFlags & ( 1U << ( k & 31 ) ) ) )
					return TFALSE;
			}

			return TTRUE;
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005d1820
void AUnitFlagSet::SetGroupFlags( TUINT& a_ruiFlags, TINT a_iFlag ) const
{
	for ( TINT i = 0; i < m_iNumGroups; i++ )
	{
		const Group& rGroup = m_aGroups[ i ];

		// NOTE: retail counts iEndFlag as part of the group here
		if ( rGroup.iFirstFlag <= a_iFlag && a_iFlag <= rGroup.iEndFlag )
		{
			a_ruiFlags |= rGroup.uiMask;
			return;
		}
	}
}

// $Barnyard: FUNCTION 005d20c0
AUnitFlagSetsManager::AUnitFlagSetsManager()
{
	m_aFlagSets[ FLAGSET_RELATIONSHIPS ].Load( "Data/SocialGroups/Relationships.trb" );
	m_aFlagSets[ FLAGSET_INTERESTS ].Load( "Data/SocialGroups/Interests.trb" );
	m_aFlagSets[ FLAGSET_REPUTATIONS ].Load( "Data/SocialGroups/Reputations.trb" );
}

// $Barnyard: FUNCTION 005d2290
// $Barnyard: FUNCTION 005d2170
AUnitFlagSetsManager::~AUnitFlagSetsManager()
{
}
