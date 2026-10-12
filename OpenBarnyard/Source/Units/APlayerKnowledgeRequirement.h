#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// The player's answer to the knowledge "question" (APlayerKnowledgeManager):
// "answer" "any" - the player answered it, "none" - the player didn't answer it,
// otherwise the answer is "answer" and the unit knows it (AUnitPlayerKnowledgeComponent,
// see APropagateKnowledgeActivity).
// TClass version 1.0, vtable 0x0072a8fc, size 0x58.
//-----------------------------------------------------------------------------
class APlayerKnowledgeRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( APlayerKnowledgeRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject
	APlayerKnowledgeRequirement()
	    : m_bAny( TFALSE )
	    , m_bNone( TFALSE )
	{}

	~APlayerKnowledgeRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	// TRUE if the player answered the knowledge a_strName
	static TBOOL HasAnswer( Toshi::TPString8 a_strName );

private:
	Toshi::TPString8 m_strQuestion; // 0x4c, "question", registered by Create if it's new
	Toshi::TPString8 m_strAnswer;   // 0x50, "answer"
	TBOOL            m_bAny;        // 0x54, "answer" is "any"
	TBOOL            m_bNone;       // 0x55, "answer" is "none"
};

TSTATICASSERT( sizeof( APlayerKnowledgeRequirement ) == 0x58 );
