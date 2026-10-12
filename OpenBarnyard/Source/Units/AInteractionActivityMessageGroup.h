#pragma once
#include "AInteractionActivityMessageGroupBase.h"

//-----------------------------------------------------------------------------
// Pool of NUM_MESSAGES messages of class T. Created by T::CreateGroup through
// AInteractionActivityMessageGroups::CreateGroup<T>.
// NOTE: the instance for AKinematicsObstructionActivityMessage has its constructor at 005c86b0
// (pool 005c8460), destructor 005c87a0 / 005c8780 (pool 005c8560), Allocate 005c8720, Free 005c8770.
//-----------------------------------------------------------------------------
template <class T>
class AInteractionActivityMessageGroup
    : public AInteractionActivityMessageGroupBase
{
public:
	static constexpr TINT NUM_MESSAGES = 60;

public:
	AInteractionActivityMessageGroup()
	    : AInteractionActivityMessageGroupBase( &TGetClass( T ) )
	{
		for ( TINT i = 0; i < NUM_MESSAGES; i++ )
			m_llFree.InsertTail( &m_aMessages[ i ] );
	}

	~AInteractionActivityMessageGroup()
	{
		// The messages are part of the array, don't let the list delete them
		m_llFree.RemoveAll();
	}

	virtual AInteractionActivityMessage* Allocate() OVERRIDE // +0x04
	{
		if ( m_llFree.IsEmpty() )
			return TNULL;

		return m_llFree.Remove( m_llFree.Head() );
	}

	virtual void Free( AInteractionActivityMessage* a_pMessage ) OVERRIDE // +0x08
	{
		m_llFree.InsertTail( a_pMessage );
	}

private:
	T                                             m_aMessages[ NUM_MESSAGES ]; // 0x14
	Toshi::TNodeList<AInteractionActivityMessage> m_llFree;
};
