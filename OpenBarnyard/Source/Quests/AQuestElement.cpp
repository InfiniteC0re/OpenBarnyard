#include "pch.h"
#include "AQuestElement.h"
#include "Units/AInteractionManager.h"
#include "Units/AUnitNameManager.h"
#include "Units/AUnit.h"
#include "HUD/AHudManager.h"
#include "HUD/ASimMiniMap.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005cb2e0
TDEFINE_CLASS_NORUNTIME( AQuestElement );

//-----------------------------------------------------------------------------
// AQuestElementLocator
//-----------------------------------------------------------------------------

// NOTE: inlined into the constructor of AQuestElement (005cb500)
AQuestElementLocator::AQuestElementLocator()
    : m_eType( TYPE_NONE )
    , m_vPosition( TVector4::VEC_ZERO )
    , m_bUnk38( TFALSE )
    , m_iMarker( -1 )
{
}

// $Barnyard: FUNCTION 005cb2f0
void AQuestElementLocator::SetLocation( const TCHAR* a_szLocation )
{
	if ( TStringManager::String8Length( a_szLocation ) == 0 )
	{
		m_eType = TYPE_NONE;
		return;
	}

	if ( TStringManager::String8Compare( a_szLocation, "goal(", 5 ) == 0 )
	{
		m_eType = TYPE_GOAL;
	}
	else if ( TStringManager::String8Compare( a_szLocation, "unit(", 5 ) == 0 )
	{
		m_eType = TYPE_UNIT;
	}
	else
	{
		// A plain goal name
		m_eType = TYPE_GOAL;
		TStringManager::String8Copy( m_szName, a_szLocation );
		return;
	}

	// Copy the name between the parentheses
	TINT i = 0;

	for ( const TCHAR* pChar = a_szLocation + 5; *pChar != ')'; pChar++ )
		m_szName[ i++ ] = *pChar;

	m_szName[ i ] = '\0';
}

// $Barnyard: FUNCTION 005cbcc0
void AQuestElementLocator::AddMarker()
{
	if ( m_iMarker >= 0 )
		return;

	ASimMiniMap* pMiniMap = AHudManager::GetSingleton()->FindElement<ASimMiniMap>();

	if ( !pMiniMap || m_iMarker >= 0 )
		return;

	if ( m_eType == TYPE_GOAL )
	{
		AInteriorNetworkManager* pNetworkManager = AInteriorNetworkManager::GetSingleton();
		AInteriorNetwork*        pNetwork        = TNULL;
		TVector4                 vPosition;

		AInteriorGoal* pGoal = pNetworkManager->FindRandomVisibleGoal( pNetworkManager->GetDefaultNetwork()->GetName(), TPString8( m_szName ), pNetwork );

		if ( pGoal )
			pGoal->GetPosition( vPosition );

		m_iMarker = pMiniMap->AddIcon( vPosition, ASimMiniMap::ICON_ITEM, 0.5f, TFALSE );
	}
	else if ( m_eType == TYPE_UNIT )
	{
		AUnit* pUnit = AUnitNameManager::GetSingleton()->RetrieveByName( TPString8( m_szName ) );

		if ( pUnit )
			m_iMarker = pMiniMap->AddUnitIcon( pUnit->GetSelfProxy(), ASimMiniMap::ICON_ITEM, 0.5f, TFALSE );
	}
}

// $Barnyard: FUNCTION 005cb3a0
void AQuestElementLocator::RemoveMarker()
{
	if ( m_iMarker >= 0 )
	{
		ASimMiniMap* pMiniMap = AHudManager::GetSingleton()->FindElement<ASimMiniMap>();

		if ( pMiniMap )
		{
			pMiniMap->RemoveIcon( m_iMarker );
			m_iMarker = -1;
		}
	}
}

//-----------------------------------------------------------------------------
// AQuestElement
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005cb500
AQuestElement::AQuestElement()
    : m_bBlock( TFALSE )
    , m_iDataOffset( -1 )
{
}

// $Barnyard: FUNCTION 005cb570
// $Barnyard: FUNCTION 005cb380
// $Barnyard: FUNCTION 005cbe10
AQuestElement::~AQuestElement()
{
}

// $Barnyard: FUNCTION 005cb650
TBOOL AQuestElement::Create( const PBProperties* a_pProperties, TINT* a_pDataOffset )
{
	TPString8 strEmpty( "" );
	m_bBlock      = TFALSE;
	m_strStartSeq = strEmpty;
	m_strEndSeq   = strEmpty;
	m_strFailSeq  = strEmpty;
	m_strName     = strEmpty;

	TPString8 strWorldLocation;

	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		const TCHAR*           szName = it->GetName().GetString();
		const PBPropertyValue* pValue = it->GetValue();

		switch ( szName[ 0 ] )
		{
			case 'b':
				if ( TStringManager::String8CompareNoCase( szName, "block" ) == 0 )
					m_bBlock = pValue->GetRaw() != TNULL;
				break;
			case 'e':
				if ( TStringManager::String8CompareNoCase( szName, "endseq" ) == 0 )
					m_strEndSeq = pValue->GetTPString8();
				break;
			case 'f':
				if ( TStringManager::String8CompareNoCase( szName, "failseq" ) == 0 )
					m_strFailSeq = pValue->GetTPString8();
				break;
			case 'n':
				if ( TStringManager::String8CompareNoCase( szName, "name" ) == 0 )
					m_strName = pValue->GetTPString8();
				break;
			case 's':
				if ( TStringManager::String8CompareNoCase( szName, "startseq" ) == 0 )
					m_strStartSeq = pValue->GetTPString8();
				break;
			case 'w':
				if ( TStringManager::String8CompareNoCase( szName, "worldlocation" ) == 0 )
					strWorldLocation = pValue->GetTPString8();
				break;
		}
	}

	m_oLocator.SetLocation( strWorldLocation.GetString() );

	m_iDataOffset = *a_pDataOffset;
	*a_pDataOffset += sizeof( Progress );

	return TTRUE;
}

