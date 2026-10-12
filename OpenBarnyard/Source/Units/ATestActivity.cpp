#include "pch.h"
#include "ATestActivity.h"
#include "AWorldPositionActivityMessage.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c2260, CreateObject 005c23d0, CreateObjectInPlace 005c2430
TDEFINE_CLASS( ATestActivity );

// $Barnyard: FUNCTION 005c22e0
ATestActivity::ATestActivity()
    : m_fTime( 0.0f )
    , m_iDataSize( 0 )
    , m_iDataOffset( -1 )
{
}

// $Barnyard: FUNCTION 005c2270
// $Barnyard: FUNCTION 005c2350
ATestActivity::~ATestActivity()
{
}

// $Barnyard: FUNCTION 005c2370
TBOOL ATestActivity::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pPrint = a_pProperties->GetProperty( "print" );

	if ( pPrint )
		m_strPrint = pPrint->GetString();

	const PBPropertyValue* pTime = a_pProperties->GetProperty( "time" );

	if ( pTime )
		m_fTime = ( pTime->GetType() == PBPropertyValue::Type::Float ) ? pTime->GetFloat() : TFLOAT( pTime->GetInteger() );

	TBOOL bResult = AInteractionActivity::Create( a_pProperties );

	m_iDataOffset = m_iDataSize;
	m_iDataSize += sizeof( TVector3 );

	return bResult;
}

// $Barnyard: FUNCTION 005c22d0
void ATestActivity::GetDataSize( TINT* a_pSize )
{
	*a_pSize = m_iDataSize;
}

// $Barnyard: FUNCTION 005c2470
AInteractionActivity::RESULT ATestActivity::Update( AInteractionSequence* a_pSequence )
{
	AInteractionActivityState& rState    = a_pSequence->GetActivityState();
	TVector3*                  pPosition = TREINTERPRETCAST( TVector3*, TSTATICCAST( TUINT8, a_pSequence->GetActivityData() ) + m_iDataOffset );

	// Uses the activity flags of the state, the sign bit marks the activity as started
	if ( !( rState.iFlags & AInteractionActivityState::FLAGS_INITIALISED ) )
	{
		rState.iFlags |= AInteractionActivityState::FLAGS_INITIALISED;
		*pPosition = TVector3::VEC_ZERO;

		// NOTE: the result is unused
		a_pSequence->GetActivityMessage<AWorldPositionActivityMessage>( TPS8D( "testposition" ) );
	}
	else
	{
		// NOTE: retail adds the (1, 0, 0) at 0x0077d0c4
		*pPosition += TVector3::VEC_POSX;
	}

	return ( rState.fTime < m_fTime ) ? RESULT_RUNNING : RESULT_DONE;
}
