#include "pch.h"
#include "AAnswerActivity.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b3380, CreateObject 005b3560
// NOTE: GetClass 005b3380, CreateObject 005b3560, CreateObjectInPlace 005b35e0
TDEFINE_CLASS( AAnswerActivity );

AAnswerActivity::AAnswerActivity()
{
}

// $Barnyard: FUNCTION 005b3390
// $Barnyard: FUNCTION 005b3650
AAnswerActivity::~AAnswerActivity()
{
}

// $Barnyard: FUNCTION 005b3430
TBOOL AAnswerActivity::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pQuestion = a_pProperties->GetProperty( "question" );

	if ( pQuestion )
		m_strQuestion = pQuestion->GetTPString8();

	const PBPropertyValue* pAnswer = a_pProperties->GetProperty( "answer" );

	if ( pAnswer )
		m_strAnswer = pAnswer->GetTPString8();

	return AInteractionActivity::Create( a_pProperties );
}
