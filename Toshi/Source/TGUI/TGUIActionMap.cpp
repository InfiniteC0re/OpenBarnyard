#include "ToshiPCH.h"
#include "TGUIActionMap.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TGUIActionMap );

// NOTE: the labels are cleared here (JPOG clears them, Nicktoons leaves them uninitialised)
// $NicktoonsPS2: FUNCTION 00384e70
// $JPOG: FUNCTION TGuiInterface.dll+00005550
TGUIActionMap::TGUIActionMap()
    : m_aMappings( 10, 0 )
{
	for ( TINT i = 0; i < MAX_ACTIONS; i++ )
		m_awszLabels[ i ] = TNULL;
}

// $NicktoonsPS2: FUNCTION 00384ec0
// $JPOG: FUNCTION TGuiInterface.dll+000062b0
TGUIActionMap::~TGUIActionMap()
{
}

// NOTE: inlined in AddActionMapping and TranslateDoodadToAction
TINT TGUIActionMap::FindMapping( TINT a_iDoodad ) const
{
	TINT iLow  = 0;
	TINT iHigh = m_aMappings.Size() - 1;

	while ( iLow <= iHigh )
	{
		TINT iMiddle  = ( iLow + iHigh ) / 2;
		TINT iCompare = a_iDoodad - m_aMappings[ iMiddle ].iDoodad;

		if ( iCompare < 0 )
		{
			iHigh = iMiddle - 1;
		}
		else if ( iCompare > 0 )
		{
			iLow = iMiddle + 1;
		}
		else
		{
			// The first mapping of the doodad
			while ( iMiddle - 1 >= iLow && m_aMappings[ iMiddle - 1 ].iDoodad == a_iDoodad )
				iMiddle--;

			return iMiddle;
		}
	}

	return -1;
}

// $NicktoonsPS2: FUNCTION 00384f58
// $JPOG: FUNCTION TGuiInterface.dll+00005f70
void TGUIActionMap::AddActionMapping( TINT a_iDoodad, TUINT a_uiActions )
{
	TINT iIndex = FindMapping( a_iDoodad );

	if ( iIndex != -1 )
	{
		m_aMappings[ iIndex ].uiActions |= a_uiActions;
		return;
	}

	// Insert after the mappings with the same doodad
	TINT iLow  = 0;
	TINT iHigh = m_aMappings.Size() - 1;

	while ( iLow <= iHigh )
	{
		TINT iMiddle  = ( iLow + iHigh ) / 2;
		TINT iCompare = a_iDoodad - m_aMappings[ iMiddle ].iDoodad;

		if ( iCompare < 0 )
		{
			iHigh = iMiddle - 1;
		}
		else
		{
			iLow = iMiddle + 1;

			if ( iCompare == 0 )
			{
				while ( iLow <= iHigh && m_aMappings[ iLow ].iDoodad == a_iDoodad )
					iLow++;

				break;
			}
		}
	}

	Mapping oMapping;
	oMapping.iDoodad   = a_iDoodad;
	oMapping.uiActions = a_uiActions;
	m_aMappings.InsertBefore( TArray<Mapping>::Iterator( iLow, m_aMappings ), oMapping );
}

// $NicktoonsPS2: FUNCTION 003853b8
// $JPOG: FUNCTION TGuiInterface.dll+00006010
TUINT TGUIActionMap::TranslateDoodadToAction( TINT a_iDoodad ) const
{
	TINT iIndex = FindMapping( a_iDoodad );

	if ( iIndex == -1 )
		return 0;

	return m_aMappings[ iIndex ].uiActions;
}

// $NicktoonsPS2: FUNCTION 003854f8
// $JPOG: FUNCTION TGuiInterface.dll+00006050
const TWCHAR* TGUIActionMap::GetActionLabel( TUINT a_uiActions ) const
{
	if ( a_uiActions == 0 )
		return TNULL;

	TINT iAction = MAX_ACTIONS - 1;

	while ( !( a_uiActions & ( 1U << iAction ) ) )
		iAction--;

	return m_awszLabels[ iAction ];
}

TOSHI_NAMESPACE_END
