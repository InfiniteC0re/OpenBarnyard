#include "ToshiPCH.h"
#include "TTask.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_NORUNTIME( TTask );

// $Barnyard: FUNCTION 006bc910
TTask::TTask()
{
	m_State = 0;
}

// $Barnyard: FUNCTION 006bc8c0
TTask::~TTask()
{
	TASSERT( IsLinked() == TFALSE );
}

// $Barnyard: FUNCTION 006bc940
TBOOL TTask::Create()
{
	TASSERT( IsCreated() == TFALSE );

	if ( !IsCreated() )
	{
		if ( !OnCreate() )
		{
			OnDestroy();
			m_Tree->Remove( this, TFALSE );
			Delete();

			return TFALSE;
		}

		TUINT32 oldState = m_State;
		m_State |= State_Created;
		Activate( TTRUE );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006bc990
TBOOL TTask::CreateFailed()
{
	TASSERT( IsCreated() == TFALSE );

	if ( !IsCreated() )
	{
		m_Tree->Remove( this, TFALSE );
		Delete();
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 006bc8e0
TBOOL TTask::Reset()
{
	TTask* firstAttached = Child();
	TTask* node          = firstAttached;
	TBOOL  result        = TTRUE;

	while ( node != TNULL && node != firstAttached )
	{
		result &= node->Reset();
		node = node->Next();
	}

	return result;
}

TBOOL TTask::OnCreate()
{
	return TTRUE;
}

TBOOL TTask::OnUpdate( TFLOAT a_fDeltaTime )
{
	return TTRUE;
}

void TTask::OnPreDestroy()
{
}

void TTask::OnDestroy()
{
}

TBOOL TTask::OnChildDying( TTask* child )
{
	return TTRUE;
}

void TTask::OnChildDied( const TClass* pClass, TTask* deletedTask )
{
}

void TTask::OnActivate()
{
}

void TTask::OnDeactivate()
{
}

// $Barnyard: FUNCTION 006bc850
void TTask::Activate( TBOOL activate )
{
	TUINT8 oldState = m_State;
	TUINT8 newFlags = activate ? State_Active : 0;

	m_State = ( m_State & ~State_Active ) | newFlags;

	if ( oldState != m_State )
	{
		if ( activate )
		{
			OnActivate();
		}
		else
		{
			OnDeactivate();
		}
	}
}

void TTask::SetParent( TTask* a_pAttachTo )
{
	if ( this != Parent() && a_pAttachTo != TNULL )
	{
		Tree()->ReInsert( this != Parent() ? a_pAttachTo : TNULL, this );
	}
}

TOSHI_NAMESPACE_END
