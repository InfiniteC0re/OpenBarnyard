#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TPString8.h>
#include <Toshi/T2Map.h>

//-----------------------------------------------------------------------------
// Purpose: Registry of the things the player can know about (the "question"
// names of the dialog choices). The index of a registered name is the bit the
// AUnitPlayerKnowledgeComponent of a unit sets once the player learned it.
// Created by AGameSystemManager::OnCreate through its TClass.
//-----------------------------------------------------------------------------
class APlayerKnowledgeManager
    : public Toshi::TObject
    , public Toshi::TSingleton<APlayerKnowledgeManager>
{
public:
	TDECLARE_CLASS( APlayerKnowledgeManager, Toshi::TObject );

	static constexpr TINT MAX_NUM_KNOWLEDGE = 32; // one bit of AUnitPlayerKnowledgeComponent for each

public:
	APlayerKnowledgeManager();
	~APlayerKnowledgeManager();

	// Returns index of the registered knowledge or -1 if it's not registered
	TINT FindKnowledge( Toshi::TPString8 a_strName );

	// Registers the knowledge and returns its index (doesn't check if it's already registered)
	TINT AddKnowledge( Toshi::TPString8 a_strName );

	// Returns value stored for the knowledge or an empty string if there's none
	Toshi::TPString8 GetKnowledgeValue( Toshi::TPString8 a_strName );

private:
	Toshi::T2Map<TINT, Toshi::TPString8> m_mapValues;                       // 0x04, knowledge index => value
	TINT                                 m_iNumKnowledge;                   // 0x1c
	Toshi::TPString8                     m_aKnowledge[ MAX_NUM_KNOWLEDGE ]; // 0x20
};

TSTATICASSERT( sizeof( APlayerKnowledgeManager ) == 0xa0 );
