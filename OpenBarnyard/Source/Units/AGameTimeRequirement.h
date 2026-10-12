#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// Compares ("compare") the current day phase with "phaseofday" or the game time
// with the time "timeofday" ("hh:mm:ss am") of the day "dayofmonth".
// TClass 0x0078abcc, vtable 0x0072a4ac, size 0x6c.
//-----------------------------------------------------------------------------
class AGameTimeRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AGameTimeRequirement, AInteractionRequirement );

public:
	AGameTimeRequirement();
	~AGameTimeRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	AInteractionComparison m_oCompare;  // 0x4c, "compare"
	TINT                   m_iMonth;    // 0x50, "monthofyear", -1 if not set
	TINT                   m_iDay;      // 0x54, "dayofmonth", -1 if not set
	TINT                   m_iHour;     // 0x58, "timeofday", -1 if not set
	TINT                   m_iMinute;   // 0x5c, "timeofday", -1 if not set
	TINT                   m_iSecond;   // 0x60, "timeofday", -1 if not set
	TINT                   m_eDayPhase; // 0x64, "phaseofday", -1 if not set
	TFLOAT                 m_fDuration; // 0x68, never set
};

TSTATICASSERT( sizeof( AGameTimeRequirement ) == 0x6c );
