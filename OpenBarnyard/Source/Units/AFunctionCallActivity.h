#pragma once
#include "AInteractionActivity.h"
#include "AInteractionManager.h"
#include "FSM/AFunctionTable.h"

#include <Toshi/TGlobalEmitter.h>
#include <Toshi/TString8.h>

class PPropertyValue;
class PPropertyValueArray;

//-----------------------------------------------------------------------------
// Calls the script function "function" of the AFunctionTable "table" (lower
// case class name) on every unit of the category with "arguments". With
// "blockuntilmet" a unit is called again until the function returns "result".
//-----------------------------------------------------------------------------
class AFunctionCallActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AFunctionCallActivity, AInteractionActivity );

	// NOTE: retail shares the function lookup with the FSM actions (AFSMInstructionAction)
	using FunctionRef = AFunctionRef;

	using Listener = Toshi::TGlobalListener<AFunctionCallActivity, AInteractionManagerEvent>;

public:
	AFunctionCallActivity();
	~AFunctionCallActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE;  // +0x0c
	virtual RESULT OnStart( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x18
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE;  // +0x1c

private:
	// Once the interactions are loaded, the requirement name argument of "forcerequirementsatisfied" is
	// replaced with the index of the requirement.
	void OnInteractionManagerEvent( const AInteractionManagerEvent& a_rcEvent );

	static void OnInteractionManagerEventCallback( void* a_pReceiver, const AInteractionManagerEvent& a_rcEvent );

private:
	FunctionRef                                     m_oFunction;      // 0x08, "table", "function"
	PPropertyValueArray*                            m_pArguments;     // 0x10, "arguments"
	PPropertyValue*                                 m_pResult;        // 0x14, "result"
	TBOOL                                           m_bBlockUntilMet; // 0x18, "blockuntilmet"
	Toshi::T2DList<AInteractionComponent>::Iterator m_itUnit;         // 0x1c, unit the function is called on next
	Listener                                        m_oListener;      // 0x20
};

TSTATICASSERT( sizeof( AFunctionCallActivity ) == 0x30 );
