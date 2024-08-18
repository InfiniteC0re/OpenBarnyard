#include "ToshiPCH.h"
#include "TInputInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_NORUNTIME( TInputDevice );

TInputDevice::TInputDevice() :
    m_Repeats( 0, 16 ),
    m_Array2( 0, 16 )
{
	m_bUnknown        = TFALSE;
	m_bIsAcquired     = TFALSE;
	m_pInputInterface = TNULL;
}

TBOOL TInputDevice::Flush()
{
	return TTRUE;
}

TINT TInputDevice::ProcessRepeats( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime )
{
	TINT iNumProcessed = 0;

	for ( TINT i = 0; i < m_Repeats.Size(); i++ )
	{
		auto& rRepeat = m_Repeats[ i ];

		if ( IsDown( rRepeat.iDoodad ) )
		{
			rRepeat.fLeftTime -= a_fDeltaTime;

			if ( rRepeat.fLeftTime <= 0.0f )
			{
				rRepeat.fLeftTime = rRepeat.fRepeatTime;
				ThrowRepeatEvent( a_rEmitter, &rRepeat, a_fDeltaTime );
				iNumProcessed += 1;
			}
		}
		else
		{
			if ( 1 < m_Repeats.Size() )
			{
				auto& rLastRepeat   = m_Repeats[ m_Repeats.Size() - 1 ];
				rRepeat.iDoodad     = rLastRepeat.iDoodad;
				rRepeat.fLeftTime   = rLastRepeat.fLeftTime;
				rRepeat.fRepeatTime = rLastRepeat.fRepeatTime;
			}

			if ( 0 < m_Repeats.Size() )
			{
				m_Repeats.Pop();
			}
		}
	}

	return iNumProcessed;
}

TBOOL TInputDevice::StartRepeat( Doodad a_iDoodad, TFLOAT a_fFirstRepeatTime, TFLOAT a_fRepeatTime )
{
	for ( auto it = m_Repeats.Begin(); it.IsOver() == TFALSE; ++it )
	{
		if ( it->iDoodad == a_iDoodad )
		{
			it->fRepeatTime = a_fRepeatTime;
			return TTRUE;
		}
	}

	m_Repeats.Push( { .iDoodad     = a_iDoodad,
	                  .fLeftTime   = a_fFirstRepeatTime,
	                  .fRepeatTime = a_fRepeatTime } );

	return TTRUE;
}

void TInputDevice::StopRepeat( Doodad a_iDoodad )
{
	if ( m_Repeats.Size() != 0 )
	{
		auto it = m_Repeats.Begin();

		TINT iIndex = 0;
		while ( it->iDoodad != a_iDoodad )
		{
			++it;
			++iIndex;
			if ( it.IsOver() ) return;
		}

		if ( 1 < m_Repeats.Size() )
		{
			auto& rLastRepeat = m_Repeats[ m_Repeats.Size() - 1 ];
			it->iDoodad       = rLastRepeat.iDoodad;
			it->fLeftTime     = rLastRepeat.fLeftTime;
			it->fRepeatTime   = rLastRepeat.fRepeatTime;
		}

		if ( 0 < m_Repeats.Size() )
		{
			m_Repeats.Pop();
		}
	}
}

void TInputDevice::StopAllRepeats()
{
	m_Repeats.Clear();
}

TBOOL TInputDevice::IsForceFeedbackDevice()
{
	return TFALSE;
}

void TInputDevice::ThrowRepeatEvent( EventEmitter& emitter, RepeatInfo* repeatInfo, TFLOAT flt )
{
	emitter.Throw(
	    TInputInterface::InputEvent(
	        this,
	        repeatInfo->iDoodad,
	        TInputInterface::EVENT_TYPE_REPEAT ) );
}

TOSHI_NAMESPACE_END
