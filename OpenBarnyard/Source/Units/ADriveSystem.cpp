#include "pch.h"
#include "ADriveSystem.h"
#include "ADriveComponent.h"
#include "AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0054e450, CreateObject 0054e5b0, CreateObjectInPlace 0054e630 (the constructor is inlined in both)
TDEFINE_CLASS( ADriveSystem );

ADriveSystem::ADriveSystem()
    : m_iNumGroups( 0 )
    , m_pGroups( TNULL )
    , m_fUnk14( 2.0f )
    , m_iDataSize( 0 )
{
}

// $Barnyard: FUNCTION 0054e460
// $Barnyard: FUNCTION 0054e6b0
ADriveSystem::~ADriveSystem()
{
	if ( m_pGroups )
		delete[] m_pGroups;
}

// $Barnyard: FUNCTION 0054e770
TBOOL ADriveSystem::Create( const PBProperties* a_pProperties )
{
	TPString8 strGroup = TPS8D( "group" );

	// Count the groups first
	TINT iNumProperties = a_pProperties->GetPropertyCount();

	for ( TINT i = 0; i < iNumProperties; i++ )
	{
		if ( TPString8( a_pProperties->GetPropertyByIndex( i )->GetName().GetString() ) == strGroup )
			m_iNumGroups++;
	}

	if ( m_iNumGroups > 0 )
	{
		m_pGroups = new ADriveGroup[ m_iNumGroups ];

		TINT iGroup = 0;

		for ( TINT i = 0; i < iNumProperties; i++ )
		{
			const PBProperties::PBProperty* pProperty = a_pProperties->GetPropertyByIndex( i );

			if ( TPString8( pProperty->GetName().GetString() ) == strGroup )
				m_pGroups[ iGroup++ ].Create( pProperty->GetValue()->GetProperties(), &m_iDataSize );
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0054e6d0
void ADriveSystem::OnUnitCreate( AUnit* a_pUnit )
{
	ADriveComponent*          pComponent = new ADriveComponent();
	AInteractionActivityData* pData      = pComponent->Initialise( a_pUnit, this );
	a_pUnit->AddComponent( pComponent );

	for ( TINT i = 0; i < m_iNumGroups; i++ )
		m_pGroups[ i ].InitData( a_pUnit, pData );
}

// $Barnyard: FUNCTION 0054e520
ADrive* ADriveSystem::SelectDrive( AUnit* a_pUnit, AInteractionActivityData* a_pData )
{
	ADrive::Rating oBest;
	oBest.pDrive = TNULL;
	oBest.fValue = 0.0f;

	for ( TINT i = m_iNumGroups - 1; i >= 0; i-- )
		m_pGroups[ i ].Evaluate( a_pUnit, a_pData, &oBest );

	return oBest.pDrive;
}

// $Barnyard: FUNCTION 0054e570
ADrive* ADriveSystem::FindDrive( const TPString8& a_rcName )
{
	for ( TINT i = 0; i < m_iNumGroups; i++ )
	{
		ADrive* pDrive = m_pGroups[ i ].FindDrive( a_rcName );

		if ( pDrive )
			return pDrive;
	}

	return TNULL;
}
