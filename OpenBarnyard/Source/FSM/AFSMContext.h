#pragma once
#include "AFSMInstruction.h"

class AFSMInstance;

//-----------------------------------------------------------------------------
// State of one update of an AFSMInstance: the locals of the current state,
// the object the natives are called on, the value of the last instruction and
// the transition that was made.
// NOTE: the name is made up, retail keeps it on the stack of AFSMInstance::Update.
//-----------------------------------------------------------------------------
class AFSMContext
{
public:
	AFSMContext( AFSMInstance* a_pInstance, const AFunctionCaller& a_rcCaller );

	// $Barnyard: FUNCTION 005fd550
	~AFSMContext() = default;

	// $Barnyard: FUNCTION 005fdd10
	void SetResult( const PPropertyValue& a_rcValue ) { m_oResult = a_rcValue; }

	void ClearResult();

	// $Barnyard: FUNCTION 005fdd80
	void SetLocal( TINT a_iIndex, const PPropertyValue& a_rcValue ) { m_aLocals[ a_iIndex ] = a_rcValue; }

	// $Barnyard: FUNCTION 005fdda0
	PPropertyValue* GetLocal( TINT a_iIndex ) { return &m_aLocals[ a_iIndex ]; }

	// $Barnyard: FUNCTION 005fe250
	void SetTransition( const AFSMTransitionResult& a_rcTransition ) { m_oTransition = a_rcTransition; }

	void ClearTransition();

	PPropertyValue&             GetResult() { return m_oResult; }
	const AFunctionCaller&      GetCaller() const { return m_oCaller; }
	const AFSMTransitionResult& GetTransition() const { return m_oTransition; }
	AFSMInstance*               GetInstance() const { return m_pInstance; }

	TBOOL IsStaying() const { return m_bStay; }
	void  SetStaying() { m_bStay = TTRUE; }

private:
	PPropertyValue       m_aLocals[ AFSMLocalTypes::MAX_NUM_LOCALS ]; // 0x00
	AFunctionCaller      m_oCaller;                                   // 0x40
	PPropertyValue       m_oResult;                                   // 0x48
	AFSMTransitionResult m_oTransition;                               // 0x50
	AFSMInstance*        m_pInstance;                                 // 0x70
	TBOOL                m_bStay;                                     // 0x74
};
