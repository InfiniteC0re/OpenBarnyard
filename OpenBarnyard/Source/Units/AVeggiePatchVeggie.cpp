#include "pch.h"
#include "AVeggiePatchVeggie.h"
#include "AUnitType.h"
#include "ARandom.h"
#include "Animation/AModel.h"
#include "Animation/ABasicAnimController.h"
#include "FX/AParticleEffectManager.h"
#include "UnitMovement/Steering/AStationarySteer.h"
#include "UnitMovement/Steering/AThrowItemSteer.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 004c5e40, CreateObject 004c62a0, CreateObjectInPlace 004c6300 (the constructor is inlined in both)
TDEFINE_CLASS( AVeggiePatchVeggie );

TPSTRING8_DECLARE( vpd_carrot );
TPSTRING8_DECLARE( vpd_melon );
TPSTRING8_DECLARE( vpd_cabbage );
TPSTRING8_DECLARE( vpd_pumpkin );

// $Barnyard: FUNCTION 004c5f70
AVeggiePatchVeggie::AVeggiePatchVeggie()
    : m_eKind( AVeggiePatchDefenderGame::VEGGIE_SPROUT )
    , m_eState( STATE_GROWING )
    , m_pSlot( TNULL )
    , m_pPatch( TNULL )
    , m_iUnkf4( 0 )
    , m_fTimer( 0.0f )
    , m_iDustEffect( AParticleEffectManager::FindEffect( "dustpuff" ) )
{
}

// $Barnyard: FUNCTION 004c6000
// $Barnyard: FUNCTION 004c5e50
AVeggiePatchVeggie::~AVeggiePatchVeggie()
{
}

// $Barnyard: FUNCTION 004c6020
TBOOL AVeggiePatchVeggie::Create( AUnitTransform* a_pTransform )
{
	if ( !ACarryableUnit::Create( a_pTransform ) )
		return TFALSE;

	const TPString8& rcTypeName = m_pType->GetName();

	if ( rcTypeName == TPS8( vpd_carrot ) )
		m_eKind = AVeggiePatchDefenderGame::VEGGIE_CARROT;
	else if ( rcTypeName == TPS8( vpd_melon ) )
		m_eKind = AVeggiePatchDefenderGame::VEGGIE_MELON;
	else if ( rcTypeName == TPS8( vpd_cabbage ) )
		m_eKind = AVeggiePatchDefenderGame::VEGGIE_CABBAGE;
	else if ( rcTypeName == TPS8( vpd_pumpkin ) )
		m_eKind = AVeggiePatchDefenderGame::VEGGIE_PUMPKIN;

	return TTRUE;
}

// $Barnyard: FUNCTION 004c6340
void AVeggiePatchVeggie::UpdateAI( TFLOAT a_fDeltaTime )
{
	if ( m_eState == STATE_DROPPED )
	{
		// Wait till the veggie has landed
		if ( !TSTATICCAST( AThrowItemSteer, m_pSteer )->IsStopped() )
			return;

		m_fTimer = 0.0f;
		m_eState = STATE_FADING;

		// NOTE: retail passes TTRUE to it, which is ignored
		GetModelInstance()->DisableDrawWithDefaultMaterials();

		m_pPatch->oCarriedVeggies.Erase( this );
		m_pPatch->oFallingVeggies.PushBack( this );
		m_pPatch->iNumCarried--;
		m_pPatch->iNumFalling++;

		if ( !m_pSteer->GetClass()->IsA( &TGetClass( AStationarySteer ) ) )
			ChangeSteer( &TGetClass( AStationarySteer ) );

		SetHashed( TTRUE );
		SetNoQuery( TFALSE );
	}
	else if ( m_eState == STATE_FADING )
	{
		TFLOAT fOpacity = 1.0f - m_fTimer;

		if ( fOpacity >= 0.0f )
		{
			GetModelInstance()->SetOpacity( fOpacity );
			m_fTimer += a_fDeltaTime;
			return;
		}

		// Faded out, grow back in a free slot of the same kind
		T2SList<AVeggiePatchDefenderGame::VeggieSlot>& rSlots = m_pPatch->aSlotLists[ m_eKind ];

		for ( auto it = rSlots.Begin(); it != rSlots.End(); it++ )
		{
			if ( !it->pVeggie )
				m_pSlot = it;
		}

		m_pSlot->pVeggie = this;
		m_pSteer->SetPosition( m_pSlot->vPosition );
		GetModelInstance()->SetTransform( m_pSlot->oTransform );

		// NOTE: retail passes TFALSE to it, which is ignored
		GetModelInstance()->DisableDrawWithDefaultMaterials();
		GetModelInstance()->GetSceneObject()->DisableOpacity();

		m_eState = STATE_GROWING;
		m_pPatch->oFallingVeggies.Erase( this );
		m_pPatch->oVeggies.PushBack( this );

		// NOTE: retail decrements iNumCarried a second time instead of iNumFalling
		m_pPatch->iNumCarried--;
		m_pPatch->iNumInPatch++;
		m_pPatch->iNumVeggiesSaved++;
		m_pPatch->iNumVeggiesAtStart++;
		m_fTimer += a_fDeltaTime;
	}
}

