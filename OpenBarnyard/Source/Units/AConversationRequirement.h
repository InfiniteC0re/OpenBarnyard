#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// Compares the time since the last interaction of the units ("action" is
// "lastplayerinteraction") or the number of interactions they had during the
// last "timespan" seconds ("lastplayerinteractionfreq") with "time".
// TClass 0x0078aaac, vtable 0x0072a2e0, size 0x5c.
//-----------------------------------------------------------------------------
class AConversationRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AConversationRequirement, AInteractionRequirement );

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE                  = 0,
		FLAGS_LAST_INTERACTION      = BITFLAG( 0 ), // "lastplayerinteraction"
		FLAGS_LAST_INTERACTION_FREQ = BITFLAG( 1 ), // "lastplayerinteractionfreq"
	};

public:
	AConversationRequirement();
	~AConversationRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	FLAGS                  m_uiFlags;   // 0x4c, "action"
	TFLOAT                 m_fTimeSpan; // 0x50, "timespan"
	TFLOAT                 m_fTime;     // 0x54, "time"
	AInteractionComparison m_oCompare;  // 0x58, "compare"
};

TSTATICASSERT( sizeof( AConversationRequirement ) == 0x5c );
