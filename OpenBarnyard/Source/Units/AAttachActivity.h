#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Attaches models and units to each other ("mode"), using the bones
// "childbonename" and "parentbonename".
//-----------------------------------------------------------------------------
class AAttachActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AAttachActivity, AInteractionActivity );

	// "mode", order of the name table at 0x0077522c
	using MODE = TINT;
	enum MODE_ : MODE
	{
		MODE_SWAP,            // "swap", moves the model attached to one unit to the other one
		MODE_CREATE,          // "create", creates "model" and attaches it to the unit
		MODE_DETACH,          // "detach", destroys the attached model
		MODE_DETACHNDESTROY,  // "detachndestory", also destroys its unit
		MODE_ATTACHINVENTORY, // "attachinventory"
		MODE_ATTACHMODEL,     // "attachmodel", see AModelAttachComponent
		MODE_DETACHMODEL,     // "detachmodel", see AModelAttachComponent
		MODE_ATTACHCHARACTER, // "attachcharacter"
		MODE_DETACHCHARACTER, // "detachcharacter"
		MODE_ATTACHTOLOCATOR, // "attachtolocator", moves the unit to the terrain locator "parentbonename"
		MODE_NUMOF,
	};

public:
	AAttachActivity();
	~AAttachActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c
	virtual TBOOL  IsOverrideable() OVERRIDE { return TFALSE; }          // +0x34, RETURNS_FALSE

private:
	MODE                           m_eMode;           // 0x08, "mode", MODE_NUMOF by default
	Toshi::TPString8               m_strModel;        // 0x0c, "model"
	Toshi::TPString8               m_strChildBone;    // 0x10, "childbonename"
	Toshi::TPString8               m_strParentBone;   // 0x14, "parentbonename"
	AInteractionSequence::CATEGORY m_eAttachCategory; // 0x18, "attachmodel", CATEGORY_TARGET by default
};

TSTATICASSERT( sizeof( AAttachActivity ) == 0x1c );
