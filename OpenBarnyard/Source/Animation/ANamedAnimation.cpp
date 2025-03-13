#include "pch.h"
#include "ANamedAnimation.h"
#include "AAnimatableObjectManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ANamedAnimationTransition::ANamedAnimationTransition()
    : m_bAutoTransition( TFALSE )
{
}

// $Barnyard: FUNCTION 00581b40
ANamedAnimationTransition::~ANamedAnimationTransition()
{
}

void ANamedAnimationTransition::SetTransitionName( const TPString8& a_strName )
{
    m_strTransitionName = a_strName;
}

// $Barnyard: FUNCTION 00583330
void ANamedAnimationTransition::SetAnimationName( const TPString8& a_strName )
{
	m_strAnimationName = a_strName;
}

void ANamedAnimationTransition::SetBlendInTime( TFLOAT a_fTime )
{
	m_fBlendInTime = a_fTime;
}

TFLOAT ANamedAnimationTransition::GetBlendInTime() const
{
	return m_fBlendInTime;
}

void ANamedAnimationTransition::SetBlendOutTime( TFLOAT a_fTime )
{
	m_fBlendOutTime = a_fTime;
}

TFLOAT ANamedAnimationTransition::GetBlendOutTime() const
{
	return m_fBlendOutTime;
}

void ANamedAnimationTransition::SetAutoTransition( TBOOL a_bAutoTransition )
{
	m_bAutoTransition = a_bAutoTransition;
}

void ANamedAnimationTransition::SetManaged( TBOOL a_bManaged )
{
	m_bManaged = a_bManaged;
}

TBOOL ANamedAnimationTransition::IsAutoTransition() const
{
	return m_bAutoTransition;
}

TBOOL ANamedAnimationTransition::IsManaged() const
{
	return m_bManaged;
}

// $Barnyard: FUNCTION 00581d90
ANamedAnimationTransitionSet::ANamedAnimationTransitionSet( ANamedAnimation* a_pNamedAnimation, Toshi::T2Allocator* a_pAllocator )
    : m_pAnimation( a_pNamedAnimation )
    , m_mapTransitions( a_pAllocator )
    , m_bUnk3( TFALSE )
{
}

// $Barnyard: FUNCTION 00582170
ANamedAnimationTransitionSet::~ANamedAnimationTransitionSet()
{
}

// $Barnyard: FUNCTION 00581ff0
void ANamedAnimationTransitionSet::AddTransition( ANamedAnimation* a_pAnimation, const Toshi::TPString8& a_strTransitionName, TFLOAT a_fBlendInTime, TFLOAT a_fBlendOutTime, TBOOL a_bIsAutoTransition, TBOOL a_bManaged )
{
	TVALIDPTR( a_pAnimation );

	// Create new transition object
	ANamedAnimationTransition transition;
	transition.m_strTransitionName = a_strTransitionName;
	transition.m_strAnimationName  = a_pAnimation->GetName();
	transition.m_fBlendOutTime     = a_fBlendOutTime;
	transition.m_fBlendInTime      = a_fBlendInTime;
	transition.m_bAutoTransition   = a_bIsAutoTransition != '\0';
	transition.m_bManaged          = a_bManaged;

	m_mapTransitions.Insert( a_pAnimation->GetName(), transition );

	if ( a_bIsAutoTransition )
		m_strAnimationName = a_pAnimation->GetName();
}

// $Barnyard: FUNCTION 00581ad0
ANamedAnimationTransition* ANamedAnimationTransitionSet::FindTransition( const Toshi::TPString8& a_strName )
{
	auto it = m_mapTransitions[ a_strName ];
	return ( it == m_mapTransitions.End() ) ? TNULL : &it.GetValue()->second;
}

// $Barnyard: FUNCTION 00580520
ANamedAnimation::ANamedAnimation( T2Allocator* a_pAllocator )
    : m_vecBreakpoints( a_pAllocator, 0, 0 )
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
	// Delete breakpoints
	T2_FOREACH( m_vecBreakpoints, it )
	{
		delete ( *it );
	}
	
	m_vecBreakpoints.Clear();
}

TPSTRING8_DECLARE( EndOfAnim );

// $Barnyard: FUNCTION 00581a90
ANamedAnimation::FINISHTYPE ANamedAnimation::GetFinishType( const Toshi::TPString8& a_rcFinishType )
{
	return ( !a_rcFinishType.GetString8().IsEmpty() && a_rcFinishType == TPS8( EndOfAnim ) ) ? FINISHTYPE_AUTO : FINISHTYPE_MANUAL;
}

// $Barnyard: FUNCTION 00580150 TODO
static AAnimationBreakpoint* CreateBreakpointRuntime( const PBProperties* a_pProperties, TBOOL a_bTimedCall )
{
	TIMPLEMENT();

	return TNULL;
}

TPSTRING8_DECLARE( Model );

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
				m_strName = it->GetValue()->GetTPString8();
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

		TPString8 strModelName;
		a_pProperties->GetParentProperties()->GetParentProperties()->GetOptionalPropertyValue( strModelName, TPS8( Model ) );

		// Allocate space to store breakpoints
		TINT iNumSoundBreakpoints = AAnimatableObjectManager::GetSingleton()->FindNumAnimationSoundBreakpoints( strModelName, this );
		m_vecBreakpoints.AccessContainer()->Reserve( iNumBreakpoints + iNumSoundBreakpoints + 4 );

		// Read breakpoints stored in the properties

		T2_FOREACH( ( *a_pProperties ), it )
		{
			const TCHAR* pchPropertyName = it->GetName().GetString();

			if ( *pchPropertyName == 'B' )
			{
				// Breakpoint
				AAnimationBreakpoint* pBreakpoint = CreateBreakpointRuntime( it->GetValue()->GetProperties(), TTRUE );

				if ( pBreakpoint )
				{
					m_vecBreakpoints.Push( pBreakpoint );
				}
			}
		}

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

// $Barnyard: FUNCTION 00580750
void ANamedAnimation::AttachBreakpoint( AAnimationBreakpoint* a_pBreakpoint )
{
	m_vecBreakpoints.Push( a_pBreakpoint );
}

// Various template functions:
// $Barnyard: FUNCTION 00580710
// $Barnyard: FUNCTION 00580440
// $Barnyard: FUNCTION 006cbde0
