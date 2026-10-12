#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TPString8.h>
#include <Toshi/TArray.h>
#include <Plugins/PPropertyParser/PBProperties.h>

#include "Units/AUnitModelStaticData.h"

class AQuestElement;

//-----------------------------------------------------------------------------
// A quest loaded from Data/Quests (see AQuestManager): the quest giver, the
// interactions played at the start and at the end and the elements (steps)
// of the quest. AQuestInstance holds the progress of an assigned quest.
//-----------------------------------------------------------------------------
class AQuest : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AQuest, Toshi::TObject );

	// When the quest can be done, from the spawn of the quest giver in the QuestTierSpawnSet of AAnimalPopulationManager
	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE  = 0,
		FLAGS_DAY   = BITFLAG( 0 ),
		FLAGS_NIGHT = BITFLAG( 1 ),
		FLAGS_UNK2  = BITFLAG( 2 ), // AAnimalPopulationManager::SPAWNFLAGS_UNK3
		FLAGS_UNK3  = BITFLAG( 3 ), // AAnimalPopulationManager::SPAWNFLAGS_UNK2
	};

public:
	AQuest();
	~AQuest();

	// Reads the quest from a "quest" property of the quest file
	virtual TBOOL Create( const PBProperties* a_pProperties ); // +0x0c

	// Localised "title" or TNULL
	const TWCHAR* GetTitle() const;

	// TRUE if a_rcName is one of the units spawned for the quest
	TBOOL HasUnit( const Toshi::TPString8& a_rcName ) const;

	// $Barnyard: FUNCTION 005c9f50
	Toshi::TPString8 GetEndSeq() const { return m_strEndSeq; }

	// NOTE: folded getter 0049b320
	Toshi::TPString8 GetStartSeq() const { return m_strStartSeq; }

	const Toshi::TPString8&                     GetName() const { return m_strName; }
	const Toshi::TPString8&                     GetQuestGiver() const { return m_strQuestGiver; }
	TINT                                        GetTitleId() const { return m_iTitle; }
	TINT                                        GetSummaryId() const { return m_iSummary; }
	TFLOAT                                      GetTimeLimit() const { return m_fTimeLimit; }
	TINT                                        GetDataSize() const { return m_iDataSize; }
	TBOOL                                       IsEndSeqBlocking() const { return m_bEndSeqBlocks; }
	FLAGS                                       GetFlags() const { return m_uiFlags; }
	TINT                                        GetNumElements() const { return m_vecElements.Size(); }
	AQuestElement*                              GetElement( TINT a_iIndex ) const { return m_vecElements[ a_iIndex ]; }
	const AUnitModelStaticData::ModelNameArray& GetUnitNames() const { return m_oUnitNames; }

private:
	// Creates the "elements" of the quest, a_pDataSize is the size of the instance data
	static TBOOL CreateElements( Toshi::TArray<AQuestElement*>& a_rElements, TINT* a_pDataSize, const PBProperties* a_pProperties );

private:
	Toshi::TPString8                     m_strName;       // 0x04, "name"
	TINT                                 m_iTitle;        // 0x08, "title", locale id
	Toshi::TArray<AQuestElement*>        m_vecElements;   // 0x0c, "elements"
	TFLOAT                               m_fTimeLimit;    // 0x1c, "timelimit", in seconds of game time
	TINT                                 m_iDataSize;     // 0x20, size of the data of an instance (the elements)
	Toshi::TPString8                     m_strEndSeq;     // 0x24, "endseq"
	TBOOL                                m_bEndSeqBlocks; // 0x28, "endseqblocks", the quest is complete once the "endseq" interaction is over
	Toshi::TPString8                     m_strStartSeq;   // 0x2c, "startseq"
	Toshi::TPString8                     m_strQuestGiver; // 0x30, "quest_giver"
	FLAGS                                m_uiFlags;       // 0x34
	AUnitModelStaticData::ModelNameArray m_oUnitNames;    // 0x38, units spawned for the quest
	TINT                                 m_iSummary;      // 0x48, "xsummary", locale id
};

TSTATICASSERT( sizeof( AQuest ) == 0x4c );
