#pragma once
#include "AUnitComponent.h"
#include "AUnitStaticData.h"

//-----------------------------------------------------------------------------
// Social profile of a unit: masks of the AUnitFlagSetsManager flag sets, used
// by the relationship, interest and reputation requirements and to adjust the
// priority of interactions (see AInteraction::UpdatePriority).
//-----------------------------------------------------------------------------
class AUnitProfileComponent
    : public AUnitComponent
{
public:
	TDECLARE_CLASS( AUnitProfileComponent, AUnitComponent );

	friend class AUnitProfileComponentData;

public:
	AUnitProfileComponent();
	~AUnitProfileComponent();

	virtual AUnitComponentManager* GetManager() OVERRIDE;                                // +0x10
	virtual void                   Save( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE; // +0x14
	virtual void                   Load( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE; // +0x18

	TUINT GetReputations( TINT a_iIndex ) const { return m_auiReputations[ a_iIndex ]; }
	TUINT GetRelationships() const { return m_uiRelationships; }
	TUINT GetInterests() const { return m_uiInterests; }

	void SetReputations( TINT a_iIndex, TUINT a_uiReputations ) { m_auiReputations[ a_iIndex ] = a_uiReputations; }
	void SetRelationships( TUINT a_uiRelationships ) { m_uiRelationships = a_uiRelationships; }

	// The interests of the unit, or the ones the player has for it (AInterestActivity "playerinterest")
	TUINT& GetInterestMask( TBOOL a_bPlayer ) { return a_bPlayer ? m_uiPlayerInterests : m_uiInterests; }

private:
	TUINT m_auiReputations[ 2 ]; // 0x10, AUnitFlagSetsManager::FLAGSET_REPUTATIONS masks, picked by AReputationRequirement
	TUINT m_uiRelationships;     // 0x18, AUnitFlagSetsManager::FLAGSET_RELATIONSHIPS mask
	TUINT m_uiInterests;         // 0x1c, AUnitFlagSetsManager::FLAGSET_INTERESTS mask
	TUINT m_uiPlayerInterests;   // 0x20
};

TSTATICASSERT( sizeof( AUnitProfileComponent ) == 0x24 );

//-----------------------------------------------------------------------------
// Static data of a unit type's profile ("AUnitProfileComponentData" block).
// Creates an AUnitProfileComponent for every unit of the type.
//-----------------------------------------------------------------------------
class AUnitProfileComponentData
    : public AUnitStaticData
{
public:
	TDECLARE_CLASS( AUnitProfileComponentData, AUnitStaticData );

public:
	AUnitProfileComponentData();
	~AUnitProfileComponentData();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual void  OnUnitCreate( AUnit* a_pUnit ) OVERRIDE;              // +0x10

private:
	TUINT m_uiInterests; // 0x0c, "interests"
};

TSTATICASSERT( sizeof( AUnitProfileComponentData ) == 0x10 );
