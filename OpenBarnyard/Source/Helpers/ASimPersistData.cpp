#include "pch.h"
#include "ASimPersistData.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0054b460, CreateObject 0054b6b0, CreateObjectInPlace 0054b710 (the constructor is inlined in both)
TDEFINE_CLASS( ASimPersistData );

const ASimPersistData::LadderBeansInfo ASimPersistData::ms_aLadderBeansInfo[ NUM_LADDER_BEANS ] = {
	{ 2, { 0.7f, -0.9325f, -0.7325f, -0.7325f, -0.7325f, -0.7325f, -0.7325f, -0.7325f, -0.7325f }, 1.0f, 1.43f },
	{ 3, { 0.7f, -0.74f, -0.54f, -0.54f, -0.54f, -0.54f, -0.54f, -0.54f, -0.54f }, 1.0f, 1.02f },
	{ 4, { 0.8f, -0.74625f, -0.44625f, -0.44625f, -0.44625f, -0.44625f, -0.44625f, -0.44625f, -0.44625f }, 0.87f, 0.87f },
	{ 5, { 0.7f, -0.81625f, -0.61625f, -0.61625f, -0.61625f, -0.61625f, -0.61625f, -0.61625f, -0.61625f }, 0.5f, 1.2f },
	{ 6, { 0.7f, -0.6125f, -0.4125f, -0.4125f, -0.4125f, -0.4125f, -0.4125f, -0.4125f, -0.4625f }, 0.5f, 1.0f },
	{ 7, { 0.7f, -0.71375f, -0.51375f, -0.51375f, -0.51375f, -0.51375f, -0.51375f, -0.51375f, -0.51375f }, 1.0f, 1.0f },
	{ 8, { 0.8f, -1.015f, -0.715f, -0.715f, -0.715f, -0.715f, -0.715f, -0.715f, -0.715f }, 0.5f, 1.3f },
	{ 9, { 0.7f, -0.9525f, -0.7525f, -0.7525f, -0.7525f, -0.7525f, -0.7525f, -0.7525f, -0.7525f }, 0.5f, 1.4f },
};

// Height of every stage summed up, and the height of the stages up to iStage
// NOTE: inlined in Reset (005371f0) and Load (005372b0)
static void UpdateLadderHeights( ASimPersistData::LadderBeans& a_rLadder, const ASimPersistData::LadderBeansInfo& a_rcInfo, TBOOL a_bCheckStage )
{
	a_rLadder.fMaxHeight = 0.0f;
	a_rLadder.fMaxHeight = a_rcInfo.afStageHeight[ 0 ];

	for ( TINT i = 1; i < TARRAYSIZE( a_rcInfo.afStageHeight ); i++ )
		a_rLadder.fMaxHeight = a_rLadder.fMaxHeight + a_rcInfo.afStageHeight[ i ];

	a_rLadder.fHeight = 0.0f;

	// NOTE: Reset doesn't check the stage (it's always 0 there), Load does
	if ( !a_bCheckStage || a_rLadder.iStage >= 0 )
	{
		TINT i = 0;

		do
		{
			a_rLadder.fHeight += a_rcInfo.afStageHeight[ i++ ];
		} while ( i <= a_rLadder.iStage );
	}

	a_rLadder.fTargetHeight = a_rLadder.fHeight;
}

// $Barnyard: FUNCTION 005371f0
void ASimPersistData::LadderBeansSet::Reset()
{
	for ( TINT i = 0; i < NUM_LADDER_BEANS; i++ )
	{
		aLadders[ i ].iStage  = 0;
		aLadders[ i ].uiFlags = 0;
		UpdateLadderHeights( aLadders[ i ], ms_aLadderBeansInfo[ i ], TFALSE );
	}
}

// $Barnyard: FUNCTION 00537270
TBOOL ASimPersistData::LadderBeansSet::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	for ( TINT i = 0; i < NUM_LADDER_BEANS; i++ )
	{
		a_pBuffer->WriteUInt8( TUINT8( aLadders[ i ].iStage ) );
		a_pBuffer->WriteUInt8( aLadders[ i ].uiFlags );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005372b0
TBOOL ASimPersistData::LadderBeansSet::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	for ( TINT i = 0; i < NUM_LADDER_BEANS; i++ )
	{
		a_pBuffer->ReadUInt8( *TREINTERPRETCAST( TUINT8*, &aLadders[ i ].iStage ) );
		a_pBuffer->ReadUInt8( aLadders[ i ].uiFlags );
		UpdateLadderHeights( aLadders[ i ], ms_aLadderBeansInfo[ i ], TTRUE );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0054b5f0
ASimPersistData::ASimPersistData()
{
	m_iTerrainVIS       = -1;
	m_eFlags            = FLAGS_NONE;
	m_pLadderBeans      = TNULL;
	m_bPipFollowsPlayer = TFALSE;
	m_iAct              = 1;
	m_iFarmerDepartDay  = 0;
	m_iFarmerArriveDay  = 0;
	m_iRank             = -1;

	m_oItemCountHudState.bSaved = TFALSE;
	TStringManager::String8Copy( m_oItemCountHudState.szIcon, "none" );
	TStringManager::String8Copy( m_oItemCountHudState.szItem, "none" );
}

// $Barnyard: FUNCTION 0054b4e0
// $Barnyard: FUNCTION 0054b690
ASimPersistData::~ASimPersistData()
{
}

// $Barnyard: FUNCTION 0054b530
TBOOL ASimPersistData::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->WriteInt( m_iFarmerDepartDay );
	a_pBuffer->WriteInt( m_iFarmerArriveDay );
	a_pBuffer->WriteUInt8( m_eFlags );
	m_pLadderBeans->Save( a_pBuffer );
	a_pBuffer->WriteInt( m_iRank );
	a_pBuffer->WriteInt( m_iAct );

	return TTRUE;
}

// $Barnyard: FUNCTION 0054b590
TBOOL ASimPersistData::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->ReadInt( m_iFarmerDepartDay );
	a_pBuffer->ReadInt( m_iFarmerArriveDay );

	TUINT8 uiFlags;
	a_pBuffer->ReadUInt8( uiFlags );
	m_eFlags = uiFlags;

	m_pLadderBeans->Load( a_pBuffer );
	a_pBuffer->ReadInt( m_iRank );
	a_pBuffer->ReadInt( m_iAct );

	return TTRUE;
}

// $Barnyard: FUNCTION 0054b470
TBOOL ASimPersistData::CreateLadderBeans()
{
	m_pLadderBeans = TSTATICCAST( LadderBeansSet, TMalloc( sizeof( LadderBeansSet ) ) );
	m_pLadderBeans->Reset();

	return TTRUE;
}

// $Barnyard: FUNCTION 0054b4a0
void ASimPersistData::DestroyLadderBeans()
{
	TFree( m_pLadderBeans );
}

// $Barnyard: FUNCTION 0054b500
void ASimPersistData::Reset()
{
	m_iRank       = -1;
	m_iTerrainVIS = -1;
	m_eFlags      = FLAGS_NONE;
	m_pLadderBeans->Reset();
	m_iAct                      = 1;
	m_oItemCountHudState.bSaved = TFALSE;
	m_bPipFollowsPlayer         = TFALSE;
}
