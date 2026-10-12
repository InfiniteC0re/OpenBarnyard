#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Data only: the "question" and its "answer".
// NOTE: the activity overrides nothing but Create.
//-----------------------------------------------------------------------------
class AAnswerActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AAnswerActivity, AInteractionActivity );

public:
	AAnswerActivity();
	~AAnswerActivity();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c

private:
	Toshi::TPString8 m_strQuestion; // 0x08, "question"
	Toshi::TPString8 m_strAnswer;   // 0x0c, "answer"
};

TSTATICASSERT( sizeof( AAnswerActivity ) == 0x10 );
