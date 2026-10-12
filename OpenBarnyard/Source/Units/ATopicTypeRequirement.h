#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// The participants know about an event of all of the topics "type" (one property
// per topic, see AInteractionEvent::GetTopicFlag) and about none of the
// topics "notopicoftype".
// NOTE: only the first heard and the first witnessed event of a unit are checked,
// see AUnitGameEventComponent::HasEventWithAllTopics.
// TClass ATopicTypeRequirement, vtable 0x0072ad9c, size 0x54.
//-----------------------------------------------------------------------------
class ATopicTypeRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( ATopicTypeRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject
	ATopicTypeRequirement()
	    : m_uiTopics( 0 )
	    , m_uiNoTopics( 0 )
	{}

	~ATopicTypeRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	TUINT m_uiTopics;   // 0x4c, "type"
	TUINT m_uiNoTopics; // 0x50, "notopicoftype"
};

TSTATICASSERT( sizeof( ATopicTypeRequirement ) == 0x54 );