// $Barnyard: FUNCTION 005cb3e0
TBOOL AQuestElement::InitData( AInteractionActivityData* a_pData )
{
	Progress* pProgress       = GetProgress( a_pData );
	pProgress->eState         = STATE_START;
	pProgress->uiStartSeqId   = TUINT( -1 );
	pProgress->uiEndSeqId     = TUINT( -1 );
	pProgress->uiFailSeqId    = TUINT( -1 );
	pProgress->bComplete      = TFALSE;
	pProgress->bFailed        = TFALSE;
	pProgress->bFailedForGood = TFALSE;

	return TTRUE;
}

// $Barnyard: FUNCTION 005cb410
void AQuestElement::Save( AHackedQuickSave::Buffer* a_pBuffer, AInteractionActivityData* a_pData )
{
	Progress* pProgress = GetProgress( a_pData );

	a_pBuffer->WriteInt( pProgress->eState );
	a_pBuffer->WriteInt( TINT( pProgress->uiStartSeqId ) );
	a_pBuffer->WriteInt( TINT( pProgress->uiEndSeqId ) );
	a_pBuffer->WriteInt( TINT( pProgress->uiFailSeqId ) );
	a_pBuffer->WriteBool( pProgress->bComplete );
	a_pBuffer->WriteBool( pProgress->bFailed );
	a_pBuffer->WriteBool( pProgress->bFailedForGood );
}

// $Barnyard: FUNCTION 005cb480
TBOOL AQuestElement::Load( AHackedQuickSave::Buffer* a_pBuffer, AInteractionActivityData* a_pData )
{
	Progress* pProgress = GetProgress( a_pData );

	TUINT uiState = 0;
	a_pBuffer->ReadUInt( uiState );
	pProgress->eState = STATE( uiState );

	TINT iValue;
	a_pBuffer->ReadInt( iValue );
	pProgress->uiStartSeqId = TUINT( iValue );
	a_pBuffer->ReadInt( iValue );
	pProgress->uiEndSeqId = TUINT( iValue );
	a_pBuffer->ReadInt( iValue );
	pProgress->uiFailSeqId = TUINT( iValue );
	a_pBuffer->ReadBool( pProgress->bComplete );
	a_pBuffer->ReadBool( pProgress->bFailed );

	return a_pBuffer->ReadBool( pProgress->bFailedForGood );
}

// $Barnyard: FUNCTION 005cbaf0
TBOOL AQuestElement::Update( AInteractionActivityData* a_pData, TFLOAT a_fDeltaTime )
{
	Progress* pProgress = GetProgress( a_pData );

	switch ( pProgress->eState )
	{
		case STATE_START:
			if ( !m_strStartSeq.IsEmpty() )
			{
				// Wait for the start sequence, retried until it can be assigned
				AInteractionSequence* pSequence = AInteractionManager::GetSingleton()->AssignInteractionToAllUnits( m_strStartSeq );

				if ( pSequence )
				{
					pProgress->eState       = STATE_STARTSEQ;
					pProgress->uiStartSeqId = pSequence->GetId();
				}

				return TFALSE;
			}

			pProgress->eState = STATE_RUNNING;
			return TFALSE;

		case STATE_RUNNING:
		{
			TBOOL bComplete = pProgress->bComplete;
			TBOOL bFailed   = pProgress->bFailed;

			if ( !bFailed && !bComplete )
			{
				PROGRESS eProgress = CheckProgress( a_pData );
				bComplete          = eProgress == PROGRESS_COMPLETE;
				bFailed            = eProgress == PROGRESS_FAILED;
			}

			if ( bComplete )
			{
				if ( !m_strEndSeq.IsEmpty() )
				{
					AInteractionSequence* pSequence = AInteractionManager::GetSingleton()->AssignInteractionToAllUnits( m_strEndSeq );

					if ( pSequence )
					{
						pProgress->uiEndSeqId = pSequence->GetId();
						pProgress->eState     = STATE_ENDSEQ;
						return TFALSE;
					}
				}

				pProgress->eState = STATE_COMPLETE;
				return TTRUE;
			}

			if ( !bFailed )
			{
				if ( pProgress->bFailedForGood )
					pProgress->eState = STATE_FAILED;

				return TFALSE;
			}

			if ( !m_strFailSeq.IsEmpty() )
			{
				AInteractionSequence* pSequence = AInteractionManager::GetSingleton()->AssignInteractionToAllUnits( m_strFailSeq );

				if ( pSequence )
				{
					pProgress->uiFailSeqId = pSequence->GetId();
					pProgress->eState      = STATE_FAILSEQ;
					return TFALSE;
				}
			}

			pProgress->eState = STATE_RUNNING;
			return TFALSE;
		}

		case STATE_COMPLETE:
			// NOTE: inlined AQuestElementLocator::RemoveMarker (005cb3a0)
			m_oLocator.RemoveMarker();
			return TTRUE;
	}

	return TFALSE;
}
