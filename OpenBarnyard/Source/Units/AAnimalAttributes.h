#pragma once
#include "AUnitComponent.h"

//-----------------------------------------------------------------------------
// The "drives" of an animal: four values in 0..1 that AUnitBehaviour and the
// drive system (AAnimalAttributeDriveFunction) look at to pick what the unit
// does next. Happiness starts random, suggestion fades over time.
//
// TClass 0x00789f94 (version 0.1, size 0x20), vtable 0x00728560.
//-----------------------------------------------------------------------------
class AAnimalAttributes
    : public AUnitComponent
{
public:
	TDECLARE_CLASS( AAnimalAttributes, AUnitComponent );

	// Order of the name table at 0x007750e8
	using ATTRIBUTE = TINT;
	enum ATTRIBUTE_ : ATTRIBUTE
	{
		ATTRIBUTE_HAPPINESS,  // "happiness"
		ATTRIBUTE_SUGGESTION, // "suggestion"
		ATTRIBUTE_COMMAND,    // "command"
		ATTRIBUTE_SCHEDULE,   // "schedule"
		ATTRIBUTE_NUMOF,
	};

public:
	AAnimalAttributes();
	~AAnimalAttributes();

	virtual AFunctionTable*        GetFunctionTable() OVERRIDE;              // +0x0c
	virtual AUnitComponentManager* GetManager() OVERRIDE;                    // +0x10
	virtual void                   UpdateAI( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x2c, the suggestion fades

	TFLOAT GetAttribute( ATTRIBUTE a_eAttribute );

	// Adds a_fValue to the attribute, clipped to 0..1.
	void ChangeAttribute( ATTRIBUTE a_eAttribute, TFLOAT a_fValue );

	// Sets the attribute, clipped to 0..1.
	void SetAttribute( ATTRIBUTE a_eAttribute, TFLOAT a_fValue );

	// FSM natives
	TFLOAT GetHappiness();
	void   SetHappiness( TFLOAT a_fValue );
	void   ModifyHappiness( TFLOAT a_fValue );
	TFLOAT GetSuggestion();
	void   SetSuggestion( TFLOAT a_fValue );
	void   ModifySuggestion( TFLOAT a_fValue );
	TFLOAT GetCommand();
	void   SetCommand( TFLOAT a_fValue );
	void   ModifyCommand( TFLOAT a_fValue );
	TFLOAT GetSchedule();
	void   SetSchedule( TFLOAT a_fValue );
	void   ModifySchedule( TFLOAT a_fValue );

	// Resets every attribute to its default and inverts the next one of them each call.
	void DebugToggle();

	// Returns the attribute called a_szName (case insensitive), ATTRIBUTE_NUMOF if there's none.
	static ATTRIBUTE GetAttributeFromName( const TCHAR* a_szName );

public:
	static AFunctionTable ms_oFunctionTable;

private:
	inline static TINT ms_iDebugAttribute = 0; // 0x00789f90, the attribute DebugToggle inverts

private:
	TFLOAT m_afAttributes[ ATTRIBUTE_NUMOF ]; // 0x10
};

TSTATICASSERT( sizeof( AAnimalAttributes ) == 0x20 );
