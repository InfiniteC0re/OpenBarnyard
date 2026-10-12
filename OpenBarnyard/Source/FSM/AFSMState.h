#pragma once
#include "AFSMInstruction.h"

#include <Toshi/TDList.h>
#include <Toshi/TPString8.h>

//-----------------------------------------------------------------------------
// State of an AFSM: the instructions that are executed every update while the
// machine is in the state, and the names of its locals.
//-----------------------------------------------------------------------------
class AFSMState
    : public Toshi::TDList<AFSMState>::TNode
{
public:
	AFSMState();
	~AFSMState();

	// $Barnyard: FUNCTION 005fccb0
	const Toshi::TPString8& GetName() const { return m_strName; }

	void SetName( const Toshi::TPString8& a_rcName );

	// $Barnyard: FUNCTION 005fcca0
	AFSMInstructionList& GetInstructions() { return m_llInstructions; }

	void AddInstruction( AFSMInstruction* a_pInstruction );

	// Returns -1 if there's no local of the name
	TINT FindLocalIndex( const Toshi::TPString8& a_rcName ) const;
	TINT AddLocal( const Toshi::TPString8& a_rcName );

private:
	Toshi::TPString8    m_strName;                                       // 0x08
	AFSMInstructionList m_llInstructions;                                // 0x0C
	TINT                m_iNumLocals;                                    // 0x14
	Toshi::TPString8    m_aLocalNames[ AFSMLocalTypes::MAX_NUM_LOCALS ]; // 0x18
};
