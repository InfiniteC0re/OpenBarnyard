#pragma once
#include "AFSM.h"

#include <Toshi/TQList.h>

class AFSMInstruction : public Toshi::TQList<AFSMInstruction>::TNode
{
public:
	AFSMInstruction() {};
	virtual ~AFSMInstruction() = default;

private:
};

class AFSMInstructionValue : public AFSMInstruction
{
public:
	AFSMInstructionValue() = default;
	~AFSMInstructionValue() = default;
};

class AFSMInstructionValueLocal : public AFSMInstructionValue
{
public:
	AFSMInstructionValueLocal() {}
	~AFSMInstructionValueLocal() = default;

private:
	AFSM* m_pFSM;
	TINT  m_iVarIdx;
};

class AFSMInstructionValueGlobal : public AFSMInstructionValue
{
public:
	// $Barnyard: FUNCTION 005ff910
	AFSMInstructionValueGlobal( AFSM* a_pFSM, TINT a_iVarIdx )
	    : m_pFSM( a_pFSM )
	    , m_iVarIdx( a_iVarIdx ) {}

	~AFSMInstructionValueGlobal() = default;

private:
	AFSM* m_pFSM;
	TINT  m_iVarIdx;
};

class AFSMInstructionAssignment : public AFSMInstruction
{
public:
	// $Barnyard: FUNCTION 005febd0
	AFSMInstructionAssignment()
	    : m_iVarIdx( -1 )
	    , m_iUnk( -1 )
	    , m_pValue( TNULL ) {}
	~AFSMInstructionAssignment() = default;

	void Setup( TINT a_iVarIdx, AFSMInstructionValue* a_pValue )
	{
		m_iVarIdx = a_iVarIdx;
		m_pValue  = a_pValue;
	}

private:
	TINT                  m_iUnk;
	TINT                  m_iVarIdx;
	AFSMInstructionValue* m_pValue;
};
