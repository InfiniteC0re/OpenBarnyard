#pragma once
#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TDList.h>
#include <Toshi/TEvent.h>

//-----------------------------------------------------------------------------
// Purpose: Remains of a system that tracked the conversations of the units.
// On PC nothing but APlayerControlComponent (the sequence the player talks to)
// writes its members and its only method does nothing.
// TClass 0x00783cd8, vtable 0x00723fc4, size 0x68, singleton at 0x00783cd4.
//-----------------------------------------------------------------------------
class AConversationManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AConversationManager>
{
public:
	TDECLARE_CLASS( AConversationManager, Toshi::TTask );

	// NOTE: the type of the list elements is unknown
	struct UnkNode
	    : public Toshi::TGenericDList::TNode
	{
	};

public:
	AConversationManager();
	~AConversationManager();

	// NOTE: retail overrides Create, OnCreate and OnUpdate with jumps to the TTask methods
	virtual TBOOL Reset() override { return TTRUE; } // +0x14, RETURNS_TRUE 0046cde0 (doesn't reset the children)

	// Called when the sequence a_uiSequenceId ends or a speech of it is skipped
	// DOES_NOTHING
	void OnSequenceEnd( TUINT a_uiSequenceId ) {}

	void SetTargetSequence( TUINT a_uiSequenceId ) { m_uiTargetSequenceId = a_uiSequenceId; }

private:
	Toshi::TDList<UnkNode>  m_llUnk20;            // 0x20
	Toshi::TDList<UnkNode>  m_llUnk28;            // 0x28
	TINT                    m_iUnk30;             // 0x30
	TFLOAT                  m_fUnk34;             // 0x34
	TFLOAT                  m_fUnk38;             // 0x38
	TBOOL                   m_bUnk3C;             // 0x3c
	TUINT                   m_uiTargetSequenceId; // 0x40, sequence the player talks to
	Toshi::TGenericListener m_oListener;          // 0x44, never connected
	Toshi::TDList<UnkNode>  m_llUnk58;            // 0x58
	TINT                    m_iUnk60;             // 0x60
	TINT                    m_iUnk64;             // 0x64
};

TSTATICASSERT( sizeof( AConversationManager ) == 0x68 );
