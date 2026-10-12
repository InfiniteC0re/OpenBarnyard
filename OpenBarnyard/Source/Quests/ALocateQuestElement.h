#pragma once
#include "AQuestElement.h"
#include "Units/AUnitRef.h"

#include <Toshi/TString16.h>

//-----------------------------------------------------------------------------
// Quest element completed by getting within "radius" of a unit ("unittype",
// optionally "unitname", spawned if "spawn" is set) or of a goal ("goal" of
// "network").
//-----------------------------------------------------------------------------
class ALocateQuestElement : public AQuestElement
{
public:
	TDECLARE_CLASS( ALocateQuestElement, AQuestElement );

	using MODE = TINT;
	enum MODE_ : MODE
	{
		MODE_GOAL = 0,
		MODE_UNIT = 1,
	};

public:
	ALocateQuestElement();
	~ALocateQuestElement();

	//-----------------------------------------------------------------------------
	// AQuestElement
	//-----------------------------------------------------------------------------
	virtual TBOOL    Create( const PBProperties* a_pProperties, TINT* a_pDataOffset ) OVERRIDE;               // +0x0c
	virtual TBOOL    InitData( AInteractionActivityData* a_pData ) OVERRIDE;                                  // +0x10
	virtual void     DeinitData( AInteractionActivityData* a_pData ) OVERRIDE;                                // +0x14
	virtual TBOOL    GetDescription( const TWCHAR** a_ppOutText ) OVERRIDE;                                   // +0x18
	virtual void     Save( AHackedQuickSave::Buffer* a_pBuffer, AInteractionActivityData* a_pData ) OVERRIDE; // +0x1c
	virtual TBOOL    Load( AHackedQuickSave::Buffer* a_pBuffer, AInteractionActivityData* a_pData ) OVERRIDE; // +0x20
	virtual PROGRESS CheckProgress( AInteractionActivityData* a_pData ) OVERRIDE;                             // +0x28

private:
	// Finds (or spawns) the unit to locate and stores it in a_pRef
	AUnit* FindUnit( AUnitRef* a_pRef );

	// The unit to locate of the instance
	AUnitRef* GetUnitRef( AInteractionActivityData* a_pData ) const { return TREINTERPRETCAST( AUnitRef*, TSTATICCAST( TUINT8, a_pData->pData ) + m_iUnitOffset ); }

private:
	Toshi::TPString8 m_strUnitType;    // 0x5c, "unittype"
	Toshi::TPString8 m_strGoal;        // 0x60, "goal"
	Toshi::TPString8 m_strNetwork;     // 0x64, "network"
	Toshi::TPString8 m_strUnitName;    // 0x68, "unitname"
	TFLOAT           m_fRadiusSq;      // 0x6c, "radius" squared
	TBOOL            m_bSpawn;         // 0x70, "spawn"
	Toshi::TPString8 m_strLocation;    // 0x74, "location"
	MODE             m_eMode;          // 0x78
	Toshi::TString16 m_strDescription; // 0x7c
	TINT             m_iUnitOffset;    // 0x88, offset of the AUnitRef in the instance data
};

TSTATICASSERT( sizeof( ALocateQuestElement ) == 0x8c );
