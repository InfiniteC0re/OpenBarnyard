#include "pch.h"
#include "ANamedAnimation.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00580520
ANamedAnimation::ANamedAnimation( T2Allocator* a_pAllocator )
    : m_vecUnk( a_pAllocator, 0, 0 )
{
	m_iSequenceId          = -1;
	m_eFlags               = FLAGS_NONE;
	m_fDefaultBlendOutTime = 0.3f;
	m_fDefaultBlendInTime  = 0.3f;
	m_fDefaultWeight       = 1.0f;
	m_fSpeed               = 1.0f;
	m_pAllocator           = a_pAllocator;

	m_eFlags = FLAGS_NONE;
	m_eFlags = m_eFlags | FLAGS_LOOP;
}

// $Barnyard: FUNCTION 005805b0
ANamedAnimation::~ANamedAnimation()
{
}

TPSTRING8_DECLARE( EndOfAnim );

// $Barnyard: FUNCTION 00581a90
ANamedAnimation::FINISHTYPE ANamedAnimation::GetFinishType( const Toshi::TPString8& a_rcFinishType )
{
	return ( !a_rcFinishType.GetString8().IsEmpty() && a_rcFinishType == TPS8( EndOfAnim ) ) ? FINISHTYPE_AUTO : FINISHTYPE_MANUAL;
}

// $Barnyard: FUNCTION 00580760
TBOOL ANamedAnimation::Create( const PBProperties* a_pProperties, Toshi::TSkeleton* a_pSkeleton )
{
	m_eFlags &= ~FLAGS_LOOP;
	m_fDefaultBlendInTime  = 0.3f;
	m_fDefaultBlendOutTime = 0.3f;
	m_fDefaultWeight       = 1.0f;

	TINT      iNumBreakpoints = 0;
	TPString8 strDefaultFinishType;

	T2_FOREACH( ( *a_pProperties ), it )
	{
		const TCHAR* pchPropertyName = it->GetName().GetString();

		switch ( *pchPropertyName )
		{
			case 'B':
				// Breakpoint
				iNumBreakpoints += 1;
				break;
			case 'D':
				if ( !T2String8::Compare( pchPropertyName, "DefaultBlendInTime" ) )
				{
					m_fDefaultBlendInTime = it->GetValue()->GetFloat();
				}
				else if ( !T2String8::Compare( pchPropertyName, "DefaultBlendOutTime" ) )
				{
					m_fDefaultBlendOutTime = it->GetValue()->GetFloat();
				}
				else if ( !T2String8::Compare( pchPropertyName, "DefaultFinishType" ) )
				{
					strDefaultFinishType = it->GetValue()->GetTPString8();
				}
				else if ( !T2String8::Compare( pchPropertyName, "DefaultWeight" ) )
				{
					m_fDefaultWeight = it->GetValue()->GetFloat();
				}
				else if ( !T2String8::Compare( pchPropertyName, "DefaultSpeed" ) )
				{
					m_fSpeed = it->GetValue()->GetFloat();
				}

				break;
			case 'E':
				// ExportedName
				m_strExportedName = it->GetValue()->GetTPString8();
				break;
			case 'L':
				// Loop

				if ( it->GetValue()->GetBoolean() )
					m_eFlags |= FLAGS_LOOP;
				else
					m_eFlags &= ~FLAGS_LOOP;

				break;
			case 'N':
				// Name
				m_strExportedName = it->GetValue()->GetTPString8();
				break;
			case 'R':
				// Reverse

				if ( it->GetValue()->GetBoolean() )
					m_eFlags |= FLAGS_REVERSE;
				else
					m_eFlags &= ~FLAGS_REVERSE;

				break;
			case 'S':
				if ( !T2String8::Compare( pchPropertyName, "Static" ) )
				{
					if ( it->GetValue()->GetBoolean() )
						m_eFlags |= FLAGS_STATIC;
					else
						m_eFlags &= ~FLAGS_STATIC;
				}
				else if ( !T2String8::Compare( pchPropertyName, "Speed" ) )
				{
					m_fSpeed = it->GetValue()->GetFloat();
				}

				break;
		}
	}

	TASSERT( !m_strName.IsEmpty() );
	TASSERT( !m_strExportedName.IsEmpty() );

	m_iSequenceId = a_pSkeleton->GetSequenceID( m_strExportedName, 0 );
	TASSERT( m_iSequenceId != -1 );

	if ( m_iSequenceId != -1 )
	{
		TSkeletonSequence* pSequence = a_pSkeleton->GetSequence( m_iSequenceId );

		if ( pSequence->IsOverlay() )
			m_eFlags |= FLAGS_OVERLAY;
		else
			m_eFlags &= ~FLAGS_OVERLAY;

		m_fDuration = pSequence->GetDuration();

		m_eFlags &= ~( FLAGS_FINISH_MANUAL | FLAGS_FINISH_ON_ANIM_END );
		m_eFlags |= ( GetFinishType( strDefaultFinishType ) == FINISHTYPE_AUTO ) ? FLAGS_FINISH_ON_ANIM_END : FLAGS_FINISH_MANUAL;

		TASSERT( !"Find model in AAnimatableObjectManager and parse breakpoints" );

		return TTRUE;
	}
	else
	{
		TERROR( "Couldn't get '%s' (Exported Name: '%s') animation from an animation set!", m_strName.GetString(), m_strExportedName.GetString() );
		return TFALSE;
	}
}

TPSTRING8_DECLARE( null );

// $Barnyard: FUNCTION 00580680
void ANamedAnimation::Reset()
{
	m_strName         = TPS8( null );
	m_strExportedName = TPS8( null );

	m_eFlags &= ~FLAGS_LOOP;
	m_eFlags &= ~FLAGS_UNK2;
	m_fDefaultBlendOutTime = 0.3f;
	m_fDefaultBlendInTime  = 0.3f;
	m_fDefaultWeight       = 1.0f;
	m_fSpeed               = 1.0f;
	m_iSequenceId          = -1;

	m_eFlags &= ~( FLAGS_FINISH_MANUAL | FLAGS_FINISH_ON_ANIM_END );
	m_eFlags |= FLAGS_FINISH_MANUAL;
}
