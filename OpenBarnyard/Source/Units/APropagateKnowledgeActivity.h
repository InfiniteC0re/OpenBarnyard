#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Gives the knowledge "question" (see APlayerKnowledgeManager) to the
// AUnitPlayerKnowledgeComponent of the second unit of the sequence and of the
// units of CATEGORY_AUXILIARY.
//-----------------------------------------------------------------------------
class APropagateKnowledgeActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( APropagateKnowledgeActivity, AInteractionActivity );

public:
	APropagateKnowledgeActivity();
	~APropagateKnowledgeActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	Toshi::TPString8 m_strQuestion; // 0x08, "question"
};

TSTATICASSERT( sizeof( APropagateKnowledgeActivity ) == 0xc );