// $Barnyard: FUNCTION 004c5e70
void AVeggiePatchVeggie::Plant( AVeggiePatchDefenderGame::VeggieSlot* a_pSlot, AVeggiePatchDefenderGame::Patch* a_pPatch )
{
	GetAnimatableObject()->FindAnimController( &TGetClass( ABasicAnimController ) )->SetDisabled( TTRUE );

	m_pPatch         = a_pPatch;
	a_pSlot->pVeggie = this;
	GetModelInstance()->SetTransform( a_pSlot->oTransform );
	m_pSlot  = a_pSlot;
	m_eState = STATE_GROWING;
}

// $Barnyard: FUNCTION 004c65e0
TBOOL AVeggiePatchVeggie::PickUp()
{
	if ( m_eState != STATE_GROWING )
		return TFALSE;

	m_pSlot->pVeggie = TNULL;
	m_pSlot          = TNULL;
	m_eState         = STATE_CARRIED;

	m_pPatch->oVeggies.Erase( this );
	m_pPatch->oCarriedVeggies.PushBack( this );
	m_pPatch->iNumInPatch--;
	m_pPatch->iNumCarried++;
	m_pPatch->iNumVeggiesTaken++;
	m_pPatch->iNumVeggiesAtStart--;

	return TTRUE;
}

// $Barnyard: FUNCTION 004c60a0
void AVeggiePatchVeggie::Drop( const TVector4& a_rcPosition )
{
	m_eState = STATE_DROPPED;

	if ( !m_pSteer->GetClass()->IsA( &TGetClass( AThrowItemSteer ) ) )
		ChangeSteer( &TGetClass( AThrowItemSteer ) );

	AThrowItemSteer* pSteer = TSTATICCAST( AThrowItemSteer, m_pSteer );
	pSteer->SetPosition( a_rcPosition );

	// Throw it back towards the middle of the patch
	TRandom& rRandom = ARandom::GetSingleton()->m_oRandom;
	TFLOAT   fZ      = ( a_rcPosition.z <= 0.0f ) ? rRandom.GetFloat() : rRandom.GetFloat() - 2.0f;
	TFLOAT   fX      = ( a_rcPosition.x <= 0.0f ) ? rRandom.GetFloat() : rRandom.GetFloat() - 2.0f;

	pSteer->SetVelocity( TVector4( fX, -3.0f, fZ, 1.0f ) );
	pSteer->SetUpdateModelTransform( TTRUE );
}

// $Barnyard: FUNCTION 004c5ed0
void AVeggiePatchVeggie::SetLost()
{
	m_eState = STATE_LOST;
	m_pPatch->iNumCarried--;
	m_pPatch->iNumLost++;
	m_pPatch->iNumVeggiesLost++;

	TStringManager::String16Format( m_pPatch->wszLeft, TARRAYSIZE( m_pPatch->wszLeft ), L"%d", m_pPatch->iMaxLost - m_pPatch->iNumLost );
}

// $Barnyard: FUNCTION 004c5f30
void AVeggiePatchVeggie::Hide()
{
	SetHashed( TFALSE );
	SetNoQuery( TTRUE );
	SetVisible( TFALSE );
}
